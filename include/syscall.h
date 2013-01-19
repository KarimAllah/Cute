#ifndef _SYSCALL_H
#define _SYSCALL_H

#define STAR	0xC0000081
#define LSTAR	0xC0000082
#define CSTAR	0xC0000083
#define SFMASK	0xC0000084

#define SYSCALL_NR 10
typedef int (*syscall_handler)(void *);

extern void syscall(void);
extern int __syscall(int cmd, void *data);
void init_syscall(void);

#endif
