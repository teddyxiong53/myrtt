#include <rthw.h>
#include <rtthread.h>

struct heap_mem {
	rt_uint16_t magic;
	rt_uint16_t used;
	rt_size_t next, prev;
};
#define HEAP_MAGIC 0x1ea0

#define MIN_SIZE 12
#define SIZEOF_STRUCT_MEM RT_ALIGN(sizeof(struct heap_mem), RT_ALIGN_SIZE)

static rt_size_t mem_size_aligned;
static rt_uint8_t *heap_ptr;
static struct heap_mem *heap_end;

static struct rt_semaphore heap_sem;

static struct heap_mem *lfree;

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

