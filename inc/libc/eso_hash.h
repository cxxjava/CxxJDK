/**
 * @file  eso_hash.c
 * @brief EM Hash Tables
 */

#ifndef __ESO_HASH_H__
#define __ESO_HASH_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * When passing a key to eso_hash_set or eso_hash_get, this value can be
 * passed to indicate a string-valued key, and have eso_hash compute the
 * length automatically.
 *
 * @remark eso_hash will use strlen(key) for the length. The NUL terminator
 *         is not included in the hash value (why throw a constant in?).
 *         Since the hash table merely references the provided key (rather
 *         than copying it), eso_hash_this() will return the NUL-term'd key.
 */
#define ES_HASH_KEY_STRING     (-1)

/**
 * Abstract type for hash tables.
 */
typedef struct es_hash_t es_hash_t;

/**
 * Abstract type for scanning hash tables.
 */
typedef struct es_hash_index_t es_hash_index_t;

/**
 * Callback functions for calculating hash values.
 * @param key The key.
 * @param klen The length of the key, or ES_HASH_KEY_STRING to use the string 
 *             length. If ES_HASH_KEY_STRING then returns the actual key length.
 */
typedef es_uint32_t es_hashfunc_t(const char *key, es_ssize_t *klen);

/**
 * Create a hash table.
 * @param initcapacity The specified initial capacity
 * @param hashfunc The callback functions for calculating hash values, 
 * if NULL then use the default internal function
 * @return The hash table just created
 */
es_hash_t* eso_hash_make(es_uint32_t initcapacity, es_hashfunc_t *hashfunc);

/**
 * Associate a value with a key in a hash table.
 * @param ht The hash table
 * @param key Pointer to the key
 * @param klen Length of the key. Can be ES_HASH_KEY_STRING to use the string length.
 * @param val Value to associate with the key
 * @remark If the value is NULL the hash entry is deleted.
 * @return The old value.
 */
const void* eso_hash_set(es_hash_t *ht, const void *key,
                               es_ssize_t klen, const void *val);

/**
 * Look up the value associated with a key in a hash table.
 * @param ht The hash table
 * @param key Pointer to the key
 * @param klen Length of the key. Can be ES_HASH_KEY_STRING to use the string length.
 * @return Returns NULL if the key is not present.
 */
const void* eso_hash_get(es_hash_t *ht, const void *key,
                                 es_ssize_t klen);

/**
 * Start iterating over the entries in a hash table. non-thread-safe iterator is used.
 * @param ht The hash table
 * @remark  There is no restriction on adding or deleting hash entries during
 * an iteration (although the results may be unpredictable unless all you do
 * is delete the current entry) and multiple iterations can be in
 * progress at the same time.

 * @example
 */
/**
 * <PRE>
 * 
 * int sum_values(es_hash_t *ht)
 * {
 *     es_hash_index_t *hi;
 *     void *val;
 *     int sum = 0;
 *     for (hi = eso_hash_first(ht); hi; hi = eso_hash_next(hi)) {
 *         eso_hash_this(hi, NULL, NULL, &val);
 *         sum += *(int *)val;
 *     }
 *     return sum;
 * }
 * </PRE>
 */
es_hash_index_t* eso_hash_first(es_hash_t *ht);

/** 
 * Continue iterating over the entries in a hash table.
 * @param hi The iteration state
 * @return a pointer to the updated iteration state.  NULL if there are no more  
 *         entries.
 */
es_hash_index_t* eso_hash_next(es_hash_index_t *hi);

/**
 * Get the current entry's details from the iteration state.
 * @param hi The iteration state
 * @param key Return pointer for the pointer to the key.
 * @param klen Return pointer for the key length.
 * @param val Return pointer for the associated value.
 * @remark The return pointers should point to a variable that will be set to the
 *         corresponding data, or they may be NULL if the data isn't interesting.
 */
void eso_hash_this(es_hash_index_t *hi, void **key,
                                es_ssize_t *klen, void **val);

/**
 * Get the number of key/value pairs in the hash table.
 * @param ht The hash table
 * @return The number of key/value pairs in the hash table.
 */
es_uint32_t eso_hash_count(es_hash_t *ht);

/**
 * Clear all items in the hash table.
 * @param ht The hash table
 */
void eso_hash_clear(es_hash_t *ht, void (*freeCall)(const void* value));

/**
 * Destroy the hash and free the memory associated with it.
 * @remark call eso_hash_clear() first to free value.
 * @param ht the hash to destroy.
 */
void eso_hash_free(es_hash_t **ht);

#ifdef __cplusplus
}
#endif

#endif	/* !__ESO_HASH_H__ */
