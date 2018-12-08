#include <stdio.h>
#include <rtthread.h>
#include <stdint.h>

#define TEST_BASIC 0
#define TEST_SEM  0

#define TEST_MEM 0
#define TEST_MUTEX 1

#if TEST_BASIC
int main(void)
{
#if 0
	uint8_t i = 0;
	while(1) {
		rt_kprintf("main thread, count:%d \n", i++);
		rt_thread_delay(100);
	}
#endif
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


#if TEST_MUTEX
struct rt_thread mutex_thread;
rt_uint8_t mutex_thread_stack[1024];
struct rt_mutex mutex_test;
void mutex_thread_entry(void *parameter)
{
	rt_thread_delay(500);
	rt_kprintf("now rt_mutex_release, current tick:%d\n", rt_tick_get());
	rt_mutex_release(&mutex_test);
}
int main(void)
{
	rt_thread_init(&mutex_thread, 
		"mutexpost",
		mutex_thread_entry,
		RT_NULL,
		&mutex_thread_stack[0],
		1024,
		30,
		10
		);
	rt_thread_startup(&mutex_thread);
	rt_mutex_init(&mutex_test, "mutex1", 0, 0);
	rt_kprintf("before rt_mutex_take,current tick:%d\n", rt_tick_get());
	rt_mutex_take(&mutex_test, -1);
	
	rt_kprintf("after rt_mutex_take, current tick:%d\n", rt_tick_get());
	
}


#endif

