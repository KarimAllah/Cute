#include "mptables.h"

#include <vga.h>
#include <pic.h>
#include <apic.h>
#include <ioapic.h>
#include <kernel.h>
#include <serial.h>
#include <keyboard.h>

void early_init(void) {
	kernel_start();
}

void __no_return arch_init(void) {
	/*
	 * Secondary-CPUs startup
	 */

	/* Discover our secondary-CPUs and system IRQs layout before
	 * initializing the local APICs */
	mptables_init();

	/* Remap and mask the PIC; it's just a disturbance */
	serial_init();
	pic_init();

	/* Initialize the APICs (and map their MMIO regs) before enabling
	 * IRQs, and before firing other cores using Inter-CPU Interrupts */
	apic_init();
	ioapic_init();

	keyboard_init();


	/* VGA init */
	vga_init(VIDMEM_MODE13);
	render_plane(&vga_logo);
}
