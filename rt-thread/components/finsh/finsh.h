#ifndef __FINSH_H__
#define __FINSH_H__

#include <rtthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct finsh_sysvar {
	const char *name;
	uint8_t type;
	void *var;
};
typedef long (*syscall_func)();
struct finsh_syscall {
	const char *name;
	syscall_func func;
};

#define FINSH_FUNCTION_EXPORT_CMD(name, cmd, desc) \
	const char __fsym_##cmd##_name[] = #cmd;\
	const struct finsh_syscall  __fsym_##cmd SECTION("FSymTab") = \
		{\
			__fsym_##cmd##_name,\
			(syscall_func) &name\
		};

#define MSH_CMD_EXPORT(command ,desc) \
	FINSH_FUNCTION_EXPORT_CMD(command, __cmd_##command, desc)


extern struct finsh_syscall *_syscall_table_begin, *_syscall_table_end;

#endif
