#ifndef __ARCH_PERCPU_H
#define __ARCH_PERCPU_H

#include <kernel.h>

struct tss {
	uint32_t reserved_0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint32_t reserved_1;
	uint32_t reserved_2;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint32_t reserved_3;
	uint32_t reserved_4;
	uint16_t reserved_5;
	uint16_t io_map_addr;
} __packed;

struct arch_percpu {
	struct tss tss;
	int apic_id;			/* Local APIC ID */
} __packed;

#endif
