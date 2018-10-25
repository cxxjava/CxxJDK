/**
 * @file  eso_array.c
 * @brief ES Array library
 */

#include "eso_array.h"
#include "eso_libc.h"
#include "eso_mpool.h"

#define ES_ARRAY_PREMALLOC_SIZE    32

/** An opaque array type */
struct es_array_t {
    /** The amount of memory allocated for each element of the array */
    es_int32_t elt_size;
    /** The number of active elements in the array */
    es_int32_t nelts;
    /** The number of elements allocated in the array */
    es_int32_t nalloc;
    /** The elements in the array */
    char *elts;
};

static void make_array_core(es_array_t *res,
				es_int32_t nelts, es_int32_t elt_size, es_bool_t clear)
{
	/*
	 * Assure sanity if someone asks for
	 * array of zero elts.
	 */
	if (nelts < 1) {
		nelts = 1;
	}

	if (clear) {
		res->elts = (char*)eso_calloc(nelts * elt_size);
	}
	else {
		res->elts = (char*)eso_malloc(nelts * elt_size);
	}

	res->elt_size = elt_size;
	res->nelts = 0;			/* No active elements yet... */
	res->nalloc = nelts;	/* ...but this many allocated */
}

es_array_t* eso_array_make(es_int32_t capacity, es_int32_t elt_size)
{
	es_array_t *arr;

	arr = (es_array_t*)eso_malloc(sizeof(es_array_t));
	make_array_core(arr, capacity, elt_size, 1);
	return arr;
}

void eso_array_resize(es_array_t *arr, es_int32_t capacity)
{
	if (capacity > arr->nelts) {
		char *new_data;
		char *old_data;

		new_data = (char*)eso_calloc(arr->elt_size * capacity);
		old_data = arr->elts;

		eso_memmove(new_data, arr->elts, arr->nalloc * arr->elt_size);
		arr->elts = new_data;
		arr->nalloc = capacity;

		//free old elts
		eso_free(old_data);
	}
}

void eso_array_push(es_array_t *arr, void *data, es_int32_t data_len)
{
	void *el;

	if (arr->nelts == arr->nalloc) {
		eso_array_resize(arr, (arr->nalloc <= 0) ? 1 : arr->nalloc + ES_ARRAY_PREMALLOC_SIZE);
	}

	++arr->nelts;
	el = arr->elts + (arr->elt_size * (arr->nelts - 1));
	eso_memcpy(el, data, data_len > arr->elt_size ? arr->elt_size : data_len);
}

es_bool_t eso_array_is_empty(const es_array_t *arr)
{
	return ((arr == NULL) || (arr->nelts == 0));
}

void* eso_array_pop(es_array_t *arr)
{
	if (eso_array_is_empty(arr)) {
		return NULL;
	}
	return arr->elts + (arr->elt_size * (--arr->nelts));
}

void* eso_array_get(es_array_t *arr, es_int32_t nelt)
{
	if (eso_array_is_empty(arr)
		|| nelt >= arr->nelts) {
		return NULL;
	}
	return arr->elts + (arr->elt_size * nelt);
}

void eso_array_set(es_array_t *arr, es_int32_t nelt, void *data, es_int32_t data_len)
{
	void *el;

	if (nelt >= arr->nelts) {
		return;
	}

	el = arr->elts + (arr->elt_size * nelt);
	eso_memset(el, 0, arr->elt_size);
	eso_memcpy(el, data, data_len > arr->elt_size ? arr->elt_size : data_len);
}

void eso_array_insert(es_array_t *arr, es_int32_t nelt, void *data, es_int32_t data_len)
{
	if (nelt > arr->nelts)
		nelt = arr->nelts;

	if (arr->nelts == arr->nalloc) {
		eso_array_resize(arr, (arr->nalloc <= 0) ? 1 : arr->nalloc + ES_ARRAY_PREMALLOC_SIZE);
	}

	eso_memmove(arr->elts + (nelt + 1) * arr->elt_size,
			arr->elts + nelt * arr->elt_size,
			(arr->nelts - nelt) * arr->elt_size);
	eso_memset(arr->elts + nelt * arr->elt_size, 0, arr->elt_size);
	eso_memcpy(arr->elts + nelt * arr->elt_size,
			data,
			data_len > arr->elt_size ? arr->elt_size : data_len);

	++arr->nelts;
}

void eso_array_delete(es_array_t *arr, es_int32_t nelt)
{
	if (nelt >= arr->nelts)
		return;

	if (nelt == (arr->nelts - 1)) {
		eso_memset(arr->elts + nelt * arr->elt_size, 0, arr->elt_size);
		--arr->nelts;
	}
	else {
		eso_memmove(arr->elts + nelt * arr->elt_size,
				arr->elts + (nelt + 1) * arr->elt_size,
				(arr->nelts - nelt -1) * arr->elt_size);
		--arr->nelts;
		eso_memset(arr->elts + arr->nelts * arr->elt_size, 0, arr->elt_size);
	}
}

void eso_array_clear(es_array_t *arr)
{
	arr->nelts = 0;
}

es_int32_t eso_array_count(es_array_t *arr)
{
	return arr->nelts;
}

es_int32_t eso_array_capacity(es_array_t *arr)
{
	return arr->nalloc;
}

void eso_array_append(es_array_t *dst,
				   const es_array_t *src)
{
	int elt_size;

	if (!dst || !src) return;

	elt_size = dst->elt_size;
	if (dst->nelts + src->nelts > dst->nalloc) {
		eso_array_resize(dst, dst->nelts + src->nelts + ES_ARRAY_PREMALLOC_SIZE);
	}

	eso_memcpy(dst->elts + dst->nelts * elt_size, src->elts, elt_size * src->nelts);
	dst->nelts += src->nelts;
}

void eso_array_free(es_array_t **arr)
{
	if (!arr || !*arr)
		return;

	eso_free((*arr)->elts);
	eso_free(*arr);
	*arr = NULL;
}
