#include <vm.h>
#include <mm.h>
#include <sys.h>
#include <file.h>
#include <fcntl.h>
#include <paging.h>
#include <percpu.h>

struct proc *clone_proc(int flags)
{
	struct proc *user_proc = uthread_create((thread_entry)(USER_START_ADDR), USER_STACK_ADDR, PAGE_SIZE/*stack_size=4Kb*/, 0);
	// Clone address space
	copy_address_space(user_proc, current);
	return user_proc;
}


static int allocate(struct proc *proc, uint64_t start, int32_t size)
{
	struct page *page;
	do {
		page = get_free_page(ZONE_ANY);
		map_range_user(proc, start, PAGE_SIZE, page_phys_addr(page));
		size -= PAGE_SIZE;
		start += PAGE_SIZE;
	} while (size >= 0);
	return 0;
}

struct proc *create_proc(char *path)
{
	struct proc *user_proc = uthread_create((thread_entry)(USER_START_ADDR), USER_STACK_ADDR, PAGE_SIZE/*stack_size=4Kb*/, 0);

	// Map stack
	allocate(user_proc, USER_STACK_ADDR, PAGE_SIZE);
	int fd = sys_open(path, O_RDONLY, 0);
	if(fd < 0)
		printk("problem opening the file\n");

	// Map process
	struct page *page;
	int32_t size = USER_SIZE;
	uint64_t vaddr = USER_START_ADDR;
	uint64_t index = 0;
	do {
		page = get_free_page(ZONE_ANY);
		map_range_user(user_proc, vaddr, PAGE_SIZE, page_phys_addr(page));
		uint64_t read_size = sys_read(fd, page_address(page), PAGE_SIZE);
		size -= PAGE_SIZE;
		vaddr += PAGE_SIZE;
	} while (size >= 0);

	sys_close(fd);
	return user_proc;
}
