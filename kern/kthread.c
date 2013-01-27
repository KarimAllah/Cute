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
#include <segment.h>
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

static void virtual_init(struct proc *target, struct proc *source)
{
	/* Create pml4 */
	target->cr3 = page_phys_addr(get_zeroed_page(ZONE_1GB));

	/* For now, only map the kernel 512 Gbyte segment */
	*((struct pml4e *)VIRTUAL(target->cr3) + pml4_index(KERN_PAGE_OFFSET)) =
			*((struct pml4e *)VIRTUAL(source->cr3) + pml4_index(KERN_PAGE_OFFSET));
}

/* Reserve space for our IRQ stack protocol */
/*
 * Values for the code to-be-executed once scheduled.
 * They will get popped and used automatically by the
 * processor at ticks handler `iretq'.
 *
 * Set to-be-executed code's %rsp to the top of the
 * newly allocated stack since this new code doesn't
 * care about the values currently 'pushed'; only
 * the ctontext switching code does.
 */
static inline struct proc *thread_create_common(uint16_t code_segment, thread_entry rip, uint64_t user_stack) {
	uint64_t kstack;
	struct proc *proc;
	struct irq_ctx *irq_ctx;

	kstack = (uint64_t)kmalloc(STACK_SIZE);
	kstack += STACK_SIZE;

	proc = kmalloc(sizeof(*proc));
	proc_init(proc);
	virtual_init(proc, current);

	proc->kstack = (uint64_t)kstack;

	irq_ctx = (struct irq_ctx *)(kstack - sizeof(*irq_ctx));
	irq_ctx_init(irq_ctx);
	irq_ctx->cs = code_segment;
	irq_ctx->rip = (uintptr_t)rip;
	/* If this is a user thread, we've to use a valid SS or we will get a #GP. */
	irq_ctx->ss = user_stack ? USER_SS | USER_RPL : 0 | KERNEL_RPL;
	/* If we are creating a kernel thread, we use the same stack for user and kernel */
	if(!user_stack)
		user_stack = kstack;
	irq_ctx->rsp = (uintptr_t)user_stack;
	irq_ctx->rflags = default_rflags().raw;

	/* For context switching code, which runs at the
	 * ticks handler context, give a stack that respects
	 * our IRQ stack protocol */
	proc->pcb.rsp = (uintptr_t)irq_ctx;
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
	return thread_create_common(KERNEL_CS | KERNEL_RPL, func, 0);
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

	return thread_create_common(USER_CS | USER_RPL, func, stack);
}

void thread_start(struct proc *proc)
{
	/* Now push to the run queue */
	sched_enqueue(proc);
}
