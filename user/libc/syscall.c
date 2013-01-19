#include <syscall.h>

int syscall(int cmd, void *data)
{
	int result;
	asm ("movq %1, %%rax\n"
		 "movq %2, %%rbx\n"
		 "syscall\n"
		 :"=a"(result):"g"(cmd), "g" (data):);
	return result;
}
