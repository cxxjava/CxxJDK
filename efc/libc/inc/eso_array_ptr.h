/**
 * @file  eso_array_ptr.h
 * @brief ES Array library only for point data
 */

#ifndef __ESO_PTRARRAY_H__
#define __ESO_PTRARRAY_H__

#include "eso_array.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Create an point array
 * @param capacity the number of elements in the initial array
 * @return The new array
 */
es_array_t* eso_ptrarray_make(es_int32_t capacity);

/**
 * Resize of this array
 * @param arr The array.
 * @param capacity The new size of the array.
 * @return The new array
 */
void eso_ptrarray_resize(es_array_t *arr, es_int32_t capacity);

/**
 * Add a new element to an array (as a first-in, last-out stack)
 * @param arr The array to add an element to.
 * @param p The data point append to location for the new element in the array
 * @remark If there are no free spots in the array, then this function will
 *         allocate new space for the new element.
 */
void eso_ptrarray_push(es_array_t *arr, void *p);

/**
 * Remove an element from an array (as a first-in, last-out stack)
 * @param arr The array to remove an element from.
 * @return Location of the element in the array.
 * @remark If there are no elements in the array, NULL is returned.
 */
void* eso_ptrarray_pop(es_array_t *arr);

/**
 * Get a element by element's index
 * @param arr The array to get an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 * @return Location of the element in the array.
 * @remark If the nelt is over then arr->nelts, NULL is returned.
*/
void* eso_ptrarray_get(es_array_t *arr, es_int32_t nelt);

/**
 * Update a element by element's index
 * @param arr The array to update an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 * @param p The data point insert to location for the new element in the array
 * @remark If the nelt is over then arr->nelts, none is update.
*/
void eso_ptrarray_set(es_array_t *arr, es_int32_t nelt, void *p);

/**
 * Insert a element by element's index
 * @param arr The array to insert an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 * @param p The data point insert to location for the new element in the array
 * @remark If the nelt is over then arr->nelts, none is update.
*/
void eso_ptrarray_insert(es_array_t *arr, es_int32_t nelt, void *p);

/**
 * Remove an element by element's index
 * @param arr The array to remove an element from.
 * @param nelt The element's index (begin 0 to arr->nelts-1)
 */
void eso_ptrarray_delete(es_array_t *arr, es_int32_t nelt);

/**
 * Clear an array
 * @param arr The array to clear all elements from.
 */
void eso_ptrarray_clear(es_array_t *arr);

/**
 * Get the number of elements in the array.
 * @param ht The array
 * @return The number of elements in the array.
 */
es_int32_t eso_ptrarray_count(es_array_t *arr);

/**
 * Determine if the array is empty
 * @param a The array to check
 * @return True if empty, False otherwise
 */
es_bool_t eso_ptrarray_is_empty(const es_array_t *arr);

/**
 * Append one array to the end of another.
 * @param dst The destination array, and the one to go first in the combined 
 *            array
 * @param src The source array to add to the destination array
 */
void eso_ptrarray_append(es_array_t *dst,
			        const es_array_t *src);

/**
 * Destroy the array and free the memory associated with it.
 * @param ht the array to destroy.
 */
void eso_ptrarray_free(es_array_t **arr);


#ifdef __cplusplus
}
#endif

#endif	/* ! __ESO_PTRARRAY_H__ */
