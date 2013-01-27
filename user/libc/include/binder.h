#ifndef __BINDER_H
#define __BINDER_H

#include <stdint.h>
#include <cute/binder.h>
#include <cute/syscall.h>

void create_thread(enum thread_cmd cmd, char *path);
uint64_t register_supervisor(void);
uint64_t register_object(char *name, uint64_t supervisor);
uint64_t get_object(char *name);
void *object_request(uint64_t handle, void *request);
void object_reply(uint64_t target, void *reply);
struct binder_consume_request *consume_request();

#endif
