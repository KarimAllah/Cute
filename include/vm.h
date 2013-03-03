#ifndef _VM_H
#define _VM_H

/*
 * Kernel virtual memory
 *
 * Copyright (C) 2010 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 */

#include <arch/vm.h>
#include <tests.h>
#include <proc.h>

typedef uint64_t vm_cpu_addr_t;
typedef uint64_t vm_bus_addr_t;
typedef uint64_t vm_virt_addr_t;

typedef struct {
	uint64_t start;
	uint64_t size;
} vm_memory_range_t;

void vm_init(void);
void map_range_kernel(uintptr_t vstart, uint64_t vlen, uintptr_t pstart);
void map_range_user(struct proc *proc, uintptr_t vstart, uint64_t vlen, uintptr_t pstart);
void copy_address_space(struct proc *dst_proc, struct proc *src_proc);
void *vm_kmap(uintptr_t pstart, uint64_t len);

#if	VM_TESTS

void vm_run_tests(void);

#else /* !VM_TESTS */

static inline void vm_run_tests(void) { }

#endif /* VM_TESTS */

#endif /* _VM_H */
