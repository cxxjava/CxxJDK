/*
 * EPushbackInputStream.hh
 *
 *  Created on: 2017-11-17
 *      Author: cxxjava@163.com
 */

#ifndef EPUSHBACKINPUTSTREAM_HH_
#define EPUSHBACKINPUTSTREAM_HH_

#include "EA.hh"
#include "EFilterInputStream.hh"
#include "ESynchronizeable.hh"

namespace efc {

/**
 * A <code>PushbackInputStream</code> adds
 * functionality to another input stream, namely
 * the  ability to "push back" or "unread"
 * one byte. This is useful in situations where
 * it is  convenient for a fragment of code
 * to read an indefinite number of data bytes
 * that  are delimited by a particular byte
 * value; after reading the terminating byte,
 * the  code fragment can "unread" it, so that
 * the next read operation on the input stream
 * will reread the byte that was pushed back.
 * For example, bytes representing the  characters
 * constituting an identifier might be terminated
 * by a byte representing an  operator character;
 * a method whose job is to read just an identifier
 * can read until it  sees the operator and
 * then push the operator back to be re-read.
 *
 * @since   JDK1.0
 */

class EPushbackInputStream: public EFilterInputStream, public ESynchronizeable {
public:
	virtual ~EPushbackInputStream();

	/**
	 * Creates a <code>PushbackInputStream</code>
	 * with a pushback buffer of the specified <code>size</code>,
	 * and saves its  argument, the input stream
	 * <code>in</code>, for later use. Initially,
	 * there is no pushed-back byte  (the field
	 * <code>pushBack</code> is initialized to
	 * <code>-1</code>).
	 *
	 * @param  in    the input stream from which bytes will be read.
	 * @param  size  the size of the pushback buffer.
	 * @exception IllegalArgumentException if size is <= 0
	 * @since  JDK1.1
	 */
	EPushbackInputStream(EInputStream* in, int size=1, boolean owned=false);

	//TODO...
	EPushbackInputStream(const EPushbackInputStream& that);
	EPushbackInputStream& operator= (const EPushbackInputStream& that);

	/**
	 * Reads the next byte of data from this input stream. The value
	 * byte is returned as an <code>int</code> in the range
	 * <code>0</code> to <code>255</code>. If no byte is available
	 * because the end of the stream has been reached, the value
	 * <code>-1</code> is returned. This method blocks until input data
	 * is available, the end of the stream is detected, or an exception
	 * is thrown.
	 *
	 * <p> This method returns the most recently pushed-back byte, if there is
	 * one, and otherwise calls the <code>read</code> method of its underlying
	 * input stream and returns whatever value that method returns.
	 *
	 * @return     the next byte of data, or <code>-1</code> if the end of the
	 *             stream has been reached.
	 * @exception  IOException  if this input stream has been closed by
	 *             invoking its {@link #close()} method,
	 *             or an I/O error occurs.
	 * @see        java.io.InputStream#read()
	 */
	virtual int read() THROWS(EIOException);

	/**
	 * Reads up to <code>len</code> bytes of data from this input stream into
	 * an array of bytes.  This method first reads any pushed-back bytes; after
	 * that, if fewer than <code>len</code> bytes have been read then it
	 * reads from the underlying input stream. If <code>len</code> is not zero, the method
	 * blocks until at least 1 byte of input is available; otherwise, no
	 * bytes are read and <code>0</code> is returned.
	 *
	 * @param      b     the buffer into which the data is read.
	 * @param      off   the start offset in the destination array <code>b</code>
	 * @param      len   the maximum number of bytes read.
	 * @return     the total number of bytes read into the buffer, or
	 *             <code>-1</code> if there is no more data because the end of
	 *             the stream has been reached.
	 * @exception  NullPointerException If <code>b</code> is <code>null</code>.
	 * @exception  IndexOutOfBoundsException If <code>off</code> is negative,
	 * <code>len</code> is negative, or <code>len</code> is greater than
	 * <code>b.length - off</code>
	 * @exception  IOException  if this input stream has been closed by
	 *             invoking its {@link #close()} method,
	 *             or an I/O error occurs.
	 * @see        java.io.InputStream#read(byte[], int, int)
	 */
	virtual int read(void *b, int len) THROWS(EIOException);

	/**
	 * Pushes back a byte by copying it to the front of the pushback buffer.
	 * After this method returns, the next byte to be read will have the value
	 * <code>(byte)b</code>.
	 *
	 * @param      b   the <code>int</code> value whose low-order
	 *                  byte is to be pushed back.
	 * @exception IOException If there is not enough room in the pushback
	 *            buffer for the byte, or this input stream has been closed by
	 *            invoking its {@link #close()} method.
	 */
	virtual void unread(int b) THROWS(EIOException);

