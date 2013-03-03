#ifndef ARCH_IDT_H
#define ARCH_IDT_H

/*
 * IDT table descriptor definitions and accessor methods
 *
 * Copyright (C) 2009 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 */

#define IDT_GATES	(0xFF + 1)
#define EXCEPTION_GATES (0x1F + 1)

#define GATE_INTERRUPT	0xe
#define GATE_TRAP	0xf

#ifndef __ASSEMBLY__

#include <stdint.h>
#include <paging.h>

#include "segment.h"

struct idt_gate {
	uint16_t offset_low;
	uint16_t selector;
	uint16_t ist: 3,
		 reserved0: 5,
		 type: 4,
		 reserved0_1: 1,
		 dpl: 2,
		 p: 1;
	uint16_t offset_middle;
	uint32_t offset_high;
	uint32_t reserved0_2;
} __packed;

struct idt_descriptor {
	uint16_t limit;
	uint64_t base;
} __packed;

/*
 * Symbols from idt.S
 *
 * Note that 'extern <type> *SYMBOL;' won't work since it'd mean we
 * don't point to meaningful data yet, which isn't the case.
 *
 * We use 'SYMBOL[]' since in a declaration, [] just leaves it open
 * to the number of base type objects which are present, not *where*
 * they are.
 *
 * SYMBOL[n] just adds more static-time safety; SYMBOL[n][size] let
 * the compiler automatically calculate an entry index for us.
 *
 * @IDT_STUB_SIZE: exception stub _code_ size.
 */
extern const struct idt_descriptor idtdesc;
extern struct idt_gate idt[IDT_GATES];
#define IDT_STUB_SIZE 12
extern const char idt_exception_stubs[EXCEPTION_GATES][IDT_STUB_SIZE];
extern void page_fault(void);
extern void default_irq_handler(void);

static inline void load_idt(const struct idt_descriptor *idt_desc)
{
	asm volatile("lidt %0"
		     :
		     :"m"(*idt_desc));
}

static inline struct idt_descriptor get_idt(void)
{
	struct idt_descriptor idt_desc;

	asm volatile("sidt %0"
		     :"=m"(idt_desc)
		     :);

	return idt_desc;
}

#endif /* !__ASSEMBLY__ */

#endif
