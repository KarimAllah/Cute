#include <stdint.h>
#include <init_array.h>

extern char __stage1_start[];
extern char __stage1_end[];

extern char __stage2_start[];
extern char __stage2_end[];

extern char __stage3_start[];
extern char __stage3_end[];

extern char __stage4_start[];
extern char __stage4_end[];

#define iterate_stage(stage) \
	for(init_fn *CURRENT = (init_fn *)__stage##stage##_start, *end = (init_fn *)__stage##stage##_end; CURRENT < end; CURRENT++)

void init_arrays()
{
	iterate_stage(1) (*CURRENT)();
	iterate_stage(2) (*CURRENT)();
	iterate_stage(3) (*CURRENT)();
	iterate_stage(4) (*CURRENT)();
}
