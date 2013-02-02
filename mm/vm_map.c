/*
 * Memory Management: kernel virtual memory
 *
 * Copyright (C) 2010 Ahmed S. Darwish <darwish.07@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2.
 *
 * So far, we've depended on the boot page tables being built early-on
 * by head.S setup code. Here, we build and apply our permanent mappings
 * for all kinds of kernel virtual addresses -- check paging.h
 *
 * NOTE! Always ask the page allocator for pages residing in the first
 * physical GB zone. Up to this point, _only_ virtual addresses represe-
 * nting that zone (beside kernel text addresses) were mapped by head.S
 */

#include <kernel.h>
#include <paging.h>
#include <percpu.h>
#include <mm.h>
#include <e820.h>
#include <vm.h>
#include <tests.h>



/*
 * Fill given PML2 table with entries mapping the virtual
 * range (@vstart - @vend) to physical @pstart upwards.
 *
 * Note-1! pass a valid table; unused entries must be zero
 * Note-2! range edges, and @pstart must be 2-MBytes aligned
 */
static void map_pml1_range(struct pml1e *pml1_base, uintptr_t vstart,
			   uintptr_t vend, uintptr_t pstart, bool kernel)
{
	struct pml1e *pml1e;
	if ((vend - vstart) > (0x1ULL << PAGE_SHIFT_2MB))
		panic("A PML2 table cant map ranges > 2-MByte. "
		      "Given range: 0x%lx - 0x%lx", vstart, vend);

	for (pml1e = pml1_base + pml1_index(vstart);
		pml1e <= pml1_base + pml1_index(vend - 1);
		pml1e++) {
		pml1e->present = 1;
		pml1e->read_write = 1;
		if(kernel)
			pml1e->user_supervisor = 0;
		else
			pml1e->user_supervisor = 1;
		pml1e->page_base = (uintptr_t)pstart >> PAGE_SHIFT;

		pstart += PML1_ENTRY_MAPPING_SIZE;
		vstart += PML1_ENTRY_MAPPING_SIZE;
	}
}

/*
 * Fill given PML2 table with entries mapping the virtual
 * range (@vstart - @vend) to physical @pstart upwards.
 *
 * Note-1! pass a valid table; unused entries must be zero
 * Note-2! range edges, and @pstart must be 2-MBytes aligned
 */
static void map_pml2_range(struct pml2e *pml2_base, uintptr_t vstart,
			   uintptr_t vend, uintptr_t pstart, bool kernel)
{
	struct pml1e *pml1_base;
	struct pml2e *pml2e;
	struct page *page;
	uintptr_t end;

	assert(page_aligned(pml2_base));

	if ((vend - vstart) > (0x1ULL << 30))
		panic("A PML2 table cant map ranges > 1-GByte. "
		      "Given range: 0x%lx - 0x%lx", vstart, vend);

	for (pml2e = pml2_base + pml2_index(vstart);
	     pml2e <= pml2_base + pml2_index(vend - 1);
	     pml2e++) {

		if (!pml2e->present) {
			pml2e->present = 1;
			pml2e->read_write = 1;
			if(kernel)
				pml2e->user_supervisor = 0;
			else
				pml2e->user_supervisor = 1;
			page = get_zeroed_page(ZONE_1GB);
			pml2e->pt_base = page_phys_addr(page) >> PAGE_SHIFT;
		}

		pml1_base = VIRTUAL((uintptr_t)pml2e->pt_base << PAGE_SHIFT);

		if (pml2e == pml2_base + pml2_index(vend - 1)) /* Last entry */
			end = vend;
		else
			end = vstart + PML2_ENTRY_MAPPING_SIZE;

		map_pml1_range(pml1_base, vstart, end, pstart, kernel);

		pstart += PML2_ENTRY_MAPPING_SIZE;
		vstart += PML2_ENTRY_MAPPING_SIZE;
	}
}

/*
 * Fill given PML3 table with entries mapping the virtual
 * range (@vstart - @vend) to physical @pstart upwards.
 *
 * Note-1! pass a valid table; unused entries must be zero
 * Note-2! range edges, and @pstart must be 2-MBytes aligned
 */
