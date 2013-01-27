#ifndef _WAIT_H
#define _WAIT_H

#include <stdint.h>

struct wait_event {
	uint32_t val;
};

void wait_init(struct wait_event *event);
void wait_on(struct wait_event *event);
void event_set(struct wait_event *event);
void event_clear(struct wait_event *event);

#endif
