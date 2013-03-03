#ifndef PAGE_ALLOC_H
#define PAGE_ALLOC_H

#include <mm.h>
#include <spinlock.h>

/*
 * Page allocator Zone Descriptor
 */
struct zone {
	/* Statically initialized */
	enum zone_id id;		/* Self reference (for iterators) */
	uint64_t start;			/* Physical range start address */
	uint64_t end;			/* Physical range end address */
	const char *description;	/* For kernel log messages */

	/* Dynamically initialized */
	struct page *freelist;		/* Connect zone's unallocated pages */
	spinlock_t freelist_lock;	/* Above list protection */
	uint64_t freepages_count;	/* Stats: # of free pages now */
	uint64_t boot_freepages;	/* Stats: # of free pages at boot */
};


/*
 * Reverse Mapping Descriptor
 *
 * This structure aids in reverse-mapping a virtual address
 * to its respective page descriptor.
 *
 * We store page descriptors representing a specific e820-
 * available range sequentially, thus a reference to a range
 * and the pfdtable cell representing its first page is
 * enough for reverse-mapping any address in such a range.
 */
struct rmap {
	uint64_t base;
	uint64_t len;
	struct page *pfd_start;
};

extern struct zone zones[];

extern uint64_t kmem_end;

void pfdtable_add_range(uint64_t base, uint64_t len, uint32_t type);
void zones_init(void);

/*
 * Do not reference the zones[] table directly
 *
 * Use below iterators in desired priority order, or check
 * get_zone(), which does the necessery ID sanity checks.
 */

#define descending_prio_for_each(zone)					\
	for (zone = &zones[0]; zone <= &zones[ZONE_ANY]; zone++)

#define ascending_prio_for_each(zone)					\
	for (zone = &zones[ZONE_ANY]; zone >= &zones[0]; zone--)

#endif
