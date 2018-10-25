/**
 * @file  eso_buffer.c
 * @brief ES buffer string functions
 */

#include "eso_buffer.h"
#include "eso_libc.h"

#define ES_BUFFER_PREMALLOC_SIZE    32

static es_buffer_t* realloc_buffer(es_buffer_t *buffer, es_int32_t shift_size)
{
	es_int32_t free_len;
	
	free_len = buffer->capacity - buffer->len;
	if (shift_size >= free_len) {
		buffer->capacity = buffer->len + shift_size + buffer->reserved;
		buffer->data = eso_realloc(buffer->data, buffer->capacity);
		if (!buffer->data) {
			buffer->len = 0;
			return NULL;
		}
	}
	
	return buffer;
}

/*
 * create one buffer
*/
es_buffer_t* eso_buffer_make(es_size_t size, es_size_t reserved)
{
	es_buffer_t *new_buffer;
	es_size_t n;
	
	new_buffer = (es_buffer_t *)eso_malloc(sizeof(es_buffer_t));
	if (!new_buffer)
		return NULL;
	
	n = (size > 0 ? size : 32); //20171113: 1->32
	new_buffer->data = eso_malloc(n);
	new_buffer->len = 0;
	new_buffer->capacity = n;
	new_buffer->reserved = (reserved > ES_BUFFER_PREMALLOC_SIZE ? reserved : ES_BUFFER_PREMALLOC_SIZE);
	
	return new_buffer;
}

/*
 * zero the sized buffer's data with offset
 */
es_buffer_t* eso_buffer_zero(es_buffer_t *buffer,
                             es_size_t off, es_size_t size)
{
	buffer = realloc_buffer(buffer, off+size);
	buffer->len = ES_MAX(buffer->len, off+size);
	eso_memset((char *)buffer->data+off, 0, size);
	return buffer;
}

/*
 * set new capacity
*/
void eso_buffer_resize(es_buffer_t *buffer, es_size_t newsize)
{
	es_int32_t shift_size;
	
	if (newsize <= buffer->len)
		return;
	
	shift_size = newsize - buffer->capacity;
	if (shift_size > 0) {
		realloc_buffer(buffer, shift_size);
	}
	else if (shift_size < (es_int32_t)(0-buffer->reserved)) {
		void *old_data = buffer->data;
		buffer->capacity = newsize;
		buffer->data = eso_malloc(newsize);
		eso_memcpy(buffer->data, old_data, buffer->len);
		eso_free(old_data);
	}
}

/*
 * append one data to buffer
*/
es_buffer_t* eso_buffer_append(es_buffer_t *buffer,
                               const void *data, es_size_t size)
{
	buffer = realloc_buffer(buffer, size);
	eso_memcpy((char *)buffer->data + buffer->len, data, size);
	buffer->len += size;
	
	return buffer;
}

/*
 * clear buffer
*/
es_buffer_t* eso_buffer_clear(es_buffer_t *buffer)
{
	buffer->len = 0;
	
	return buffer;
}

/*
 * insert data to buffer
*/
es_buffer_t* eso_buffer_insert(es_buffer_t *buffer, es_int32_t pos,
                               const void *data, es_size_t size)
{
	if ((es_size_t)pos > buffer->len) {
		return NULL;
	}
	realloc_buffer(buffer, size);
	eso_memmove((char *)buffer->data+pos+size, (char *)buffer->data+pos, buffer->len-pos);
	eso_memcpy((char *)buffer->data+pos, data, size);
	buffer->len += size;

	return buffer;
}

/*
 * update buffer's data
*/
es_buffer_t* eso_buffer_update(es_buffer_t *buffer, 
                                           es_int32_t pos, es_size_t old_size,
                                           const void *data, es_size_t new_size)
{
	es_int32_t ninc;
	
	if ((es_size_t)pos > buffer->len) {
		return NULL;
	}
	
	ninc = new_size - old_size;
	if (ninc > 0) {
		realloc_buffer(buffer, ninc);
	}
	
	eso_memmove((char *)buffer->data + pos + new_size, 
	        (char *)buffer->data + pos + old_size, 
	        buffer->len - pos - old_size);
	if (new_size > 0) {
		eso_memcpy((char *)buffer->data + pos, data, new_size);
	}
	buffer->len += ninc;
	
	return buffer;
}

/*
 * delete data buffer
*/
es_buffer_t* eso_buffer_delete(es_buffer_t *buffer, es_int32_t pos, es_size_t size)
{
	if ((es_size_t)pos >= buffer->len) {
		return buffer;
	}
	
	if (size >= buffer->len-pos) {
		buffer->len = pos;
		return buffer;
	}
	
	eso_memmove((char *)buffer->data+pos, (char *)buffer->data+pos+size, buffer->len-pos-size);
	buffer->len -= size;
	
	return buffer;
}

/*
 * Destroy one buffer
*/
void eso_buffer_free(es_buffer_t **buffer)
{
	if (!buffer || !*buffer)
		return;
	eso_free((*buffer)->data);
	eso_free(*buffer);
	*buffer = NULL;
}
