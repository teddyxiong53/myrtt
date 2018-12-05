#include <rtthread.h>
#include <rthw.h>
#include <board.h>


void rt_hw_board_init()
{
	rt_hw_interrupt_init();
	rt_system_heap_init(HEAP_BEGIN, HEAP_END);
	rt_components_board_init();
	rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
	
}