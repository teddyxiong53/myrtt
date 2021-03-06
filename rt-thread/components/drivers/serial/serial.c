#include <rtthread.h>
#include <rthw.h>
#include <rtdevice.h>


static rt_err_t rt_serial_init(struct rt_device *dev)
{
	rt_err_t result = RT_EOK;
	struct rt_serial_device *serial;
	serial = (struct rt_serial_device *)dev;
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
		if(!rx_fifo) {
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
		//do nothing
	}
	return RT_EOK;
	
}

static rt_err_t rt_serial_close(struct rt_device *dev)
{
	struct rt_serial_device *serial;
	serial = (struct rt_serial_device *)dev;
	if(dev->ref_count > 1) {
		return RT_EOK;
	}
	if(dev->open_flag & RT_DEVICE_FLAG_INT_RX) {
		struct rt_serial_rx_fifo *rx_fifo;
		rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
		rt_free(rx_fifo);
		serial->serial_rx = RT_NULL;
		dev->open_flag &= ~RT_DEVICE_FLAG_INT_RX;
		serial->ops->control(serial, RT_DEVICE_CTRL_CLR_INT, (void *)RT_DEVICE_FLAG_INT_RX);
	}
	//tx need no process
	
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
		register rt_base_t level;
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


rt_inline rt_size_t _serial_poll_tx(
	struct rt_serial_device *serial,
	const rt_uint8_t *data, 
	int length	
)
{
	int size;
	size = length;
	while(length) {
		if(*data == '\n' && (serial->parent.open_flag&RT_DEVICE_FLAG_STREAM)) {
			serial->ops->putc(serial, '\r');
		}
		serial->ops->putc(serial, *data);
		data++;
		length --;
	}
	return size - length;
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
	//return _serial_int_tx(serial, buffer, size);
	return _serial_poll_tx(serial, buffer, size);
}

rt_err_t rt_serial_control(
	struct rt_device *dev,
	int cmd,
	void *args
)
{
	return RT_EOK;
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
	device->open = rt_serial_open;
	device->read = rt_serial_read;
	device->write = rt_serial_write;
	device->control = rt_serial_control;
	device->user_data = data;

	ret = rt_device_register(device ,name, flag);
	return ret;
}



void rt_hw_serial_isr(struct rt_serial_device *serial, int event)
{
	switch(event & 0xff) {
		case RT_SERIAL_EVENT_RX_IND:
		{
			int ch = -1;
			register rt_base_t level;
			struct rt_serial_rx_fifo *rx_fifo;
			rx_fifo = (struct rt_serial_rx_fifo *)serial->serial_rx;
			while(1) {
				ch = serial->ops->getc(serial);
				if(ch==-1) {
					break;
				}
				level = rt_hw_interrupt_disable();
				rx_fifo->buffer[rx_fifo->put_index] = ch;
				rx_fifo->put_index++;
				if(rx_fifo->put_index >= serial->config.bufsz) {
					rx_fifo->put_index = 0;
				}
				if(rx_fifo->put_index == rx_fifo->get_index) {
					rx_fifo->get_index+=1;
					rx_fifo->is_full = RT_TRUE;
					if(rx_fifo->get_index >= serial->config.bufsz) {
						rx_fifo->get_index = 0;
					}
				}
				rt_hw_interrupt_enable(level);
				
			}
			if(serial->parent.rx_indicate != RT_NULL) {
				rt_size_t rx_length;
				level = rt_hw_interrupt_disable();
				rx_length = (rx_fifo->put_index >= rx_fifo->get_index)? (rx_fifo->put_index - rx_fifo->get_index):
                    (serial->config.bufsz - (rx_fifo->get_index - rx_fifo->put_index));
				rt_hw_interrupt_enable(level);
				serial->parent.rx_indicate(&serial->parent, rx_length);
			}
			break;
			
		}
		case RT_SERIAL_EVENT_TX_DONE:
		{
			struct rt_serial_tx_fifo *tx_fifo;
			tx_fifo = (struct rt_serial_tx_fifo *)serial->serial_tx;
			rt_completion_done(&(tx_fifo->completion));
			break;	
		}
	}
}
