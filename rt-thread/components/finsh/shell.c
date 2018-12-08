#include <rtthread.h>
#include "finsh.h"
#include "msh.h"
#include "shell.h"

struct finsh_shell *shell;
struct finsh_shell _shell;

static struct rt_thread finsh_thread;
static char finsh_thread_stack[4096];

static rt_err_t finsh_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&(shell->rx_sem));
	return RT_EOK;
}


void finsh_set_device(const char *device_name)
{
	rt_device_t dev;
	dev = rt_device_find(device_name);
	if(dev == shell->device) {
		return;
	}
	if(RT_EOK == rt_device_open(dev, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM)) {
		if(shell->device != RT_NULL) {
			rt_device_close(shell->device);
			rt_device_set_rx_indicate(shell->device,RT_NULL );
		}
		memset(shell->line, 0, sizeof(shell->line));
		shell->device = dev;
		rt_device_set_rx_indicate(dev, finsh_rx_ind);
	}
}

static char finsh_getchar()
{
	char ch;
	while(rt_device_read(shell->device, -1, &ch, 1) != 1) {
		rt_sem_take(&shell->rx_sem, RT_WAITING_FOREVER);
	}
	return ch;
}
void finsh_thread_entry(void *parameter)
{
	rt_kprintf("finsh_thread_entry\n");
	char ch;
	shell->echo_mode = 1;
	if(shell->device == RT_NULL) {
		rt_device_t console = rt_console_get_device();
		if(console) {
			finsh_set_device(console->parent.name);
		}
		
	}
	rt_kprintf(FINSH_PROMPT);
	while(1) {
		ch = finsh_getchar();
		if(ch=='\0') {
			continue;//for 'CR', never happen 
		} else if(ch == '\t') {
			
		}

		if(ch == '\r' || ch =='\n') {
			if(shell->echo_mode) {
				rt_kprintf("\n");
				msh_exec(shell->line, shell->line_position);
			}
			rt_kprintf(FINSH_PROMPT);
			memset(shell->line, 0, sizeof(shell->line));
			shell->line_curpos = shell->line_position = 0;
			continue;
		}

		if(shell->line_position >= FINSH_CMD_SIZE) {
			shell->line_position = 0;
		}
		if(shell->line_curpos < shell->line_position) {
			
		} else {
			shell->line[shell->line_position] = ch;
			if(shell->echo_mode) {
				rt_kprintf("%c", ch);
			}
		}

		ch = 0;
		shell->line_position++;
		shell->line_curpos++;
		if(shell->line_position >= FINSH_CMD_SIZE) {
			shell->line_position = 0;
			shell->line_curpos = 0;
		}
		
	}
}

int finsh_system_init()
{
	rt_err_t result;
	shell = &_shell;
	memset(shell, 0, sizeof(struct finsh_shell));
	rt_kprintf("finsh_system_init \n");
	rt_sem_init(&(shell->rx_sem), "shrx", 0, 0);
	result = rt_thread_init(
		&finsh_thread,
		"tshell",
		finsh_thread_entry,
		RT_NULL,
		&finsh_thread_stack[0],
		sizeof(finsh_thread_stack),
		29,
		10
	);
	rt_thread_startup(&finsh_thread);
	return 0;
}
INIT_APP_EXPORT(finsh_system_init);

