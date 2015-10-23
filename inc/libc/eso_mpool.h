/**
 * @file  es_mpool.h
 * @brief ES memory pool manager, not thread safe!
 */

#ifndef __ESO_MEMPOOL_H__
#define __ESO_MEMPOOL_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef WIN32
//#define ES_MEMPOOL_DEBUG
#endif

/*
 * memory node find mode
 */
typedef enum {
	ES_MFIND_SIZE,
	ES_MFIND_FAST
} es_mfind_e;

typedef struct es_mpool_t  es_mpool_t;

/**
 * memory pool create
 */
es_mpool_t* eso_mempool_create(void);

/**
 * memory pool destroy
 */
void eso_mempool_free(es_mpool_t **mpool);

/**
 * get memory pool count information
 */
void eso_mempool_count(es_mpool_t *mpool, 
                     es_size_t *user_malloc_size,
                     es_size_t *real_malloc_size);

/**
 * dump for debug (only WIN32)
 */
void eso_mempool_dump(es_mpool_t *mpool, const char *flag);

/**
 * check memory overlap for debug (only WIN32)
 */
void eso_mempool_check(es_mpool_t *mpool, const char *flag);

/**
 * realloc
 */
void* eso_mprealloc(void *pold, es_size_t size);
void* eso_mprealloc_ext(void *pold, es_size_t size, es_mfind_e type);

/**
 * free
 */
void eso_mpfree(void* ptr);
void eso_mpfree0(void** ptr);
#define eso_mpfree_and_nil(p)    eso_mpfree0((void**)(p))

/**
 * get memory node max size
 */
es_size_t eso_mpnode_size(void *ptr);

#ifdef ES_MEMPOOL_DEBUG

/**
 * malloc
 */
void* eso_mpmalloc_debug(es_mpool_t *mpool, es_size_t size, const es_int8_t *file, es_int32_t line);
void* eso_mpmalloc_ext_debug(es_mpool_t *mpool, es_size_t size, es_mfind_e type, const es_int8_t *file, es_int32_t line);

/**
 * calloc
 */
void* eso_mpcalloc_debug(es_mpool_t *mpool, es_size_t size, const es_int8_t *file, es_int32_t line);
void* eso_mpcalloc_ext_debug(es_mpool_t *mpool, es_size_t size, es_mfind_e type, const es_int8_t *file, es_int32_t line);

#define eso_mpmalloc(pool,n)           eso_mpmalloc_debug(pool, n, (const es_int8_t*)__FILE__, __LINE__)
#define eso_mpmalloc_ext(pool,n,t)     eso_mpmalloc_ext_debug(pool, n, t, (const es_int8_t*)__FILE__, __LINE__)
#define eso_mpcalloc(pool,n)           eso_mpcalloc_debug(pool, n, (const es_int8_t*)__FILE__, __LINE__)
#define eso_mpcalloc_ext(pool,n,t)     eso_mpcalloc_ext_debug(pool, n, t, (const es_int8_t*)__FILE__, __LINE__)

#else

/**
 * malloc
 */
void* eso_mpmalloc(es_mpool_t *mpool, es_size_t size);
void* eso_mpmalloc_ext(es_mpool_t *mpool, es_size_t size, es_mfind_e type);

/**
 * calloc
 */
void* eso_mpcalloc(es_mpool_t *mpool, es_size_t size);
void* eso_mpcalloc_ext(es_mpool_t *mpool, es_size_t size, es_mfind_e type);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __ESO_MEMPOOL_H__ */
