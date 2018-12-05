#include <rthw.h>
#include <rtthread.h>

volatile rt_uint8_t rt_interrupt_nest;

void rt_interrupt_enter()
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_interrupt_nest++;
	rt_hw_interrupt_enable(level);
}

void rt_interrupt_leave()
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_interrupt_nest--;
	rt_hw_interrupt_enable(level);
}
