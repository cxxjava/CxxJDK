/**
 * @file  eso_thread_mutex.c
 * @brief ES Thread Mutex Routines
 */

#include "eso_thread_mutex.h"
#include "eso_libc.h"
#include "eso_thread.h"

#ifdef WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500  //for TryEnterCriticalSection
#endif
#include <windows.h>

typedef enum thread_mutex_type {
    THREAD_MUTEX_CRITICAL_SECTION,
    THREAD_MUTEX_UNNESTED_EVENT,
    THREAD_MUTEX_NESTED_MUTEX
} thread_mutex_type;

/* handle applies only to unnested_event on all platforms 
 * and nested_mutex on Win9x only.  Otherwise critical_section 
 * is used for NT nexted mutexes providing optimal performance.
 */
struct es_thread_mutex_t {
    thread_mutex_type type;
    HANDLE            handle;
    CRITICAL_SECTION  section;
};

es_thread_mutex_t* eso_thread_mutex_create(unsigned int flags)
{
	es_thread_mutex_t *new_mutex;
	
	new_mutex = eso_malloc(sizeof(es_thread_mutex_t));
	if (new_mutex == NULL) {
		return NULL;
	}

	if (flags & ES_THREAD_MUTEX_UNNESTED) {
        /* Use an auto-reset signaled event, ready to accept one
         * waiting thread.
         */
        new_mutex->type = THREAD_MUTEX_UNNESTED_EVENT;
        new_mutex->handle = CreateEvent(NULL, FALSE, TRUE, NULL);
    }
    else {
        /* Critical Sections are terrific, performance-wise, on NT.
         * On Win9x, we cannot 'try' on a critical section, so we 
         * use a [slower] mutex object, instead.
         */
        InitializeCriticalSection(&new_mutex->section);
        new_mutex->type = THREAD_MUTEX_CRITICAL_SECTION;
    }

	return new_mutex;
}

es_status_t eso_thread_mutex_lock(es_thread_mutex_t *mutex)
{
	if (mutex->type == THREAD_MUTEX_CRITICAL_SECTION) {
        EnterCriticalSection(&mutex->section);
    }
    else {
        DWORD rv = WaitForSingleObject(mutex->handle, INFINITE);
		if ((rv != WAIT_OBJECT_0) && (rv != WAIT_ABANDONED)) {
            return (rv == WAIT_TIMEOUT) ? ES_EBUSY : GetLastError();
		}
    }
	
	return ES_SUCCESS;
}

es_status_t eso_thread_mutex_trylock(es_thread_mutex_t *mutex)
{
	if (mutex->type == THREAD_MUTEX_CRITICAL_SECTION) {
        if (!TryEnterCriticalSection(&mutex->section)) {
            return ES_EBUSY;
        }
    }
    else {
        DWORD rv = WaitForSingleObject(mutex->handle, 0);
        if ((rv != WAIT_OBJECT_0) && (rv != WAIT_ABANDONED)) {
            return (rv == WAIT_TIMEOUT) ? ES_EBUSY : GetLastError();
        }
    }
    return ES_SUCCESS;
}

es_status_t eso_thread_mutex_timedlock(es_thread_mutex_t *mutex, es_uint32_t timeout)
{
	if (mutex->type == THREAD_MUTEX_CRITICAL_SECTION) {
		es_status_t rv;
		es_int64_t tm = timeout;
		do {
			rv = eso_thread_mutex_trylock(mutex);
			if (rv == ES_EBUSY) {
				/* Sleep for 10 milliseconds before trying again. */
				eso_thread_sleep(10);
				tm -= 10;
			} else {
				break;
			}
		} while (rv != 0 && tm > 0);

		return rv ? rv : ES_SUCCESS;
	}
	else {
		DWORD rv = WaitForSingleObject(mutex->handle, timeout);
		if ((rv != WAIT_OBJECT_0) && (rv != WAIT_ABANDONED)) {
			return (rv == WAIT_TIMEOUT) ? ES_EBUSY : GetLastError();
		}
		return ES_SUCCESS;
	}
}

es_status_t eso_thread_mutex_unlock(es_thread_mutex_t *mutex)
{
	if (mutex->type == THREAD_MUTEX_CRITICAL_SECTION) {
        LeaveCriticalSection(&mutex->section);
    }
    else if (mutex->type == THREAD_MUTEX_UNNESTED_EVENT) {
        if (!SetEvent(mutex->handle)) {
            return GetLastError();
        }
    }
    else if (mutex->type == THREAD_MUTEX_NESTED_MUTEX) {
        if (!ReleaseMutex(mutex->handle)) {
            return GetLastError();
        }
    }
    return ES_SUCCESS;
}

void eso_thread_mutex_destroy(es_thread_mutex_t **mutex)
{
	if (!mutex || !(*mutex))
		return;
	
	if ((*mutex)->type == THREAD_MUTEX_CRITICAL_SECTION) {
        (*mutex)->type = -1;
        DeleteCriticalSection(&(*mutex)->section);
    }
    else {
        CloseHandle((*mutex)->handle);
    }

	eso_free(*mutex);
	*mutex = NULL;
}

