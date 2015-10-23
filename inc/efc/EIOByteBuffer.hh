/*
 * EIOByteBuffer.hh
 *
 *  Created on: 2013-9-3
 *      Author: cxxjava@163.com
 */

#ifndef EIOBYTEBUFFER_HH_
#define EIOBYTEBUFFER_HH_

#include "EBase.hh"
#include "EIllegalArgumentException.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EBufferUnderflowException.hh"

namespace efc {
namespace nio {

class EIOByteBuffer : EObject {
public:
	virtual ~EIOByteBuffer();

	static EIOByteBuffer* allocate(int capacity=512);
	static EIOByteBuffer* wrap(void* address, int capacity, int offset=0);

	/**
	 * Default is to use heap allocted memory and need to free.
	 * @see allocate() api
	 */
	EIOByteBuffer(int capacity);

	/**
	 * Use external allocted memory and not need to free.
	 * @see allocateDirect() and wrap() api
	 */
	EIOByteBuffer(void* address, int capacity, int offset=0);

	/**
	 * Returns this buffer's capacity. </p>
	 *
	 * @return  The capacity of this buffer
	 */
	int capacity();

	/**
	 * Returns this buffer's position. </p>
	 *
	 * @return  The position of this buffer
	 */
	int position();

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
	EIOByteBuffer* position(int newPosition);

	/**
	 * Returns this buffer's limit. </p>
	 *
	 * @return  The limit of this buffer
	 */
	int limit();

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
	EIOByteBuffer* limit(int newLimit);

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
	EIOByteBuffer* clear();

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
	EIOByteBuffer* flip();

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
	EIOByteBuffer* rewind();

	/**
	 * Returns the number of elements between the current position and the
	 * limit. </p>
	 *
	 * @return  The number of elements remaining in this buffer
	 */
	int remaining();

	/**
	 * Tells whether there are any elements between the current position and
	 * the limit. </p>
	 *
	 * @return  <tt>true</tt> if, and only if, there is at least one element
	 *          remaining in this buffer
	 */
	boolean hasRemaining();

	/**
	 * Tells whether or not this buffer is read-only. </p>
	 *
	 * @return  <tt>true</tt> if, and only if, this buffer is read-only
	 */
	boolean isReadOnly();

	/**
	 * Relative <i>get</i> method.  Reads the byte at this buffer's
	 * current position, and then increments the position. </p>
	 *
	 * @return  The byte at the buffer's current position
	 *
	 * @throws  BufferUnderflowException
	 *          If the buffer's current position is not smaller than its limit
	 */
	byte get();
	byte get(int index);
	EIOByteBuffer& get(void* addr, int size, int length);

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
	EIOByteBuffer& put(byte b);
	EIOByteBuffer& put(int index, byte b);
	EIOByteBuffer& put(const void* addr, int length);

	/**
	 * Returns the current hash code of this buffer.
	 *
	 * <p> The hash code of a byte buffer depends only upon its remaining
	 * elements; that is, upon the elements from <tt>position()</tt> up to, and
	 * including, the element at <tt>limit()</tt>&nbsp;-&nbsp;<tt>1</tt>.
	 *
	 * <p> Because buffer hash codes are content-dependent, it is inadvisable
	 * to use buffers as keys in hash maps or similar data structures unless it
	 * is known that their contents will not change.  </p>
	 *
	 * @return  The current hash code of this buffer
	 */
	int hashCode();

	/**
	 * Tells whether or not this buffer is equal to another object.
	 *
	 * <p> Two byte buffers are equal if, and only if,
	 *
	 * <p><ol>
	 *
	 *   <li><p> They have the same element type,  </p></li>
	 *
	 *   <li><p> They have the same number of remaining elements, and
	 *   </p></li>
	 *
	 *   <li><p> The two sequences of remaining elements, considered
	 *   independently of their starting positions, are pointwise equal.
	 *   </p></li>
	 *
	 * </ol>
	 *
	 * <p> A byte buffer is not equal to any other type of object.  </p>
	 *
	 * @param  ob  The object to which this buffer is to be compared
	 *
	 * @return  <tt>true</tt> if, and only if, this buffer is equal to the
	 *           given object
	 */
	boolean equals(EIOByteBuffer* that);

	/**
	 * Compares this buffer to another.
	 *
	 * <p> Two byte buffers are compared by comparing their sequences of
	 * remaining elements lexicographically, without regard to the starting
	 * position of each sequence within its corresponding buffer.
	 *
	 * <p> A byte buffer is not comparable to any other type of object.
	 *
	 * @return  A negative integer, zero, or a positive integer as this buffer
	 *		is less than, equal to, or greater than the given buffer
	 */
	int compareTo(EIOByteBuffer* that);

	/**
	 * Returns a string summarizing the state of this buffer.  </p>
	 *
	 * @return  A summary string
	 */
	EString toString();

	/**
	 * Return base address.
	 */
	void* address();

	/**
	 * Return current address.
	 */
	void* current();

protected:
	// Invariants: mark <= position <= limit <= capacity
	//int _mark = -1;
	long _position;	// = 0;
	long _limit;
	long _capacity;

	void* _address;
	boolean _defaultAllocated;

	/**
	 *
	 */
	EIOByteBuffer();

	/**
	 * Checks the current position against the limit, throwing a {@link
	 * BufferUnderflowException} if it is not smaller than the limit, and then
	 * increments the position. </p>
	 *
	 * @return  The current position value, before it is incremented
	 */
	int nextGetIndex(int nb=1);

	static void checkBounds(int off, int len, int size) THROWS(EIndexOutOfBoundsException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EIOBYTEBUFFER_HH_ */
