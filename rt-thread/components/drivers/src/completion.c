#include <rtthread.h>
#include <rtdevice.h>

#define RT_COMPLETED 1
#define RT_UNCOMPLETED 0


void rt_completion_init(struct rt_completion *completion)
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	completion->flag = RT_UNCOMPLETED;
	rt_list_init(&completion->suspended_list);
	rt_hw_interrupt_enable(level);
}




