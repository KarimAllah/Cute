#include <wait.h>
#include <kernel.h>
#include <atomic.h>

void wait_init(struct wait_event *event)
{
	event->val = 0;
	barrier();
}

void wait_on(struct wait_event *event)
{
	while(event->val == 0)
			cpu_pause();
}

void event_set(struct wait_event *event)
{
	event->val = 1;
}

void event_clear(struct wait_event *event)
{
	event->val = 0;
}
