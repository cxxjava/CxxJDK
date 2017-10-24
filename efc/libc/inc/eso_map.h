/**
 * @@file  eso_map.h
 * @@brief ES map (map table)
 */


#ifndef __ESO_MAP_H__
#define __ESO_MAP_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * opaque structure
 */
typedef struct es_map_t es_map_t;

/**
 * item element structure
 */
typedef struct es_map_elem_t es_map_elem_t;

/**
 * free item data callback
 */
typedef void es_map_free_t(void *data);

/** 
 * create a map
 * @returns The new map
 */
es_map_t* eso_map_create(es_map_free_t *free);

/**
 * put/add a key-object to the map
 *
 * @param map the map table
 * @param key the map table key
 * @param data the map table data
 */
es_int32_t eso_map_set(es_map_t *map, es_int32_t key, const void *data);

/**
 * get an object from the map
 *
 * @param map the map table
 * @param key the map table key
 * @returns the map table data
 */
void* eso_map_get(es_map_t *map, es_int32_t key);

/**
 * returns the size of the map.
 *
 * @param map the map table
 * @returns the size of the map
 */
es_int32_t eso_map_size(es_map_t *map);

/**
 * Destroy a map
 * @param t The map to destroy
 */
void eso_map_destroy(es_map_t **map);

es_map_elem_t* eso_map_first(es_map_t *map);

es_map_elem_t* eso_map_next(es_map_t *map, es_map_elem_t *mi);

void eso_map_this(es_map_elem_t *mi, es_int32_t *key, void **val);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_MAP_H__ */
