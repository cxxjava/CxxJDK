/**
 * @file eso_shm.c
 * @brief Shared Memory Routines
 */

#include "eso_shm.h"
#include "eso_file.h"
#include "eso_string.h"
#include "eso_util.h"

#ifdef WIN32

#include <Windows.h>

typedef struct memblock_t {
    es_size_t size;
    es_size_t length;
} memblock_t;

struct es_shm_t {
    memblock_t *memblk;
    void       *usrmem;
    es_size_t  size;
    es_size_t  length;
    HANDLE      hMap;
    es_string_t *filename;
};

es_shm_t* eso_shm_create(es_size_t reqsize, const char *filename)
{
	static es_size_t memblock = 0;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMap;
    es_size_t size;
    void *base;
    void *mapkey = NULL;
    char res_name[ES_PATH_MAX] = {0};
    DWORD err, sizelo, sizehi;
    es_shm_t *new_shm;

    reqsize += sizeof(memblock_t);

    if (!memblock) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        memblock = si.dwAllocationGranularity;
    }

    /* Compute the granualar multiple of the pagesize */
    size = memblock * (1 + (reqsize - 1) / memblock);
    sizelo = (DWORD)size;
    sizehi = 0;

	if (!filename) {
        /* Do Anonymous, which must be passed as a duplicated handle */
        hFile = INVALID_HANDLE_VALUE;
        mapkey = NULL;
    }
    else {
    	/* Do file backed, which is not an inherited handle 
         * While we could open APR_EXCL, it doesn't seem that Unix
         * ever did.  Ignore that error here, but fail later when
         * we discover we aren't the creator of the file map object.
         */
        hFile = CreateFile(filename,
				GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
				NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			return NULL;
		}
		SetFilePointer(hFile, size, NULL, FILE_BEGIN);
		SetEndOfFile(hFile);
        
        mapkey = eso_resname_from_filename(filename, 1, res_name, sizeof(res_name));
    }

	hMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 
                                  sizehi, sizelo, mapkey);
	err = GetLastError();
	if (hFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hFile);
	}
	if (hMap && (err == ES_EEXIST)) {
        CloseHandle(hMap);
        return NULL;
    }
    if (!hMap) {
        return NULL;
    }
	
	base = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE,
                         0, 0, size);
    if (!base) {
        CloseHandle(hMap);
        return NULL;
    }
    
    new_shm = (es_shm_t *) eso_calloc(sizeof(es_shm_t));
    new_shm->hMap = hMap;
    new_shm->memblk = base;
    new_shm->size = size;

    new_shm->usrmem = (char*)base + sizeof(memblock_t);
    new_shm->length = reqsize - sizeof(memblock_t);;
    
    new_shm->memblk->length = new_shm->length;
    new_shm->memblk->size = new_shm->size;
    new_shm->filename = filename ? eso_mstrdup(filename) : NULL;
    
    return new_shm;
}

es_status_t eso_shm_remove(const char *filename)
{
	eso_unlink(filename);
    return GetLastError();
}

es_status_t eso_shm_destroy(es_shm_t **m)
{
	es_shm_t *shm;
	es_status_t rv = ES_SUCCESS;
	
	if (!m || !*m) {
		return ES_BADARG;
	}
	shm = *m;
	
	if (!UnmapViewOfFile(shm->memblk)) {
        rv = GetLastError();
    }
    if (!CloseHandle(shm->hMap)) {
        rv = (rv != ES_SUCCESS) ? rv : GetLastError();
    }
    if (shm->filename) {
        /* Remove file if file backed */
        eso_unlink(shm->filename);
        ESO_MFREE(&shm->filename);
    }
    
    eso_free(shm);
    *m = NULL;
    
	return rv;
}

es_status_t eso_shm_attach(es_shm_t **m, const char *filename)
{
	HANDLE hMap;
    char mapkey[ES_PATH_MAX] = {0};
    void *base;

    if (!filename) {
        return ES_EINVAL;
    }
    else {
        /* res_name_from_filename turns file into a pseudo-name
         * without slashes or backslashes, and prepends the \global
         * prefix on Win2K and later
         */
        eso_resname_from_filename(filename, 1, mapkey, sizeof(mapkey));
    }
    
    hMap = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, mapkey);
    if (!hMap) {
        return GetLastError();
    }
    
    base = MapViewOfFile(hMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (!base) {
        CloseHandle(hMap);
        return GetLastError();
    }
    
    *m = (es_shm_t *) eso_calloc(sizeof(es_shm_t));
    (*m)->memblk = base;
    /* Real (*m)->mem->size could be recovered with VirtualQuery */
    (*m)->size = (*m)->memblk->size;
    (*m)->hMap = hMap;
    (*m)->length = (*m)->memblk->length;
    (*m)->usrmem = (char*)base + sizeof(memblock_t);
    (*m)->filename = NULL;
	
	return ES_SUCCESS;
}

