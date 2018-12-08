#ifndef __SHELL_H__
#define __SHELL_H__
#include <rtthread.h>

enum input_stat {
	WAIT_NORMAL,
	WAIT_SPEC_KEY,
	WAIT_FUNC_KEY,
};
#define FINSH_CMD_SIZE 80

#define FINSH_PROMPT "msh >"
struct finsh_shell {
	struct rt_semaphore rx_sem;
	enum input_stat stat;
	rt_uint8_t echo_mode:1;
	char line[FINSH_CMD_SIZE];
	rt_uint8_t line_position;
	rt_uint8_t line_curpos;
	rt_device_t device;
};

#endif