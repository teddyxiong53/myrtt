#ifndef __COMPONENTS_SERIAL_H__
#define __COMPONENTS_SERIAL_H__



#define RT_SERIAL_EVENT_RX_IND 0X01
#define RT_SERIAL_EVENT_TX_DONE 0X02


struct rt_serial_rx_fifo {
	rt_uint8_t *buffer;
	rt_uint16_t put_index, get_index;
	rt_bool_t is_full;
};

struct rt_serial_tx_fifo {
	struct rt_completion completion;
};


#endif