static void map_pml3_range(struct pml3e *pml3_base, uintptr_t vstart,
			   uintptr_t vend, uintptr_t pstart, bool kernel)
{
	struct pml3e *pml3e;
	struct pml2e *pml2_base;
	struct page *page;
	uintptr_t end;

	assert(page_aligned(pml3_base));

	if ((vend - vstart) > PML3_MAPPING_SIZE)
		panic("A PML3 table can't map ranges > 512-GBytes. "
		      "Given range: 0x%lx - 0x%lx", vstart, vend);

	for (pml3e = pml3_base + pml3_index(vstart);
	     pml3e <= pml3_base + pml3_index(vend - 1);
	     pml3e++) {
		assert((char *)pml3e < (char *)pml3_base + PAGE_SIZE);
		if (!pml3e->present) {
			pml3e->present = 1;
			pml3e->read_write = 1;
			if(kernel)
				pml3e->user_supervisor = 0;
			else
				pml3e->user_supervisor = 1;
			page = get_zeroed_page(ZONE_1GB);
			pml3e->pml2_base = page_phys_addr(page) >> PAGE_SHIFT;
		}

		pml2_base = VIRTUAL((uintptr_t)pml3e->pml2_base << PAGE_SHIFT);

		if (pml3e == pml3_base + pml3_index(vend - 1)) /* Last entry */
			end = vend;
		else
			end = vstart + PML3_ENTRY_MAPPING_SIZE;

		map_pml2_range(pml2_base, vstart, end, pstart, kernel);

		pstart += PML3_ENTRY_MAPPING_SIZE;
		vstart += PML3_ENTRY_MAPPING_SIZE;
	}
}

/*
 * Fill given PML4 table with entries mapping the virtual
 * range (@vstart - @vend) to physical @pstart upwards.
 *
 * Note-1! pass a valid table; unused entries must be zero
 * Note-2! range edges, and @pstart must be 2-MBytes aligned
 */
static void map_pml4_range(struct pml4e *pml4_base, uintptr_t vstart,
			   uintptr_t vend, uintptr_t pstart, bool kernel)
{
	struct pml4e *pml4e;
	struct pml3e *pml3_base;
	struct page *page;
	uintptr_t end;

	assert(page_aligned(pml4_base));

	if ((vend - vstart) > PML4_MAPPING_SIZE)
		panic("Mapping a virtual range that exceeds the 48-bit "
		      "architectural limit: 0x%lx - 0x%lx", vstart, vend);

	for (pml4e = pml4_base + pml4_index(vstart);
	     pml4e <= pml4_base + pml4_index(vend - 1);
	     pml4e++) {
		assert((char *)pml4e < (char *)pml4_base + PAGE_SIZE);
		if (!pml4e->present) {
			if (kernel) {
				pml4e->user_supervisor = 0;
			} else {
				pml4e->user_supervisor = 1;
			}
			pml4e->present = 1;
			pml4e->read_write = 1;
			page = get_zeroed_page(ZONE_1GB);
			pml4e->pml3_base = page_phys_addr(page) >> PAGE_SHIFT;
		}

		pml3_base = VIRTUAL((uintptr_t)pml4e->pml3_base << PAGE_SHIFT);

		if (pml4e == pml4_base + pml4_index(vend - 1)) /* Last entry */
			end = vend;
		else
			end = vstart + PML4_ENTRY_MAPPING_SIZE;

		map_pml3_range(pml3_base, vstart, end, pstart, kernel);

		pstart += PML4_ENTRY_MAPPING_SIZE;
		vstart += PML4_ENTRY_MAPPING_SIZE;
	}
}

static void map_range_common(struct pml4e *pml4, uintptr_t vstart, uint64_t vlen, uintptr_t pstart, bool kernel)
{
	map_pml4_range(pml4, vstart, vstart + vlen, pstart, kernel);
}

/*
 * Map given kernel virtual region to physical @pstart upwards.
 * @vstart is region start, while @vlen is its length. All
 * sanity checks are done in the map_pml{2,3,4}_range() code
 * where all the work is really done.
 *
 * Note-1! Given range, and its subranges, must be unmapped
 * Note-2! region edges, and @pstart must be 2-MBytes aligned
 */
void map_range_kernel(uintptr_t vstart, uint64_t vlen, uintptr_t pstart)
{
	struct pml4e *pml4 = (struct pml4e *)VIRTUAL(current->cr3);
	map_range_common(pml4, vstart, vlen, pstart, true);
}

void map_range_user(struct proc *proc, uintptr_t vstart, uint64_t vlen, uintptr_t pstart)
{
	struct pml4e *pml4 = (struct pml4e *)VIRTUAL(proc->cr3);
	map_range_common(pml4, vstart, vlen, pstart, false);
}

