#include <rtthread.h>
#include <rthw.h>

static struct rt_thread idle;
static rt_uint8_t rt_thread_stack[256];

static int count;
void rt_thread_idle_entry(void *parameter)
{
	while(1) {
		count ++;
	}
}
void rt_thread_idle_init()
{
	rt_thread_init(&idle,
		"tidle",
		rt_thread_idle_entry,
		RT_NULL,
		&rt_thread_stack[0],
		sizeof(rt_thread_stack),
		RT_THREAD_PRIORITY_MAX-1,
		32
		);
	rt_thread_startup(&idle);
}

