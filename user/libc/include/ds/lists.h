#ifndef __LISTS_H
#define __LISTS_H

struct stack_node {
	void *data;
	struct stack_node *next;
};

struct stack {
	struct stack_node *top;
};

struct stack *create_stack();
void stack_push(struct stack *stack, void *data);
void *stack_peek(struct stack *stack);
void *stack_pop(struct stack *stack);

struct fifo_node {
	void *data;
	struct fifo_node *next;
};

struct fifo {
	struct fifo_node *tail;
	struct fifo_node *head;
};

struct fifo *create_fifo();
void fifo_enqueue(struct fifo *fifo, void *data);
void *fifo_dequeue(struct fifo *fifo);

#endif
