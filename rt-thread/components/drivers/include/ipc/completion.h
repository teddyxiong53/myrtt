#ifndef __COMPLETION_H__
#define __COMPLETION_H__
#include <rtthread.h>


struct rt_completion {
	rt_uint32_t flag;
	rt_list_t suspended_list;
};


#endif