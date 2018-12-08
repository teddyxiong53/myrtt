#include <rthw.h>
#include <rtthread.h>

static inline rt_err_t rt_ipc_object_init(struct rt_ipc_object *ipc)
{
	rt_list_init(&(ipc->suspend_thread));
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
#if 0
	register rt_base_t temp;
	temp = rt_hw_interrupt_disable();
	if(sem->value > 0) {
		sem->value --;
		rt_hw_interrupt_enable(temp);
	} else {
		
	}
#endif
	return RT_EOK;
}

rt_err_t rt_sem_release(rt_sem_t sem)
{
	return RT_EOK;
}