static void copy_pml1_range(struct pml1e *dst_pml1_base, struct pml1e *src_pml1_base,
		uintptr_t vstart, uintptr_t vend)
{
	struct pml1e *src_pml1e;
	struct pml1e *dst_pml1e;
	if ((vend - vstart) > (0x1ULL << PAGE_SHIFT_2MB))
		panic("A PML2 table cant map ranges > 2-MByte. "
		      "Given range: 0x%lx - 0x%lx", vstart, vend);

	for (dst_pml1e = dst_pml1_base + pml1_index(vstart), src_pml1e = src_pml1_base + pml1_index(vstart);
		src_pml1e <= src_pml1_base + pml1_index(vend - 1);
		src_pml1e++) {
		*dst_pml1e = *src_pml1e;
		vstart += PML1_ENTRY_MAPPING_SIZE;
	}
}

static void copy_pml2_range(struct pml2e *dst_pml2_base, struct pml2e *src_pml2_base, uintptr_t vstart,
			   uintptr_t vend)
{
	struct pml1e *src_pml1_base;
	struct pml1e *dst_pml1_base;
	struct pml2e *src_pml2e;
	struct pml2e *dst_pml2e;
	struct page *page;
	uintptr_t end;

	assert(page_aligned(src_pml2_base));

	if ((vend - vstart) > (0x1ULL << 30))
		panic("A PML2 table cant map ranges > 1-GByte. "
		      "Given range: 0x%lx - 0x%lx", vstart, vend);

	for (dst_pml2e = dst_pml2_base + pml2_index(vstart), src_pml2e = src_pml2_base + pml2_index(vstart);
	     src_pml2e <= src_pml2_base + pml2_index(vend - 1);
	     dst_pml2e++, src_pml2e++) {

		if (!src_pml2e->present)
			continue;

		*dst_pml2e = *src_pml2e;
		page = get_zeroed_page(ZONE_1GB);
		dst_pml2e->pt_base = page_phys_addr(page) >> PAGE_SHIFT;

		src_pml1_base = VIRTUAL((uintptr_t)src_pml2e->pt_base << PAGE_SHIFT);
		dst_pml1_base = VIRTUAL((uintptr_t)src_pml2e->pt_base << PAGE_SHIFT);

		if (src_pml2e == src_pml2_base + pml1_index(vend - 1)) /* Last entry */
			end = vend;
		else
			end = vstart + PML2_ENTRY_MAPPING_SIZE;

		copy_pml1_range(dst_pml1_base, src_pml1_base, vstart, end);

		vstart += PML2_ENTRY_MAPPING_SIZE;
	}
}

static void copy_pml3_range(struct pml3e *dst_pml3_base, struct pml3e *src_pml3_base,
			uintptr_t vstart, uintptr_t vend)
{
	struct pml3e *src_pml3e;
	struct pml3e *dst_pml3e;
	struct pml2e *src_pml2_base;
	struct pml2e *dst_pml2_base;
	struct page *page;
	uintptr_t end;

	assert(page_aligned(src_pml3_base));

	if ((vend - vstart) > PML3_MAPPING_SIZE)
		panic("A PML3 table can't copy ranges > 512-GBytes. "
		      "Given range: 0x%lx - 0x%lx", vstart, vend);

	for (dst_pml3e = dst_pml3_base + pml3_index(vstart), src_pml3e = src_pml3_base + pml3_index(vstart);
	     src_pml3e <= src_pml3_base + pml3_index(vend - 1);
	     dst_pml3e++, src_pml3e++) {
		assert((char *)src_pml3e < (char *)src_pml3_base + PAGE_SIZE);
		if (!src_pml3e->present)
			continue;

		*dst_pml3e = *src_pml3e;
		page = get_zeroed_page(ZONE_1GB);
		dst_pml3e->pml2_base = page_phys_addr(page) >> PAGE_SHIFT;

		src_pml2_base = VIRTUAL((uintptr_t)src_pml3e->pml2_base << PAGE_SHIFT);
		dst_pml2_base = VIRTUAL((uintptr_t)dst_pml3e->pml2_base << PAGE_SHIFT);

		if (src_pml3e == src_pml3_base + pml3_index(vend - 1)) /* Last entry */
			end = vend;
		else
			end = vstart + PML3_ENTRY_MAPPING_SIZE;

		copy_pml2_range(dst_pml2_base, src_pml2_base, vstart, end);

		vstart += PML3_ENTRY_MAPPING_SIZE;
	}
}

