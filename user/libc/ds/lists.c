#include <malloc.h>
#include <ds/lists.h>

struct stack *create_stack()
{
	struct stack *stack = (struct stack *)malloc(sizeof(struct stack));
	stack->top = 0;
	return stack;
}

void *stack_pop(struct stack *stack)
{
	void *tmp = stack->top->data;
	free(stack->top);
	stack->top = stack->top->next;
	return tmp;
}

void *stack_peek(struct stack *stack)
{
	return stack->top->data;
}

void stack_push(struct stack *stack, void *data)
{
	struct stack_node *tmp = malloc(sizeof(struct stack_node));
	tmp->data = data;
	tmp->next = stack->top;
	stack->top = tmp;
}

struct fifo *create_fifo()
{

	struct fifo *fifo = (struct fifo *)malloc(sizeof(struct fifo));
	fifo->head = 0;
	fifo->tail = 0;
	return fifo;
}

void fifo_enqueue(struct fifo *fifo, void *data)
{
	struct fifo_node *tmp = malloc(sizeof(struct fifo_node));
	tmp->data = data;

	if(fifo->tail)
		fifo->tail->next = tmp;

	fifo->tail = tmp;

	if(!fifo->head)
		fifo->head = tmp;
}

void *fifo_dequeue(struct fifo *fifo)
{
	void *tmp = fifo->head;
	if(fifo->head) {
		free(fifo->head);
		fifo->head = fifo->head->next;
	}
	return tmp;
}