#else //linux

#include <pthread.h>

/* Define if recursive pthread mutexes are available */
#define HAVE_PTHREAD_MUTEX_RECURSIVE    1

struct es_thread_mutex_t {
    pthread_mutex_t mutex;
};

es_thread_mutex_t* eso_thread_mutex_create(unsigned int flags)
{
	es_thread_mutex_t *new_mutex;
	es_status_t rv;
	
#if !(HAVE_PTHREAD_MUTEX_RECURSIVE)
	if (flags & ES_THREAD_MUTEX_NESTED) {
		return NULL;
	}
#endif

	new_mutex = eso_malloc(sizeof(es_thread_mutex_t));
	if (new_mutex == NULL) {
		return NULL;
	}

#if (HAVE_PTHREAD_MUTEX_RECURSIVE)
	if (flags & ES_THREAD_MUTEX_NESTED) {
		pthread_mutexattr_t mattr;
		
		rv = pthread_mutexattr_init(&mattr);
		if (rv) {
			eso_free(new_mutex);
			return NULL;
		}
		/**< `PTHREAD_MUTEX_RECURSIVE' undeclared !!!
		* Yeah, linux pthreads sort of sucks. PTHREAD_MUTEX_RECURSIVE 
		* is what posix says should be supported, but some versions of glibc 
		* have only PTHREAD_MUTEX_RECURSIVE_NP (the np stands for non-portable), 
		* when they have the same meaning. 

		* At least we have NPTL plus kernel 2.6 to look forward to, which will 
		* make pthreads on linux fully conforming and faster too.
		*/
		rv = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
		if (rv) {
			pthread_mutexattr_destroy(&mattr);
			eso_free(new_mutex);
			return NULL;
		}

		rv = pthread_mutex_init(&new_mutex->mutex, &mattr);
		
		pthread_mutexattr_destroy(&mattr);
	} else
#endif
		rv = pthread_mutex_init(&new_mutex->mutex, NULL);

	if (rv) {
		eso_free(new_mutex);
		return NULL;
	}

	return new_mutex;
}

es_status_t eso_thread_mutex_lock(es_thread_mutex_t *mutex)
{
	es_status_t rv;

	rv = pthread_mutex_lock(&mutex->mutex);
#ifdef PTHREAD_SETS_ERRNO
	if (rv) {
		rv = errno;
	}
#endif
	
	return rv;
}

es_status_t eso_thread_mutex_trylock(es_thread_mutex_t *mutex)
{
	es_status_t rv;

	rv = pthread_mutex_trylock(&mutex->mutex);
	if (rv) {
#ifdef PTHREAD_SETS_ERRNO
		rv = errno;
#endif
		return (rv == EBUSY) ? ES_EBUSY : rv;
	}

	return ES_SUCCESS;
}

es_status_t eso_thread_mutex_timedlock(es_thread_mutex_t *mutex, es_uint32_t timeout)
{
	es_status_t rv;

#ifdef __APPLE__
	es_int64_t tm = timeout;
	do {
		rv = eso_thread_mutex_trylock(mutex);
		if (rv == ES_EBUSY) {
			/* Sleep for 10 milliseconds before trying again. */
			eso_thread_sleep(10);
			tm -= 10;
		} else {
			break;
		}
	} while (rv != 0 && tm > 0);

	if (rv) {
		return rv;
	}
#else
	struct timeval tv;
	struct timespec abstime;

	gettimeofday(&tv, NULL);
	abstime.tv_sec = tv.tv_sec + timeout/1000;
	abstime.tv_nsec = tv.tv_usec*1000 + timeout%1000*1000000; /* nanoseconds */

	rv = pthread_mutex_timedlock(&mutex->mutex, &abstime);
	if (rv) {
#ifdef PTHREAD_SETS_ERRNO
		rv = errno;
#endif
		return (rv == EBUSY) ? ES_EBUSY : rv;
	}
#endif

	return ES_SUCCESS;
}

es_status_t eso_thread_mutex_unlock(es_thread_mutex_t *mutex)
{
	es_status_t status;

	status = pthread_mutex_unlock(&mutex->mutex);
#ifdef PTHREAD_SETS_ERRNO
	if (status) {
		status = errno;
	}
#endif

	return status;
}

void eso_thread_mutex_destroy(es_thread_mutex_t **mutex)
{
	es_status_t rv;

	if (!mutex || !(*mutex))
		return;
	
	rv = pthread_mutex_destroy(&(*mutex)->mutex);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
	/**
     *  EBUSY  The implementation has detected an attempt to destroy the object referenced by mutex while it is  locked
     *         or  referenced  (for  example, while being used in a pthread_cond_timedwait() or pthread_cond_wait()) by
     *         another thread.
	 */
	ES_ASSERT (rv != EBUSY);

	eso_free(*mutex);
	*mutex = NULL;
}

#endif //!WIN32
