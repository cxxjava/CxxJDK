/**
 * @@file  eso_vector.h
 * @@brief ES Vector library
 */


#ifndef __ESO_VECTOR_H__
#define __ESO_VECTOR_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_vector_t es_vector_t;
typedef void es_vector_autofree_t (void *obj);

/**
 * Create a vector.
 */
es_vector_t* eso_vector_create(es_vector_autofree_t *autofree_func, es_int32_t capacity);

/**
 * Destroy a vector.
 */
void eso_vector_destroy(es_vector_t **vector);

/**
 * Set a vector auto free function.
 */
void eso_vector_set_autofree_func(es_vector_t *vector, es_vector_autofree_t *autofree_func);

void eso_vector_add_element(es_vector_t *vector, const void* obj);
es_bool_t eso_vector_contains(es_vector_t *vector, const void* obj);
void* eso_vector_element_at(es_vector_t *vector, es_int32_t index);
void eso_vector_insert_element_at(es_vector_t *vector, const void* obj, es_int32_t index);
es_bool_t eso_vector_is_empty(es_vector_t *vector);
void eso_vector_remove_all_elements(es_vector_t *vector);
es_bool_t eso_vector_remove_element(es_vector_t *vector, const void* obj);
void eso_vector_remove_element_at(es_vector_t *vector, es_int32_t index);
void eso_vector_set_element_at(es_vector_t *vector, const void* obj, es_int32_t index);
void eso_vector_resize(es_vector_t *vector, es_int32_t newSize);
es_int32_t eso_vector_size(es_vector_t *vector);
es_int32_t eso_vector_indexof(es_vector_t *vector, const void* obj);
es_int32_t eso_vector_indexof_from_index(es_vector_t *vector, const void* obj, es_int32_t index);
es_int32_t eso_vector_last_indexof(es_vector_t *vector, const void* obj);
es_int32_t eso_vector_last_indexof_from_index(es_vector_t *vector, const void* obj, es_int32_t index);

es_vector_t* eso_vector_concat(es_vector_t* vector, es_vector_t* other);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_VECTOR_H__ */
