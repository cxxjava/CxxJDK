/**
 * @@file  eso_queue.c
 * @@brief ES Queue library
 */

#include "eso_queue.h"
#include "eso_ring.h"
#include "eso_libc.h"

typedef struct queue_elem_t queue_elem_t;

struct queue_elem_t {
    ES_RING_ENTRY(queue_elem_t) link;
    void *data;
};

ES_RING_HEAD(es_queue_t, queue_elem_t);

/** 
 * create a FIFO queue
 * @returns The new queue
 */
es_queue_t* eso_queue_create(void)
{
	es_queue_t *queue = NULL;
	
	queue = eso_calloc(sizeof(es_queue_t));
	ES_RING_INIT(queue, queue_elem_t, link);
	
	return queue;
}

/**
 * push a object to the queue
 *
 * @param queue the queue
 * @param data the data
 */
void eso_queue_push(es_queue_t *queue, const void *data)
{
	queue_elem_t *elem;
	
	elem = (queue_elem_t *)eso_malloc(sizeof(queue_elem_t));
    ES_RING_ELEM_INIT(elem, link);
    elem->data = (void *)data;
    ES_RING_INSERT_TAIL(queue, elem, queue_elem_t, link);
}

/**
 * pop an object from the queue
 *
 * @param queue the queue
 * @returns the data
 */
void* eso_queue_pop(es_queue_t *queue)
{
	queue_elem_t *ep;
	void *data = NULL;
	
	if (ES_RING_EMPTY(queue, queue_elem_t, link))
		return NULL;
	
	ep = ES_RING_FIRST(queue);
	if (ep != ES_RING_SENTINEL(queue, queue_elem_t, link)) {
		ES_RING_REMOVE(ep, link);
		data = (void *)ep->data;
    	eso_free(ep);
    }
    
    return data;
}

/**
 * pop an object from the queue
 *
 * @param queue the queue
 * @returns the data
 */
void* eso_queue_pop_this(es_queue_t *queue, es_int32_t index)
{
	queue_elem_t *ep;
	void *data = NULL;
	es_int32_t n = 0;
	
	if (ES_RING_EMPTY(queue, queue_elem_t, link) || index < 0)
		return NULL;
	
	ep = ES_RING_FIRST(queue);
	while (ep != ES_RING_SENTINEL(queue, queue_elem_t, link)) {
		if (index == n) {
			ES_RING_REMOVE(ep, link);
			data = (void *)ep->data;
    		eso_free(ep);
            return data;
    	}
    	n++;
		ep = ES_RING_NEXT(ep, link);
    }
    
    return NULL;
}

/**
 * get the first object from the queue
 *
 * @param queue the queue
 * @returns the data
 */
void* eso_queue_trypop(es_queue_t *queue)
{
	queue_elem_t *ep;
	void *data = NULL;
	
	if (ES_RING_EMPTY(queue, queue_elem_t, link))
		return NULL;
	
	ep = ES_RING_FIRST(queue);
	if (ep) {
		data = (void *)ep->data;
	}
    
    return data;
}

/**
 * get an object from the queue
 *
 * @param queue the queue
 * @param index the index of data
 * @returns the data
 */
void* eso_queue_get(es_queue_t *queue, es_int32_t index)
{
	queue_elem_t *ep;
	es_int32_t n = 0;
	
	if (ES_RING_EMPTY(queue, queue_elem_t, link) || index < 0)
		return NULL;
	
	ep = ES_RING_FIRST(queue);
    while (ep != ES_RING_SENTINEL(queue,
                                 queue_elem_t, link)) {
		if (index == n) {
			return (void *)ep->data;
		}
		n++;
		ep = ES_RING_NEXT(ep, link);
	}
    
    return NULL;
}

/**
 * returns the size of the queue.
 *
 * @param queue the queue
 * @returns the size of the queue
 */
es_int32_t eso_queue_size(es_queue_t *queue)
{
	es_int32_t n = 0;
	queue_elem_t *ep;
	
	ep = ES_RING_FIRST(queue);
    while (ep != ES_RING_SENTINEL(queue,
                                 queue_elem_t, link)) {
		n++;
		ep = ES_RING_NEXT(ep, link);
	}
	
	return n;
}

/**
 * Destroy a queue
 * @param t The queue to destroy
 */
void eso_queue_destroy(es_queue_t **queue)
{
	queue_elem_t *ep, *eptmp;
	
	if (!queue || !(*queue))
		return;
	
	if (!ES_RING_EMPTY(*queue, queue_elem_t, link)) {
        ep = ES_RING_FIRST(*queue);
        while (ep != ES_RING_SENTINEL(*queue,
                                     queue_elem_t, link)) {
			eptmp = ES_RING_NEXT(ep, link);
			eso_free(ep);
			ep = eptmp;
		}
    }
	
	ESO_FREE(queue);
}
