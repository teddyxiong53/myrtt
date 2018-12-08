#include "finsh.h"
#define RT_FINSH_ARG_MAX 10 //10 args for cmd at most

typedef int (*cmd_function_t)(int argc, char **argv);

static cmd_function_t msh_get_cmd(char *cmd, int size)
{
	struct finsh_syscall *index;
	cmd_function_t cmd_func = RT_NULL;
	for(index=_syscall_table_begin; index < _syscall_table_end; index ++) {
		#if 0
		if(strncmp(index->name, "__cmd_", 6) != 0) {
			continue;
		}
		#endif
		#define IDX 0 //can be 6 
		if((strncmp(&index->name[IDX], cmd, size) == 0) 
			&&(index->name[IDX+size] == '\0') ) {
			cmd_func = (cmd_function_t)index->func;
			break;
		}
			
	}
	return cmd_func;
}
static int msh_split(char *cmd, rt_size_t length, char *argv[RT_FINSH_ARG_MAX])
{
    char *ptr;
    rt_size_t position;
    rt_size_t argc;
    rt_size_t i;

    ptr = cmd;
    position = 0; argc = 0;

    while (position < length)
    {
        /* strip bank and tab */
        while ((*ptr == ' ' || *ptr == '\t') && position < length)
        {
            *ptr = '\0';
            ptr ++; position ++;
        }

        if(argc >= RT_FINSH_ARG_MAX)
        {
            rt_kprintf("Too many args ! We only Use:\n");
            for(i = 0; i < argc; i++)
            {
                rt_kprintf("%s ", argv[i]);
            }
            rt_kprintf("\n");
            break;
        }

        if (position >= length) break;

        /* handle string */
        if (*ptr == '"')
        {
            ptr ++; position ++;
            argv[argc] = ptr; argc ++;

            /* skip this string */
            while (*ptr != '"' && position < length)
            {
                if (*ptr == '\\')
                {
                    if (*(ptr + 1) == '"')
                    {
                        ptr ++; position ++;
                    }
                }
                ptr ++; position ++;
            }
            if (position >= length) break;

            /* skip '"' */
            *ptr = '\0'; ptr ++; position ++;
        }
        else
        {
            argv[argc] = ptr;
            argc ++;
            while ((*ptr != ' ' && *ptr != '\t') && position < length)
            {
                ptr ++; position ++;
            }
            if (position >= length) break;
        }
    }

    return argc;
}

static int _msh_exec_cmd(char *cmd, rt_size_t length, int *retp)
{
	int cmd0_size = 0;
	cmd_function_t cmd_func;
	char *argv[RT_FINSH_ARG_MAX];
	int argc;
	while((cmd[cmd0_size] != ' '  )
		&& (cmd[cmd0_size] != '\t')
		&& (cmd0_size < length)) {
		cmd0_size++;
	}
	if(cmd0_size == 0) {
		return -RT_ERROR;
	}
	cmd_func = msh_get_cmd(cmd, cmd0_size);
	if(cmd_func == RT_NULL) {
		return -RT_ERROR;
	}
	memset(argv, 0, sizeof(argv));
	argc = msh_split(cmd, length, argv);
	if(argc == 0) {
		return -RT_ERROR;
	}
	*retp = cmd_func(argc, argv);
	return 0;
}
int msh_exec(char *cmd, rt_size_t length)
{
	int cmd_ret;
	//strip begin space and tab
	while(*cmd==' ' || *cmd == '\t') {
		cmd++;
		length--;
	}
	if (length == 0) {
		return 0;
	}
	//process priority
	//1. built-in cmd
	//2. module 
	//3. chdir
	if(_msh_exec_cmd(cmd, length, &cmd_ret) == 0) {
		return cmd_ret;
	}
	//not found 
	{
        char *tcmd;
        tcmd = cmd;
        while (*tcmd != ' ' && *tcmd != '\0')
        {
            tcmd++;
        }
        *tcmd = '\0';
    }
    rt_kprintf("%s: command not found.\n", cmd);
    return -1;
}
