/**
 * @@file  eso_vector.c
 * @@brief ES Vector library
 */

#include "eso_vector.h"
#include "eso_array_ptr.h"
#include "eso_libc.h"

struct es_vector_t
{
	es_array_t*             arrayBuffer;
	es_vector_autofree_t*   autoFree;
};

es_vector_t* eso_vector_create(es_vector_autofree_t *autofree_func, es_int32_t capacity)
{
	es_vector_t* vector = (es_vector_t*)eso_calloc(sizeof(es_vector_t));
	vector->arrayBuffer = eso_ptrarray_make(capacity);
	vector->autoFree = autofree_func;
	return vector;
}

void eso_vector_destroy(es_vector_t **vector)
{
	if (!vector || !*vector) {
		return;
	}
	if ((*vector)->autoFree) {
		eso_vector_remove_all_elements(*vector);
	}
	eso_ptrarray_free(&(*vector)->arrayBuffer);
	ESO_FREE(vector);
}

void eso_vector_set_autofree_func(es_vector_t *vector, es_vector_autofree_t *autofree_func)
{
	vector->autoFree = autofree_func;
}

void eso_vector_add_element(es_vector_t *vector, const void* obj)
{
	eso_ptrarray_push(vector->arrayBuffer, (void*)obj);
}

es_bool_t eso_vector_contains(es_vector_t *vector, const void* obj)
{
	return eso_vector_indexof_from_index(vector, obj, 0) >= 0;
}

void eso_vector_remove_all_elements(es_vector_t *vector)
{
	if (vector->autoFree) {
		int i;
		for (i=0; i<eso_vector_size(vector); i++) {
			void *pObj = eso_ptrarray_get(vector->arrayBuffer, i);
			vector->autoFree(pObj);
		}
	}
	eso_ptrarray_clear(vector->arrayBuffer);
}

es_int32_t eso_vector_indexof(es_vector_t *vector, const void* obj)
{
	return eso_vector_indexof_from_index(vector, obj, 0);
}

es_int32_t eso_vector_indexof_from_index(es_vector_t *vector, const void* obj, es_int32_t index)
{
	int i;
	if (obj == NULL) {
		for (i = index; i < eso_vector_size(vector); i++)
			if (eso_ptrarray_get(vector->arrayBuffer, i) == NULL)
				return i;
	}
	else {
		for (i = index; i < eso_vector_size(vector); i++)
			if ((void *)eso_ptrarray_get(vector->arrayBuffer, i) == obj)
				return i;
	}
	return -1;
}

es_int32_t eso_vector_size(es_vector_t *vector)
{
	return eso_ptrarray_count(vector->arrayBuffer);
}

void* eso_vector_element_at(es_vector_t *vector, es_int32_t index)
{
	return (void *)eso_ptrarray_get(vector->arrayBuffer, index);
}

void eso_vector_insert_element_at(es_vector_t *vector, const void* obj, es_int32_t index)
{
	eso_ptrarray_insert(vector->arrayBuffer, index, (void *)obj);
}

es_bool_t eso_vector_is_empty(es_vector_t *vector)
{
	return (!vector || eso_ptrarray_is_empty(vector->arrayBuffer));
}

es_bool_t eso_vector_remove_element(es_vector_t *vector, const void* obj)
{
	int i = eso_vector_indexof(vector, obj);
	if (i >= 0) {
		eso_vector_remove_element_at(vector, i);
		return TRUE;
	}
	return FALSE;
}

void eso_vector_remove_element_at(es_vector_t *vector, es_int32_t index)
{
	if (vector->autoFree) {
		void *pObj = (void *)eso_ptrarray_get(vector->arrayBuffer, index);
		if (pObj) {
			vector->autoFree(pObj);
		}
	}
	eso_ptrarray_delete(vector->arrayBuffer, index);
}

void eso_vector_set_element_at(es_vector_t *vector, const void* obj, es_int32_t index)
{
	if (vector->autoFree) {
		void *pObj = (void *)eso_ptrarray_get(vector->arrayBuffer, index);
		if (pObj) {
			vector->autoFree(pObj);
		}
	}
	eso_ptrarray_set(vector->arrayBuffer, index, (void *)obj);
}

void eso_vector_resize(es_vector_t *vector, es_int32_t newSize)
{
	if (newSize > eso_vector_size(vector)) {
		if (vector->autoFree) {
			int i;
			for (i=newSize; i<eso_vector_size(vector); i++) {
				void *pObj = (void *)eso_ptrarray_get(vector->arrayBuffer, i);
				if (pObj) {
					vector->autoFree(pObj);
				}
			}
		}
		eso_ptrarray_resize(vector->arrayBuffer, newSize);
	}
}

es_int32_t eso_vector_last_indexof(es_vector_t *vector, const void* obj)
{
	return eso_vector_last_indexof_from_index(vector, obj, eso_vector_size(vector)-1);
}

es_int32_t eso_vector_last_indexof_from_index(es_vector_t *vector, const void* obj, es_int32_t index)
{
	int i;

	if (index < 0 || index >= eso_vector_size(vector)) return -1;
	
	if (obj == NULL) {
		for (i = index; i >= 0; i--)
			if (eso_ptrarray_get(vector->arrayBuffer, i) == NULL)
				return i;
	}
	else {
		for (i = index; i >= 0; i--)
			if ((void *)eso_ptrarray_get(vector->arrayBuffer, i) == obj)
				return i;
	}
	return -1;
}

es_vector_t* eso_vector_concat(es_vector_t* vector, es_vector_t* other)
{
	if (other) {
		eso_ptrarray_append(vector->arrayBuffer, other->arrayBuffer);
	}
	return vector;
}
