#include <exit.h>
#include <stdio.h>
#include <binder.h>
#include <syscall.h>

void _start()
{
	printf("Starting the new process\n");
	uint64_t handle = get_object("NEW_OBJECT");
	printf("The returned handle : %llx\n", handle);
	object_request(handle, (void *)0x10000);
	printf("Completed our request\n");
	exit_app();
}
