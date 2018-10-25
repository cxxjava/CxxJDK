/**
 * @file  eso_array_int.c
 * @brief ES Array library only for int data
 */

#include "eso_array_int.h"
#include "eso_util.h"


es_array_t* eso_intarray_make(es_int32_t capacity)
{
	return eso_array_make(capacity, sizeof(es_int32_t));
}

void eso_intarray_resize(es_array_t *arr, es_int32_t capacity)
{
	eso_array_resize(arr, capacity);
}

es_bool_t eso_intarray_is_empty(const es_array_t *arr)
{
    return eso_array_is_empty(arr);
}

void eso_intarray_push(es_array_t *arr, es_int32_t n)
{
	eso_array_push(arr, &n, sizeof(es_int32_t));
}

es_int32_t eso_intarray_pop(es_array_t *arr)
{
    void *val;

    val = eso_array_pop(arr);
    if (val)
    	return (es_int32_t)eso_array2llong((es_byte_t*)val, sizeof(es_int32_t));
    else
    	return 0; //fix it?
}

void eso_intarray_set(es_array_t *arr, es_int32_t nelt, es_int32_t n)
{
    eso_array_set(arr, nelt, &n, sizeof(es_int32_t));
}

es_int32_t eso_intarray_get(es_array_t *arr, es_int32_t nelt)
{
	void *val;

    val = eso_array_get(arr, nelt);
    if (val)
    	return (es_int32_t)eso_array2llong((es_byte_t*)val, sizeof(es_int32_t));
    else
    	return 0; //fix it?
}

void eso_intarray_insert(es_array_t *arr, es_int32_t nelt, es_int32_t n)
{
	eso_array_insert(arr, nelt, &n, sizeof(es_int32_t));
}

void eso_intarray_delete(es_array_t *arr, es_int32_t nelt)
{
	eso_array_delete(arr, nelt);
}

void eso_intarray_clear(es_array_t *arr)
{
	eso_array_clear(arr);
}

es_int32_t eso_intarray_count(es_array_t *arr)
{
	return eso_array_count(arr);
}

void eso_intarray_append(es_array_t *dst,
			       const es_array_t *src)
{
    eso_array_append(dst, src);
}

void eso_intarray_free(es_array_t **arr)
{
	eso_array_free(arr);
}
