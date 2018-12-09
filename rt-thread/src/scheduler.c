#include <rtthread.h>
#include <rthw.h>

static rt_int16_t rt_scheduler_lock_nest;

rt_list_t rt_thread_priority_table[RT_THREAD_PRIORITY_MAX];

extern volatile rt_uint8_t rt_interrupt_nest;

rt_uint8_t rt_current_priority;
struct rt_thread *rt_current_thread;
rt_uint32_t rt_thread_ready_priority_group;

rt_list_t rt_thread_defunct;


void rt_enter_critical()
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_scheduler_lock_nest++;
	rt_hw_interrupt_enable(level);
}
void rt_exit_critical()
{
	rt_base_t level;
	level = rt_hw_interrupt_disable();
	rt_scheduler_lock_nest--;
	if(rt_scheduler_lock_nest <= 0) {
		rt_scheduler_lock_nest = 0;
		rt_hw_interrupt_enable(level);
		rt_schedule();
	} else {
		rt_hw_interrupt_enable(level);
	}
}
void rt_schedule_remove_thread(struct rt_thread *thread)
{
	rt_base_t temp;
	temp = rt_hw_interrupt_disable();
	rt_list_remove(&(thread->tlist));
	if(rt_list_isempty(&(rt_thread_priority_table[thread->current_priority]))) {
		rt_thread_ready_priority_group &= ~thread->number_mask;
	}
	rt_hw_interrupt_enable(temp);
}


void rt_schedule()
{
	register rt_base_t level;
	struct rt_thread *from_thread, *to_thread;

	level = rt_hw_interrupt_disable();

	if(rt_scheduler_lock_nest == 0) {
		rt_base_t highest_ready_priority;
		highest_ready_priority = __rt_ffs(rt_thread_ready_priority_group)-1;

		to_thread = rt_list_entry(rt_thread_priority_table[highest_ready_priority].next,
				struct rt_thread, tlist);
		if(to_thread != rt_current_thread) {
			rt_current_priority = (rt_uint8_t)highest_ready_priority;
			from_thread = rt_current_thread;
			rt_current_thread = to_thread;

			if(rt_interrupt_nest == 0) {
				rt_hw_context_switch((rt_uint32_t)&from_thread->sp, (rt_uint32_t)&to_thread->sp);
				rt_hw_interrupt_enable(level);
			} else {
				rt_hw_context_switch_interrupt((rt_uint32_t)&from_thread->sp, (rt_uint32_t)&to_thread->sp);
				rt_hw_interrupt_enable(level);
			}
		}
	} else {
		rt_hw_interrupt_enable(level);
	}
}

void rt_schedule_insert_thread(struct rt_thread *thread)
{
	rt_base_t temp;
	temp = rt_hw_interrupt_disable();

	thread->stat = RT_THREAD_READY;

	rt_list_insert_before(&(rt_thread_priority_table[thread->current_priority]),
		&(thread->tlist));
	rt_thread_ready_priority_group |= thread->number_mask;
	rt_hw_interrupt_enable(temp);
}

void rt_system_scheduler_init()
{
	rt_base_t offset;
	for(offset =0 ;offset<RT_THREAD_PRIORITY_MAX; offset++) {
		rt_list_init(&rt_thread_priority_table[offset]);
	}
	rt_current_priority = RT_THREAD_PRIORITY_MAX-1;
	rt_current_thread = RT_NULL;
	rt_thread_ready_priority_group = 0;

	rt_list_init(&rt_thread_defunct);
}

void rt_system_scheduler_start()
{
	register struct rt_thread *to_thread;
	register rt_ubase_t highest_ready_priority;
	highest_ready_priority = __rt_ffs(rt_thread_ready_priority_group)-1;

	to_thread = rt_list_entry(rt_thread_priority_table[highest_ready_priority].next,
		struct rt_thread, tlist);
	rt_current_thread = to_thread;
	rt_hw_context_switch_to((rt_uint32_t)&to_thread->sp);
	
}



