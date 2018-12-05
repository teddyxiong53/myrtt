#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>


static rt_err_t rt_serial_init(struct rt_device *dev)
{
	rt_err_t result = RT_EOK;
	struct rt_serial_device *serial;
	serial = (sturct rt_serial_device *)dev;
	serial->serial_rx = RT_NULL;
	serial->serial_tx = RT_NULL;
	if(serial->ops->configure) {
		result = serial->ops->configure(serial, &serial->config);
	}
	return result;
}

static rt_err_t rt_serial_open(struct rt_device *dev, rt_uint16_t oflag)
{
	struct rt_serial_device *serial = (struct rt_serial_device *)dev;
	dev->open_flag = oflag & 0xff;

	if(serial->serial_rx == RT_NULL) {
		//only process int 
		struct rt_serial_rx_fifo *rx_fifo;
		rx_fifo = (struct rt_serial_rx_fifo *)rt_malloc(sizeof(struct rt_serial_rx_fifo) + serial->config.bufsz);
		if(rx_fifo) {
			return -RT_ENOMEM;
		}
		rx_fifo->buffer = (rt_uint8_t *)(rx_fifo + 1);//+1 means skip sizeof(struct rt_serial_rx_fifo)
		rt_memset(rx_fifo->buffer, 0, serial->config.bufsz);
		rx_fifo->get_index = 0;
		rx_fifo->put_index = 0;
		rx_fifo->is_full = RT_FALSE;
		serial->serial_rx = rx_fifo;
		dev->open_flag |= RT_DEVICE_FLAG_INT_RX;
		serial->ops->control(serial, RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_RX);
	}	
	if(serial->serial_tx == RT_NULL) {
		struct rt_serial_tx_fifo *tx_fifo;
		tx_fifo = rt_malloc(sizeof(struct rt_serial_tx_fifo));
		rt_completion_init(&(tx_fifo->completion));
		serial->serial_tx = tx_fifo;
		dev->open_flag |= RT_DEVICE_FLAG_INT_TX;
		serial->ops->control(serial, RT_DEVICE_CTRL_SET_INT, (void *)RT_DEVICE_FLAG_INT_TX);
	}
	return RT_EOK;
	
}

static rt_err_t rt_serial_close(struct rt_device *dev)
{
	return RT_EOK;
}


rt_inline int _serial_int_rx(struct rt_serial_device *serial,
	rt_uint8_t *data, int len
)
{
	int size;
	struct rt_serial_rx_fifo *rx_fifo;
	size = len;
	rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
	while(len) {
		int ch;
		rt_base_t level;
		level = rt_hw_interrupt_disable();
		if((rx_fifo->get_index == rx_fifo->put_index) && 
			(rx_fifo->is_full != RT_TRUE)) {
			//empty
			rt_hw_interrupt_enable(level);
			break;
		}
		ch = rx_fifo->buffer[rx_fifo->get_index];
		rx_fifo->get_index +=1;
		if(rx_fifo->get_index >= serial->config.bufsz) {
			rx_fifo->get_index = 0;
		}
		if(rx_fifo->is_full == RT_TRUE) {
			rx_fifo->is_full = RT_FALSE;
		}
		rt_hw_interrupt_enable(level);
		*data = ch&0xff;
		data ++;
		len --;
	}
	return size-len;
}
static rt_size_t rt_serial_read(
	struct rt_device *dev,
	rt_off_t pos,
	void *buffer,
	rt_size_t size
)
{
	struct rt_serial_device *serial;
	if(size == 0) {
		return 0;
	}
	serial = (struct rt_serial_device *)dev;
	return _serial_int_rx(serial, buffer, size);
}

rt_inline int _serial_int_tx(struct rt_serial_device *serial,
	const rt_uint8_t *data, int length
)
{
	
}
static rt_size_t rt_serial_write(struct rt_device *dev,
	rt_off_t pos,
	const void *buffer,
	rt_size_t size
)
{
	struct rt_serial_device *serial;

	if(size == 0) {
		return 0;
	}
	serial = (struct rt_serial_device *)dev;
	return _serial_int_tx(serial, buffer, size)
}
rt_err_t rt_hw_serial_register(
	struct rt_serial_device *serial,
	const char *name,
	rt_uint32_t flag,
	void *data
)
{
	rt_err_t ret;
	struct  rt_device *device;
	device = &(serial->parent);
	device->type = RT_Device_Class_Char;
	device->rx_indicate = RT_NULL;
	device->tx_complete = RT_NULL;
	device->init = rt_serial_init;
}



