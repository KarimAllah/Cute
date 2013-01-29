#ifndef __MALLOC_H
#define __MALLOC_H

#include <stdint.h>

void *malloc(uint32_t size);
int vm_map(uint64_t vstart, uint64_t size, uint64_t pstart);

#endif
