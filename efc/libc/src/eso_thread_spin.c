/**
 * @file  eso_thread_spin.c
 * @brief ES Thread Spin Lock Routines
 */

#include "eso_thread_spin.h"
#include "eso_libc.h"
#include "eso_thread.h"

#ifdef WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x500  //for TryEnterCriticalSection
#endif
#include <windows.h>

/* handle applies only to unnested_event on all platforms 
 * and nested_mutex on Win9x only.  Otherwise critical_section 
 * is used for NT nexted mutexes providing optimal performance.
 */
struct es_thread_spin_t {
    CRITICAL_SECTION  lock;
};

#define ES_DEFAULT_SPIN_COUNT 4000

es_thread_spin_t* eso_thread_spin_create(void)
{
	es_thread_spin_t *new_spin;
	BOOL rv;
	
	new_spin = eso_malloc(sizeof(es_thread_spin_t));
	if (new_spin == NULL) {
		return NULL;
	}
	
	rv = InitializeCriticalSectionAndSpinCount(&new_spin->lock, ES_DEFAULT_SPIN_COUNT);
	if (!rv) {
		eso_free(new_spin);
		return NULL;
	}
	
	return new_spin;
}

es_status_t eso_thread_spin_lock(es_thread_spin_t *spin)
{
	EnterCriticalSection(&spin->lock);
	return ES_SUCCESS;
}

es_status_t eso_thread_spin_trylock(es_thread_spin_t *spin)
{
	if (!TryEnterCriticalSection(&spin->lock)) {
		return ES_EBUSY;
	}
	return ES_SUCCESS;
}

es_status_t eso_thread_spin_timedlock(es_thread_spin_t *spin, es_uint32_t timeout)
{
	es_status_t rv;
	es_int64_t tm = timeout;
	do {
		rv = eso_thread_spin_trylock(spin);
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

es_status_t eso_thread_spin_unlock(es_thread_spin_t *spin)
{
	LeaveCriticalSection(&spin->lock);
	return ES_SUCCESS;
}

void eso_thread_spin_destroy(es_thread_spin_t **spin)
{
	if (!spin || !(*spin))
		return;
	
	DeleteCriticalSection(&(*spin)->lock);

	ESO_FREE(spin);
}

#else //linux

#ifdef __APPLE__

//@see: http://stackoverflow.com/questions/8177031/does-mac-os-x-have-pthread-spinlock-t-type

#include <sched.h>

typedef int pthread_spinlock_t;

int pthread_spin_init(pthread_spinlock_t *lock, int pshared) {
	__asm__ __volatile__ ("" ::: "memory");
	*lock = 0;
    return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock) {
    return 0;
}

int pthread_spin_lock(pthread_spinlock_t *lock) {
	while (1) {
		int i;
		for (i=0; i < 10000; i++) {
			if (__sync_bool_compare_and_swap(lock, 0, 1)) {
				return 0;
			}
		}
		sched_yield();
	}
}

int pthread_spin_trylock(pthread_spinlock_t *lock) {
	 if (__sync_bool_compare_and_swap(lock, 0, 1)) {
		return 0;
	}
    return EBUSY;
}

int pthread_spin_unlock(pthread_spinlock_t *lock) {
	__asm__ __volatile__ ("" ::: "memory");
	*lock = 0;
    return 0;
}

#endif //!__APPLE__

#include <pthread.h>

struct es_thread_spin_t {
    pthread_spinlock_t lock;
};

es_thread_spin_t* eso_thread_spin_create(void)
{
	es_thread_spin_t *new_spin;
	int rv;
	
	new_spin = eso_malloc(sizeof(es_thread_spin_t));
	if (new_spin == NULL) {
		return NULL;
	}
	
	rv = pthread_spin_init(&new_spin->lock, 0);
	if (rv) {
		eso_free(new_spin);
		return NULL;
	}
	
	return new_spin;
}

es_status_t eso_thread_spin_lock(es_thread_spin_t *spin)
{
	es_status_t rv;

	rv = pthread_spin_lock(&spin->lock);
#ifdef PTHREAD_SETS_ERRNO
	if (rv) {
		rv = errno;
	}
#endif
	
	return rv;
}

es_status_t eso_thread_spin_trylock(es_thread_spin_t *spin)
{
	es_status_t rv;

	rv = pthread_spin_trylock(&spin->lock);
	if (rv) {
#ifdef PTHREAD_SETS_ERRNO
		rv = errno;
#endif
		return (rv == EBUSY) ? ES_EBUSY : rv;
	}

	return ES_SUCCESS;
}

es_status_t eso_thread_spin_timedlock(es_thread_spin_t *spin, es_uint32_t timeout)
{
	es_status_t rv;
	es_int64_t tm = timeout;

	do {
		rv = eso_thread_spin_trylock(spin);
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

	return ES_SUCCESS;
}

es_status_t eso_thread_spin_unlock(es_thread_spin_t *spin)
{
	es_status_t status;

	status = pthread_spin_unlock(&spin->lock);
#ifdef PTHREAD_SETS_ERRNO
	if (status) {
		status = errno;
	}
#endif

	return status;
}

void eso_thread_spin_destroy(es_thread_spin_t **spin)
{
	es_status_t rv;

	if (!spin || !(*spin))
		return;
	
	rv = pthread_spin_destroy(&(*spin)->lock);
#ifdef PTHREAD_SETS_ERRNO
    if (rv) {
        rv = errno;
    }
#endif
	/**
     *  EBUSY  The implementation has detected an attempt to destroy the object referenced by spin while it is  locked
     *         or  referenced.
	 */
	ES_ASSERT (rv != EBUSY);

	eso_free(*spin);
	*spin = NULL;
}

#endif //!WIN32
