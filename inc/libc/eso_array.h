/**
 * @file  eso_array.h
 * @brief ES Array library
 */

#ifndef __ESO_ARRAY_H__
#define __ESO_ARRAY_H__

#include "es_types.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @see es_array_t */
typedef struct es_array_t es_array_t;


/**
 * Create an array
 * @param capacity the number of elements in the initial array
 * @param elt_size The size of each element in the array.
 * @return The new array
 */
es_array_t* eso_array_make(es_int32_t capacity, es_int32_t elt_size);

/**
 * Resize of this array
 * @param arr The array.
 * @param capacity The new size of the array.
 * @return The new array
 */
void eso_array_resize(es_array_t *arr, es_int32_t capacity);

/**
 * Add a new element to an array (as a first-in, last-out stack)
 * @param arr The array to add an element to.
 * @param data The data append to location for the new element in the array
 * @param data_len The data len (if data_len > arr->elt_size then data_len = arr->elt_size)
 * @remark If there are no free spots in the array, then this function will
 *         allocate new space for the new element.
 */
void eso_array_push(es_array_t *arr, void *data, es_int32_t data_len);

/**
 * Remove an element from an array (as a first-in, last-out stack)
 * @param arr The array to remove an element from.
 * @return Location of the element in the array.
 * @remark If there are no elements in the array, NULL is returned.
 */
void* eso_array_pop(es_array_t *arr);

/**
 * Get a element by element's index
 * @param arr The array to get an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 * @return Location of the element in the array.
 * @remark If the nelt is over then arr->nelts, NULL is returned.
*/
void* eso_array_get(es_array_t *arr, es_int32_t nelt);

/**
 * Update a element by element's index
 * @param arr The array to update an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 * @param data The data insert to location for the new element in the array
 * @param data_len The data len (if data_len > arr->elt_size then data_len = arr->elt_size)
 * @remark If the nelt is over then arr->nelts, none is update.
*/
void eso_array_set(es_array_t *arr, es_int32_t nelt, void *data, es_int32_t data_len);

/**
 * Insert a element by element's index
 * @param arr The array to insert an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 * @param data The data insert to location for the new element in the array
 * @param data_len The data len (if data_len > arr->elt_size then data_len = arr->elt_size)
 * @remark If the nelt is over then arr->nelts, none is update.
*/
void eso_array_insert(es_array_t *arr, es_int32_t nelt, void *data, es_int32_t data_len);

/**
 * Remove an element by element's index
 * @param arr The array to remove an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 */
void eso_array_delete(es_array_t *arr, es_int32_t nelt);

/**
 * Clear an array
 * @param arr The array to clear all elements from.
 */
void eso_array_clear(es_array_t *arr);

/**
 * Get the number of elements in the array.
 * @param arr The array
 * @return The number of elements in the array.
 */
es_int32_t eso_array_count(es_array_t *arr);

/**
 * Get the current capacity of elements in the array.
 * @param arr The array
 * @return The capacity of elements in the array.
 */
es_int32_t eso_array_capacity(es_array_t *arr);

/**
 * Determine if the array is empty
 * @param a The array to check
 * @return True if empty, False otherwise
 */
es_bool_t eso_array_is_empty(const es_array_t *arr);

/**
 * Append one array to the end of another.
 * @param dst The destination array, and the one to go first in the combined 
 *            array
 * @param src The source array to add to the destination array
 */
void eso_array_append(es_array_t *dst,
			        const es_array_t *src);

/**
 * Destroy the array and free the memory associated with it.
 * @param ht the array to destroy.
 */
void eso_array_free(es_array_t **arr);


#ifdef __cplusplus
}
#endif

#endif	/* ! __ESO_ARRAY_H__ */