static void copy_pml4_range(struct pml4e *dst_pml4_base, struct pml4e *src_pml4_base, uintptr_t vstart,
			   uintptr_t vend)
{
	struct pml4e *src_pml4e;
	struct pml4e *dst_pml4e;
	struct pml3e *src_pml3_base;
	struct pml3e *dst_pml3_base;
	struct page *page;
	uintptr_t end;

	assert(page_aligned(src_pml4_base));

	if ((vend - vstart) > PML4_MAPPING_SIZE)
		panic("copying a virtual range that exceeds the 48-bit "
		      "architectural limit: 0x%lx - 0x%lx", vstart, vend);

	for (dst_pml4e = dst_pml4_base + pml4_index(vstart), src_pml4e = src_pml4_base + pml4_index(vstart);
	     src_pml4e <= src_pml4_base + pml4_index(vend - 1);
	     dst_pml4e++, src_pml4e++) {
		assert((char *)src_pml4e < (char *)src_pml4_base + PAGE_SIZE);
		if (!src_pml4e->present)
			continue;

		*dst_pml4e = *src_pml4e;
		page = get_zeroed_page(ZONE_1GB);
		dst_pml4e->pml3_base = page_phys_addr(page) >> PAGE_SHIFT;

		src_pml3_base = VIRTUAL((uintptr_t)src_pml4e->pml3_base << PAGE_SHIFT);
		dst_pml3_base = VIRTUAL((uintptr_t)dst_pml4e->pml3_base << PAGE_SHIFT);

		if (src_pml4e == src_pml4_base + pml4_index(vend - 1)) /* Last entry */
			end = vend;
		else
			end = vstart + PML4_ENTRY_MAPPING_SIZE;

		copy_pml3_range(dst_pml3_base, src_pml3_base, vstart, end);

		vstart += PML4_ENTRY_MAPPING_SIZE;
	}
}

void copy_address_space(struct proc *dst_proc, struct proc *src_proc)
{
	uintptr_t vstart = 0x0;
	uint64_t vlen = PAGE_SIZE * 1024;
	struct pml4e *src_pml4 = (struct pml4e *)VIRTUAL(src_proc->cr3);
	struct pml4e *dst_pml4 = (struct pml4e *)VIRTUAL(dst_proc->cr3);
	copy_pml4_range(dst_pml4, src_pml4, vstart, vstart + vlen);
}

/*
 * Check if given virtual address is mapped at our permanent
 * kernel page tables. If so, also assure that given address
 * is mapped to the expected physical address.
 */
static bool vaddr_is_mapped(void *vaddr)
{
	struct pml4e *pml4e;
	struct pml3e *pml3e;
	struct pml2e *pml2e;
	struct pml1e *pml1e;

	pml4e = ((struct pml4e *)VIRTUAL(current->cr3)) + pml4_index(vaddr);
	if (!pml4e->present)
		return false;

	pml3e = pml3_base(pml4e);
	pml3e += pml3_index(vaddr);
	if (!pml3e->present)
		return false;

	pml2e = pml2_base(pml3e);
	pml2e += pml2_index(vaddr);
	if (!pml2e->present)
		return false;

	pml1e = pml1_base(pml2e);
	pml1e += pml1_index(vaddr);
	if (!pml1e->present)
		return false;

	assert((uintptr_t)page_base(pml1e) ==
	       round_down((uintptr_t)vaddr, PAGE_SIZE));
	return true;
}

/*
 * Map given physical range (@pstart -> @pstart+@len) at
 * kernel physical mappings space. Return mapped virtual
 * address.
 */
void *vm_kmap(uintptr_t pstart, uint64_t len)
{
	uintptr_t pend;
	void *vstart, *ret;

	assert(len > 0);
	pend = pstart + len;

	if (pend >= KERN_PHYS_END_MAX)
		panic("VM - Mapping physical region [0x%lx - 0x%lx] "
		      ">= max supported physical addresses end 0x%lx",
		      pstart, pend, KERN_PHYS_END_MAX);

	ret = VIRTUAL(pstart);
	pstart = round_down(pstart, PAGE_SIZE_2MB);
	pend = round_up(pend, PAGE_SIZE_2MB);

	while (pstart < pend) {
		vstart = VIRTUAL(pstart);
		if (!vaddr_is_mapped(vstart))
			map_range_kernel((uintptr_t)vstart,
					 PAGE_SIZE_2MB, pstart);

		pstart += PAGE_SIZE_2MB;
	}

	return ret;
}

