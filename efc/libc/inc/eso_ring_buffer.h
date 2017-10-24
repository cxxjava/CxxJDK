/*
 * eso_ring_buffer.h
 *
 *  Created on: 2015-1-12
 *      Author: cxxjava@163.com
 */

#ifndef ESO_RING_BUFFER_H_
#define ESO_RING_BUFFER_H_

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_ring_buffer_t es_ring_buffer_t;
struct es_ring_buffer_t {
	const void* _address;
	es_ulong_t _write_position;
	es_ulong_t _read_position;
	es_ulong_t _capacity;
	es_bool_t  _turn_around;

	es_bool_t _defaultAllocated;
};

/**
 * Create a new ring buffer.
 * @param capacity The buffer size.
 * @param address If null, then use external allocted memory and not need to free.
 * @param offset The offset of external allocted memory address.
 */
es_ring_buffer_t* eso_ring_buffer_create(es_ulong_t capacity, void* address, es_ulong_t offset);

/**
 * Destroy a ring buffer.
 */
void eso_ring_buffer_free(es_ring_buffer_t** buffer) ;

/**
 * Returns this buffer's capacity. </p>
 *
 * @return  The capacity of this buffer
 */
es_ulong_t eso_ring_buffer_capacity(es_ring_buffer_t* buffer);

es_ulong_t eso_ring_buffer_write_position(es_ring_buffer_t* buffer);

es_ulong_t eso_ring_buffer_read_position(es_ring_buffer_t* buffer);

es_ulong_t eso_ring_buffer_available_size(es_ring_buffer_t* buffer);

/**
 * Clears this buffer.  The position is set to zero, the limit is set to
 * the capacity, and the mark is discarded.
 *
 * <p> Invoke this method before using a sequence of channel-read or
 * <i>put</i> operations to fill this buffer.  For example:
 *
 * <blockquote><pre>
 * buf.clear();     // Prepare buffer for reading
 * in.read(buf);    // Read data</pre></blockquote>
 *
 * <p> This method does not actually erase the data in the buffer, but it
 * is named as if it did because it will most often be used in situations
 * in which that might as well be the case. </p>
 *
 * @return  This buffer
 */
es_ring_buffer_t* eso_ring_buffer_clear(es_ring_buffer_t* buffer);

/**
 * Returns the number of elements between the current position and the
 * limit. </p>
 *
 * @return  The number of elements remaining in this buffer
 */
es_long_t eso_ring_buffer_remaining(es_ring_buffer_t* buffer);

/**
 * Tells whether there are any elements between the current position and
 * the limit. </p>
 *
 * @return  <tt>true</tt> if, and only if, there is at least one element
 *          remaining in this buffer
 */
es_bool_t eso_ring_buffer_has_remaining(es_ring_buffer_t* buffer);

/**
 * Return base address.
 */
const void* eso_ring_buffer_base_address(es_ring_buffer_t* buffer);

/**
 * Relative <i>get</i> method.  Reads the byte at this buffer's
 * current position, and then increments the position. </p>
 *
 * @return  The byte at the buffer's current position
 *
 * @throws  BufferUnderflowException
 *          If the buffer's current position is not smaller than its limit
 */
es_ulong_t eso_ring_buffer_get(es_ring_buffer_t* buffer, void* to_addr, es_ulong_t length);

/**
 * Relative <i>put</i> method&nbsp;&nbsp;<i>(optional operation)</i>.
 *
 * <p> Writes the given byte into this buffer at the current
 * position, and then increments the position. </p>
 *
 * @param  b
 *         The byte to be written
 *
 * @return  This buffer
 *
 * @throws  BufferOverflowException
 *          If this buffer's current position is not smaller than its limit
 *
 * @throws  ReadOnlyBufferException
 *          If this buffer is read-only
 */
es_ring_buffer_t* eso_ring_buffer_put(es_ring_buffer_t* buffer, const void* from_addr, es_ulong_t length);

#ifdef __cplusplus
}
#endif

#endif /* ESO_RING_BUFFER_H_ */
