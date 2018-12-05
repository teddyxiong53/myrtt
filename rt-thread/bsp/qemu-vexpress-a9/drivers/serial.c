#include <rtthread.h>
#include <rthw.h>
#include <serial.h>

struct hw_uart_device {
	rt_uint32_t hw_base;
	rt_uint32_t irqno;
};

#define UART_DR(base)  __REG32(base + 0x00)
#define UART_FR(base)	__REG32(base + 0x18)
#define UART_CR(base)	__REG32(base + 0x30)
#define UART_IMSC(base)	__REG32(base + 0x38)
#define UART_ICR(base)	__REG32(base + 0x44)

#define UARTFR_RXFE 0X10
#define UARTFR_TXFF	0X20
#define UARTIMSC_RXIM	0X10
#define UARTIMSC_TXIM	0X20
#define UARTICR_RXIC	0X10
#define UARTICR_TXIC	0x20

#define RT_SERIAL_CONFIG_DEFAULT \
{\
	BAUD_RATE_115200,\
	DATA_BITS_8,\
	STOP_BITS_1,\
	PARITY_NONE,\
	BIT_ORDER_LSB,\
	NRZ_NORMAL,\
	RT_SERIAL_RB_BUFSZ,\
	0\
}

static struct hw_uart_device _uart0_device = {
	REALVIEW_UART0_BASE,
	IRQ_PBA8_UART0,
};
static struct rt_serial_device _serial0;


static rt_err_t uart_configure(struct rt_serial_device *serial, struct serial_configure *config)
{
	return RT_EOK;
}
static rt_err_t uart_control(struct rt_serial_device *serial, int cmd, void *arg)
{
	struct hw_uart_device *uart;
	uart = (struct hw_uart_device *)serial->parent.user_data;

	switch(cmd) {
	case RT_DEVICE_CTRL_CLR_INT:
		UART_IMSC(uart->hw_base) &= ~UARTIMSC_RXIM;
		break;
	case RT_DEVICE_CTRL_SET_INT:
		UART_IMSC(uart->hw_base) |= UARTIMSC_RXIM;
		rt_hw_interrupt_umask(uart->irqno);
		break;
	}	
	return RT_EOK;
}


static int uart_putc(struct rt_serial_device *serial, char c)
{
	struct hw_uart_device *uart;
	uart = (struct hw_uart_device *)serial->parent.user_data;
	while(UART_FR(uart->hw_base) & UARTFR_TXFF);
	UART_DR(uart->hw_base) = c;
	return 1;
}

static int uart_getc(struct rt_serial_device *serial)
{
	int ch;
	struct hw_uart_device *uart;
	uart = (struct hw_uart_device *)serial->parent.user_data;
	ch = -1;
	if(!(UART_FR(uart->hw_base) & UARTFR_RXFE) ) {
		ch = UART_DR(uart->hw_base) & 0xff;
	}
	return ch;
}

static const struct rt_uart_ops _uart_ops = {
	uart_configure,
	uart_control,
	uart_putc,
	uart_getc,
};


int rt_hw_uart_init(void)
{
	struct hw_uart_device *uart;
	struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

	uart = &_uart0_device;
	_serial0.ops = &_uart_ops;
	_serial0.config = config;
	
}

