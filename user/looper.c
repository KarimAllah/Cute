#include <stdio.h>
#include <syscall.h>
#include <binder.h>

void _start()
{
	printf("Starting the new process\n");
	uint64_t handle = get_object("NEW_OBJECT");
	printf("The returned handle : %llx\n", handle);
	object_request(handle, (void *)0x10000);
	printf("Completed our request\n");
	while(1){}
}
