#include "msr.h"
#include "segment.h"
#include "syscall.h"

void arch_syscall_init(void)
{
	extern void arch_syscall(void);
	write_msr(STAR, (((uint64_t)KERNEL_CS | KERNEL_RPL) << 32) | (((uint64_t)LEGACY_CS | USER_RPL)<< 48));
	write_msr(LSTAR, (uint64_t)arch_syscall);
}
