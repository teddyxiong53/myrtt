#include <rthw.h>
#include "realview.h"

#define MAX_HANDLERS NR_IRQS_PBA8

struct rt_irq_desc isr_table[MAX_HANDLERS];

rt_uint32_t rt_interrupt_from_thread;
rt_uint32_t rt_interrupt_to_thread;
rt_uint32_t rt_thread_switch_interrupt_flag;

extern volatile rt_uint8_t rt_interrupt_nest;
extern int system_vectors;

static rt_hw_vector_init()
{
	rt_cpu_vector_set_base((unsigned int)&system_vectors);
}
void rt_hw_interrupt_init()
{
	rt_uint32_t gic_cpu_base;
	rt_uint32_t gic_dist_base;
	
	rt_hw_vector_init();
	memset(isr_table, 0, sizeof(isr_table));
	gic_cpu_base = REALVIEW_GIC_CPU_BASE;
	gic_dist_base = REALVIEW_GIC_DIST_BASE;
	arm_gic_dist_init(0, gic_dist_base, 0);
	arm_gic_cpu_init(0, gic_cpu_base);

	rt_interrupt_nest  = 0;
	rt_interrupt_from_thread = 0;
	rt_interrupt_to_thread = 0;
	rt_thread_switch_interrupt_flag = 0;
	
}


void rt_hw_interrupt_umask(int vector)
{
	arm_gic_umask(0, vector);
}
