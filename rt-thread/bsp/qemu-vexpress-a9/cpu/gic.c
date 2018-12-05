#include <rtthread.h>
#include <rthw.h>
#include "realview.h"


struct arm_gic {
	rt_uint32_t offset;
	rt_uint32_t dist_hw_base;
	rt_uint32_t cpu_hw_base;
};

struct arm_gic _gic_table[ARM_GIC_MAX_NR];

#define GIC_CPU_CTRL(hw_base)               __REG32((hw_base) + 0x00)
#define GIC_CPU_PRIMASK(hw_base)            __REG32((hw_base) + 0x04)
#define GIC_CPU_BINPOINT(hw_base)           __REG32((hw_base) + 0x08)
#define GIC_CPU_INTACK(hw_base)             __REG32((hw_base) + 0x0c)
#define GIC_CPU_EOI(hw_base)                __REG32((hw_base) + 0x10)
#define GIC_CPU_RUNNINGPRI(hw_base)         __REG32((hw_base) + 0x14)
#define GIC_CPU_HIGHPRI(hw_base)            __REG32((hw_base) + 0x18)

#define GIC_DIST_CTRL(hw_base)              __REG32((hw_base) + 0x000)
#define GIC_DIST_TYPE(hw_base)              __REG32((hw_base) + 0x004)
#define GIC_DIST_IGROUP(hw_base, n)         __REG32((hw_base) + 0x080 + ((n)/32) * 4)
#define GIC_DIST_ENABLE_SET(hw_base, n)     __REG32((hw_base) + 0x100 + ((n)/32) * 4)
#define GIC_DIST_ENABLE_CLEAR(hw_base, n)   __REG32((hw_base) + 0x180 + ((n)/32) * 4)
#define GIC_DIST_PENDING_SET(hw_base, n)    __REG32((hw_base) + 0x200 + ((n)/32) * 4)
#define GIC_DIST_PENDING_CLEAR(hw_base, n)  __REG32((hw_base) + 0x280 + ((n)/32) * 4)
#define GIC_DIST_ACTIVE_SET(hw_base, n)     __REG32((hw_base) + 0x300 + ((n)/32) * 4)
#define GIC_DIST_ACTIVE_CLEAR(hw_base, n)   __REG32((hw_base) + 0x380 + ((n)/32) * 4)
#define GIC_DIST_PRI(hw_base, n)            __REG32((hw_base) + 0x400 +  ((n)/4) * 4)
#define GIC_DIST_TARGET(hw_base, n)         __REG32((hw_base) + 0x800 +  ((n)/4) * 4)
#define GIC_DIST_CONFIG(hw_base, n)         __REG32((hw_base) + 0xc00 + ((n)/16) * 4)
#define GIC_DIST_SOFTINT(hw_base)           __REG32((hw_base) + 0xf00)
#define GIC_DIST_CPENDSGI(hw_base, n)       __REG32((hw_base) + 0xf10 + ((n)/4) * 4)
#define GIC_DIST_ICPIDR2(hw_base)           __REG32((hw_base) + 0xfe8)

static unsigned int _gic_max_irq;


int arm_gic_dist_init(rt_uint32_t index, rt_uint32_t dist_base, int irq_start)
{
	unsigned int gic_type, i;
	_gic_table[index].dist_hw_base = dist_base;
	_gic_table[index].offset = irq_start;
	
	gic_type = GIC_DIST_TYPE(dist_base);
	_gic_max_irq = ((gic_type&0x1f) + 1) *32;
	
	if(_gic_max_irq > 1020) {
		_gic_max_irq = 1020;
	}
	if(_gic_max_irq > ARM_GIC_NR_IRQS) {
		_gic_max_irq = ARM_GIC_NR_IRQS;
	}
	rt_uint32_t cpumask = 1<<0;

	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;

	GIC_DIST_CTRL(dist_base) = 0x0;

	//set all global interrupts to be level triggered, active low
	for(i=32; i<_gic_max_irq; i+=16) {
		GIC_DIST_CONFIG(dist_base, i) = 0x0;
	}
	//set all global interrupts to this  cpu only
	for(i=32; i<_gic_max_irq; i+=4) {
		GIC_DIST_TARGET(dist_base, i) = cpumask;
		
	}
	//set priority on all interrupts
	for(i=0; i<_gic_max_irq; i+=4) {
		GIC_DIST_PRI(dist_base, i) = 0xa0a0a0a0;
	}
	//disable all interrupts
	for(i=0; i<_gic_max_irq; i+=32) {
		GIC_DIST_ENABLE_CLEAR(dist_base, i) = 0xffffffff;
	}
	//enable group0 and group1 interrupt forwarding
	GIC_DIST_CTRL(dist_base) = 0x03;
}


int arm_gic_cpu_init(rt_uint32_t index, rt_uint32_t cpu_base)
{
	_gic_table[index].cpu_hw_base = cpu_base;
	GIC_CPU_PRIMASK(cpu_base) = 0xf0;
	//enable cpu interrupt
	GIC_CPU_CTRL(cpu_base) = 0x01;
	return 0;
}

int arm_gic_get_active_irq(rt_uint32_t index)
{
	int irq;
	irq = GIC_CPU_INTACK(_gic_table[index].cpu_hw_base);
	irq += _gic_table[index].offset;
	return irq;
}

void arm_gic_ack(rt_uint32_t index, int irq)
{
	rt_uint32_t mask = 1<<(irq%32);
	irq = irq - _gic_table[index].offset;
	GIC_DIST_ENABLE_CLEAR(_gic_table[index].dist_hw_base, irq) = mask;
	GIC_CPU_EOI(_gic_table[index].cpu_hw_base) = irq;
	GIC_DIST_ENABLE_SET(_gic_table[index].dist_hw_base, irq) = mask;
	
}

void arm_gic_umask(rt_uint32_t index, int irq)
{
	rt_uint32_t mask = 1<< (irq%32);
	irq = irq - _gic_table[index].offset;
	GIC_DIST_ENABLE_SET(_gic_table[index].dist_hw_base, irq) = mask;
}

