/**
 * @@file  eso_queue.h
 * @@brief ES Queue library
 */


#ifndef __ESO_QUEUE_H__
#define __ESO_QUEUE_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * @defgroup ES_FIFO Thread Safe FIFO bounded stack
 * @ingroup ES
 * @{
 */

/**
 * opaque structure
 */
typedef struct es_queue_t es_queue_t;


/** 
 * create a LIFO stack
 * @returns The new stack
 */
es_queue_t* eso_queue_create(void);

/**
 * push a object to the queue
 *
 * @param queue the queue
 * @param data the data
 */
void eso_queue_push(es_queue_t *queue, const void *data);

/**
 * pop an object from the queue
 *
 * @param queue the queue
 * @returns the data
 */
void* eso_queue_pop(es_queue_t *queue);

/**
 * pop an object from the queue
 *
 * @param queue the queue
 * @returns the data
 */
void* eso_queue_pop_this(es_queue_t *queue, es_int32_t index);

/**
 * get the first object from the queue
 *
 * @param queue the queue
 * @returns the data
 */
void* eso_queue_trypop(es_queue_t *queue);

/**
 * get an object from the queue
 *
 * @param queue the queue
 * @param index the index of data
 * @returns the data
 */
void* eso_queue_get(es_queue_t *queue, es_int32_t index);

/**
 * returns the size of the queue.
 *
 * @param queue the queue
 * @returns the size of the queue
 */
es_int32_t eso_queue_size(es_queue_t *queue);

/**
 * Destroy a queue
 * @param t The queue to destroy
 */
void eso_queue_destroy(es_queue_t **queue);

/** @} */
#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_QUEUE_H__ */
