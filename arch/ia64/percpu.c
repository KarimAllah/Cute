#include <percpu.h>

/*
 * To make '__current' available to early boot code, it's statically
 * allocated in the first slot. Thus, slot 0 is reserved for the BSC.
 */
extern struct percpu cpus[CPUS_MAX];
#define BOOTSTRAP_PERCPU_AREA	((uintptr_t)&cpus[0])

void arch_percpu_area_init(enum cpu_type t)
{
	if (t == BOOTSTRAP)
		set_gs(BOOTSTRAP_PERCPU_AREA);

	/* else, we're on a secondary core where %gs
	 * is already set-up by the trampoline. */

	percpu_set(self, get_gs());
	init_tss();
}
