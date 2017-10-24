/**
 * @file eso_shm.h
 * @brief Shared Memory Routines
 */


#ifndef __ESO_SHM_H__
#define __ESO_SHM_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_shm_t es_shm_t;

/**
 * Create and make accessable a shared memory segment.
 * @param reqsize The desired size of the segment.
 * @param filename The file to use for shared memory on platforms that
 *        require it.
 * @remark A note about Anonymous vs. Named shared memory segments:
 *         Not all plaforms support anonymous shared memory segments, but in
 *         some cases it is prefered over other types of shared memory
 *         implementations. Passing a NULL 'file' parameter to this function
 *         will cause the subsystem to use anonymous shared memory segments.
 *         If such a system is not available, EZ_ENOTIMPL is returned.
 * @remark A note about allocation sizes:
 *         On some platforms it is necessary to store some metainformation
 *         about the segment within the actual segment. In order to supply
 *         the caller with the requested size it may be necessary for the
 *         implementation to request a slightly greater segment length
 *         from the subsystem. In all cases, the eso_shm_baseaddr_get()
 *         function will return the first usable byte of memory.
 * 
 */
es_shm_t* eso_shm_create(es_size_t reqsize, const char *filename);

/**
 * Remove shared memory segment associated with a filename.
 * @param filename The filename associated with shared-memory segment which
 *        needs to be removed
 * @remark This function is only supported on platforms which support
 * name-based shared memory segments, and will return EZ_ENOTIMPL on
 * platforms without such support.
 */
es_status_t eso_shm_remove(const char *filename);

/**
 * Destroy a shared memory segment and associated memory.
 * @param m The shared memory segment structure to destroy.
 */
es_status_t eso_shm_destroy(es_shm_t **m);

/**
 * Attach to a shared memory segment that was created
 * by another process.
 * @param m The shared memory structure to create.
 * @param filename The file used to create the original segment.
 *        (This MUST match the original filename.)
 */
es_status_t eso_shm_attach(es_shm_t **m, const char *filename);

/**
 * Detach from a shared memory segment without destroying it.
 * @param m The shared memory structure representing the segment
 *        to detach from.
 */
es_status_t eso_shm_detach(es_shm_t **m);

/**
 * Retrieve the base address of the shared memory segment.
 * NOTE: This address is only usable within the callers address
 * space, since this API does not guarantee that other attaching
 * processes will maintain the same address mapping.
 * @param m The shared memory segment from which to retrieve
 *        the base address.
 * @return address, aligned by EZ_ALIGN_DEFAULT.
 */
void* eso_shm_baseaddr_get(const es_shm_t *m);

/**
 * Retrieve the length of a shared memory segment in bytes.
 * @param m The shared memory segment from which to retrieve
 *        the segment length.
 */
es_size_t eso_shm_size_get(const es_shm_t *m);


#ifdef __cplusplus
}
#endif

#endif  /* __ESO_SHM_H__ */
