#ifndef _SYSCALL_H
#define _SYSCALL_H

#define STAR	0xC0000081
#define LSTAR	0xC0000082
#define CSTAR	0xC0000083
#define SFMASK	0xC0000084

extern void syscall(void);
extern void __syscall(void);
void init_syscall(void);

#endif
