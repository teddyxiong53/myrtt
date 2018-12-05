#ifndef __RT_HW_H__
#define __RT_HW_H__

#include <rtdef.h>

#define HWREG32(x)          (*((volatile rt_uint32_t *)(x)))
#define HWREG16(x)          (*((volatile rt_uint16_t *)(x)))
#define HWREG8(x)           (*((volatile rt_uint8_t *)(x)))

#define RT_CPU_CACHE_LINE_SZ 32

typedef void (*rt_isr_handler_t)(int vector, void *param);
struct rt_irq_desc {
	rt_isr_handler_t handler;
	void *param;
};

rt_base_t rt_hw_interrupt_disable();

void rt_hw_interrupt_enable(rt_base_t level);

#endif