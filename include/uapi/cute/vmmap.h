#ifndef __VMMAP_H
#define __VMMAP_H

#define MMAP_ANONYMOUS 0x1

struct vmmap {
	uint32_t size;
	uint32_t flags;
	uint64_t start;
};

#endif
