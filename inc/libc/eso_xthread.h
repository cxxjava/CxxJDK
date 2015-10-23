/**
 * @file  eso_xthread.h
 * @brief Simulation thread
 */

#ifndef __ESO_XTHREAD_H__
#define __ESO_XTHREAD_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Virtual Thread
 */

typedef struct es_xthread_t es_xthread_t;

/**
sample:

static void thread_proc(es_xthread_t* handle)
{
	while (1) {
		//do something.
		eso_xthread_suspend(handle);
	}
}

void OSRun(void) {
	es_xthread_t* handle;
	
	handle = eso_xthread_new(4096);
	if (!eso_xthread_is_execute(handle)) {
		eso_xthread_start(handle, thread_proc);
	}
	else {
		eso_xthread_resume(handle);
	}
	eso_xthread_die(handle);
}
*/

/**
 * Define xthread handle
 */
typedef void es_xthread_start_t(es_xthread_t* handle);

/**
 * create
 */
es_xthread_t* eso_xthread_new(es_uint32_t stack_size);

/**
 * destroy
 */
void eso_xthread_die(es_xthread_t* handle);

/**
 * start
 */
void eso_xthread_start(es_xthread_t* handle, es_xthread_start_t *callback);

/**
 * suspend
 */
void eso_xthread_suspend(es_xthread_t* handle);

/**
 * resume
 */
void eso_xthread_resume(es_xthread_t* handle);

/**
 * is execute
 */
es_bool_t eso_xthread_is_execute(es_xthread_t* handle);

/**
 * is suspend
 */
es_bool_t eso_xthread_is_suspend(es_xthread_t* handle);

/**
 * get stack size
 */
es_uint32_t eso_xthread_stack_size(es_xthread_t* handle);

#ifdef __cplusplus
}
#endif

#endif /* !__ESO_XTHREAD_H__ */

