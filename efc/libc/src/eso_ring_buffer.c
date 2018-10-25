/*
 * eso_ring_buffer.c
 *
 *  Created on: 2015-1-12
 *      Author: cxxjava@163.com
 */

#include "eso_ring_buffer.h"
#include "eso_libc.h"

es_ring_buffer_t* eso_ring_buffer_create(es_ulong_t capacity, void* address, es_ulong_t offset) {
	es_ring_buffer_t *buffer;

	if (address && offset > capacity) {
		return NULL;
	}

	buffer = (es_ring_buffer_t*)eso_calloc(sizeof(es_ring_buffer_t));

	if (address) {
		buffer->_write_position = offset;
		buffer->_read_position = offset;
		buffer->_defaultAllocated = FALSE;
		buffer->_address = (char*)address + offset;
	}
	else {
		buffer->_write_position = 0;
		buffer->_read_position = 0;
		buffer->_defaultAllocated = TRUE;
		buffer->_address = eso_malloc(capacity);
	}
	buffer->_capacity = capacity;
	buffer->_turn_around = FALSE;

	return buffer;
}

void eso_ring_buffer_free(es_ring_buffer_t** buffer) {
	if (!buffer || !(*buffer))
		return;
	if ((*buffer)->_defaultAllocated) {
		eso_free((*buffer)->_address);
	}
	ESO_FREE(buffer);
}

es_ring_buffer_t* eso_ring_buffer_clear(es_ring_buffer_t* buffer)
{
	buffer->_write_position = 0;
	buffer->_read_position = 0;
	buffer->_turn_around = FALSE;
	return buffer;
}

es_ulong_t eso_ring_buffer_capacity(es_ring_buffer_t* buffer)
{
	return buffer->_capacity;
}

es_ulong_t eso_ring_buffer_write_position(es_ring_buffer_t* buffer)
{
	return buffer->_write_position;
}

es_ulong_t eso_ring_buffer_read_position(es_ring_buffer_t* buffer)
{
	return buffer->_read_position;
}

es_ulong_t eso_ring_buffer_available_size(es_ring_buffer_t* buffer)
{
	if (buffer->_turn_around) {
		ES_ASSERT(buffer->_write_position - buffer->_read_position <= 0);
		return buffer->_capacity - (buffer->_read_position - buffer->_write_position);
	}
	else {
		ES_ASSERT(buffer->_write_position - buffer->_read_position >= 0);
		return buffer->_write_position - buffer->_read_position;
	}
}

es_long_t eso_ring_buffer_remaining(es_ring_buffer_t* buffer)
{
	return buffer->_capacity - eso_ring_buffer_available_size(buffer);
}

es_bool_t eso_ring_buffer_has_remaining(es_ring_buffer_t* buffer)
{
	if (buffer->_turn_around) {
		ES_ASSERT(buffer->_write_position - buffer->_read_position <= 0);
		return buffer->_read_position - buffer->_write_position;
	}
	else {
		return buffer->_capacity - buffer->_write_position;
	}
}

const void* eso_ring_buffer_base_address(es_ring_buffer_t* buffer)
{
	return buffer->_address;
}

es_ulong_t eso_ring_buffer_get(es_ring_buffer_t* buffer, void* to_addr, es_ulong_t length)
{
	char *p = (char*)buffer->_address + buffer->_read_position;
	es_ulong_t read_bytes = 0;

	if (buffer->_turn_around) {
		es_ulong_t right = buffer->_capacity - buffer->_read_position;
		es_ulong_t left = buffer->_write_position;
		if (length <= right) {
			read_bytes = length;
			eso_memcpy(to_addr, p, read_bytes);
		}
		else {
			read_bytes = right + ES_MIN(length - right, left);
			eso_memcpy(to_addr, p, right);
			eso_memcpy((char*)to_addr + right, buffer->_address, read_bytes - right);
		}
	}
	else {
		read_bytes = ES_MIN(length, buffer->_write_position - buffer->_read_position);
		eso_memcpy(to_addr, p, read_bytes);
	}

	buffer->_read_position += read_bytes;
	if (buffer->_read_position >= buffer->_capacity) {
		buffer->_read_position -= buffer->_capacity;
        
        buffer->_turn_around = FALSE;
	}

	return read_bytes;
}

es_ring_buffer_t* eso_ring_buffer_put(es_ring_buffer_t* buffer, const void* from_addr, es_ulong_t length)
{
	es_long_t remaing;

	if (buffer->_turn_around) {
		remaing = buffer->_read_position - buffer->_write_position;
	}
	else {
		remaing = buffer->_capacity - buffer->_write_position + buffer->_read_position;
	}
    
	if (remaing < length) {
		return NULL;
	}

	if (buffer->_turn_around) {
		eso_memcpy((char*)buffer->_address + buffer->_write_position, from_addr, length);
	}
	else {
		es_ulong_t right = buffer->_capacity - buffer->_write_position;

		if (length <= right) {
			eso_memcpy((char*)buffer->_address + buffer->_write_position, from_addr, length);
		}
		else {
			eso_memcpy((char*)buffer->_address + buffer->_write_position, from_addr, right);
			eso_memcpy((char*)buffer->_address, (char*)from_addr + right, length - right);
		}
	}

	buffer->_write_position += length;
	if (buffer->_write_position >= buffer->_capacity) {
		buffer->_write_position -= buffer->_capacity;

		buffer->_turn_around = TRUE;
	}

	return buffer;
}
