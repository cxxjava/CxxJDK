/**
 * @file  eso_thread_rwlock.c
 * @brief ES Reader/Writer Lock Routines
 */

#include "eso_thread_rwlock.h"
#include "eso_libc.h"

#ifdef WIN32

#include <windows.h>

struct es_thread_rwlock_t {
    HANDLE      write_mutex;
    HANDLE      read_event;
    LONG        readers;
};

es_thread_rwlock_t* eso_thread_rwlock_create(void)
{
	es_thread_rwlock_t *new_rwlock;

	new_rwlock = (es_thread_rwlock_t *)eso_calloc(sizeof(es_thread_rwlock_t));
	if (!new_rwlock) {
		return NULL;
	}

    if (! (new_rwlock->read_event = CreateEvent(NULL, TRUE, FALSE, NULL))) {
        eso_free(new_rwlock);
        return NULL;
    }

    if (! (new_rwlock->write_mutex = CreateMutex(NULL, FALSE, NULL))) {
        CloseHandle(new_rwlock->read_event);
        eso_free(new_rwlock);
        return NULL;
    }

	return new_rwlock;
}

static es_status_t _thread_rwlock_rdlock_core(es_thread_rwlock_t *rwlock,
                                              DWORD  milliseconds)
{
    DWORD code = WaitForSingleObject(rwlock->write_mutex, milliseconds);

    if (code == WAIT_FAILED)
        return code;
	
	if (code == WAIT_TIMEOUT)
		return ES_EBUSY;
	
    /* We've successfully acquired the writer mutex, we can't be locked
     * for write, so it's OK to add the reader lock.  The writer mutex
     * doubles as race condition protection for the readers counter.   
     */
    InterlockedIncrement(&rwlock->readers);
    
    if (! ResetEvent(rwlock->read_event))
        return GetLastError();
    
    if (! ReleaseMutex(rwlock->write_mutex))
        return GetLastError();
    
    return ES_SUCCESS;
}

es_status_t eso_thread_rwlock_rdlock(es_thread_rwlock_t *rwlock)
{
	return _thread_rwlock_rdlock_core(rwlock, INFINITE);
}

es_status_t eso_thread_rwlock_tryrdlock(es_thread_rwlock_t *rwlock)
{
	return _thread_rwlock_rdlock_core(rwlock, 0);
}

static es_status_t _thread_rwlock_wrlock_core(es_thread_rwlock_t *rwlock,
											  DWORD milliseconds)
{
    DWORD code = WaitForSingleObject(rwlock->write_mutex, milliseconds);

    if (code == WAIT_FAILED)
        return code;

	if (code == WAIT_TIMEOUT)
		return ES_EBUSY;
		
    /* We've got the writer lock but we have to wait for all readers to
     * unlock before it's ok to use it.
     */
    if (rwlock->readers) {
        /* Must wait for readers to finish before returning, unless this
         * is an trywrlock (milliseconds == 0):
         */
        code = milliseconds
          ? WaitForSingleObject(rwlock->read_event, milliseconds)
          : WAIT_TIMEOUT;
        
        if (code == WAIT_FAILED || code == WAIT_TIMEOUT) {
            /* Unable to wait for readers to finish, release write lock: */
            if (! ReleaseMutex(rwlock->write_mutex))
                return GetLastError();
            
            return (code == WAIT_TIMEOUT) ? ES_EBUSY : code;
        }
    }

    return ES_SUCCESS;
}

es_status_t eso_thread_rwlock_wrlock(es_thread_rwlock_t *rwlock)
{
	return _thread_rwlock_wrlock_core(rwlock, INFINITE);
}

es_status_t eso_thread_rwlock_trywrlock(es_thread_rwlock_t *rwlock)
{
	return _thread_rwlock_wrlock_core(rwlock, 0);
}

es_status_t eso_thread_rwlock_unlock(es_thread_rwlock_t *rwlock)
{
	es_status_t rv = 0;

    /* First, guess that we're unlocking a writer */
    if (! ReleaseMutex(rwlock->write_mutex))
        rv = GetLastError();
    
    if (rv == ERROR_NOT_OWNER) {
        /* Nope, we must have a read lock */
        if (rwlock->readers &&
            ! InterlockedDecrement(&rwlock->readers) &&
            ! SetEvent(rwlock->read_event)) {
            rv = GetLastError();
        }
        else {
            rv = 0;
        }
    }

    return rv;
}

void eso_thread_rwlock_destroy(es_thread_rwlock_t **rwlock)
{
	if (!rwlock || !(*rwlock))
		return;
	
	CloseHandle((*rwlock)->read_event);
	CloseHandle((*rwlock)->write_mutex);
	
	eso_free(*rwlock);
	*rwlock = NULL;
}

#else //linux

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

struct es_thread_rwlock_t {
    pthread_rwlock_t rwlock;
};

