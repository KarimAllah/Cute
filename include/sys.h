#ifndef _USER_H
#define _USER_H

#include <paging.h>

#define USER_START_ADDR (32*1024*1024)
#define USER_SIZE (PAGE_SIZE * 100)
#define USER_STACK_ADDR (64*1024*1024)

#endif
