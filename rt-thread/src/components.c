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

void rt_components_board_init()
{

	int result;
	const struct rt_init_desc *desc;
	//rt_kprintf("do components board init\n");
	for(desc=&__rt_init_desc_rti_board_start; desc<&__rt_init_desc_rti_board_end; desc++) {
		//rt_kprintf("init func[%s], ", desc->fn_name);
		result = desc->fn();
		//rt_kprintf("result:%d\n", result);
	}

}

void rt_components_init()
{
	int result;
	const struct rt_init_desc *desc;
	rt_kprintf("do components init\n");
	for(desc=&__rt_init_desc_rti_board_end; desc < & __rt_init_desc_rti_end; desc++) {
		rt_kprintf("init func[%s], ", desc->fn_name);
		result = desc->fn();
		rt_kprintf("result:%d\n", result);
	}
}
void main_thread_entry(void *parameter)
{
	rt_components_init();
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

