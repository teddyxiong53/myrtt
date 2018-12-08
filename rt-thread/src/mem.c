#include <rthw.h>
#include <rtthread.h>

struct heap_mem {
	rt_uint16_t magic;
	rt_uint16_t used;
	rt_size_t next, prev;
};
#define HEAP_MAGIC 0x1ea0

#define MIN_SIZE 12
#define MIN_SIZE_ALIGNED RT_ALIGN(MIN_SIZE, RT_ALIGN_SIZE)
#define SIZEOF_STRUCT_MEM RT_ALIGN(sizeof(struct heap_mem), RT_ALIGN_SIZE)

static rt_size_t mem_size_aligned;
static rt_uint8_t *heap_ptr;
static struct heap_mem *heap_end;

static struct rt_semaphore heap_sem;

static struct heap_mem *lfree;


void *rt_malloc(rt_size_t size)
{
	if(size == 0) {
		return RT_NULL;
	}
	size = RT_ALIGN(size, RT_ALIGN_SIZE);
	if(size > mem_size_aligned) {
		return RT_NULL;
	}
	rt_sem_take(&heap_sem, RT_WAITING_FOREVER);
	rt_size_t ptr, ptr2;
	struct heap_mem *mem, *mem2;
	for(
		ptr = (rt_uint8_t *)lfree - heap_ptr;
		ptr < mem_size_aligned - size;
		ptr = ((struct heap_mem *)&heap_ptr[ptr])->next
	) {
		mem = (struct heap_mem *)&heap_ptr[ptr];
		if((!mem->used) && (mem->next - (ptr + SIZEOF_STRUCT_MEM))>=size) {
			if(mem->next - (ptr + SIZEOF_STRUCT_MEM) >= 
				(size + SIZEOF_STRUCT_MEM + MIN_SIZE_ALIGNED)) {
				//this is always happen
				ptr2 = ptr + SIZEOF_STRUCT_MEM + size;
				mem2 = (struct heap_mem *)&heap_ptr[ptr2];
				mem2->magic = HEAP_MAGIC;
				mem2->used = 0;
				mem2->next = mem->next;
				mem2->prev = ptr;

				mem->next = ptr2;
				mem->used = 1;

				if(mem2->next != mem_size_aligned + SIZEOF_STRUCT_MEM) {
					((struct heap_mem *)&heap_ptr[mem2->next])->prev = ptr2;
				}
			} else {
				mem->used = 1;
			}
			mem->magic = HEAP_MAGIC;
			if(mem == lfree) {
				while(lfree->used && lfree!=heap_end) {
					lfree = (struct heap_mem *)&heap_ptr[lfree->next];
				}
			}
			rt_sem_release(&heap_sem);
			return (rt_uint8_t *)mem + SIZEOF_STRUCT_MEM;
		}
	}
	rt_sem_release(&heap_sem);
	return RT_NULL;
}

static void plug_holes(struct heap_mem *mem)
{
	struct heap_mem *nmem;
	struct heap_mem *pmem;
	//plug hole forward
	nmem = (struct heap_mem *)&heap_ptr[mem->next];
	if((mem != nmem) &&
		(nmem->used == 0) &&
		((rt_uint8_t*)nmem != (rt_uint8_t *)heap_end)) {
		if(lfree == nmem) {
			lfree = mem;
		}
		mem->next = nmem->next;
		((struct heap_mem *)&heap_ptr[nmem->next])->prev = (rt_uint8_t *)mem - heap_ptr;
		
	}
	//plug holes backward
	pmem = (struct heap_mem *)&heap_ptr[mem->prev];
	if((pmem != mem) &&
		(pmem->used == 0)) {
		if(lfree == mem) {
			lfree = pmem;
		}
		pmem->next = mem->next;
		((struct heap_mem *)&heap_ptr[mem->next])->prev = (rt_uint8_t *)pmem - heap_ptr;
	}
		
}

void rt_free(void *rmem)
{
	struct heap_mem *mem;
	if(rmem == RT_NULL) {
		return;
	}
	if(((rt_uint8_t *)rmem < (rt_uint8_t*)heap_ptr) ||
		((rt_uint8_t*)rmem >= (rt_uint8_t*)heap_end)) {
		return;//illegal addr
	}
	mem = (struct heap_mem *)((rt_uint8_t*)rmem - SIZEOF_STRUCT_MEM);
	rt_sem_take(&heap_sem, RT_WAITING_FOREVER);
	mem->used = 0;
	mem->magic = HEAP_MAGIC;
	if(mem < lfree) {
		lfree = mem;
	}
	plug_holes(mem);
	rt_sem_release(&heap_sem);
}
void rt_system_heap_init(
	void *begin_addr,
	void *end_addr
)
{
	struct heap_mem *mem;
	rt_uint32_t begin_align = RT_ALIGN((rt_uint32_t)begin_addr, RT_ALIGN_SIZE);
	rt_uint32_t end_align = RT_ALIGN_DOWN((rt_uint32_t)end_addr, RT_ALIGN_SIZE);

	if((end_align > (2*SIZEOF_STRUCT_MEM))
		&& (end_align - 2*SIZEOF_STRUCT_MEM >= begin_align))
	{
		mem_size_aligned = end_align - begin_align - 2*SIZEOF_STRUCT_MEM;
	} else {
		rt_kprintf("mem init error, begin:0x%x, end:0x%x", begin_addr, end_addr);
		return;
	}
	heap_ptr = (rt_uint8_t *)begin_align;
	mem = (struct heap_mem *)heap_ptr;
	mem->magic = HEAP_MAGIC;
	mem->next = mem_size_aligned + SIZEOF_STRUCT_MEM;
	mem->prev = 0;
	mem->used = 0;

	heap_end = (struct heap_mem *)&heap_ptr[mem->next];
	heap_end->magic = HEAP_MAGIC;
	heap_end->used = 1;
	heap_end->next = mem_size_aligned + SIZEOF_STRUCT_MEM;
	heap_end->prev = mem_size_aligned + SIZEOF_STRUCT_MEM;

	rt_sem_init(&heap_sem, "heap", 1, RT_IPC_FLAG_FIFO);
	lfree = (struct heap_mem *)heap_ptr;
	
}

