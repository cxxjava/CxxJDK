/**
 * @file  eso_proc_mutex.c
 * @brief ES Process Mutex Routines
 */

#include "eso_proc_mutex.h"
#include "eso_libc.h"
#include "eso_util.h"

#ifdef WIN32

es_proc_mutex_t* eso_proc_mutex_create(void)
{
	return NULL;
}

es_status_t eso_proc_mutex_lock(es_proc_mutex_t *mutex)
{
	return ES_ENOTIMPL;
}

es_status_t eso_proc_mutex_trylock(es_proc_mutex_t *mutex)
{
	return ES_ENOTIMPL;
}

es_status_t eso_proc_mutex_unlock(es_proc_mutex_t *mutex)
{
	return ES_ENOTIMPL;
}

void eso_proc_mutex_destroy(es_proc_mutex_t **mutex)
{

}

#else //linux

#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h> /* close */

#include <sys/types.h> /* for mac */

struct es_proc_mutex_t {
    int curr_locked;
    pthread_mutex_t *pthread_interproc;
};

es_proc_mutex_t* eso_proc_mutex_create(void)
{
	es_proc_mutex_t *new_mutex;
	int fd;
	pthread_mutexattr_t mattr;
	
	fd = open("/dev/zero", O_RDWR);
	if (fd < 0) {
		return NULL;
	}
	
	new_mutex = (es_proc_mutex_t *)eso_calloc(sizeof(es_proc_mutex_t));
	if (!new_mutex)	{
		close(fd);
		return NULL;
	}
	
	new_mutex->pthread_interproc = (pthread_mutex_t *)mmap(
									   (caddr_t) 0, 
									   sizeof(pthread_mutex_t), 
									   PROT_READ | PROT_WRITE, MAP_SHARED,
									   fd, 0); 
	if (new_mutex->pthread_interproc == (pthread_mutex_t *) (caddr_t) -1) {
		eso_free(new_mutex);
		close(fd);
		return NULL;
	}
	close(fd);
	
	new_mutex->curr_locked = -1; /* until the mutex has been created */

	pthread_mutexattr_init(&mattr);
	pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(new_mutex->pthread_interproc, &mattr);
	
	new_mutex->curr_locked = 0; /* mutex created now */

	pthread_mutexattr_destroy(&mattr);
	
	return new_mutex;
}

es_status_t eso_proc_mutex_lock(es_proc_mutex_t *mutex)
{
	es_status_t rv;

	if ((rv = pthread_mutex_lock(mutex->pthread_interproc))) {
#ifdef PTHREAD_SETS_ERRNO
		rv = errno;
#endif
		return rv;
	}
	mutex->curr_locked = 1;
	return ES_SUCCESS;
}

es_status_t eso_proc_mutex_trylock(es_proc_mutex_t *mutex)
{
	es_status_t rv;
 
    if ((rv = pthread_mutex_trylock(mutex->pthread_interproc))) {
#ifdef PTHREAD_SETS_ERRNO
        rv = errno;
#endif
        if (rv == EBUSY) {
            return ES_EBUSY;
        }

        return rv;
    }
    mutex->curr_locked = 1;
    return rv;
}

es_status_t eso_proc_mutex_unlock(es_proc_mutex_t *mutex)
{
	es_status_t rv;

	mutex->curr_locked = 0;
	if ((rv = pthread_mutex_unlock(mutex->pthread_interproc))) {
#ifdef PTHREAD_SETS_ERRNO
		rv = errno;
#endif
		return rv;
	}
	return ES_SUCCESS;
}

void eso_proc_mutex_destroy(es_proc_mutex_t **mutex)
{
	if (!mutex || !*mutex) {
		return;
	}
	
	if ((*mutex)->curr_locked == 1) {
		pthread_mutex_unlock((*mutex)->pthread_interproc);
	}
	
	/* curr_locked is set to -1 until the mutex has been created */
	if ((*mutex)->curr_locked != -1) {
		pthread_mutex_destroy((*mutex)->pthread_interproc);
	}
	
	munmap((caddr_t)(*mutex)->pthread_interproc, sizeof(pthread_mutex_t));
	
	ESO_FREE(mutex);
}

#endif //!WIN32
