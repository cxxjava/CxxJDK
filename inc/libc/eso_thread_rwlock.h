/**
 * @file  eso_thread_rwlock.h
 * @brief ES Reader/Writer Lock Routines
 */

#ifndef __ESO_THREAD_RWLOCK_H__
#define __ESO_THREAD_RWLOCK_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Opaque read-write thread-safe lock. */
typedef struct es_thread_rwlock_t es_thread_rwlock_t;

/**
 * Note: The following operations have undefined results: unlocking a
 * read-write lock which is not locked in the calling thread; write
 * locking a read-write lock which is already locked by the calling
 * thread; destroying a read-write lock more than once; clearing or
 * destroying the pool from which a <b>locked</b> read-write lock is
 * allocated.
 */

/**
 * Create and initialize a read-write lock that can be used to synchronize
 * threads.
 */
es_thread_rwlock_t* eso_thread_rwlock_create(void);

/**
 * Acquire a shared-read lock on the given read-write lock. This will allow
 * multiple threads to enter the same critical section while they have acquired
 * the read lock.
 * @param rwlock the read-write lock on which to acquire the shared read.
 */
es_status_t eso_thread_rwlock_rdlock(es_thread_rwlock_t *rwlock);

/**
 * Attempt to acquire the shared-read lock on the given read-write lock. This
 * is the same as eso_thread_rwlock_rdlock(), only that the function fails
 * if there is another thread holding the write lock, or if there are any
 * write threads blocking on the lock. If the function fails for this case,
 * EZ_EBUSY will be returned. Note: it is important that the
 * EZ_STATUS_IS_EBUSY(s) macro be used to determine if the return value was
 * EZ_EBUSY, for portability reasons.
 * @param rwlock the rwlock on which to attempt the shared read.
 */
es_status_t eso_thread_rwlock_tryrdlock(es_thread_rwlock_t *rwlock);

/**
 * Acquire an exclusive-write lock on the given read-write lock. This will
 * allow only one single thread to enter the critical sections. If there
 * are any threads currently holding the read-lock, this thread is put to
 * sleep until it can have exclusive access to the lock.
 * @param rwlock the read-write lock on which to acquire the exclusive write.
 */
es_status_t eso_thread_rwlock_wrlock(es_thread_rwlock_t *rwlock);

/**
 * Attempt to acquire the exclusive-write lock on the given read-write lock. 
 * This is the same as eso_thread_rwlock_wrlock(), only that the function fails
 * if there is any other thread holding the lock (for reading or writing),
 * in which case the function will return EZ_EBUSY. Note: it is important
 * that the EZ_STATUS_IS_EBUSY(s) macro be used to determine if the return
 * value was EZ_EBUSY, for portability reasons.
 * @param rwlock the rwlock on which to attempt the exclusive write.
 */
es_status_t eso_thread_rwlock_trywrlock(es_thread_rwlock_t *rwlock);

/**
 * Release either the read or write lock currently held by the calling thread
 * associated with the given read-write lock.
 * @param rwlock the read-write lock to be released (unlocked).
 */
es_status_t eso_thread_rwlock_unlock(es_thread_rwlock_t *rwlock);

/**
 * Destroy the read-write lock and free the associated memory.
 * @param rwlock the rwlock to destroy.
 */
void eso_thread_rwlock_destroy(es_thread_rwlock_t **rwlock);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_THREAD_RWLOCK_H__ */
