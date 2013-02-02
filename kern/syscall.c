#include <mm.h>
#include <msr.h>
#include <sched.h>
#include <binder.h>
#include <kernel.h>
#include <segment.h>
#include <syscall.h>
#include <percpu.h>
#include <uapi/cute/vmmap.h>

static int thread_syscall(void *data)
{
	struct thread_syscall *op = (struct thread_syscall *)data;
	switch(op->cmd)
	{
	case NEW_PROC:
	{
		printk("process : %s\n", (char *)op->data);
		struct proc *new_proc = create_proc((char *)op->data);
		if(!new_proc)
		{
			printk("No proc was created\n");
			return -1;
		}
		thread_start(new_proc);
		break;
	}
	case CLONE_PROC:
	{
		struct proc *new_proc = clone_proc(op->flags);
		if(!new_proc)
			return -1;
		thread_start(new_proc);
		break;
	}
	default:
		break;
	}
	return 0;
}

static int printf_syscall(void *data)
{
	printk("syscall msg : %s", (char *)data);
	return 0;
}

static int allocate(uint64_t start, int32_t size)
{
	struct page *page;
	do {
		page = get_zeroed_page(ZONE_ANY);
		map_range_user(current, start, PAGE_SIZE, page_phys_addr(page));
		size -= PAGE_SIZE;
		start += PAGE_SIZE;
	} while (size >= 0);
	return 0;
}

static int vmmap_syscall(void *data)
{
	struct vmmap *map = (struct vmmap *)data;
	switch(map->target)
	{
	case MEMORY_ALLOCATOR:
		return allocate(map->vstart, map->size);
		break;
	case MEMORY_MAPPER:
		map_range_user(current, map->vstart, map->size, map->offset);
		break;
	default:
		printk("Wrong target\n");
		break;
	}
	return -1;
}

static int null_syscall(void *data)
{
	printk("syscall data : %x\n", (char *)data);
	return 0;
}

static syscall_handler syscalls[SYSCALL_NR] = {
		(syscall_handler)printf_syscall,
		(syscall_handler)vmmap_syscall,
		(syscall_handler)binder_syscall,
		(syscall_handler)thread_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall,
		(syscall_handler)null_syscall
};

int __syscall(int cmd, void *data)
{
	printk("Syscall cmd : %x\n", cmd);
	if(cmd < SYSCALL_NR - 1)
	{
		int result;
		result = syscalls[cmd](data);
		return result;
	}
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
