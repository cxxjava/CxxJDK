/*
 * EBoundedInputStream.hh
 *
 *  Created on: 2017-9-7
 *      Author: cxxjava@163.com
 */

#ifndef EBOUNDEDINPUTSTREAM_HH_
#define EBOUNDEDINPUTSTREAM_HH_

#include "Efc.hh"

namespace efc {
namespace utils {

/**
 * This is a stream that will only supply bytes up to a certain length - if its
 * position goes above that, it will stop.
 * <p>
 * This is useful to wrap ServletInputStreams. The ServletInputStream will block
 * if you try to read content from it that isn't there, because it doesn't know
 * whether the content hasn't arrived yet or whether the content has finished.
 * So, one of these, initialized with the Content-length sent in the
 * ServletInputStream's header, will stop it blocking, providing it's been sent
 * with a correct content length.
 *
 * @version $Id: BoundedInputStream.java 1586342 2014-04-10 15:36:29Z $
 */

class EBoundedInputStream: public EInputStream, virtual public ESynchronizeable {
public:
	virtual ~EBoundedInputStream();

	/**
	 * Creates a new <code>BoundedInputStream</code> that wraps the given input
	 * stream and limits it to a certain size.
	 *
	 * @param in The wrapped input stream
	 * @param size The maximum number of bytes to return
	 */
	EBoundedInputStream(EInputStream* in, long size = EOF,
			boolean propagateClose = false, boolean owned = false);

	//TODO...
	EBoundedInputStream(const EBoundedInputStream& that);
	EBoundedInputStream& operator= (const EBoundedInputStream& that);

	/**
	 * Indicates whether the {@link #close()} method
	 * should propagate to the underling {@link InputStream}.
	 *
	 * @return {@code true} if calling {@link #close()}
	 * propagates to the <code>close()</code> method of the
	 * underlying stream or {@code false} if it does not.
	 */
	boolean isPropagateClose();

	/**
	 * Set whether the {@link #close()} method
	 * should propagate to the underling {@link InputStream}.
	 *
	 * @param propagateClose {@code true} if calling
	 * {@link #close()} propagates to the <code>close()</code>
	 * method of the underlying stream or
	 * {@code false} if it does not.
	 */
	void setPropagateClose(boolean propagateClose);

	/**
	 * Invokes the delegate's <code>read()</code> method if
	 * the current position is less than the limit.
	 * @return the byte read or -1 if the end of stream or
	 * the limit has been reached.
	 * @throws IOException if an I/O error occurs
	 */
	virtual int read() THROWS(EIOException);

	/**
	 * Invokes the delegate's <code>read(byte[], int, int)</code> method.
	 * @param b the buffer to read the bytes into
	 * @param len The number of bytes to read
	 * @return the number of bytes read or -1 if the end of stream or
	 * the limit has been reached.
	 * @throws IOException if an I/O error occurs
	 */
	virtual int read(void *b, int len) THROWS(EIOException);

	/**
	 * Invokes the delegate's <code>skip(long)</code> method.
	 * @param n the number of bytes to skip
	 * @return the actual number of bytes skipped
	 * @throws IOException if an I/O error occurs
	 */
	virtual long skip(long n) THROWS(EIOException);

	/**
	 * {@inheritDoc}
	 */
	virtual long available() THROWS(EIOException);

	/**
	 * Invokes the delegate's <code>close()</code> method
	 * if {@link #isPropagateClose()} is {@code true}.
	 * @throws IOException if an I/O error occurs
	 */
	virtual void close() THROWS(EIOException);

	/**
	 * Invokes the delegate's <code>reset()</code> method.
	 * @throws IOException if an I/O error occurs
	 */
	virtual synchronized void reset() THROWS(EIOException);

	/**
	 * Invokes the delegate's <code>mark(int)</code> method.
	 * @param readlimit read ahead limit
	 */
	virtual synchronized void mark(int readlimit);

	/**
	 * Invokes the delegate's <code>markSupported()</code> method.
	 * @return true if mark is supported, otherwise false
	 */
	virtual boolean markSupported();

	/**
	 * Invokes the delegate's <code>toString()</code> method.
	 * @return the delegate's <code>toString()</code>
	 */
	virtual EStringBase toString();

protected:
	/**
	 *
	 */
	boolean owned;

private:
	/** the wrapped input stream */
	EInputStream* in;

	/** the max length to provide */
	long max;

	/** the number of bytes already returned */
	long pos;// = 0;

	/** the marked position */
	long mark_;// = EOF;

	/** flag if close shoud be propagated */
	boolean propagateClose;// = true;
};

} /* namespace utils */
} /* namespace efc */
#endif /* EBOUNDEDINPUTSTREAM_HH_ */