	/**
	 * Pushes back a portion of an array of bytes by copying it to the front
	 * of the pushback buffer.  After this method returns, the next byte to be
	 * read will have the value <code>b[off]</code>, the byte after that will
	 * have the value <code>b[off+1]</code>, and so forth.
	 *
	 * @param b the byte array to push back.
	 * @param off the start offset of the data.
	 * @param len the number of bytes to push back.
	 * @exception IOException If there is not enough room in the pushback
	 *            buffer for the specified number of bytes,
	 *            or this input stream has been closed by
	 *            invoking its {@link #close()} method.
	 * @since     JDK1.1
	 */
	virtual void unread(void *b, int len) THROWS(EIOException);

	/**
	 * Returns an estimate of the number of bytes that can be read (or
	 * skipped over) from this input stream without blocking by the next
	 * invocation of a method for this input stream. The next invocation might be
	 * the same thread or another thread.  A single read or skip of this
	 * many bytes will not block, but may read or skip fewer bytes.
	 *
	 * <p> The method returns the sum of the number of bytes that have been
	 * pushed back and the value returned by {@link
	 * java.io.FilterInputStream#available available}.
	 *
	 * @return     the number of bytes that can be read (or skipped over) from
	 *             the input stream without blocking.
	 * @exception  IOException  if this input stream has been closed by
	 *             invoking its {@link #close()} method,
	 *             or an I/O error occurs.
	 * @see        java.io.FilterInputStream#in
	 * @see        java.io.InputStream#available()
	 */
	virtual long available() THROWS(EIOException);

	/**
	 * Skips over and discards <code>n</code> bytes of data from this
	 * input stream. The <code>skip</code> method may, for a variety of
	 * reasons, end up skipping over some smaller number of bytes,
	 * possibly zero.  If <code>n</code> is negative, no bytes are skipped.
	 *
	 * <p> The <code>skip</code> method of <code>PushbackInputStream</code>
	 * first skips over the bytes in the pushback buffer, if any.  It then
	 * calls the <code>skip</code> method of the underlying input stream if
	 * more bytes need to be skipped.  The actual number of bytes skipped
	 * is returned.
	 *
	 * @param      n  {@inheritDoc}
	 * @return     {@inheritDoc}
	 * @exception  IOException  if the stream does not support seek,
	 *            or the stream has been closed by
	 *            invoking its {@link #close()} method,
	 *            or an I/O error occurs.
	 * @see        java.io.FilterInputStream#in
	 * @see        java.io.InputStream#skip(long n)
	 * @since      1.2
	 */
	virtual long skip(long n) THROWS(EIOException);

	/**
	 * Tests if this input stream supports the <code>mark</code> and
	 * <code>reset</code> methods, which it does not.
	 *
	 * @return   <code>false</code>, since this class does not support the
	 *           <code>mark</code> and <code>reset</code> methods.
	 * @see     java.io.InputStream#mark(int)
	 * @see     java.io.InputStream#reset()
	 */
	virtual boolean markSupported();

	/**
	 * Marks the current position in this input stream.
	 *
	 * <p> The <code>mark</code> method of <code>PushbackInputStream</code>
	 * does nothing.
	 *
	 * @param   readlimit   the maximum limit of bytes that can be read before
	 *                      the mark position becomes invalid.
	 * @see     java.io.InputStream#reset()
	 */
	virtual synchronized void mark(int readlimit);

	/**
	 * Repositions this stream to the position at the time the
	 * <code>mark</code> method was last called on this input stream.
	 *
	 * <p> The method <code>reset</code> for class
	 * <code>PushbackInputStream</code> does nothing except throw an
	 * <code>IOException</code>.
	 *
	 * @exception  IOException  if this method is invoked.
	 * @see     java.io.InputStream#mark(int)
	 * @see     java.io.IOException
	 */
	virtual synchronized void reset() THROWS(EIOException);

	/**
	 * Closes this input stream and releases any system resources
	 * associated with the stream.
	 * Once the stream has been closed, further read(), unread(),
	 * available(), reset(), or skip() invocations will throw an IOException.
	 * Closing a previously closed stream has no effect.
	 *
	 * @exception  IOException  if an I/O error occurs.
	 */
	virtual synchronized void close() THROWS(EIOException);

protected:
	/**
	 * The pushback buffer.
	 * @since   JDK1.1
	 */
	EA<byte>* buf;

	/**
	 * The position within the pushback buffer from which the next byte will
	 * be read.  When the buffer is empty, <code>pos</code> is equal to
	 * <code>buf.length</code>; when the buffer is full, <code>pos</code> is
	 * equal to zero.
	 *
	 * @since   JDK1.1
	 */
	int pos;

private:
	/**
	 * Check to make sure that this stream has not been closed
	 */
	void ensureOpen() THROWS(EIOException);
};

} /* namespace efc */
#endif /* EPUSHBACKINPUTSTREAM_HH_ */
