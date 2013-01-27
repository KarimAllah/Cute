#ifndef __BINDER_H
#define __BINDER_H

#include <list.h>
#include <wait.h>
#include <uapi/cute/binder.h>

struct binder_object {
	char *name;
	struct proc *supervisor;
	/* List of objects attached to the same proc */
	struct list_node node;

	/* List of objects registered with the system */
	struct list_node binder_node;
};

struct binder_work {
	uint64_t target;
	struct list_node node;
	struct wait_event event;
	void *work;
};

int binder_syscall(void *data);
int binder_init(void);

#endif
