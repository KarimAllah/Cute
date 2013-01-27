#ifndef __UAPI_SYSCALL_H
#define __UAPI_SYSCALL_H

#define SYSCALL_PRINTF 0
#define SYSCALL_VMMAP 1
#define SYSCALL_BINDER 2
#define SYSCALL_THREAD 3

enum thread_cmd {
	NEW_PROC,
	CLONE_PROC
};

struct thread_syscall {
	enum thread_cmd cmd;
	union {
		int flags;
		void *data;
	};
};

#endif
