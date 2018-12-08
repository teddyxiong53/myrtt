#include <rtthread.h>
#include <rtdevice.h>

#define RT_COMPLETED 1
#define RT_UNCOMPLETED 0


void rt_completion_init(struct rt_completion *completion)
{
	register rt_base_t level;
	level = rt_hw_interrupt_disable();
	completion->flag = RT_UNCOMPLETED;
	rt_list_init(&completion->suspended_list);
	rt_hw_interrupt_enable(level);
}


rt_err_t rt_completion_wait(struct rt_completion *completion, 
	rt_int32_t timeout
)
{
	rt_err_t result = RT_EOK;
	rt_thread_t thread;
	register rt_base_t level;
	
	thread = rt_thread_self();
	level = rt_hw_interrupt_disable();

	if(completion->flag != RT_COMPLETED) {
		if(timeout == 0) {
			result = -RT_ETIMEOUT;
			goto __exit;
		} else {
			thread->error = RT_EOK;
			rt_thread_suspend(thread);
			rt_list_insert_before(&(completion->suspended_list),
				&(thread->tlist));
			if(timeout > 0) {
				rt_timer_control(&(thread->thread_timer),
					RT_TIMER_CTRL_SET_TIME, &timeout);
				rt_timer_start(&(thread->thread_timer));
			}
		}
	}
	return RT_EOK;

__exit:
	rt_hw_interrupt_enable(level);
	return result;
}

void rt_completion_done(struct rt_completion *completion)
{
	
}



