/*
 * eso_flip_buffer.h
 *
 *  Created on: 2015-1-12
 *      Author: cxxjava@163.com
 */

#ifndef ESO_FLIP_BUFFER_H_
#define ESO_FLIP_BUFFER_H_

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_flip_buffer_t es_flip_buffer_t;
struct es_flip_buffer_t {
	es_ulong_t _position;	// = 0;
	es_ulong_t _limit;
	es_ulong_t _capacity;

	void* _address;
	es_bool_t _defaultAllocated;
};

/**
 * Create a new ring buffer.
 * @param capacity The buffer size.
 * @param address If null, then use external allocted memory and not need to free.
 * @param offset The offset of external allocted memory address.
 */
es_flip_buffer_t* eso_flip_buffer_create(es_ulong_t capacity, void* address, es_ulong_t offset);

/**
 * Destroy a ring buffer.
 */
void eso_flip_buffer_free(es_flip_buffer_t** buffer) ;

/**
 * Returns this buffer's capacity. </p>
 *
 * @return  The capacity of this buffer
 */
es_ulong_t eso_flip_buffer_capacity(es_flip_buffer_t* buffer);

/**
 * Sets this buffer's position.  If the mark is defined and larger than the
 * new position then it is discarded. </p>
 *
 * @param  newPosition
 *         The new position value; must be non-negative
 *         and no larger than the current limit
 *
 * @return  This buffer
 *
 * @throws  IllegalArgumentException
 *          If the preconditions on <tt>newPosition</tt> do not hold
 */
es_flip_buffer_t* eso_flip_buffer_position_set(es_flip_buffer_t* buffer, es_ulong_t newPosition);

/**
 * Returns this buffer's limit. </p>
 *
 * @return  The limit of this buffer
 */
es_ulong_t eso_flip_buffer_limit(es_flip_buffer_t* buffer);

/**
 * Sets this buffer's limit.  If the position is larger than the new limit
 * then it is set to the new limit.  If the mark is defined and larger than
 * the new limit then it is discarded. </p>
 *
 * @param  newLimit
 *         The new limit value; must be non-negative
 *         and no larger than this buffer's capacity
 *
 * @return  This buffer
 *
 * @throws  IllegalArgumentException
 *          If the preconditions on <tt>newLimit</tt> do not hold
 */
es_flip_buffer_t* eso_flip_buffer_limit_set(es_flip_buffer_t* buffer, es_ulong_t newLimit);

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
es_flip_buffer_t* eso_flip_buffer_clear(es_flip_buffer_t* buffer);

/**
 * Flips this buffer.  The limit is set to the current position and then
 * the position is set to zero.  If the mark is defined then it is
 * discarded.
 *
 * <p> After a sequence of channel-read or <i>put</i> operations, invoke
 * this method to prepare for a sequence of channel-write or relative
 * <i>get</i> operations.  For example:
 *
 * <blockquote><pre>
 * buf.put(magic);    // Prepend header
 * in.read(buf);      // Read data into rest of buffer
 * buf.flip();        // Flip buffer
 * out.write(buf);    // Write header + data to channel</pre></blockquote>
 *
 * <p> This method is often used in conjunction with the {@link
 * java.nio.ByteBuffer#compact compact} method when transferring data from
 * one place to another.  </p>
 *
 * @return  This buffer
 */
es_flip_buffer_t* eso_flip_buffer_flip(es_flip_buffer_t* buffer);

/**
 * Rewinds this buffer.  The position is set to zero and the mark is
 * discarded.
 *
 * <p> Invoke this method before a sequence of channel-write or <i>get</i>
 * operations, assuming that the limit has already been set
 * appropriately.  For example:
 *
 * <blockquote><pre>
 * out.write(buf);    // Write remaining data
 * buf.rewind();      // Rewind buffer
 * buf.get(array);    // Copy data into array</pre></blockquote>
 *
 * @return  This buffer
 */
es_flip_buffer_t* eso_flip_buffer_rewind(es_flip_buffer_t* buffer);

/**
 * Returns the number of elements between the current position and the
 * limit. </p>
 *
 * @return  The number of elements remaining in this buffer
 */
es_long_t eso_flip_buffer_remaining(es_flip_buffer_t* buffer);

/**
 * Tells whether there are any elements between the current position and
 * the limit. </p>
 *
 * @return  <tt>true</tt> if, and only if, there is at least one element
 *          remaining in this buffer
 */
es_bool_t eso_flip_buffer_has_remaining(es_flip_buffer_t* buffer);

/**
 * Return base address.
 */
void* eso_flip_buffer_base_address(es_flip_buffer_t* buffer);

/**
 * Return current address.
 */
void* eso_flip_buffer_current_address(es_flip_buffer_t* buffer);

/**
 * Relative <i>get</i> method.  Reads the byte at this buffer's
 * current position, and then increments the position. </p>
 *
 * @return  The byte at the buffer's current position
 *
 * @throws  BufferUnderflowException
 *          If the buffer's current position is not smaller than its limit
 */
es_ulong_t eso_flip_buffer_get(es_flip_buffer_t* buffer, void* to_addr, es_ulong_t length);
int eso_flip_buffer_get_byte(es_flip_buffer_t* buffer, es_byte_t* byte);
int eso_flip_buffer_get_byte_at(es_flip_buffer_t* buffer, es_ulong_t index, es_byte_t* byte);

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
es_flip_buffer_t* eso_flip_buffer_put(es_flip_buffer_t* buffer, const void* from_addr, es_ulong_t length);
es_flip_buffer_t* eso_flip_buffer_put_byte(es_flip_buffer_t* buffer, es_byte_t b);
es_flip_buffer_t* eso_flip_buffer_put_byte_at(es_flip_buffer_t* buffer, es_ulong_t index, es_byte_t b);

#ifdef __cplusplus
}
#endif

#endif /* ESO_FLIP_BUFFER_H_ */