es_thread_rwlock_t* eso_thread_rwlock_create(void)
{
	es_thread_rwlock_t *new_rwlock;

	new_rwlock = (es_thread_rwlock_t *)eso_malloc(sizeof(es_thread_rwlock_t));
	if (!new_rwlock) {
		return NULL;
	}

	if (pthread_rwlock_init(&new_rwlock->rwlock, NULL)) {
		eso_free(new_rwlock);
		return NULL;
	}

	return new_rwlock;
}

es_status_t eso_thread_rwlock_rdlock(es_thread_rwlock_t *rwlock)
{
	es_status_t stat;

	stat = pthread_rwlock_rdlock(&rwlock->rwlock);
#ifdef PTHREAD_SETS_ERRNO
	if (stat) {
		stat = errno;
	}
#endif
	return stat;
}

es_status_t eso_thread_rwlock_tryrdlock(es_thread_rwlock_t *rwlock)
{
	es_status_t stat;

	stat = pthread_rwlock_tryrdlock(&rwlock->rwlock);
#ifdef PTHREAD_SETS_ERRNO
	if (stat) {
		stat = errno;
	}
#endif
	/* Normalize the return code. */
	if (stat == EBUSY)
		stat = ES_EBUSY;
	return stat;
}

es_status_t eso_thread_rwlock_wrlock(es_thread_rwlock_t *rwlock)
{
	es_status_t stat;

	stat = pthread_rwlock_wrlock(&rwlock->rwlock);
#ifdef PTHREAD_SETS_ERRNO
	if (stat) {
		stat = errno;
	}
#endif
	return stat;
}

es_status_t eso_thread_rwlock_trywrlock(es_thread_rwlock_t *rwlock)
{
	es_status_t stat;

	stat = pthread_rwlock_trywrlock(&rwlock->rwlock);
#ifdef PTHREAD_SETS_ERRNO
	if (stat) {
		stat = errno;
	}
#endif
	/* Normalize the return code. */
	if (stat == EBUSY)
		stat = ES_EBUSY;
	return stat;
}

es_status_t eso_thread_rwlock_unlock(es_thread_rwlock_t *rwlock)
{
	es_status_t stat;

	stat = pthread_rwlock_unlock(&rwlock->rwlock);
#ifdef PTHREAD_SETS_ERRNO
	if (stat) {
		stat = errno;
	}
#endif
	return stat;
}

void eso_thread_rwlock_destroy(es_thread_rwlock_t **rwlock)
{
	es_status_t rv;
	
	if (!rwlock || !(*rwlock))
		return;
	
	rv = pthread_rwlock_destroy(&(*rwlock)->rwlock);
	#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
	/**
	 * EBUSY  The implementation has detected an attempt to destroy the  object  referenced  by  rwlock  while  it  is
     *        locked.
     */
	ES_ASSERT (rv != EBUSY);
	
	eso_free(*rwlock);
	*rwlock = NULL;
}

#if 0
/** TIMEUP */
#define RWLOCK_TIMEUP           (ES_USERERR_START + 2)

/**
 * eso_thread_rwlock_rdlock with timeout.
 * @param rwlock the read-write lock on which to acquire the shared read.
 */
es_status_t eso_thread_rwlock_timedrdlock(es_thread_rwlock_t *rwlock,
                                          es_uint32_t timeout);
es_status_t eso_thread_rwlock_timedrdlock(es_thread_rwlock_t *rwlock,
                                          es_uint32_t timeout)
{
    es_status_t rv;
    struct timeval tv;
    struct timespec abstime;

    gettimeofday(&tv, NULL);
    abstime.tv_sec = tv.tv_sec + timeout/1000;
    abstime.tv_nsec = tv.tv_usec*1000 + timeout%1000*1000000; /* nanoseconds */

    rv = pthread_rwlock_timedrdlock(&rwlock->rwlock, &abstime);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
    if (ETIMEDOUT == rv) {
        return RWLOCK_TIMEUP;
    }
    return rv;
}

/**
 * eso_thread_rwlock_wrlock with timeout.
 * @param rwlock the read-write lock on which to acquire the exclusive write.
 */
es_status_t eso_thread_rwlock_timedwrlock(es_thread_rwlock_t *rwlock,
                                          es_uint32_t timeout);
es_status_t eso_thread_rwlock_timedwrlock(es_thread_rwlock_t *rwlock,
                                         es_uint32_t timeout)
{
    es_status_t rv;
    struct timeval tv;
    struct timespec abstime;

    gettimeofday(&tv, NULL);
    abstime.tv_sec = tv.tv_sec + timeout/1000;
    abstime.tv_nsec = tv.tv_usec*1000 + timeout%1000*1000000; /* nanoseconds */

    rv = pthread_rwlock_timedwrlock(&rwlock->rwlock, &abstime);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
    if (ETIMEDOUT == rv) {
        return RWLOCK_TIMEUP;
    }
    return rv;
}
#endif

#endif
