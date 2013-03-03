#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <uapi/cute/syscall.h>

#define SYSCALL_NR 10
typedef int (*syscall_handler)(void *);

extern int do_generic_syscall(int cmd, void *data);
void syscall_init(void);

#endif
