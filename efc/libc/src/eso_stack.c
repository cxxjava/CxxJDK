/**
 * @@file  eso_stack.c
 * @@brief ES Stack library
 */

#include "eso_stack.h"
#include "eso_ring.h"
#include "eso_libc.h"

typedef struct stack_elem_t stack_elem_t;

struct stack_elem_t {
    ES_RING_ENTRY(stack_elem_t) link;
    void *data;
};

ES_RING_HEAD(es_stack_t, stack_elem_t);

/** 
 * create a LIFO stack
 * @returns The new stack
 */
es_stack_t* eso_stack_create(void)
{
	es_stack_t *stack = NULL;
	
	stack = eso_calloc(sizeof(es_stack_t));
	ES_RING_INIT(stack, stack_elem_t, link);
	
	return stack;
}

/**
 * push/add a object to the stack
 *
 * @param stack the stack
 * @param data the data
 */
void eso_stack_push(es_stack_t *stack, const void *data)
{
	stack_elem_t *elem;
	
	elem = (stack_elem_t *)eso_malloc(sizeof(stack_elem_t));
    ES_RING_ELEM_INIT(elem, link);
    elem->data = (void *)data;
    ES_RING_INSERT_TAIL(stack, elem, stack_elem_t, link);
}

/**
 * pop/get an object from the stack
 *
 * @param stack the stack
 * @returns the data
 */
void* eso_stack_pop(es_stack_t *stack)
{
	stack_elem_t *ep;
	void *data = NULL;
	
	if (ES_RING_EMPTY(stack, stack_elem_t, link))
		return NULL;
	
	ep = ES_RING_LAST(stack);
	if (ep != ES_RING_SENTINEL(stack, stack_elem_t, link)) {
		ES_RING_REMOVE(ep, link);
		data = (void *)ep->data;
    	eso_free(ep);
    }
    
    return data;
}

/**
 * pop/get an object from the stack
 *
 * @param stack the stack
 * @returns the data
 */
void* eso_stack_pop_this(es_stack_t *stack, es_int32_t index)
{
	stack_elem_t *ep;
	void *data = NULL;
	es_int32_t n = 0;
	
	if (ES_RING_EMPTY(stack, stack_elem_t, link) || index < 0)
		return NULL;
	
	ep = ES_RING_FIRST(stack);
    while (ep != ES_RING_SENTINEL(stack,
                                 stack_elem_t, link)) {
		if (index == n) {
			ES_RING_REMOVE(ep, link);
			data = (void *)ep->data;
			eso_free(ep);
			break;
		}
		n++;
		ep = ES_RING_NEXT(ep, link);
	}
	
	return data;
}

/**
 * get the last object from the stack
 *
 * @param stack the stack
 * @returns the data
 */
void* eso_stack_trypop(es_stack_t *stack)
{
	stack_elem_t *ep;
	void *data = NULL;
	
	if (ES_RING_EMPTY(stack, stack_elem_t, link))
		return NULL;
	
	ep = ES_RING_LAST(stack);
	if (ep) {
		data = (void *)ep->data;
	}
    
    return data;
}

/**
 * get an object from the stack
 *
 * @param stack the stack
 * @param index the index of data
 * @returns the data
 */
void* eso_stack_get(es_stack_t *stack, es_int32_t index)
{
	stack_elem_t *ep;
	es_int32_t n = 0;
	
	if (ES_RING_EMPTY(stack, stack_elem_t, link) || index < 0)
		return NULL;
	
	ep = ES_RING_FIRST(stack);
    while (ep != ES_RING_SENTINEL(stack,
                                 stack_elem_t, link)) {
		if (index == n) {
			return (void *)ep->data;
		}
		n++;
		ep = ES_RING_NEXT(ep, link);
	}
    
    return NULL;
}

/**
 * returns the size of the stack.
 *
 * @param stack the stack
 * @returns the size of the stack
 */
es_int32_t eso_stack_size(es_stack_t *stack)
{
	es_int32_t n = 0;
	stack_elem_t *ep;
	
	ep = ES_RING_FIRST(stack);
    while (ep != ES_RING_SENTINEL(stack,
                                 stack_elem_t, link)) {
		n++;
		ep = ES_RING_NEXT(ep, link);
	}
	
	return n;
}

/**
 * Destroy a stack
 * @param t The stack to destroy
 */
void eso_stack_destroy(es_stack_t **stack)
{
	stack_elem_t *ep, *eptmp;
	
	if (!stack || !(*stack))
		return;
	
	if (!ES_RING_EMPTY(*stack, stack_elem_t, link)) {
        ep = ES_RING_FIRST(*stack);
        while (ep != ES_RING_SENTINEL(*stack,
                                     stack_elem_t, link)) {
			eptmp = ES_RING_NEXT(ep, link);
			eso_free(ep);
			ep = eptmp;
		}
    }
	
	ESO_FREE(stack);
}
