#include <rtthread.h>

#include "finsh.h"
#include "msh.h"
#include <dfs_posix.h>

extern char working_directory[];

int cmd_ls(int argc, char **argv)
{
	extern void ls(const char *pathname);
	if(argc == 1) {
		ls("/");
	} else {
		ls(argv[1]);
	}
	return 0;
}
FINSH_FUNCTION_EXPORT_ALIAS(cmd_ls, __cmd_ls, list dir );

