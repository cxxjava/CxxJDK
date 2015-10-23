/**
 * @file  eso_mmap.h
 * @brief MMAP (Memory Map) routines
 */

#ifndef __ESO_MMAP_H__
#define __ESO_MMAP_H__

#include "es_comm.h"
#include "es_types.h"
#include "eso_file.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_mmap_t es_mmap_t;

/** MMap opened for reading */
#define ES_MMAP_READ    1
/** MMap opened for writing */
#define ES_MMAP_WRITE   2


/** MMap specifies that an update be scheduled, but the call returns immediately */
#define ES_MMAP_SYNC    1
/** MMap asks for an update and waits for it to complete */
#define ES_MMAP_ASYNC   2


/** 
 * Create a new mmap'ed file out of an existing file.
 * @param filename The file turn into an mmap.
 * @param flag bit-wise or of:
 * <PRE>
 *          ES_MMAP_READ       MMap opened for reading
 *          ES_MMAP_WRITE      MMap opened for writing
 * </PRE>
 * @param offset The offset into the file to start the data pointer at.
 * @param size The size of the file
 * @mark at linux, only use ES_MMAP_WRITE will get errno=13
 * @see http://hi.baidu.com/ah__fu/item/fc8737dd57c73b57d73aae50
 */
es_mmap_t* eso_mmap_create(const char* filename, int flag,
		                   es_off_t offset, es_size_t size);

/**
 * Duplicate the specified MMAP.
 * @param mm The mmap to duplicate.
 */         
es_mmap_t* eso_mmap_dup(es_mmap_t *mm);

/**
 * Remove a mmap'ed.
 * @param mm The mmap'ed file.
 */
void eso_mmap_delete(es_mmap_t **mm);

/** 
 * Move the pointer into the mmap'ed file to the specified offset.
 * @param mm The mmap'ed file.
 * @param offset The offset to move to.
 * @return The pointer to the offset specified.
 */
void* eso_mmap_offset(es_mmap_t *mm, es_off_t offset);

/**
 * Flushes changes made to the in-core copy of a file that was mapped into memory back to disk.
 * @param mm The mmap'ed file.
 * @param flag one of:
 * <PRE>
 *          ES_MMAP_SYNC       MMap specifies that an update be scheduled, but the call returns immediately
 *          ES_MMAP_ASYNC      MMap asks for an update and waits for it to complete
 * </PRE>
 */
void eso_mmap_flush(es_mmap_t *mm, int flag);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_MMAP_H__ */
