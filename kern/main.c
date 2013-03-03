/*
 * Copyright (C) 2009-2011 Ahmed S. Darwish <darwish.07@gmail.com>

 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 */

#include <io.h>
#include <kernel.h>
#include <list.h>
#include <sys.h>
#include <unrolled_list.h>
#include <hash.h>
#include <bitmap.h>
#include <string.h>
#include <sections.h>
#include <smpboot.h>
#include <ramdisk.h>
#include <mm.h>
#include <binder.h>
#include <syscall.h>
#include <vm.h>
#include <paging.h>
#include <kmalloc.h>
#include <percpu.h>
#include <atomic.h>
#include <sched.h>
#include <file.h>
#include <memory.h>
#include <idt.h>
#include <init_array.h>

static void print_info(void)
{
	printk("Cute 0.0\n\n");

	printk("Text start = 0x%lx\n", __text_start);
	printk("Text end   = 0x%lx\n", __text_end);
	printk("Text size  = %d bytes\n\n", __text_end - __text_start);

	printk("Data start = 0x%lx\n", __data_start);
	printk("Data end   = 0x%lx\n", __data_end);
	printk("Data size  = %d bytes\n\n", __data_end - __data_start);

	printk("BSS start  = 0x%lx\n", __bss_start);
	printk("BSS end    = 0x%lx\n", __bss_end);
	printk("BSS size   = %d bytes\n\n", __bss_end - __bss_start);
}

/*
 * Run compiled testcases, if any
 */
static void run_test_cases(void)
{
	list_run_tests();
	unrolled_run_tests();
	hash_run_tests();
	bitmap_run_tests();
	string_run_tests();
	printk_run_tests();
	vm_run_tests();
	pagealloc_run_tests();
	kmalloc_run_tests();
	pit_run_tests();
	apic_run_tests();
	percpu_run_tests();
	atomic_run_tests();
	sched_run_tests();
	ext2_run_tests();
	file_run_tests();
}

void arch_init(void);

/*
 * Bootstrap-CPU start; we came from head.S
 */
void __no_return kernel_start(void)
{
	/* Before anything else, zero the bss section. As said by C99:
	 * “All objects with static storage duration shall be inited
	 * before program startup”, and that the implicit init is done
	 * with zero. Kernel assembly code also assumes a zeroed BSS
	 * space */
	clear_bss();

	/*
	 * Memory Management init
	 */
	//print_info();

	/* First, don't override the ramdisk area (if any) */
	ramdisk_init();

	/* and tokenize the available memory into allocatable pages */
	memory_init();

	/*
	 * Very-early setup: Do not call any code that will use
	 * `current', per-CPU vars, or a spin lock.
	 */
	arch_setup_idt();


	schedulify_this_code_path(BOOTSTRAP);

	/* With the page allocator in place, git rid of our temporary
	 * early-boot page tables and setup dynamic permanent ones */
	vm_init();


	/* MM basics done, enable dynamic heap memory to kernel code
	 * early on .. */
	kmalloc_init();

	/* Create our current kstack */
	uint64_t kstack;
	kstack = (uint64_t)kmalloc(STACK_SIZE);
	kstack += STACK_SIZE;
	current->kstack = kstack;

	arch_init();

	/* SMP infrastructure ready, fire the CPUs! */
	smpboot_init();

	init_arrays();

	/* Startup finished, roll-in the scheduler! */
	local_irq_enable();


#define VALUE_AT(addr) (*(unsigned long *)(addr))

	struct proc *user_proc;
	for (int i = 0; i < 1; i++)
	{
		user_proc = create_proc("/init");
		thread_start(user_proc);
	}

	while(1){}

	/* Launch this process */
	run_test_cases();
	halt();
}

void clear_bss(void)
{
	memset(__bss_start , 0, __bss_end - __bss_start);
}
