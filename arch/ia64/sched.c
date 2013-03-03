#include <idt.h>
#include <pit.h>
#include <sched.h>
#include <ioapic.h>
#include <percpu.h>
#include <vectors.h>

#include "segment.h"

void arch_init_sched()
{
	extern void ticks_handler(void);
	uint8_t vector;

	/*
	 * Setup the timer ticks handler
	 *
	 * It's likely that the PIT will trigger before we enable
	 * interrupts, but even if this was the case, the vector
	 * will get 'latched' in the bootstrap local APIC IRR
	 * register and get serviced once interrupts are enabled.
	 */
	vector = TICKS_IRQ_VECTOR;
	set_intr_gate(vector, ticks_handler);
	ioapic_setup_isairq(0, vector, IRQ_BROADCAST);

	/*
	 * We can program the PIT as one-shot and re-arm it in the
	 * handler, or let it trigger IRQs monotonically. The arm
	 * method sounds a bit risky: if a single edge trigger got
	 * lost\, the entire kernel will halt.
	 */
	pit_monotonic(1000 / HZ);
}

void init_tss(void)
{
	/* Initialize TSS */
	struct gdt_register gdt = get_gdt();
	struct system_descriptor *tss_desc = (struct system_descriptor *)gdt_desc_at(gdt.base, KERNEL_TSS);
	set_tss_descriptor(tss_desc, 0x0, (uint64_t)&(percpu_addr(arch)->tss), 0x67);
	set_tr(KERNEL_TSS);
}


void arch_preempt(struct proc *new_proc, int new_prio)
{
	((struct arch_percpu *)percpu_addr(arch))->tss.rsp0 = new_proc->kstack;
}
