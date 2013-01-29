#ifndef __VMMAP_H
#define __VMMAP_H

// Targets
#define MEMORY_ALLOCATOR 0x1
#define MEMORY_MAPPER 0x2

struct vmmap {
	uint64_t vstart; // Virtual start
	uint64_t target; // Target object
	uint64_t offset; // Offset into target object
	uint32_t size;
	uint32_t flags;
	void *cookie;
};

#endif
