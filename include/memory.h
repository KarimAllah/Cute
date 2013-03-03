#ifndef MEMORY_H
#define MEMORY_H

void memory_init(void);
void arch_memory_init(void);
uint64_t get_phys_addr_end(void);

#endif
