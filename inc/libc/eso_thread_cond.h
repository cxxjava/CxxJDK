/**
 * @file  eso_thread_cond.h
 * @brief ES Condition Variable Routines
 */

#ifndef ESO_THREAD_COND_H
#define ESO_THREAD_COND_H

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"
#include "eso_thread_mutex.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Opaque structure for thread condition variables */
typedef struct es_thread_cond_t es_thread_cond_t;

/**
 * Note: destroying a condition variable (or likewise, destroying or
 * clearing the pool from which a condition variable was allocated) if
 * any threads are blocked waiting on it gives undefined results.
 */

/**
 * Create and initialize a condition variable that can be used to signal
 * and schedule threads in a single process.
 */
es_thread_cond_t* eso_thread_cond_create(void);

/**
 * Put the active calling thread to sleep until signaled to wake up. Each
 * condition variable must be associated with a mutex, and that mutex must
 * be locked before  calling this function, or the behavior will be
 * undefined. As the calling thread is put to sleep, the given mutex
 * will be simultaneously released; and as this thread wakes up the lock
 * is again simultaneously acquired.
 * @param cond the condition variable on which to block.
 * @param mutex the mutex that must be locked upon entering this function,
 *        is released while the thread is asleep, and is again acquired before
 *        returning from this function.
 */
es_status_t eso_thread_cond_wait(es_thread_cond_t *cond,
                                 es_thread_mutex_t *mutex);

/**
 * Put the active calling thread to sleep until signaled to wake up or
 * the timeout is reached. Each condition variable must be associated
 * with a mutex, and that mutex must be locked before calling this
 * function, or the behavior will be undefined. As the calling thread
 * is put to sleep, the given mutex will be simultaneously released;
 * and as this thread wakes up the lock is again simultaneously acquired.
 * @param cond the condition variable on which to block.
 * @param mutex the mutex that must be locked upon entering this function,
 *        is released while the thread is asleep, and is again acquired before
 *        returning from this function.
 * @param nanosTimeout the maximum time to wait, in nanoseconds.
 *        If the condition is signaled, we will wake up before this time,
 *        otherwise the error COND_TIMEUP is returned.
 */
es_status_t eso_thread_cond_timedwait(es_thread_cond_t *cond,
                                      es_thread_mutex_t *mutex,
                                      es_int64_t nanos);

/**
 * Signals a single thread, if one exists, that is blocking on the given
 * condition variable. That thread is then scheduled to wake up and acquire
 * the associated mutex. Although it is not required, if predictable scheduling
 * is desired, that mutex must be locked while calling this function.
 * @param cond the condition variable on which to produce the signal.
 */
es_status_t eso_thread_cond_signal(es_thread_cond_t *cond);

/**
 * Signals all threads blocking on the given condition variable.
 * Each thread that was signaled is then scheduled to wake up and acquire
 * the associated mutex. This will happen in a serialized manner.
 * @param cond the condition variable on which to produce the broadcast.
 */
es_status_t eso_thread_cond_broadcast(es_thread_cond_t *cond);

/**
 * Destroy the condition variable and free the associated memory.
 * @param cond the condition variable to destroy.
 */
void eso_thread_cond_destroy(es_thread_cond_t **cond);


#ifdef __cplusplus
}
#endif

#endif  /* ! ESO_THREAD_COND_H */
