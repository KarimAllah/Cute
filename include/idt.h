#ifndef IDT_H
#define IDT_H

#include <stdint.h>

void arch_setup_idt(void);

void set_intr_gate(unsigned int n, void *addr);

void local_irq_disable(void);
void local_irq_enable(void);

uint64_t local_irq_disable_save(void);
void local_irq_restore(uint64_t flags);

#endif
