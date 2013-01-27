#ifndef __UAPI_BINDER_H
#define __UAPI_BINDER_H

#include <stdint.h>

enum binder_cmd {
	/* Commands */
	bcREGISTER_SUPERVISOR,
	bcREGISTER_OBJECT,
	bcGET_OBJECT,
	bcCONSUME_REQUEST,
	bcREQUEST,
	bcREPLY,

	/* Reference counting */
    bcINCREFS,
    bcDECREFS,
    bcACQUIRE,
    bcRELEASE
};

struct binder_register_supervisor {
	/* OUT */
	uint64_t handle;
};

struct binder_register_object {
	/* IN */
	char *name;
	uint64_t supervisor;

	/* OUT */
	uint64_t handle;
};

struct binder_get_object {
	/* IN */
	char *name;

	/* OUT */
	uint64_t handle;
};

struct binder_request {
	/* IN */
	uint64_t target;
	void *request;

	/* OUT */
	void *reply;
};

struct binder_consume_request {
	/* OUT */
	void *request;
	uint64_t target;
};

struct binder_reply {
	/* IN */
	uint64_t target;
	void *data;
};

struct binder_inc {

};

struct binder_dec {

};

struct binder_acquire {

};

struct binder_release {

};

struct binder_operation {
	enum binder_cmd type;
	void *data;
};

struct binder_transaction {
	int num_ops;
	struct binder_operation *ops;
};

#endif
