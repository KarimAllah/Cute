/*
 * Kernel threads
 *
 * Copyright (C) 2010 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 */

#include <proc.h>
#include <sched.h>
#include <atomic.h>
#include <percpu.h>
#include <mm.h>
#include <x86.h>
#include <paging.h>
#include <kmalloc.h>

/*
 * Allocate a unique thread ID
 */
uint64_t kthread_alloc_pid(void)
{
	static uint64_t pids;

	return atomic_inc(&pids);
}

extern void arch_thread_create(struct proc *proc, uint64_t kstack, uint64_t user_stack, thread_entry rip, uint16_t kernel);

static inline struct proc *thread_create_common(uint16_t kernel, thread_entry rip, uint64_t user_stack) {
	uint64_t kstack;
	struct proc *proc;

	kstack = (uint64_t)kmalloc(STACK_SIZE);
	kstack += STACK_SIZE;

	proc = kmalloc(sizeof(*proc));
	proc_init(proc);

	arch_thread_create(proc, kstack, user_stack, rip, kernel);
	return proc;
}

/*
 * Create a new kernel thread running given function
 * code, and attach it to the runqueue.
 *
 * NOTE! given function must never exit!
 */
struct proc *kthread_create(thread_entry func)
{
	return thread_create_common(1, func, 0);
}

/*
 * Create a new user thread running given function
 * code, and attach it to the runqueue.
 *
 * NOTE! given function must exit through a sys_exit system call!
 */
struct proc *uthread_create(thread_entry func, uint64_t stack, uint32_t stack_size, uint32_t __unused flags)
{
	/* New thread stack, moving down */
	stack_size = round_up(stack_size, 8);
	if(!stack)
		stack = (uint64_t)kmalloc(stack_size);
	stack += stack_size;

	return thread_create_common(0, func, stack);
}

void thread_start(struct proc *proc)
{
	/* Now push to the run queue */
	sched_enqueue(proc);
}
