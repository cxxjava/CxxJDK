/**
 * @@file  eso_stack.h
 * @@brief ES Stack library
 */


#ifndef __ESO_STACK_H__
#define __ESO_STACK_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * @defgroup ES_LIFO Thread Safe LIFO bounded stack
 * @ingroup ES
 * @{
 */

/**
 * opaque structure
 */
typedef struct es_stack_t es_stack_t;


/** 
 * create a LIFO stack
 * @returns The new stack
 */
es_stack_t* eso_stack_create(void);

/**
 * push/add a object to the stack
 *
 * @param stack the stack
 * @param data the data
 */
void eso_stack_push(es_stack_t *stack, const void *data);

/**
 * pop/get an object from the stack
 *
 * @param stack the stack
 * @returns the data
 */
void* eso_stack_pop(es_stack_t *stack);

/**
 * pop/get an object from the stack
 *
 * @param stack the stack
 * @returns the data
 */
void* eso_stack_pop_this(es_stack_t *stack, es_int32_t index);

/**
 * get the last object from the stack
 *
 * @param stack the stack
 * @returns the data
 */
void* eso_stack_trypop(es_stack_t *stack);

/**
 * get an object from the stack
 *
 * @param stack the stack
 * @param index the index of data
 * @returns the data
 */
void* eso_stack_get(es_stack_t *stack, es_int32_t index);

/**
 * returns the size of the stack.
 *
 * @param stack the stack
 * @returns the size of the stack
 */
es_int32_t eso_stack_size(es_stack_t *stack);

/**
 * Destroy a stack
 * @param t The stack to destroy
 */
void eso_stack_destroy(es_stack_t **stack);

/** @} */
#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_STACK_H__ */
