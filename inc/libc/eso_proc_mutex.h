/**
 * @file  eso_proc_mutex.h
 * @brief ES Process Mutex Routines
 */

#ifndef __ESO_PROC_MUTEX_H__
#define __ESO_PROC_MUTEX_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Opaque structure representing a process mutex. */
typedef struct es_proc_mutex_t es_proc_mutex_t;


/*   Function definitions */

/**
 * Create and initialize a mutex that can be used to synchronize processes.
 * @return the memory address where the newly created mutex will be
 *        stored.
 */
es_proc_mutex_t* eso_proc_mutex_create(void);

/**
 * Acquire the lock for the given mutex. If the mutex is already locked,
 * the current thread will be put to sleep until the lock becomes available.
 * @param mutex the mutex on which to acquire the lock.
 */
es_status_t eso_proc_mutex_lock(es_proc_mutex_t *mutex);

/**
 * Attempt to acquire the lock for the given mutex. If the mutex has already
 * been acquired, the call returns immediately with EZ_EBUSY. Note: it
 * is important that the EZ_STATUS_IS_EBUSY(s) macro be used to determine
 * if the return value was EZ_EBUSY, for portability reasons.
 * @param mutex the mutex on which to attempt the lock acquiring.
 */
es_status_t eso_proc_mutex_trylock(es_proc_mutex_t *mutex);

/**
 * Release the lock for the given mutex.
 * @param mutex the mutex from which to release the lock.
 */
es_status_t eso_proc_mutex_unlock(es_proc_mutex_t *mutex);

/**
 * Destroy the mutex and free the memory associated with the lock.
 * @param mutex the mutex to destroy.
 */
void eso_proc_mutex_destroy(es_proc_mutex_t **mutex);


#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_PROC_MUTEX_H__ */

