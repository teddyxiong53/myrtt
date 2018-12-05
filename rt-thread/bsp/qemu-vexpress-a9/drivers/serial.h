#ifndef __SERIAL_H__
#define __SERIAL_H__

#define BAUD_RATE_115200	115200

#define DATA_BITS_8 8

#define STOP_BITS_1 0

#define PARITY_NONE 0

#define BIT_ORDER_LSB 0
#define BIT_ORDER_MSB 1
#define NRZ_NORMAL  0
#define NRZ_INVERTED 1

#define RT_SERIAL_RB_BUFSZ 64

#define RT_DEVICE_CTRL_SET_INT 0x10
#define RT_DEVICE_CTRL_CLR_INT 0x11

struct serial_configure {
	rt_uint32_t baud_rate;
	rt_uint32_t data_bits:4;
	rt_uint32_t stop_bits:2;
	rt_uint32_t parity:2;
	rt_uint32_t bit_order:1;
	rt_uint32_t invert:1;
	rt_uint32_t bufsz:16;
	rt_uint32_t reserved:4;
};

struct rt_serial_device {
	struct rt_device parent;
	const struct rt_uart_ops *ops;
	struct serial_configure config;
	void *serial_rx;
	void *serial_tx;
};


struct rt_uart_ops {
	rt_err_t (*configure)(struct rt_serial_device *serial, struct serial_configure *config);
	rt_err_t (*control)(struct rt_serial_device *serial, int cmd, void *arg);
	int (*putc)(struct rt_serial_device *serial, char c);
	int (*getc)(struct rt_serial_device *serial);
	rt_size_t (*dma_transmit)(struct rt_serial_device *serial, rt_uint8_t *buf, rt_size_t size, int direction);
};
#endif