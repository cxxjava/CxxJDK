/**
 * @file  eso_thread.h
 * @brief OS thread
 */

#ifndef __ESO_THREAD_H__
#define __ESO_THREAD_H__

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * OS Thread
 */

/** Opaque Thread structure. */
typedef struct es_thread_t     es_thread_t;

/** Opaque Thread attributes structure. */
typedef struct es_threadattr_t es_threadattr_t;

/**
 * The prototype for any ES thread worker functions.
 */
typedef void* es_thread_func_t(es_thread_t* handle);

struct es_thread_t {
	#ifdef WIN32
	HANDLE td;
	#else
    pthread_t td;
    #endif
    void *data;
    es_thread_func_t *func;
    es_status_t exitval;
};

struct es_threadattr_t {
	#ifdef WIN32
	es_bool_t detach;
    es_size_t stacksize;
    #else
    pthread_attr_t attr;
    #endif
};

#ifdef WIN32
typedef HANDLE                es_os_thread_t;      /**< native thread */
#else
typedef pthread_t             es_os_thread_t;      /**< native thread */
#endif

/**
 * Create and initialize a new threadattr variable
 */
es_status_t eso_threadattr_init(es_threadattr_t *attr);

/**
 * Destroy a thread attr and make it can't use before next inited.
 */
void eso_threadattr_destroy(es_threadattr_t *attr);

/**
 * Set if newly created threads should be created in detached state.
 */
es_status_t eso_threadattr_detach_set(es_threadattr_t *attr, es_bool_t on);

/**
 * Get the detach state for this threadattr.
 */
es_bool_t eso_threadattr_detach_get(es_threadattr_t *attr);

/**
 * Set the stack size attribute of the thread attributes object.
 */
es_status_t eso_threadattr_stacksize_set(es_threadattr_t *attr, es_size_t stacksize);

/**
 * Set the guard size attribute of the thread attributes object.
 */
es_status_t eso_threadattr_guardsize_set(es_threadattr_t *attr, es_size_t guardsize);

/**
 * Create a new thread.
 */
es_thread_t* eso_thread_create(es_threadattr_t *attr,
                               es_thread_func_t *func, 
                               void *data);

/**
 * stop the current thread
 * @param thd The thread to stop
 * @param retval The return value to pass back to any thread that cares
 */
es_status_t eso_thread_exit(es_thread_t *thd, 
                            es_status_t retval);

/**
 * block until the desired thread stops executing.
 * @param retval The return value from the dead thread.
 * @param thd The thread to join
 */
es_status_t eso_thread_join(es_status_t *retval, 
                            es_thread_t *thd);

/**
 * detach a thread
 * @param thd The thread to detach 
 */
es_status_t eso_thread_detach(es_thread_t *thd);

/**
 * force the current thread to yield the processor
 */
void eso_thread_yield(void);

/**
 * Destroy a thread.
 */
void eso_thread_destroy(es_thread_t **thd);

/**
 * Run the specified function one time, regardless of how many threads
 * call it.
 * @param func The function to call.
 */
es_status_t eso_thread_once(void (*func)(void));

/**
 * thread sleep
 */
void eso_thread_sleep(long milliseconds);

/**
 * native os thread handler
 */
es_os_thread_t eso_os_thread_current(void);
es_os_thread_t eso_os_thread_get(es_thread_t *thethd);
es_bool_t eso_os_thread_equal(es_os_thread_t tid1, es_os_thread_t tid2);

/**
 *
 */
int eso_os_thread_sigaction_init(void(*handler)(int sig));

/**
 * thread kill
 */
void eso_os_thread_kill(es_os_thread_t thd);

/**
 * native os thread id
 */
es_ulong_t eso_os_thread_current_id(void);

/**
 * get/set os thread priority
 */
es_status_t eso_os_thread_priority_get(es_ulong_t tid, int *ppriority);
es_status_t eso_os_thread_priority_set(es_ulong_t tid, int priority);

#ifdef __cplusplus
}
#endif

#endif /* !__ESO_THREAD_H__ */