es_status_t eso_shm_detach(es_shm_t **m)
{
	return eso_shm_destroy(m);
}

void* eso_shm_baseaddr_get(const es_shm_t *m)
{
    return m->usrmem;
}

es_size_t eso_shm_size_get(const es_shm_t *m)
{
    return m->length;
}

#else //linux

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#if !defined(SHM_R)
#define SHM_R 0400
#endif
#if !defined(SHM_W)
#define SHM_W 0200
#endif
#include <sys/file.h>

/**
 * Private, platform-specific data struture representing a shared memory
 * segment.
 */
struct es_shm_t {
    void *base;          /* base real address */
    void *usable;        /* base usable address */
    es_size_t reqsize;  /* requested segment size */
    es_size_t realsize; /* actual segment size */
    es_string_t *filename;      /* NULL if anonymous */
    int shmid;          /* shmem ID returned from shmget() */
};

es_shm_t* eso_shm_create(es_size_t reqsize, const char *filename)
{
    es_shm_t *new_m = NULL;
    struct shmid_ds shmbuf;
    key_t shmkey;
    es_size_t nbytes;
    es_ssize_t rv;
    int fd = -1;   /* file where metadata is stored */

	new_m = (es_shm_t *) eso_calloc(sizeof(es_shm_t));
	if (!new_m)
		return NULL;

    /* Check if they want anonymous or name-based shared memory */
    if (filename == NULL) {
        new_m->reqsize = reqsize;
        new_m->realsize = reqsize;
        new_m->filename = NULL;
        new_m->shmid = -1;

        if ((new_m->shmid = shmget(IPC_PRIVATE, new_m->realsize,
                                   SHM_R | SHM_W | IPC_CREAT)) < 0) {
        	goto FAILURE;
        }

        if ((new_m->base = shmat(new_m->shmid, NULL, 0)) == (void *)-1) {
        	goto FAILURE;
        }
        new_m->usable = new_m->base;

        if (shmctl(new_m->shmid, IPC_STAT, &shmbuf) == -1) {
        	goto FAILURE;
        }
        shmbuf.shm_perm.uid = getuid();
        shmbuf.shm_perm.gid = getpid();
        if (shmctl(new_m->shmid, IPC_SET, &shmbuf) == -1) {
        	goto FAILURE;
        }

        /* Remove the segment once use count hits zero.
         * We will not attach to this segment again, since it is
         * anonymous memory, so it is ok to mark it for deletion.
         */
        if (shmctl(new_m->shmid, IPC_RMID, NULL) == -1) {
        	goto FAILURE;
        }
    }

    /* Name-based shared memory */
    else {
        new_m->reqsize = reqsize;
        new_m->realsize = reqsize;
        new_m->filename = eso_mstrdup(filename);
        new_m->shmid = -1;

        /* FIXME: EZ_OS_DEFAULT is too permissive, switch to 600 I think. */
		fd = open(filename, O_CREAT | O_WRONLY | O_EXCL, 0600);
        if (fd == -1) {
        	goto FAILURE;
        }

        /* ftok() (on solaris at least) requires that the file actually
         * exist before calling ftok(). */
        shmkey = ftok(filename, 1);
        if (shmkey == (key_t)-1) {
        	goto FAILURE;
        }

        if ((new_m->shmid = shmget(shmkey, new_m->realsize,
                                   SHM_R | SHM_W | IPC_CREAT | IPC_EXCL)) < 0) {
        	goto FAILURE;
        }

        if ((new_m->base = shmat(new_m->shmid, NULL, 0)) == (void *)-1) {
        	goto FAILURE;
        }
        new_m->usable = new_m->base;

        if (shmctl(new_m->shmid, IPC_STAT, &shmbuf) == -1) {
        	goto FAILURE;
        }
        shmbuf.shm_perm.uid = getuid();
        shmbuf.shm_perm.gid = getpid();
        if (shmctl(new_m->shmid, IPC_SET, &shmbuf) == -1) {
        	goto FAILURE;
        }

        nbytes = sizeof(reqsize);
        do {
        	rv = write(fd, (const void *)&reqsize, nbytes);
        } while (rv == -1 && errno == EINTR);
        if (rv != nbytes) {
        	goto FAILURE;
        }
        close(fd);
    }

    return new_m;

FAILURE:
	if (new_m) {
		if (new_m->filename) {
			eso_mfree(new_m->filename);
		}
		if (new_m->shmid != -1) {
			shmctl(new_m->shmid, IPC_RMID, NULL);
		}
		if (new_m->base != NULL) {
			shmdt(new_m->base);
		}
		eso_free(new_m);
	}
	if (fd != -1) {
		close(fd);
	}
	return NULL;
}

