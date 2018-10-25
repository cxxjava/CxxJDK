/**
 * @file  eso_thread_cond.c
 * @brief ES Condition Variable Routines
 */

#include "eso_thread_cond.h"
#include "eso_libc.h"

#ifdef WIN32

#include <windows.h>
#include <limits.h> /* LONG_MAX */

//@see: apr-1.4.6/locks/win32/thread_cond.c

struct es_thread_cond_t {
    HANDLE semaphore;
    CRITICAL_SECTION csection;
    unsigned long num_waiting;
    unsigned long num_wake;
    unsigned long generation;
};

es_thread_cond_t* eso_thread_cond_create(void)
{
    es_thread_cond_t *new_cond;
	
	new_cond = (es_thread_cond_t *)eso_malloc(sizeof(es_thread_cond_t));
	if (!new_cond) {
		return NULL;
	}
	
	new_cond->semaphore = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
    if (new_cond->semaphore == NULL) {
        eso_free(new_cond);
		return NULL;
    }

    InitializeCriticalSection(&new_cond->csection);
	
    return new_cond;
}

static es_status_t _thread_cond_timedwait(es_thread_cond_t *cond,
                                          es_thread_mutex_t *mutex,
                                          DWORD timeout_ms )
{
    DWORD res;
    es_status_t rv;
    unsigned int wake = 0;
    unsigned long generation;

    EnterCriticalSection(&cond->csection);
    cond->num_waiting++;
    generation = cond->generation;
    LeaveCriticalSection(&cond->csection);

    eso_thread_mutex_unlock(mutex);

    do {
        res = WaitForSingleObject(cond->semaphore, timeout_ms);

        EnterCriticalSection(&cond->csection);

        if (cond->num_wake) {
            if (cond->generation != generation) {
                cond->num_wake--;
                cond->num_waiting--;
                rv = ES_SUCCESS;
                break;
            } else {
                wake = 1;
            }
        }
        else if (res != WAIT_OBJECT_0) {
            cond->num_waiting--;
            rv = ES_TIMEUP;
            break;
        }

        LeaveCriticalSection(&cond->csection);

        if (wake) {
            wake = 0;
            ReleaseSemaphore(cond->semaphore, 1, NULL);
        }
    } while (1);

    LeaveCriticalSection(&cond->csection);
    eso_thread_mutex_lock(mutex);

    return rv;
}

es_status_t eso_thread_cond_wait(es_thread_cond_t *cond,
                                 es_thread_mutex_t *mutex)
{
    return _thread_cond_timedwait(cond, mutex, INFINITE);
}

es_status_t eso_thread_cond_timedwait(es_thread_cond_t *cond,
                                      es_thread_mutex_t *mutex,
                                      es_int64_t timeout)
{
	return _thread_cond_timedwait(cond, mutex, (DWORD)(timeout / ES_NANOS_PER_MILLISEC)); //millisec is the best accuracy.
}


es_status_t eso_thread_cond_signal(es_thread_cond_t *cond)
{
    unsigned int wake = 0;

    EnterCriticalSection(&cond->csection);
    if (cond->num_waiting > cond->num_wake) {
        wake = 1;
        cond->num_wake++;
        cond->generation++;
    }
    LeaveCriticalSection(&cond->csection);

    if (wake) {
        ReleaseSemaphore(cond->semaphore, 1, NULL);
    }

    return ES_SUCCESS;
}

es_status_t eso_thread_cond_broadcast(es_thread_cond_t *cond)
{
    unsigned long num_wake = 0;

    EnterCriticalSection(&cond->csection);
    if (cond->num_waiting > cond->num_wake) {
        num_wake = cond->num_waiting - cond->num_wake;
        cond->num_wake = cond->num_waiting;
        cond->generation++;
    }
    LeaveCriticalSection(&cond->csection);

    if (num_wake) {
        ReleaseSemaphore(cond->semaphore, num_wake, NULL);
    }
    
    return ES_SUCCESS;
}

void eso_thread_cond_destroy(es_thread_cond_t **cond)
{
	if (!cond || !(*cond))
		return;
	
    CloseHandle((*cond)->semaphore);
    DeleteCriticalSection(&(*cond)->csection);
    
	eso_free(*cond);
	*cond = NULL;
}

#else //linux

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>

struct es_thread_cond_t {
    pthread_cond_t cond;
};

es_thread_cond_t* eso_thread_cond_create(void)
{
    es_thread_cond_t *new_cond;
	
	new_cond = (es_thread_cond_t *)eso_malloc(sizeof(es_thread_cond_t));
	if (!new_cond) {
		return NULL;
	}
	
    if (pthread_cond_init(&new_cond->cond, NULL)) {
		eso_free(new_cond);
		return NULL;
    }
	
    return new_cond;
}

es_status_t eso_thread_cond_wait(es_thread_cond_t *cond,
                                 es_thread_mutex_t *mutex)
{
    es_status_t rv;
    pthread_mutex_t *os_mutex;

	os_mutex = (pthread_mutex_t*)mutex;  /* Notice! 强制类型转换 */
    rv = pthread_cond_wait(&cond->cond, os_mutex);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

es_status_t eso_thread_cond_timedwait(es_thread_cond_t *cond,
                                      es_thread_mutex_t *mutex,
                                      es_int64_t timeout)
{
    es_status_t rv;
	struct timeval tv;
    struct timespec abstime;
    pthread_mutex_t *os_mutex;

	gettimeofday(&tv, NULL);
    abstime.tv_sec = tv.tv_sec + (timeout / ES_NANOS_PER_SECOND);
    abstime.tv_nsec = tv.tv_usec*1000 + (timeout % ES_NANOS_PER_SECOND); /* nanoseconds */

	os_mutex = (pthread_mutex_t*)mutex;  /* Notice! 强制类型转换 */
    rv = pthread_cond_timedwait(&cond->cond, os_mutex, &abstime);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
    if (ETIMEDOUT == rv) {
        return ES_TIMEUP;
    }
    return rv;
}


es_status_t eso_thread_cond_signal(es_thread_cond_t *cond)
{
    es_status_t rv;

    rv = pthread_cond_signal(&cond->cond);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

es_status_t eso_thread_cond_broadcast(es_thread_cond_t *cond)
{
    es_status_t rv;

    rv = pthread_cond_broadcast(&cond->cond);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
    return rv;
}

void eso_thread_cond_destroy(es_thread_cond_t **cond)
{
    es_status_t rv;
	
	if (!cond || !(*cond))
		return;
	
    rv = pthread_cond_destroy(&(*cond)->cond);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
	/**
	 * EBUSY  The implementation has detected an attempt to destroy the object referenced by cond while it  is  refer-
     *        enced  (for  example,  while being used in a pthread_cond_wait() or pthread_cond_timedwait()) by another
     *        thread.
    */
	ES_ASSERT (rv != EBUSY);

	eso_free(*cond);
	*cond = NULL;
}

#endif // !WIN32
