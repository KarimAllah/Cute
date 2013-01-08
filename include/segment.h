#ifndef _SEGMENT_H
#define _SEGMENT_H

/*
 * Segmentation definitions; minimal by the nature of x86-64
 *
 * Copyright (C) 2009 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 */

#define USER_RPL	0x3
#define KERNEL_RPL	0x0

#define KERNEL_DS	0x10

#define KERNEL_CS	0x08
#define KERNEL_SS	0x10
#define LEGACY_CS	0x18
#define USER_SS		0x20
#define USER_CS		0x28
#define KERNEL_TSS	0x30

#define KERNEL_CS16	0x18
#define KERNEL_DS16	0x20

#ifndef __ASSEMBLY__

#include <kernel.h>
#include <stdint.h>

struct gdt_register {
	uint16_t limit;
	uint64_t base;
} __packed;

struct gdt_descriptor {
	uint64_t val;
} __packed;

struct system_descriptor {
	uint16_t limit_low;
	uint16_t base_low;

	uint8_t base_mid0;
	uint8_t type:4,
			ign_0:1,
			dpl:2,
			p:1;
	uint8_t limit_high:4,
			avl:1,
			reserved_0:2,
			g:1;
	uint8_t base_mid1;

	uint32_t base_high;

	uint32_t ign_1;
} __packed;

//#define gdt_desc_at(base, index) (&(((struct gdt_descriptor *)(base))[(index)]))
#define gdt_desc_at(base, index) ((char *)(base) + (index))

#define set_tss_descriptor(_tss_ptr, _dpl, _base, _limit) { \
	memset(_tss_ptr, 0x0, sizeof(*_tss_ptr)); \
	_tss_ptr->type = 0x9; \
	_tss_ptr->dpl = _dpl; \
	_tss_ptr->p = 0x1; \
	_tss_ptr->base_low = 0xFFFF & _base; \
	_tss_ptr->base_mid0 = ((((uint64_t)0xFF) << 16) & _base) >> 16; \
	_tss_ptr->base_mid1 = ((((uint64_t)0xFF) << 24) & _base) >> 24; \
	_tss_ptr->base_high = ((((uint64_t)0xFFFFFFFF) << 32) & _base) >> 32; \
	_tss_ptr->limit_low = 0xFFFF & _limit ; \
	_tss_ptr->limit_high = ((0xFF << 16) & _limit) >> 16; \
}

/*
 * FIXME: Reload the segment caches with the new GDT
 * values. Just changing the GDTR won't cut it.
 */
static inline void load_gdt(const struct gdt_register *gdt_desc)
{
	asm volatile("lgdt %0"
			::"m"(*gdt_desc));
}

static inline struct gdt_register get_gdt(void)
{
	struct gdt_register gdt_desc;

	asm volatile("sgdt %0"
		     :"=m"(gdt_desc)
		     :);

	return gdt_desc;
}

static inline void set_tr(uint16_t selector)
{
	asm volatile("ltr %0"
			 ::"m"(selector)
		     );
}

#endif /* !__ASSEMBLY__ */

#endif /* _SEGMENT_H */
