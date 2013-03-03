#ifndef __INIT_ARRAY_H
#define __INIT_ARRAY_H

void init_arrays(void);

typedef void (*init_fn)(void);

#define REGISTER_STAGE1_INIT(fn) init_fn *fn##__init__1 __attribute__ ((section (".init.array.stage1"))) = &fn
#define REGISTER_STAGE2_INIT(fn) init_fn *fn##__init__2 __attribute__ ((section (".init.array.stage2"))) = &fn
#define REGISTER_STAGE3_INIT(fn) init_fn *fn##__init__3 __attribute__ ((section (".init.array.stage3"))) = &fn
#define REGISTER_STAGE4_INIT(fn) init_fn *fn##__init__4 __attribute__ ((section (".init.array.stage4"))) = &fn

#endif
