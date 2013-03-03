#include <mm.h>
#include <paging.h>
#include <ramdisk.h>
#include <page_alloc.h>

#include "e820.h"

uint64_t get_phys_addr_end() {
	return e820_get_phys_addr_end();
}

void arch_memory_init(void) {
	struct e820_range *range;
	struct e820_setup *setup;
	uint64_t avail_pages, avail_ranges;

	/*
	 * While building the page descriptors in the pfdtable,
	 * we want to be sure not to include pages that override
	 * our own pfdtable area. i.e. we want to know the
	 * pfdtable area length _before_ forming its entries.
	 *
	 * Since the pfdtable length depends on available phys
	 * memory, we move over the e820 map in two passes.
	 *
	 * First pass: estimate pfdtable area length by counting
	 * provided e820-availale pages and ranges.
	 */

	e820_init();
	setup = e820_get_memory_setup();
	avail_pages = setup->avail_pages;
	avail_ranges = setup->avail_ranges;

	pagealloc_init(avail_ranges, avail_pages);

	/*
	 * Second Pass: actually fill the pfdtable entries
	 *
	 * Including add_range(), this loop is O(n), where
	 * n = number of available memory pages in the system
	 */
	e820_for_each(range) {
		if (range->type != E820_AVAIL)
			continue;
		if (e820_sanitize_range(range, kmem_end))
			continue;

		pfdtable_add_range(range->base, range->len, range->type);
	}
}
