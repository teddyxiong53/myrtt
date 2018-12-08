#include <rthw.h>
#include <rtthread.h>

static rt_uint8_t main_stack[2048];
struct rt_thread main_thread;

static int rti_start()
{
	return 0;
}
INIT_EXPORT(rti_start, "0");

static int rti_board_start()
{
	return 0;
}
INIT_EXPORT(rti_board_start, "0.end");

static int rti_board_end()
{
	return 0;
}
INIT_EXPORT(rti_board_end, "1.end");

static int rti_end()
{
	return 0;
}
INIT_EXPORT(rti_end, "6.end");


void main_thread_entry(void *parameter)
{
	extern int main();
	main();
}
void rt_application_init()
{
	rt_thread_t tid;
	rt_err_t result;
	tid = &main_thread;
	result = rt_thread_init(tid, "main", main_thread_entry, 
		RT_NULL, main_stack, sizeof(main_stack),10, 50 );
	rt_thread_startup(tid);
}
int rtthread_startup()
{
	rt_hw_interrupt_disable();
	rt_hw_board_init();
	rt_show_version();
	rt_system_timer_init();
	rt_system_scheduler_init();
	rt_application_init();

	rt_thread_idle_init();
	rt_system_scheduler_start();

}

void rt_components_board_init()
{
#if 1
	const init_fn_t *fn_ptr;
	for(fn_ptr=&__rt_init_rti_board_start; fn_ptr<&__rt_init_rti_board_end; fn_ptr++) {
		(*fn_ptr)();
	}
#endif
}