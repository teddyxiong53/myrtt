#include "finsh.h"
#include <rtthread.h>


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
