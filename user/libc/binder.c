#include <binder.h>
#include <malloc.h>
#include <syscall.h>
#include <cute/binder.h>
#include <cute/syscall.h>

void create_thread(enum thread_cmd cmd, char *path)
{
	struct thread_syscall *tr = malloc(sizeof(struct thread_syscall));
	tr->cmd = cmd;
	tr->data = path;
	syscall(SYSCALL_THREAD, (void *)tr);
}

static int apply_transaction(struct binder_transaction *transaction)
{
	return syscall(SYSCALL_BINDER, (void *)transaction);
}

static struct binder_transaction *transaction_register_supervisor(void)
{
	struct binder_transaction *tr = malloc(sizeof(struct binder_transaction));
	struct binder_operation *op = malloc(sizeof(struct binder_operation));
	struct binder_register_supervisor *reg_sup = malloc(sizeof(struct binder_register_supervisor));

	tr->num_ops = 1;
	tr->ops = op;
	op->type = bcREGISTER_SUPERVISOR;
	op->data = reg_sup;
	return tr;
}

static struct binder_transaction *transaction_register_object(char *name, uint64_t supervisor)
{
	struct binder_transaction *tr = malloc(sizeof(struct binder_transaction));
	struct binder_operation *op = malloc(sizeof(struct binder_operation));
	struct binder_register_object *reg_obj = malloc(sizeof(struct binder_register_object));

	tr->num_ops = 1;
	tr->ops = op;
	reg_obj->name = name;
	reg_obj->supervisor = supervisor;
	op->type = bcREGISTER_OBJECT;
	op->data = reg_obj;
	return tr;
}

static struct binder_transaction *transaction_get_object(char *name)
{
	struct binder_transaction *tr = malloc(sizeof(struct binder_transaction));
	struct binder_operation *op = malloc(sizeof(struct binder_operation));
	struct binder_get_object *req = malloc(sizeof(struct binder_get_object));

	tr->num_ops = 1;
	tr->ops = op;
	op->type = bcGET_OBJECT;
	op->data = (void *)req;
	req->name = name;
	return tr;
}

static struct binder_transaction *transaction_object_request(uint64_t handle, void *request)
{
	struct binder_transaction *tr = malloc(sizeof(struct binder_transaction));
	struct binder_operation *op = malloc(sizeof(struct binder_operation));
	struct binder_request *req = malloc(sizeof(struct binder_request));

	tr->num_ops = 1;
	op->type = bcREQUEST;
	op->data = (void *)req;
	req->target = handle;
	req->request = request;
	tr->ops = (void *)op;
	return tr;
}

static struct binder_transaction *transaction_object_reply(uint64_t target, void *reply)
{
	struct binder_transaction *tr = malloc(sizeof(struct binder_transaction));
	struct binder_operation *op = malloc(sizeof(struct binder_operation));
	struct binder_reply *req = malloc(sizeof(struct binder_reply));

	tr->num_ops = 1;
	op->type = bcREPLY;
	op->data = (void *)req;
	req->target = target;
	req->data = reply;
	tr->ops = (void *)op;
	return tr;
}

static struct binder_transaction *transaction_consume_request()
{
	struct binder_transaction *tr = malloc(sizeof(struct binder_transaction));
	struct binder_operation *op = malloc(sizeof(struct binder_operation));
	struct binder_consume_request *req = malloc(sizeof(struct binder_consume_request));

	tr->num_ops = 1;
	op->type = bcCONSUME_REQUEST;
	op->data = (void *)req;
	tr->ops = (void *)op;
	return tr;
}

uint64_t register_supervisor(void)
{
	struct binder_transaction *tr = transaction_register_supervisor();
	apply_transaction(tr);
	return ((struct binder_register_supervisor *)tr->ops[0].data)->handle;
}

uint64_t register_object(char *name, uint64_t supervisor)
{
	struct binder_transaction *tr = transaction_register_object(name, supervisor);
	apply_transaction(tr);
	return ((struct binder_register_object *)tr->ops[0].data)->handle;
}

uint64_t get_object(char *name)
{
	struct binder_transaction *tr = transaction_get_object(name);
	apply_transaction(tr);
	return ((struct binder_get_object *)tr->ops[0].data)->handle;
}

void *object_request(uint64_t handle, void *request)
{
	struct binder_transaction *tr = transaction_object_request(handle, request);
	apply_transaction(tr);
	return ((struct binder_request *)tr->ops[0].data)->reply;
}

void object_reply(uint64_t target, void *reply)
{
	struct binder_transaction *tr = transaction_object_reply(target, reply);
	apply_transaction(tr);
}

struct binder_consume_request *consume_request()
{
	struct binder_transaction *tr = transaction_consume_request();
	apply_transaction(tr);
	return (struct binder_consume_request *)tr->ops[0].data;
}
