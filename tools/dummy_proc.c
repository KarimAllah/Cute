void dummy_work()
{
	unsigned int count;
	while(1)
	{
		for(count = 0; count < 0xFFFFFF; count++)
			asm("");
		asm("syscall");
	}
}
void _start()
{
	dummy_work();
}
