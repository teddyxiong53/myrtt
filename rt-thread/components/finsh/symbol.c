#include "finsh.h"
#include "shell.h"
#include <rtthread.h>

long hello(void);
long version(void);
long list_thread(void);

struct finsh_syscall _syscall_table[] = {
	{"hello", hello},
	{"version", version},
	{"list_thread", list_thread},

};

struct finsh_syscall *_syscall_table_begin = &_syscall_table[0];
struct finsh_syscall *_syscall_table_end = &_syscall_table[sizeof(_syscall_table)/sizeof(_syscall_table[0])];

struct finsh_sysvar *_sysvar_table_begin = NULL;
struct finsh_sysvar *_sysvar_table_end = NULL;
