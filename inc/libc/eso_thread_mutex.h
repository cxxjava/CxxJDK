/**
 * @file  eso_thread_mutex.h
 * @brief ES Thread Mutex Routines
 */

#ifndef __ESO_THREAD_MUTEX_H__
#define __ESO_THREAD_MUTEX_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Opaque thread-local mutex structure */
typedef struct es_thread_mutex_t es_thread_mutex_t;

#define ES_THREAD_MUTEX_DEFAULT  0x0   /**< platform-optimal lock behavior */
#define ES_THREAD_MUTEX_NESTED   0x1   /**< enable nested (recursive) locks */
#define ES_THREAD_MUTEX_UNNESTED 0x2   /**< disable nested locks */

/**
 * Create and initialize a mutex that can be used to synchronize threads.
 * @param mutex the memory address where the newly created mutex will be
 *        stored.
 * @param flags Or'ed value of:
 * <PRE>
 *           ES_THREAD_MUTEX_DEFAULT   platform-optimal lock behavior.
 *           ES_THREAD_MUTEX_NESTED    enable nested (recursive) locks.
 *           ES_THREAD_MUTEX_UNNESTED  disable nested locks (non-recursive).
 * </PRE>
 * @param pool the pool from which to allocate the mutex.
 * @warning Be cautious in using ES_THREAD_MUTEX_DEFAULT.  While this is the
 * most optimial mutex based on a given platform's performance charateristics,
 * it will behave as either a nested or an unnested lock.
 */
es_thread_mutex_t* eso_thread_mutex_create(unsigned int flags);

/**
 * Acquire the lock for the given mutex. If the mutex is already locked,
 * the current thread will be put to sleep until the lock becomes available.
 * @param mutex the mutex on which to acquire the lock.
 */
es_status_t eso_thread_mutex_lock(es_thread_mutex_t *mutex);

/**
 * Attempt to acquire the lock for the given mutex. If the mutex has already
 * been acquired, the call returns immediately with ES_EBUSY.
 * @param mutex the mutex on which to attempt the lock acquiring.
 */
es_status_t eso_thread_mutex_trylock(es_thread_mutex_t *mutex);

/**
 * Attempt to acquire the lock for the given mutex. If the mutex has already
 * been acquired, the call while retry lock until timeout.
 * @param mutex the mutex on which to attempt the lock acquiring.
 * @param timeout the amount of time in milliseconds to wait.
 */
es_status_t eso_thread_mutex_timedlock(es_thread_mutex_t *mutex, es_uint32_t timeout);

/**
 * Release the lock for the given mutex.
 * @param mutex the mutex from which to release the lock.
 */
es_status_t eso_thread_mutex_unlock(es_thread_mutex_t *mutex);

/**
 * Destroy the mutex and free the memory associated with the lock.
 * @param mutex the mutex to destroy.
 */
void eso_thread_mutex_destroy(es_thread_mutex_t **mutex);


#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_THREAD_MUTEX_H__ */

