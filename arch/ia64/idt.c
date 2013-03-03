/*
 * IDT table descriptor definitions and accessor methods
 *
 * Copyright (C) 2009 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 */

#include <idt.h>
#include <x86.h>
#include <stdint.h>
#include <paging.h>
#include <vectors.h>

#include "idt.h"
#include "segment.h"

void do_page_fault(uint32_t err_code)
{
	generic_page_fault(err_code);
}

static inline void pack_idt_gate(struct idt_gate *gate, uint8_t type, void *addr)
{
	gate->offset_low = (uintptr_t)addr & 0xffff;
	gate->selector = KERNEL_CS;
	gate->ist = 0;
	gate->reserved0 = 0;
	gate->type = type;
	gate->reserved0_1 = 0;
	gate->dpl = 0;
	gate->p = 1;
	gate->offset_middle = ((uintptr_t)addr >> 16) & 0xffff;
	gate->offset_high = (uintptr_t)addr >> 32;
	gate->reserved0_2 = 0;
}

static inline void write_idt_gate(struct idt_gate *gate, struct idt_gate *idt,
				  unsigned offset)
{
	assert(offset < IDT_GATES);
	idt[offset] = *gate;
}

/*
 * The only difference between an interrupt gate and a trap gate
 * is the way the processor handles the IF flag in the EFLAGS.
 *
 * Trap gates leaves the IF flag set while servicing the interrupt,
 * which means handlers can get interrupted indefinitely, and our
 * stack can get overflowed in a matter of milliseconds.
 *
 * Interrupt gates on the other hand clear the IF flag upon entry.
 * A subsequent IRET instruction restores the IF flag to its value
 * in the saved contents.
 */

void set_intr_gate(unsigned int n, void *addr)
{
	struct idt_gate gate;
	pack_idt_gate(&gate, GATE_INTERRUPT, addr);
	write_idt_gate(&gate, idt, n);
}

void local_irq_disable(void)
{
	asm volatile ("cli"
		      ::
		      :"cc", "memory");
}

void local_irq_enable(void)
{
	asm volatile ("sti"
		      ::
		      :"cc", "memory");
}

uint64_t local_irq_disable_save(void)
{
	union x86_rflags flags;

	flags = get_rflags();
	if (flags.irqs_enabled)
		local_irq_disable();

	return flags.raw;
}

void local_irq_restore(uint64_t flags)
{
	union x86_rflags _flags = (union x86_rflags)flags;
	if (_flags.irqs_enabled)
		set_rflags(_flags);
}

void arch_setup_idt(void)
{
	for (int i = 0; i < EXCEPTION_GATES; i ++)
		set_intr_gate(i, &idt_exception_stubs[i]);

	set_intr_gate(PAGE_FAULT, page_fault);
	set_intr_gate(HALT_CPU_IPI_VECTOR, halt_cpu_ipi_handler);

	load_idt(&idtdesc);
}
