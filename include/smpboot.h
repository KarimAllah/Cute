#ifndef SMPBOOT_H
#define SMPBOOT_H

#include <kernel.h>

void __no_return secondary_start(void);	/* Silence-out GCC */
int smpboot_get_nr_alive_cpus(void);
void smpboot_init(void);

#endif
