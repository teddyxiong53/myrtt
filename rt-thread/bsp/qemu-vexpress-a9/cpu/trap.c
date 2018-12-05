#include <rtthread.h>
#include <rthw.h>
#include "realview.h"


void rt_hw_trap_undef(struct rt_hw_exp_stack *regs)
{
	
}



void rt_hw_trap_swi(struct rt_hw_exp_stack *regs)
{
	
}

void rt_hw_trap_pabt(struct rt_hw_exp_stack *regs)
{
	
}

void rt_hw_trap_dabt(struct rt_hw_exp_stack *regs)
{
	
}

void rt_hw_trap_resv(struct rt_hw_exp_stack *regs)
{
	
}
#define GIC_ACK_INTID_MASK 0x000003ff

void rt_hw_trap_irq(void)
{
	void *param;
	unsigned long ir, fullir;
	rt_isr_handler_t isr_func;
	extern struct rt_irq_desc isr_table[];
	fullir = arm_gic_get_active_irq(0);
	ir = fullir & GIC_ACK_INTID_MASK;
	if(ir == 1023) {
		return;
	}
	isr_func = isr_table[ir].handler;
	if(isr_func) {
		param = isr_table[ir].param;
		isr_func(ir, param);
	}
	arm_gic_ack(0, fullir);
}

void rt_hw_trap_fiq(void)
{
	//void *param;
}
