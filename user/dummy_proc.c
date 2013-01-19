#include <syscall.h>

void dummy_work()
{
	unsigned int count;
	unsigned int index = 0;
	char *msg = "Karim is just testing your own code";
	while(1)
	{
		for(count = 0; count < 0xFFFFFF; count++)
			asm("");

		syscall(index++, (void *)msg);
		index = index % 10;
	}
}
void _start()
{
	dummy_work();
}
