#include "finsh.h"
#include <rtthread.h>

static int object_name_maxlen(struct rt_list_node *list)
{
	struct rt_list_node *node;
	struct rt_object *object = NULL;
	int max_length = 0, length;
	rt_enter_critical();
	for(node=list->next;node!=list; node=node->next) {
		object = rt_list_entry(node, struct rt_object, list);
		length = rt_strlen(object->name);
		if(length > max_length) {
			max_length = length;
		}
	}
	rt_exit_critical();
	if(max_length > RT_NAME_MAX || max_length == 0) {
		max_length = RT_NAME_MAX;
	}
	return max_length;
}
rt_inline void object_split(int len)
{
	while(len--) {
		rt_kprintf("-");
	}
}

static long _list_thread(struct rt_list_node *list)
{
	int maxlen;
	rt_uint8_t *ptr;
	struct rt_thread *thread;
	struct rt_list_node *node;

	maxlen = object_name_maxlen(list);

	rt_kprintf("%-*.s pri  status	   sp	  stack size max used left tick  error\n", maxlen, "thread"); object_split(maxlen);
	rt_kprintf( 	" ---  ------- ---------- ----------  ------  ---------- ---\n");
	for (node = list->next; node != list; node = node->next)
	{
		rt_uint8_t stat;
		thread = rt_list_entry(node, struct rt_thread, list);
		rt_kprintf("%-*.*s %3d ", maxlen, RT_NAME_MAX, thread->name, thread->current_priority);

		stat = (thread->stat & RT_THREAD_STAT_MASK);
		if (stat == RT_THREAD_READY)		rt_kprintf(" ready	");
		else if (stat == RT_THREAD_SUSPEND) rt_kprintf(" suspend");
		else if (stat == RT_THREAD_INIT)	rt_kprintf(" init	");
		else if (stat == RT_THREAD_CLOSE)	rt_kprintf(" close	");

		ptr = (rt_uint8_t *)thread->stack_addr;
		while (*ptr == '#')ptr ++;

		rt_kprintf(" 0x%08x 0x%08x	  %02d%%   0x%08x %03d\n",
				   thread->stack_size + ((rt_uint32_t)thread->stack_addr - (rt_uint32_t)thread->sp),
				   thread->stack_size,
				   (thread->stack_size - ((rt_uint32_t) ptr - (rt_uint32_t) thread->stack_addr)) * 100
						/ thread->stack_size,
				   thread->remaining_tick,
				   thread->error);
	}

	return 0;
}



long hello()
{
	rt_kprintf("hello msh\n");
	return 0;
}
MSH_CMD_EXPORT(hello, show hello info);

extern void rt_show_version(void);

long version()
{
	rt_show_version();
	return 0;
}
MSH_CMD_EXPORT(version, show rt-thread version);

long list_thread()
{
	struct rt_object_information *info;
	info = rt_object_get_information(RT_Object_Class_Thread);
	return _list_thread(&info->object_list);
	return 0;
}
MSH_CMD_EXPORT(list_thread, show rt-thread thread);



