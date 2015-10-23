/**
 * @file  eso_thread_spin.h
 * @brief ES Thread Spin Lock Routines
 */

#ifndef __ESO_THREAD_SPIN_H__
#define __ESO_THREAD_SPIN_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Opaque read-write thread-safe lock. */
typedef struct es_thread_spin_t es_thread_spin_t;

/**
 * Create and initialize a spin lock that can be used to synchronize
 * threads.
 */
es_thread_spin_t* eso_thread_spin_create(void);

/**
 * Acquire the lock for the given spin. If the spin is already locked,
 * the current thread will be put to cycle until the lock becomes available.
 * @param spin the spin on which to acquire the lock.
 */
es_status_t eso_thread_spin_lock(es_thread_spin_t *spin);

/**
 * Attempt to acquire the lock for the given spin. If the spin has already
 * been acquired, the call returns immediately with ES_EBUSY.
 * @param spin the spin on which to attempt the lock acquiring.
 */
es_status_t eso_thread_spin_trylock(es_thread_spin_t *spin);

/**
 * Attempt to acquire the lock for the given spin. If the spin has already
 * been acquired, the call while retry lock until timeout.
 * @param spin the spin on which to attempt the lock acquiring.
 * @param timeout the amount of time in milliseconds to wait.
 */
es_status_t eso_thread_spin_timedlock(es_thread_spin_t *spin, es_uint32_t timeout);

/**
 * Release the lock for the given spin.
 * @param spin the spin from which to release the lock.
 */
es_status_t eso_thread_spin_unlock(es_thread_spin_t *spin);

/**
 * Destroy the spin and free the memory associated with the lock.
 * @param spin the spin to destroy.
 */
void eso_thread_spin_destroy(es_thread_spin_t **spin);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_THREAD_SPIN_H__ */
