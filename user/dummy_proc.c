#include <exit.h>
#include <stdio.h>
#include <binder.h>
#include <syscall.h>
#include <cute/binder.h>

void _start();
void dummy_work();

void _start()
{
	dummy_work();
}

void dummy_work()
{
	printf("Teeeeting : %d\n", *(unsigned long *)(0x0)); // Creating a page fault

	uint64_t supervisor_handle = register_supervisor();
	uint64_t object_handle = register_object("NEW_OBJECT", supervisor_handle);
	create_thread(NEW_PROC, "looper");
	create_thread(NEW_PROC, "vga_worker");
	create_thread(NEW_PROC, "algorithms");

	struct binder_consume_request *request = consume_request();
	printf("The request is from : %d\n", request->target);
	object_reply(request->target, (void *)0xffffffff);
	exit_app();
}
