#include <rthw.h>
#include <rtthread.h>

rt_inline rt_err_t rt_ipc_object_init(struct rt_ipc_object *ipc)
{
	rt_list_init(&(ipc->suspend_thread));
	return RT_EOK;
}

rt_inline rt_err_t rt_ipc_list_suspend(
	rt_list_t *list,
	struct rt_thread *thread,
	rt_uint8_t flag
)
{
	rt_thread_suspend(thread);
	switch(flag) {
		//case RT_IPC_FLAG_FIFO:
		default:
			rt_list_insert_before(list, &(thread->tlist));
			break;
	}
	return RT_EOK;
}

rt_inline rt_err_t rt_ipc_list_resume(rt_list_t *list)
{
	struct rt_thread *thread;
	thread = rt_list_entry(list->next, struct rt_thread, tlist);
	rt_thread_resume(thread);
	return RT_EOK;
}

rt_err_t rt_sem_init(
	rt_sem_t sem,
	const char *name,
	rt_uint32_t value,
	rt_uint8_t flag
)
{
	rt_object_init(&(sem->parent.parent), RT_Object_Class_Semaphore, "name");
	rt_ipc_object_init(&(sem->parent));
	sem->value = value;
	sem->parent.parent.flag = flag;
	return RT_EOK;
}


rt_err_t rt_sem_take(rt_sem_t sem, rt_uint32_t time)
{

	register rt_base_t temp;
	struct rt_thread *thread;
	temp = rt_hw_interrupt_disable();
	if(sem->value > 0) {
		sem->value --;
		rt_hw_interrupt_enable(temp);
	} else {
		if(time == 0) {
			rt_hw_interrupt_enable(temp);
			return -RT_ETIMEOUT;
		} else {
			thread = rt_thread_self();
			thread->error = RT_EOK;
			rt_ipc_list_suspend(&(sem->parent.suspend_thread),
				thread, sem->parent.parent.flag);
			if(time > 0) {
				rt_timer_control(&(thread->thread_timer), 
					RT_TIMER_CTRL_SET_TIME, &time);
				rt_timer_start(&(thread->thread_timer));
			}
		}
		rt_hw_interrupt_enable(temp);
		rt_schedule();
		if(thread->error != RT_EOK) {
			return thread->error;
		}
	}

	return RT_EOK;
}

rt_err_t rt_sem_trytake(rt_sem_t sem)
{
	return rt_sem_take(sem, 0);
}
rt_err_t rt_sem_release(rt_sem_t sem)
{
	rt_base_t temp;
	rt_bool_t need_schedule;
	need_schedule = RT_FALSE;
	temp = rt_hw_interrupt_disable();

	if(!rt_list_isempty(&sem->parent.suspend_thread)) {
		rt_ipc_list_resume(&(sem->parent.suspend_thread));
		need_schedule = RT_TRUE;
	} else {
		sem->value ++;
	}
	rt_hw_interrupt_enable(temp);
	if(need_schedule) {
		rt_schedule();
	}
	return RT_EOK;
}
