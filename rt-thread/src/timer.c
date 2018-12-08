#include <rtthread.h>
#include <rthw.h>

static rt_list_t rt_timer_list[RT_TIMER_SKIP_LIST_LEVEL];

rt_inline void _rt_timer_remove(rt_timer_t timer)
{
	int i;
	for(i=0; i<RT_TIMER_SKIP_LIST_LEVEL; i++) {
		rt_list_remove(&timer->row[i]);
	}
}
rt_err_t rt_timer_detach(rt_timer_t timer)
{
	register rt_base_t level;
	level = rt_hw_interrupt_disable();
	_rt_timer_remove(timer);
	rt_hw_interrupt_enable(level);
	rt_object_detach((rt_object_t)timer);
	return RT_EOK;
}

rt_err_t rt_timer_control(rt_timer_t timer, int cmd ,void *arg )
{
	switch(cmd) {
		case RT_TIMER_CTRL_GET_TIME:
			*(rt_tick_t *)arg = timer->init_tick;
			break;
		case RT_TIMER_CTRL_SET_TIME:
			timer->init_tick = *(rt_tick_t *)arg;
			break;
		
	}
	return RT_EOK;
}



rt_err_t rt_timer_start(rt_timer_t timer)
{
	register rt_base_t level;
	rt_list_t *timer_list;
	rt_list_t *row_head[RT_TIMER_SKIP_LIST_LEVEL];
	int row_lvl;
	unsigned int tst_nr;
	static unsigned int random_nr;
	
	level = rt_hw_interrupt_disable();
	_rt_timer_remove(timer);
	timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVED;
	rt_hw_interrupt_enable(level);

	timer->timeout_tick = rt_tick_get() + timer->init_tick;

	level = rt_hw_interrupt_disable();

	timer_list = rt_timer_list;

	row_head[0] = &timer_list[0];
	for(row_lvl=0; row_lvl<RT_TIMER_SKIP_LIST_LEVEL; row_lvl++) {
		for(;row_head[row_lvl]!=timer_list[row_lvl].prev;
		row_head[row_lvl] = row_head[row_lvl]->next) {
			struct rt_timer* t;
			rt_list_t *p = row_head[row_lvl]->next;
			t = rt_list_entry(p, struct rt_timer, row[row_lvl]);
			if((t->timeout_tick - timer->timeout_tick) == 0) {
				continue;
			} else {
				break;
			}
			
		}
		if(row_lvl != RT_TIMER_SKIP_LIST_LEVEL-1) {
			row_head[row_lvl+1] = row_head[row_lvl] + 1;
		}
	}
	random_nr++;
	tst_nr = random_nr;
	rt_list_insert_after(row_head[RT_TIMER_SKIP_LIST_LEVEL-1],
		&(timer->row[RT_TIMER_SKIP_LIST_LEVEL-1]));
	
	timer->parent.flag |= RT_TIMER_FLAG_ACTIVED;

	rt_hw_interrupt_enable(level);

	return RT_EOK;
}
rt_err_t rt_timer_stop(rt_timer_t timer)
{
	register rt_base_t level;
	if(!(timer->parent.flag & RT_TIMER_FLAG_ACTIVED)) {
		return -RT_ERROR;
	}
	level = rt_hw_interrupt_disable();
	_rt_timer_remove(timer);
	rt_hw_interrupt_enable(level);
	timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVED;
	return RT_EOK;
}

static void _rt_timer_init(
	rt_timer_t timer,
		void (*timeout)(void *parameter),
		void *parameter,
		rt_tick_t time,
		rt_uint8_t flag

)
{
	int i;
	timer->parent.flag = flag;
	timer->parent.flag &= ~RT_TIMER_FLAG_ACTIVED;

	timer->timeout_func = timeout;
	timer->parameter = parameter;

	timer->timeout_tick = 0;
	timer->init_tick = time;

	for(i=0; i<RT_TIMER_SKIP_LIST_LEVEL; i++) {
		rt_list_init(&(timer->row[i]));
	}
}
void rt_timer_check()
{
	struct rt_timer *t;
	rt_tick_t current_tick;
	register rt_base_t level;
	current_tick = rt_tick_get();
	level = rt_hw_interrupt_disable();
	while(!rt_list_isempty(&rt_timer_list[RT_TIMER_SKIP_LIST_LEVEL-1])) {
		t = rt_list_entry(
			rt_timer_list[RT_TIMER_SKIP_LIST_LEVEL-1].next,
			struct rt_timer, 
			row[RT_TIMER_SKIP_LIST_LEVEL-1]
		);
		if((current_tick - t->timeout_tick) < RT_TICK_MAX/2) {
			_rt_timer_remove(t);
			t->timeout_func(t->parameter);
			current_tick = rt_tick_get();
			//if is periodic
			if((t->parent.flag & RT_TIMER_FLAG_PERIODIC) &&
				(t->parent.flag & RT_TIMER_FLAG_ACTIVED)) {
				t->parent.flag &= ~RT_TIMER_FLAG_ACTIVED;
				rt_timer_start(t);
			} else {
				t->parent.flag &= ~RT_TIMER_FLAG_ACTIVED;
			}
		} else {
			break;
		}
	}
}

void rt_timer_init(
	rt_timer_t timer,
	const char *name,
	void (*timeout)(void *parameter),
	void *parameter,
	rt_tick_t time,
	rt_uint8_t flag
)
{
	rt_object_init((rt_object_t)timer, RT_Object_Class_Timer, name);
	_rt_timer_init(timer, timeout, parameter, time, flag);
}
void rt_system_timer_init()
{
	int i;
	for(i=0;i <RT_TIMER_SKIP_LIST_LEVEL; i++) {
		rt_list_init(rt_timer_list+i);
	}
}

