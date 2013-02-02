#include <mm.h>
#include <kernel.h>
#include <paging.h>
#include <percpu.h>

#define ERRCODE_RW 0x2
#define ERRCODE_US 0x4

#define ALIGN(value, align) ((value) & ~((align) - 1))

void __page_fault(uint32_t err_code) {
	uint64_t fault_addr = get_cr2();

	printk("Page fault at addr : %llx. During '%s' by '%s'\n", fault_addr, err_code & ERRCODE_RW ? "write" : "read",
			err_code & ERRCODE_US ? "user" : "supervisor");

	fault_addr = ALIGN(fault_addr, PAGE_SIZE);
	struct page *page = get_zeroed_page(ZONE_ANY);
	memset64(page_address(page), 123456, PAGE_SIZE);
	map_range_user(current, fault_addr, PAGE_SIZE, page_phys_addr(page));
}
