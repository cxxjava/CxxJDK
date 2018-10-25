/*
 * eso_flip_buffer.c
 *
 *  Created on: 2015-1-12
 *      Author: cxxjava@163.com
 */

#include "eso_flip_buffer.h"
#include "eso_libc.h"

es_flip_buffer_t* eso_flip_buffer_create(es_ulong_t capacity, void* address, es_ulong_t offset) {
	es_flip_buffer_t *buffer;

	if (address && offset > capacity) {
		return NULL;
	}

	buffer = (es_flip_buffer_t*)eso_calloc(sizeof(es_flip_buffer_t));

	if (address) {
		buffer->_position = offset;
		buffer->_limit = capacity;
		buffer->_capacity = capacity;
		buffer->_defaultAllocated = FALSE;
		buffer->_address = (char*)address + offset;
	}
	else {
		buffer->_position = 0;
		buffer->_limit = capacity;
		buffer->_capacity = capacity;
		buffer->_defaultAllocated = TRUE;
		buffer->_address = eso_malloc(capacity);
	}

	return buffer;
}

void eso_flip_buffer_free(es_flip_buffer_t** buffer) {
	if (!buffer || !(*buffer))
		return;
	if ((*buffer)->_defaultAllocated) {
		eso_free((*buffer)->_address);
	}
	ESO_FREE(buffer);
}

es_ulong_t eso_flip_buffer_capacity(es_flip_buffer_t* buffer)
{
	return buffer->_capacity;
}

es_ulong_t eso_flip_buffer_position(es_flip_buffer_t* buffer)
{
	return buffer->_position;
}

es_flip_buffer_t* eso_flip_buffer_position_set(es_flip_buffer_t* buffer, es_ulong_t newPosition)
{
	if (newPosition > buffer->_limit)
		return NULL;
	buffer->_position = newPosition;
	return buffer;
}

es_ulong_t eso_flip_buffer_limit(es_flip_buffer_t* buffer)
{
	return buffer->_limit;
}

es_flip_buffer_t* eso_flip_buffer_limit_set(es_flip_buffer_t* buffer, es_ulong_t newLimit)
{
	if (newLimit > buffer->_capacity)
		return NULL;
	buffer->_limit = newLimit;
	if (buffer->_position > buffer->_limit)
		buffer->_position = buffer->_limit;
	return buffer;
}

es_flip_buffer_t* eso_flip_buffer_clear(es_flip_buffer_t* buffer)
{
	buffer->_position = 0;
	buffer->_limit = buffer->_capacity;
	return buffer;
}

es_flip_buffer_t* eso_flip_buffer_flip(es_flip_buffer_t* buffer)
{
	buffer->_limit = buffer->_position;
	buffer->_position = 0;
	return buffer;
}

es_flip_buffer_t* eso_flip_buffer_rewind(es_flip_buffer_t* buffer)
{
	buffer->_position = 0;
	return buffer;
}

es_long_t eso_flip_buffer_remaining(es_flip_buffer_t* buffer)
{
	return buffer->_limit - buffer->_position;
}

es_bool_t eso_flip_buffer_has_remaining(es_flip_buffer_t* buffer)
{
	return buffer->_position < buffer->_limit;
}

void* eso_flip_buffer_base_address(es_flip_buffer_t* buffer)
{
	return buffer->_address;
}

void* eso_flip_buffer_current_address(es_flip_buffer_t* buffer)
{
	return buffer->_address ? ((char*)buffer->_address + buffer->_position) : NULL;
}

static es_ulong_t nextGetIndex(es_flip_buffer_t* buffer, int nb) {
	es_ulong_t p = buffer->_position;
	ES_ASSERT (buffer->_limit - buffer->_position >= nb);
	buffer->_position += nb;
	return p;
}

es_ulong_t eso_flip_buffer_get(es_flip_buffer_t* buffer, void* to_addr, es_ulong_t length)
{
	char *p = (char*)eso_flip_buffer_current_address(buffer);
	length = ES_MIN(length, buffer->_limit - buffer->_position);
	eso_memcpy(to_addr, p, length);
	buffer->_position += length;
	return length;
}
int eso_flip_buffer_get_byte(es_flip_buffer_t* buffer, es_byte_t* byte)
{
	if (buffer->_position + 1 > buffer->_limit) {
		return 0;
	}
	if (byte) {
		*byte = *((es_byte_t*)buffer->_address + buffer->_position);
	}
	buffer->_position++;
	return 1;
}

int eso_flip_buffer_get_byte_at(es_flip_buffer_t* buffer, es_ulong_t index, es_byte_t* byte)
{
	if (index >= buffer->_limit) {
		return 0;
	}
	if (byte) {
		*byte = *((es_byte_t*)buffer->_address + index);
	}
	return 1;
}

es_flip_buffer_t* eso_flip_buffer_put(es_flip_buffer_t* buffer, const void* from_addr, es_ulong_t length)
{
	if (buffer->_position + length > buffer->_limit)
		return NULL;
	eso_memcpy((char*)eso_flip_buffer_current_address(buffer), from_addr, length);
	buffer->_position += length;
	return buffer;
}

es_flip_buffer_t* eso_flip_buffer_put_byte(es_flip_buffer_t* buffer, es_byte_t b)
{
    if (buffer->_position + 1 > buffer->_limit)
		return NULL;
	*((es_byte_t*)buffer->_address + nextGetIndex(buffer, 1)) = b;
	return buffer;
}

es_flip_buffer_t* eso_flip_buffer_put_byte_at(es_flip_buffer_t* buffer, es_ulong_t index, es_byte_t b)
{
	if (index >= buffer->_limit)
		return NULL;
	*((es_byte_t*)buffer->_address + index) = b;
	return buffer;
}