/*
 * Ditch boot page tables and build kernel permanent,
 * dynamically handled, ones.
 */
void vm_init(void)
{
	uint64_t phys_end;

	/* Map 512-MByte kernel text area */
	map_range_kernel(KTEXT_PAGE_OFFSET, KTEXT_AREA_SIZE, KTEXT_PHYS_OFFSET);

	/* Map the entire available physical space */
	phys_end = e820_get_phys_addr_end();
	phys_end = round_up(phys_end, PAGE_SIZE_2MB);
	map_range_kernel(KERN_PAGE_OFFSET, phys_end, KERN_PHYS_OFFSET);
	printk("Memory: Mapping range 0x%lx -> 0x%lx to physical 0x0\n",
	       KERN_PAGE_OFFSET, KERN_PAGE_OFFSET + phys_end);

	/* Heaven be with us .. */
	load_cr3(current->cr3);
}

/*
 * Kernel virtual memory test cases
 */

#if	VM_TESTS

/*
 * Check if all physical memory is really
 * mapped as supossed to be; bailout otherwise
 */
static void vm_check_phys_memory(void)
{
	uint64_t phys_end;

	phys_end = e820_get_phys_addr_end();

	for (uintptr_t vaddr = KERN_PAGE_OFFSET;
	     vaddr < (KERN_PAGE_OFFSET + phys_end); vaddr++) {

		/* This also assures that given address is mapped
		 * to the expected physical address, according to
		 * our kernel address space mapping rules */
		if (!vaddr_is_mapped((char *)vaddr))
			panic("_VM: Reporting supposedly mapped address 0x%lx "
			      "as unmapped", vaddr);

		/* Limit the output a bit .. */
		if (vaddr > (KERN_PAGE_OFFSET + 0x20000) &&
		    is_aligned(vaddr, 0x200000))
			printk("Success: e820-avail phys range [0x%lx - 0x%lx] "
			       "mapped\n", PHYS(vaddr - 0x200000), PHYS(vaddr));
	}
}

/*
 * Check if 1-byte regions mapped using kmap()
 * are really mapped, with right values
 */
static void vm_check_kmap1(void)
{
	uintptr_t paddr;
	void *vaddr;
	int count;

	count = PAGE_SIZE_2MB * 10;
	paddr = 0x100000000000;

	while (count--) {
		vaddr = vm_kmap(paddr, 1);
		assert(vaddr == VIRTUAL(paddr));

		if (!vaddr_is_mapped(vaddr))
			panic("_VM: Reporting supposedly mapped address 0x%lx "
			      "as unmapped", vaddr);

		if (is_aligned(paddr, 0x200000))
			printk("Success: phys addrs [0x%lx - 0x%lx] mapped\n",
			       paddr - 0x200000, paddr);

		paddr++;
	}
}

/*
 * Check if variable length regions mapped using
 * kmap() are really mapped, with right values
 */
static void vm_check_kmap2(void)
{
	uintptr_t paddr;
	void *vaddr;
	int count, len;

	count = PAGE_SIZE_2MB * 10;
	paddr = 0x200000000000;

	for (len = 1; len <= count; len += PAGE_SIZE_2MB/8) {

		/* To let the test be effective, assure we're
		 * mapping previously unmapped address */
		assert(!vaddr_is_mapped((void *)round_up((uintptr_t)
		       VIRTUAL(paddr), PAGE_SIZE_2MB)));

		vaddr = vm_kmap(paddr, len);
		assert(vaddr == VIRTUAL(paddr));

		for (int i = 0; i < len; i++) {
			if (!vaddr_is_mapped(vaddr))
				panic("_VM: Reporting supposedly mapped "
				      "address 0x%lx as unmapped", vaddr);
			vaddr++;
		}

		printk("Success: [region len=0x%lx] phys [0x%lx - 0x%lx] "
		       "mapped\n", len, paddr, paddr + len);

		paddr += len;
	}
}

/*
 * Test cases driver
 */
void vm_run_tests(void)
{
	vm_check_phys_memory();
	vm_check_kmap1();
	vm_check_kmap2();
}

#endif	/* VM_TESTS */
