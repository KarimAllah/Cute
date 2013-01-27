#include <malloc.h>
#include <syscall.h>
#include <stdint.h>
#include <cute/vmmap.h>



#define USER_MALLOC_ADDR (128*1024*1024)
#define USER_MALLOC_SIZE (1024 * 1024)

static bool initialized = false;
static uint64_t cur_addr = USER_MALLOC_ADDR;

static int mmap(uint64_t start, int32_t size, uint32_t flags)
{
	struct vmmap map;
	map.start = start;
	map.size = size;
	map.flags = flags;
	return syscall(SYSCALL_VMMAP, (void *)&map);
}

static void _init()
{
	if(initialized)
		return;
	mmap(cur_addr, USER_MALLOC_SIZE, MMAP_ANONYMOUS);
	initialized = true;
}

void *malloc(uint32_t size)
{
	uint64_t addr;
	if(!initialized)
		_init();
	addr = cur_addr;
	cur_addr += size;
	return (void *)addr;
}
