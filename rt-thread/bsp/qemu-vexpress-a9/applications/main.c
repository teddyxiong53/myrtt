#include <stdio.h>
#include <rtthread.h>
#define TEST_BASIC 0
#define TEST_SEM  0

#define TEST_MEM 1

#if TEST_BASIC
int main(void)
{
	int i = 0;
	while(1) {
		rt_kprintf("main thread, count:%d \n", i++);
		rt_thread_delay(100);
	}
	
}

#endif
#if TEST_SEM
struct rt_thread sem_thread;
rt_uint8_t sem_thread_stack[1024];
struct rt_semaphore sem_test;
void sem_thread_entry(void *parameter)
{
	rt_thread_delay(500);
	rt_kprintf("now rt_sem_release, current tick:%d\n", rt_tick_get());
	rt_sem_release(&sem_test);
}
int main(void)
{
	rt_thread_init(&sem_thread, 
		"sempost",
		sem_thread_entry,
		RT_NULL,
		&sem_thread_stack[0],
		1024,
		30,
		10
		);
	rt_thread_startup(&sem_thread);
	rt_sem_init(&sem_test, "sem1", 0, 0);
	rt_kprintf("before rt_sem_take,current tick:%d\n", rt_tick_get());
	rt_sem_take(&sem_test, -1);
	
	rt_kprintf("after rt_sem_take, current tick:%d\n", rt_tick_get());
	
}

#endif

#if TEST_MEM
int main(void)
{
	char *p1, *p2, *p3;
	p1 = rt_malloc(1);
	p2 = rt_malloc(20);
	p3 = rt_malloc(21);
	rt_kprintf("malloc test begin\n");
	rt_kprintf("p1:%p, p2:%p, p3:%p\n",p1, p2, p3);

	rt_kprintf("now free the pointer\n");
	
	rt_free(p1);
	rt_free(p2);
	rt_free(p3);

	rt_kprintf("second time malloc");
	p1 = rt_malloc(1);
	p2 = rt_malloc(20);
	p3 = rt_malloc(21);
	rt_kprintf("p1:%p, p2:%p, p3:%p\n",p1, p2, p3);
}
#endif
