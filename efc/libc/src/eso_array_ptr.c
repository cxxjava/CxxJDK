/**
 * @file  eso_array_ptr.c
 * @brief ES Array library only for point data
 */

#include "eso_array_ptr.h"


es_array_t* eso_ptrarray_make(es_int32_t capacity)
{
    return eso_array_make(capacity, sizeof(void*));
}

void eso_ptrarray_resize(es_array_t *arr, es_int32_t capacity)
{
	eso_array_resize(arr, capacity);
}

void eso_ptrarray_push(es_array_t *arr, void *p)
{
	eso_array_push(arr, &p, sizeof(void*));
}

es_bool_t eso_ptrarray_is_empty(const es_array_t *arr)
{
    return eso_array_is_empty(arr);
}

void* eso_ptrarray_pop(es_array_t *arr)
{
    void **ref;

    ref = (void **)eso_array_pop(arr);
    if (ref)
    	return *ref;
    else
    	return NULL;
}

void* eso_ptrarray_get(es_array_t *arr, es_int32_t nelt)
{
	void **ref;

    ref = (void **)eso_array_get(arr, nelt);
    if (ref)
    	return *ref;
    else
    	return NULL;
}

void eso_ptrarray_set(es_array_t *arr, es_int32_t nelt, void *p)
{
    eso_array_set(arr, nelt, &p, sizeof(void*));
}

void eso_ptrarray_insert(es_array_t *arr, es_int32_t nelt, void *p)
{
	eso_array_insert(arr, nelt, &p, sizeof(void*));
}

void eso_ptrarray_delete(es_array_t *arr, es_int32_t nelt)
{
	eso_array_delete(arr, nelt);
}

void eso_ptrarray_clear(es_array_t *arr)
{
	eso_array_clear(arr);
}

es_int32_t eso_ptrarray_count(es_array_t *arr)
{
	return eso_array_count(arr);
}

void eso_ptrarray_append(es_array_t *dst,
			       const es_array_t *src)
{
    eso_array_append(dst, src);
}

void eso_ptrarray_free(es_array_t **arr)
{
	eso_array_free(arr);
}
