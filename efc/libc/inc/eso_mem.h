/**
 * @file  es_mem.h
 * @brief ES memory manager.
 */

#ifndef __ESO_MEM_H__
#define __ESO_MEM_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void* eso_mmalloc(es_size_t size);
void* eso_mcalloc(es_size_t size);
void* eso_mrealloc(void *ptr, es_size_t newsize);
void  eso_mfree(void *ptr);
void  eso_mfree_and_nil(void **ptr);

es_size_t eso_mnode_size(void *ptr);

#define ESO_MFREE(pp) eso_mfree_and_nil((void**)(pp));

#ifdef __cplusplus
}
#endif

#endif /* __ESO_MEM_H__ */
