#include <rtthread.h>
#include <rthw.h>

static rt_tick_t rt_tick;



rt_tick_t rt_tick_get()
{
	return rt_tick;
}

void rt_tick_set(rt_tick_t tick)
{
	register rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_tick = tick;
	rt_hw_interrupt_enable(level);
}


void rt_tick_increase()
{
	struct rt_thread *thread;
	++rt_tick;

	thread = rt_thread_self();
	-- thread->remaining_tick;

	if(thread->remaining_tick == 0) {
		thread->remaining_tick = thread->init_tick;
		rt_thread_yield();
	
	}
	rt_timer_check();
}

void rt_system_tick_init()
{
	
}
