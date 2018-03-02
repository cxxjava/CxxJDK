/*
 * EBufferedOutputStream.hh
 *
 *  Created on: 2015-6-9
 *      Author: cxxjava@163.com
 */

#ifndef EBUFFEREDOUTPUTSTREAM_HH_
#define EBUFFEREDOUTPUTSTREAM_HH_

#include "EA.hh"
#include "EFilterOutputStream.hh"
#include "ESynchronizeable.hh"

namespace efc {

/**
 * The class implements a buffered output stream. By setting up such
 * an output stream, an application can write bytes to the underlying
 * output stream without necessarily causing a call to the underlying
 * system for each byte written.
 *
 * @since   JDK1.0
 */
class EBufferedOutputStream: public EFilterOutputStream, public ESynchronizeable {
public:
	virtual ~EBufferedOutputStream();

	/**
	 * Creates a new buffered output stream to write data to the
	 * specified underlying output stream with the specified buffer
	 * size.
	 *
	 * @param   out    the underlying output stream.
	 * @param   size   the buffer size.
	 * @exception IllegalArgumentException if size &lt;= 0.
	 */
	EBufferedOutputStream(EOutputStream* out, int size=8192, boolean owned=false);

	//TODO...
	EBufferedOutputStream(const EBufferedOutputStream& that);
	EBufferedOutputStream& operator= (const EBufferedOutputStream& that);

	/**
	 * Writes the specified byte to this buffered output stream.
	 *
	 * @param      b   the byte to be written.
	 * @exception  IOException  if an I/O error occurs.
	 */
	virtual synchronized void write(int b) THROWS(EIOException);
	virtual synchronized void write(const char* s) THROWS(EIOException);

	/**
	 * Writes <code>len</code> bytes from the specified byte array
	 * starting at offset <code>off</code> to this buffered output stream.
	 *
	 * <p> Ordinarily this method stores bytes from the given array into this
	 * stream's buffer, flushing the buffer to the underlying output stream as
	 * needed.  If the requested length is at least as large as this stream's
	 * buffer, however, then this method will flush the buffer and write the
	 * bytes directly to the underlying output stream.  Thus redundant
	 * <code>BufferedOutputStream</code>s will not copy data unnecessarily.
	 *
	 * @param      b     the data.
	 * @param      off   the start offset in the data.
	 * @param      len   the number of bytes to write.
	 * @exception  IOException  if an I/O error occurs.
	 */
	virtual synchronized void write(const void *b, int len) THROWS(EIOException);

	/**
	 * Flushes this buffered output stream. This forces any buffered
	 * output bytes to be written out to the underlying output stream.
	 *
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	virtual synchronized void flush() THROWS(EIOException);

protected:
	/**
	 * The internal buffer where data is stored.
	 */
	EA<byte>* buf;

	/**
	 * The number of valid bytes in the buffer. This value is always
	 * in the range <tt>0</tt> through <tt>buf.length</tt>; elements
	 * <tt>buf[0]</tt> through <tt>buf[count-1]</tt> contain valid
	 * byte data.
	 */
	int count;

private:
	/** Flush the internal buffer */
	void flushBuffer() THROWS(EIOException);
};

} /* namespace efc */
#endif /* EBUFFEREDOUTPUTSTREAM_HH_ */
