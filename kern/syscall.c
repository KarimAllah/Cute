#include <msr.h>
#include <kernel.h>
#include <segment.h>
#include <syscall.h>


static int printf_syscall(void *data)
{
	printk("syscall msg : %s\n", (char *)data);
	return 0;
}

static int null_syscall(void *data)
{
	printk("syscall data : %x\n", (char *)data);
	return 0;
}

static syscall_handler syscalls[SYSCALL_NR] = {
		(syscall_handler)printf_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall
};

int __syscall(int cmd, void *data)
{
	printk("syscall number (%x)\n", cmd);

	if(cmd < SYSCALL_NR - 1)
		return syscalls[cmd](data);
	else
	{
		printk("invalid syscall\n");
		return -1;
	}
}

void init_syscall(void)
{
	write_msr(STAR, (((uint64_t)KERNEL_CS | KERNEL_RPL) << 32) | (((uint64_t)LEGACY_CS | USER_RPL)<< 48));
	write_msr(LSTAR, (uint64_t)syscall);
}
