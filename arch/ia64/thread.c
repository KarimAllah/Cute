#include <mm.h>
#include <proc.h>
#include <sched.h>
#include <percpu.h>

#include "segment.h"

static void virtual_init(struct proc *target, struct proc *source)
{
	/* Create pml4 */
	target->cr3 = page_phys_addr(get_zeroed_page(ZONE_1GB));

	/* For now, only map the kernel 512 Gbyte segment */
	*((struct pml4e *)VIRT(target->cr3) + pml4_index(KERN_PAGE_OFFSET)) =
			*((struct pml4e *)VIRT(source->cr3) + pml4_index(KERN_PAGE_OFFSET));
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
 * the context switching code does.
 */
void arch_thread_create(struct proc *proc, uint64_t kstack, uint64_t user_stack, thread_entry rip, uint16_t kernel)
{
	uint16_t code_segment = kernel ? (KERNEL_CS | KERNEL_RPL) : (USER_CS | USER_RPL);
	struct irq_ctx *irq_ctx;

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
}