es_status_t eso_shm_remove(const char *filename)
{
    es_status_t status;
    key_t shmkey;
    int shmid;
    int fd = -1;

    /* Presume that the file already exists; just open for writing */    
    fd = open(filename, O_WRONLY, 0600);
    if (fd == -1) {
        return errno;
    }
    close(fd);

    /* ftok() (on solaris at least) requires that the file actually
     * exist before calling ftok(). */
    shmkey = ftok(filename, 1);
    if (shmkey == (key_t)-1) {
        goto FAILURE;
    }

    if ((shmid = shmget(shmkey, 0, SHM_R | SHM_W)) < 0) {
        goto FAILURE;
    }

    /* Indicate that the segment is to be destroyed as soon
     * as all processes have detached. This also disallows any
     * new attachments to the segment. */
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        goto FAILURE;
    }

    return unlink(filename);

FAILURE:
    status = errno;
    /* ensure the file has been removed anyway. */
    unlink(filename);
    return status;
} 

es_status_t eso_shm_destroy(es_shm_t **m)
{
    if (!m || !*m)
    	return ES_SUCCESS;

    /* anonymous shared memory */
    if ((*m)->filename == NULL) {
        if (shmdt((*m)->base) == -1) {
            return errno;
        }
        /* This segment will automatically remove itself after all
         * references have detached. */
        
        ESO_FREE(m);
        
        return ES_SUCCESS;
    }

    /* name-based shared memory */
    else {
        /* Indicate that the segment is to be destroyed as soon
         * as all processes have detached. This also disallows any
         * new attachments to the segment. */
        if (shmctl((*m)->shmid, IPC_RMID, NULL) == -1 && errno != EINVAL) {
            return errno;
        }
        if (shmdt((*m)->base) == -1) {
            return errno;
        }
        if (!access((*m)->filename, F_OK)) {
            unlink((*m)->filename);
        }
        
        eso_mfree((*m)->filename);
        ESO_FREE(m);
        
        return ES_SUCCESS;
    }
}

es_status_t eso_shm_attach(es_shm_t **m, const char *filename)
{
    if (filename == NULL) {
        /* It doesn't make sense to attach to a segment if you don't know
         * the filename. */
        return ES_EINVAL;
    }
    else {
        es_shm_t *new_m;
        es_status_t status;
        es_size_t nbytes;
        es_ssize_t rv;
        key_t shmkey;
        int fd = -1;   /* file where metadata is stored */

        new_m = (es_shm_t *)eso_calloc(sizeof(es_shm_t));
        if (!new_m)
        	return ES_ENOMEM;

        fd = open(filename, O_RDONLY, 0600);
        if (fd == -1) {
        	goto FAILURE;
        }

        nbytes = sizeof(new_m->reqsize);
		do {
			rv = read(fd, (void *) &(new_m->reqsize), nbytes);
		} while (rv == -1 && errno == EINTR);
        if (rv != nbytes) {
        	goto FAILURE;
        }


        new_m->filename = eso_mstrdup(filename);
        shmkey = ftok(filename, 1);
        if (shmkey == (key_t)-1) {
        	goto FAILURE;
        }
        if ((new_m->shmid = shmget(shmkey, 0, SHM_R | SHM_W)) == -1) {
        	goto FAILURE;
        }
        if ((new_m->base = shmat(new_m->shmid, NULL, 0)) == (void *)-1) {
        	goto FAILURE;
        }
        new_m->usable = new_m->base;
        new_m->realsize = new_m->reqsize;

        close(fd);
        *m = new_m;
        return ES_SUCCESS;

FAILURE:
		status = errno;
		if (new_m) {
			if (new_m->filename) {
				eso_mfree(new_m->filename);
			}
			if (new_m->base != NULL) {
				shmdt(new_m->base);
			}
			eso_free(new_m);
		}
		if (fd != -1) {
			close(fd);
		}
		return status;
    }
}

es_status_t eso_shm_detach(es_shm_t **m)
{
	if (!m || !*m)
	   	return ES_SUCCESS;

    if ((*m)->filename == NULL) {
        /* It doesn't make sense to detach from an anonymous memory segment. */
        return ES_EINVAL;
    }
    else {
        if (shmdt((*m)->base) == -1) {
            return errno;
        }
        
        eso_mfree((*m)->filename);
        ESO_FREE(m);
        
        return ES_SUCCESS;
	}
}

void* eso_shm_baseaddr_get(const es_shm_t *m)
{
    return m->usable;
}

es_size_t eso_shm_size_get(const es_shm_t *m)
{
    return m->reqsize;
}

#endif //!WIN32
