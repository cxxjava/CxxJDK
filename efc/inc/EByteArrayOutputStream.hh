/*
 * EByteArrayOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EByteArrayOutputStream_HH_
#define EByteArrayOutputStream_HH_

#include "EA.hh"
#include "EOutputStream.hh"
#include "ESynchronizeable.hh"
#include "EByteBuffer.hh"
#include "EString.hh"
#include "ESharedPtr.hh"
#include "EIOException.hh"
#include "ENullPointerException.hh"

namespace efc {

/**
 * This class implements an output stream in which the data is 
 * written into a byte array. The buffer automatically grows as data 
 * is written to it. 
 * The data can be retrieved using <code>toByteArray()</code> and
 * <code>toString()</code>.
 * <p>
 * Closing a <tt>ByteArrayOutputStream</tt> has no effect. The methods in
 * this class can be called after the stream has been closed without
 * generating an <tt>IOException</tt>.
 *
 * @version 1.53, 06/07/06
 * @since   JDK1.0
 */

class EByteArrayOutputStream : public EOutputStream, public ESynchronizeable {
public:
	virtual ~EByteArrayOutputStream();
	
	/**
     * Creates a new byte array output stream, with a buffer capacity of 
     * the specified size, in bytes. 
     *
     * @param   size   the initial size.
     * @exception  IllegalArgumentException if size is negative.
     */
    EByteArrayOutputStream(int size=32, boolean lock=true);

    //TODO...
    EByteArrayOutputStream(const EByteArrayOutputStream& that);
    EByteArrayOutputStream& operator= (const EByteArrayOutputStream& that);
	
    /**
     * Writes <code>len</code> bytes from the specified byte array 
     *
     * @param   b     the data.
     * @param   len   the number of bytes to write.
     */
    virtual synchronized void write(const void *b, int len);
    virtual synchronized void write(const char *s) THROWS(EIOException);
	virtual synchronized void write(int b) THROWS(EIOException);

    /**
     * Writes the complete contents of this byte array output stream to 
     * the specified output stream argument, as if by calling the output 
     * stream's write method using <code>out.write(buf, 0, count)</code>.
     *
     * @param      out   the output stream to which to write the data.
     * @exception  IOException  if an I/O error occurs.
     */
	virtual synchronized void writeTo(EOutputStream *out) THROWS(EIOException);

    /**
     * Resets the <code>count</code> field of this byte array output 
     * stream to zero, so that all currently accumulated output in the 
     * output stream is discarded. The output stream can be used again, 
     * reusing the already allocated buffer space. 
     *
     * @see     java.io.ByteArrayInputStream#count
     */
	virtual synchronized void reset();

    /**
     * When copy is false then return old buf and create a new buf;
     * else if copy is true then copy this data and return it.
     */
	virtual synchronized sp<EA<byte> > reset(boolean copy);

    /**
     * Creates a newly allocated byte array. Its size is the current 
     * size of this output stream and the valid contents of the buffer 
     * have been copied into it. 
     *
     * @return  the current contents of this output stream, as a byte array.
     * @see     java.io.ByteArrayOutputStream#size()
     */
	virtual synchronized sp<EA<byte> > toByteArray();
	virtual synchronized void toByteArray(EByteBuffer *obj);

    /**
     * Returns the current size of the buffer.
     *
     * @return  the value of the <code>count</code> field, which is the number
     *          of valid bytes in this output stream.
     * @see     java.io.ByteArrayOutputStream#count
     */
	virtual synchronized int size();

	/**
	 * Get the buffer where data is stored.
	 */
	virtual synchronized byte* data();

    /**
     * Converts the buffer's contents into a string decoding bytes using the
     * platform's default character set. The length of the new <tt>String</tt>
     * is a function of the character set, and hence may not be equal to the 
     * size of the buffer.
     *
     * <p> This method always replaces malformed-input and unmappable-character
     * sequences with the default replacement string for the platform's
     * default character set. The {@linkplain java.nio.charset.CharsetDecoder}
     * class should be used when more control over the decoding process is
     * required.
     *
     * @return String decoded from the buffer's contents.
     * @since  JDK1.1
     */
	virtual synchronized EString toString();
    
protected:
    /**
	 * The buffer where data is stored.
	 */
	byte *bufData;
	int   bufLen;

	/**
	 * The number of valid bytes in the buffer.
	 */
	int count;

	/**
	 * If false then no synchronized!
	 */
	boolean lock;

private:
	int defaultCapacity;

	/**
	 * Increases the capacity if necessary to ensure that it can hold
	 * at least the number of elements specified by the minimum
	 * capacity argument.
	 *
	 * @param minCapacity the desired minimum capacity
	 * @throws OutOfMemoryError if {@code minCapacity < 0}.  This is
	 * interpreted as a request for the unsatisfiably large capacity
	 * {@code (long) Integer.MAX_VALUE + (minCapacity - Integer.MAX_VALUE)}.
	 */
	void ensureCapacity(int minCapacity);

	/**
	 * Increases the capacity to ensure that it can hold at least the
	 * number of elements specified by the minimum capacity argument.
	 *
	 * @param minCapacity the desired minimum capacity
	 */
	void grow(int minCapacity);
};

} /* namespace efc */
#endif //!EByteArrayOutputStream_HH_
