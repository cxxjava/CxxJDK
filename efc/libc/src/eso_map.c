/**
 * @@file  es_map.c
 * @@brief ES map library
 */

#include "eso_map.h"
#include "eso_ring.h"
#include "eso_libc.h"

struct es_map_elem_t {
    ES_RING_ENTRY(es_map_elem_t) link;
    es_int32_t key;
    void *data;
};

ES_RING_HEAD(es_map_t, es_map_elem_t);


/** 
 * create a map
 * @returns The new map
 */
es_map_t* eso_map_create(es_map_free_t *free)
{
	es_map_t *map = NULL;
	
	map = eso_calloc(sizeof(es_map_t));
	ES_RING_INIT(map, es_map_elem_t, link);
	if (free) {
		map->data = (long)free;
	}
	else {
		map->data = 0;
	}
	
	return map;
}


/**
 * Destroy a map
 * @param t The map to destroy
 */
void eso_map_destroy(es_map_t **map)
{
	es_map_elem_t *ep, *eptmp;
	es_map_t *this;
	es_map_free_t *free_ = NULL;
	
	if (!map || !(*map))
		return;
	
	this = *map;
	
	if (!ES_RING_EMPTY(this, es_map_elem_t, link)) {
        ep = ES_RING_FIRST(this);
        while (ep != ES_RING_SENTINEL(this, es_map_elem_t, link)) {
			eptmp = ES_RING_NEXT(ep, link);
			if (this->data > 0) {
				free_ = (es_map_free_t*)this->data;
				free_(ep->data);
			}
			ES_RING_REMOVE(ep, link);
			eso_free(ep);
			ep = eptmp;
		}
    }
	
	eso_free(this);
	*map = NULL;
}

/**
 * put/add a key-object to the map
 *
 * @param map the map table
 * @param key the map table key
 * @param data the map table data, if NULL then delete the mapped apair
 * @returns 0-success, other is fail.
 */
es_int32_t eso_map_set(es_map_t *map, es_int32_t key, const void *data)
{
	es_map_elem_t *ep;
	
	if (data) { //add
		//Old先删除
		eso_map_set(map, key, NULL);
		
		//New后添加
		ep = (es_map_elem_t *)eso_malloc(sizeof(es_map_elem_t));
    	ES_RING_ELEM_INIT(ep, link);
    	
    	ep->key = key;
    	ep->data = (void *)data;
    	ES_RING_INSERT_TAIL(map, ep, es_map_elem_t, link);
    }
    else { //del
    	if (ES_RING_EMPTY(map, es_map_elem_t, link))
			return 0;
		
		ep = ES_RING_FIRST(map);
		while (ep != ES_RING_SENTINEL(map, es_map_elem_t, link)) {
			if (ep->key == key) {
				if (map->data > 0) {
					es_map_free_t *free;
					free = (es_map_free_t*)map->data;
					free(ep->data);
				}
				ES_RING_REMOVE(ep, link);
	    		eso_free(ep);
	    		break;
	    	}
			ep = ES_RING_NEXT(ep, link);
	    }
    }
    
    return 0;
}

/**
 * get an object from the map
 *
 * @param map the map table
 * @param key the map table key
 * @returns the map table data
 */
void* eso_map_get(es_map_t *map, es_int32_t key)
{
	es_map_elem_t *ep;
	void *data = NULL;
	
	if (!ES_RING_EMPTY(map, es_map_elem_t, link)) {
        ep = ES_RING_FIRST(map);
        while (ep != ES_RING_SENTINEL(map, es_map_elem_t, link)) {
			if (ep->key == key) {
				data = ep->data;
				break;
			}
			ep = ES_RING_NEXT(ep, link);
		}
    }
    
    return data;
}

/**
 * returns the size of the map.
 *
 * @param map the map
 * @returns the size of the map
 */
es_int32_t eso_map_size(es_map_t *map)
{
	es_int32_t n = 0;
	es_map_elem_t *ep;
	
	ep = ES_RING_FIRST(map);
    while (ep != ES_RING_SENTINEL(map, es_map_elem_t, link)) {
		n++;
		ep = ES_RING_NEXT(ep, link);
	}
	
	return n;
}

es_map_elem_t* eso_map_first(es_map_t *map)
{
	if (ES_RING_EMPTY(map, es_map_elem_t, link))
		return NULL;
	
	return ES_RING_FIRST(map);
}

es_map_elem_t* eso_map_next(es_map_t *map, es_map_elem_t *mi)
{
	mi = ES_RING_NEXT(mi, link);
	if (mi != ES_RING_SENTINEL(map, es_map_elem_t, link))
		return mi;
	else
		return NULL;
}

void eso_map_this(es_map_elem_t *mi, es_int32_t *key, void **val)
{
	if (mi) {
		if (key) *key = mi->key;
		if (val) *val = mi->data;
	}
}
