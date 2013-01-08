#include <segment.h>
#include <syscall.h>
#include <msr.h>

void __no_return __syscall(void)
{
	printk("Received a syscall ....\n");
}

void init_syscall(void)
{
	write_msr(STAR, (((uint64_t)KERNEL_CS | KERNEL_RPL) << 32) | (((uint64_t)LEGACY_CS | USER_RPL)<< 48));
	write_msr(LSTAR, (uint64_t)syscall);
}
