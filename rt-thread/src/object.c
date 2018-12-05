#include <rthw.h>
#include <rtthread.h>

#define _OBJ_CONTAINER_LIST_INIT(c) \
	{&(rt_object_container[c].object_list), &(rt_object_container[c].object_list)}
static struct rt_object_information rt_object_container[RT_Object_Class_Unknown] = {
	{
		RT_Object_Class_Thread, 
		_OBJ_CONTAINER_LIST_INIT(RT_Object_Class_Thread),
		sizeof(struct rt_thread)			
	},
	{
		RT_Object_Class_Semaphore, 
		_OBJ_CONTAINER_LIST_INIT(RT_Object_Class_Semaphore),
		sizeof(struct rt_semaphore)			
	},
};

struct rt_object_information *rt_object_get_information(
	enum rt_object_class_type type
)
{
	int index;
	for(index=0; index< RT_Object_Class_Unknown; index++) {
		if(rt_object_container[index].type == type) {
			return &rt_object_container[index];
		}
	}
	return RT_NULL;
}


void rt_object_init(struct rt_object *object,
	enum rt_object_class_type type,
	const char *name
)
{
	rt_base_t temp;
	struct rt_object_information *info;
	info = rt_object_get_information(type);
	object->type = type | RT_Object_Class_Static;
	rt_strncpy(object->name, name, RT_NAME_MAX);
	temp = rt_hw_interrupt_disable();
	rt_list_insert_after(&(info->object_list), &(object->list));
	rt_hw_interrupt_enable(temp);
	
}
