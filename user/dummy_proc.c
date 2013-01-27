#include <stdio.h>
#include <syscall.h>
#include <cute/binder.h>
#include <binder.h>

void _start();
void dummy_work();
void another_dummy_work();

void _start()
{
	dummy_work();
}

void dummy_work()
{
	uint64_t supervisor_handle = register_supervisor();
	uint64_t object_handle = register_object("NEW_OBJECT", supervisor_handle);
	create_thread(NEW_PROC, "looper");

	struct binder_consume_request *request = consume_request();
	printf("The request is from : %d\n", request->target);
	object_reply(request->target, (void *)0xffffffff);
	while(1){}
}
