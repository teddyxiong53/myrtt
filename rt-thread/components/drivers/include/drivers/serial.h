#ifndef __COMPONENTS_SERIAL_H__
#define __COMPONENTS_SERIAL_H__

#include <completion.h>

struct rt_serial_rx_fifo {
	rt_uint8_t *buffer;
	rt_uint16_t put_index, get_index;
	rt_bool_t is_full;
};

struct rt_serial_tx_fifo {
	struct rt_completion completion;
};


#endif