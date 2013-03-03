#include <hash.h>
#include <percpu.h>
#include <binder.h>
#include <kmalloc.h>
#include <spinlock.h>
#include <init_array.h>

/*
 *
 * We've a producer and a consumer. Any thread can be a producer,
 * however only a thread who registered as a consumer can consume
 * messages.
 */

#define OBJECTS_SIZE 100
#define SUPERVISORS_SIZE 100

/* All binder objects registered in the system */
struct list_node binder_objects;

/* All supervisors registered in the system */
struct hash *binder_supervisors;

static void binder_register_supervisor_op(void *data)
{
	struct proc *supervisor = current;
	printk("Registering a supervisor thread : %llx\n", current);
	struct binder_register_supervisor *tr = (struct binder_register_supervisor *)data;
	uint64_t handle = hash_insert(binder_supervisors, supervisor);
	tr->handle = handle;
}

static int binder_register_object_op(void *data)
{
	struct binder_register_object *tr = (struct binder_register_object *)data;
	struct proc *supervisor = (struct proc *)hash_find(binder_supervisors, tr->supervisor);
	if(supervisor == NULL)
		/* Incorrect supervisor */
		return -1;

	struct binder_object *object = kmalloc(sizeof(struct binder_object));

	int len = strlen(tr->name);
	object->name = kmalloc(len);
	memcpy(object->name, tr->name, len + 1);
	object->supervisor = supervisor;

	/* add the object to the supervisor proc */
	list_add_tail(&object->supervisor->attached_objects, &object->node);

	/* Add the object to our system */
	list_add_tail(&binder_objects, &object->binder_node);
	uint64_t handle = (uint64_t)object;

	/* Return the object 'key' back to the user */
	tr->handle = handle;

	printk("Registered object ( %s at %llx ) for supervisor : %lx\n", tr->name, object, tr->supervisor);
	return 0;
}

static uint64_t find_object_by_name(char *name)
{
	struct binder_object *object;
	uint32_t size = strlen(name) - 1;
	list_for_each(&binder_objects, object, binder_node) {
		if(!strncmp(object->name, name, size))
			return (uint64_t)object;
	}
	return 0;
}

static struct binder_object *find_object_by_handle(uint64_t handle)
{
	return (struct binder_object *)handle;
}

static void binder_get_object_op(void *data)
{
	struct binder_get_object *tr = (struct binder_get_object *)data;
	tr->handle = find_object_by_name(tr->name);
}

static void binder_request_op(void *data)
{
	struct binder_request *tr = (struct binder_request *)data;
	struct binder_object *target_object = find_object_by_handle(tr->target);

	/* Add this work to the target thread */
	struct binder_work *bwork = kmalloc(sizeof(struct binder_work));
	wait_init(&bwork->event);
	bwork->work = tr->request;
	bwork->target = tr->target;
	list_add(&target_object->supervisor->todo, &bwork->node);
	printk("The target object is : %llx\n", target_object->supervisor);
	event_set(&target_object->supervisor->work);
	printk("Wait for a response from the target object\n");
	wait_on(&bwork->event);
	printk("Request completed\n");
}

static void binder_consumer_request_op(void *data)
{
	struct binder_consume_request *tr = (struct binder_consume_request *)data;
	struct proc *supervisor = current;
	if(list_empty(&supervisor->todo)) {
		wait_on(&supervisor->work);
	}

	struct binder_work *bwork, *spare;
	list_for_each_safe(&supervisor->todo, bwork, spare, node)
	{
		printk("Job completed, signaling producer\n");
		tr->request = bwork->work;
		tr->target = (uint64_t)&bwork->event;
		list_del(&bwork->node);
	}
}

static void binder_reply_op(void *data)
{
	struct binder_reply *tr = (struct binder_reply *)data;
	event_set(((struct wait_event *)tr->target));
}

static void binder_inc_dec_op(void *data, int inc)
{
}

static void binder_acq_rel_op(void *data, int acquire)
{
}

int binder_syscall(void *data)
{
	struct binder_transaction *tr = (struct binder_transaction *)data;
	for (int index = 0;index < tr->num_ops;index++)
	{
		printk("Binder cmd : %x, data : %lld\n", tr->ops[index].type, tr->ops[index].data);
		switch(tr->ops[index].type)
		{
		case bcREGISTER_SUPERVISOR:
			binder_register_supervisor_op(tr->ops[index].data);
			break;
		case bcREGISTER_OBJECT:
			binder_register_object_op(tr->ops[index].data);
			break;
		case bcGET_OBJECT:
			binder_get_object_op(tr->ops[index].data);
			break;
		case bcREQUEST:
			binder_request_op(tr->ops[index].data);
			break;
		case bcCONSUME_REQUEST:
			binder_consumer_request_op(tr->ops[index].data);
			break;
		case bcREPLY:
			binder_reply_op(tr->ops[index].data);
			break;
		case bcINCREFS:
		case bcDECREFS:
			binder_inc_dec_op(tr->ops[index].data, tr->ops[index].type == bcINCREFS);
			break;
		case bcACQUIRE:
		case bcRELEASE:
			binder_acq_rel_op(tr->ops[index].data, tr->ops[index].type == bcACQUIRE);
			break;
		}
	}
	return 0;
}

int binder_init()
{
	binder_supervisors = hash_new(SUPERVISORS_SIZE);
	list_init(&binder_objects);
	return 0;
}

REGISTER_STAGE1_INIT(binder_init);
