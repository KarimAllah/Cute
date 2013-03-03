#include <page_alloc.h>
#include <memory.h>

void memory_init() {
	zones_init();
	arch_memory_init();

	/*
	 * Statistics
	 */
	struct zone *zone;
	ascending_prio_for_each(zone) {
		zone->boot_freepages = zone->freepages_count;
	}
}
