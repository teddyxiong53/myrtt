#include <rthw.h>
#include <rtthread.h>


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


int rtthread_startup()
{
	rt_hw_interrupt_disable();
	rt_hw_board_init();
	rt_show_version();
	
}

void rt_components_board_init()
{
#if 0
	const init_fn_t *fn_ptr;
	for(fn_ptr=&__rt_init_rti_board_start; fn_ptr<&__rt_init_rti_board_end; fn_ptr++) {
		(*fn_ptr)();
	}
#endif
}