/*
 * EPipedInputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EPipedInputStream_HH_
#define EPipedInputStream_HH_

#include "EInputStream.hh"
#include "EIOException.hh"

namespace efc {

/**
 * A piped input stream should be connected
 * to a piped output stream; the piped  input
 * stream then provides whatever data bytes
 * are written to the piped output  stream.
 * Typically, data is read from a <code>PipedInputStream</code>
 * object by one thread  and data is written
 * to the corresponding <code>PipedOutputStream</code>
 * by some  other thread. Attempting to use
 * both objects from a single thread is not
 * recommended, as it may deadlock the thread.
 * The piped input stream contains a buffer,
 * decoupling read operations from write operations,
 * within limits.
 * A pipe is said to be <a name=BROKEN> <i>broken</i> </a> if a
 * thread that was providing data bytes to the connected
 * piped output stream is no longer alive.
 *
 * @version 1.40, 12/01/05
 * @see     java.io.PipedOutputStream
 * @since   JDK1.0
 */

class EPipedOutputStream;

class EPipedInputStream : public EInputStream {
public:
	friend class EPipedOutputStream;
	
	virtual ~EPipedInputStream();
	
	/**
     * Creates a <code>PipedInputStream</code> so
     * that it is connected to the piped output
     * stream <code>src</code>. Data bytes written
     * to <code>src</code> will then be  available
     * as input from this stream.
     *
     * @param      src   the stream to connect to.
     * @exception  IOException  if an I/O error occurs.
     */
    EPipedInputStream(EPipedOutputStream *src) THROWS(EIOException);

    /**
     * Creates a <code>PipedInputStream</code> so
     * that it is not yet {@linkplain #connect(java.io.PipedOutputStream)
     * connected}.
     * It must be {@linkplain java.io.PipedOutputStream#connect(
     * java.io.PipedInputStream) connected} to a
     * <code>PipedOutputStream</code> before being used.
     */
    EPipedInputStream();

    // TODO:
    EPipedInputStream(const EPipedInputStream& that);
    EPipedInputStream& operator= (const EPipedInputStream& that);

    /**
     * Causes this piped input stream to be connected
     * to the piped  output stream <code>src</code>.
     * If this object is already connected to some
     * other piped output  stream, an <code>IOException</code>
     * is thrown.
     * <p>
     * If <code>src</code> is an
     * unconnected piped output stream and <code>snk</code>
     * is an unconnected piped input stream, they
     * may be connected by either the call:
     * <p>
     * <pre><code>snk.connect(src)</code> </pre>
     * <p>
     * or the call:
     * <p>
     * <pre><code>src.connect(snk)</code> </pre>
     * <p>
     * The two
     * calls have the same effect.
     *
     * @param      src   The piped output stream to connect to.
     * @exception  IOException  if an I/O error occurs.
     */
    void connect(EPipedOutputStream *src) THROWS(EIOException);
    
    /**
     * Reads up to <code>len</code> bytes of data from this piped input
     * stream into an array of bytes. Less than <code>len</code> bytes
     * will be read if the end of the data stream is reached or if 
     * <code>len</code> exceeds the pipe's buffer size.
     * If <code>len </code> is zero, then no bytes are read and 0 is returned; 
     * otherwise, the method blocks until at least 1 byte of input is 
     * available, end of the stream has been detected, or an exception is
     * thrown.
     *
     * @param      b     the buffer into which the data is read.
     * @param      len   the maximum number of bytes read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the stream has been reached.
     * @exception  NullPointerException If <code>b</code> is <code>null</code>.
     * @exception  IndexOutOfBoundsException If <code>off</code> is negative, 
     * <code>len</code> is negative, or <code>len</code> is greater than 
     * <code>b.length - off</code>
     * @exception  IOException if the pipe is <a href=#BROKEN> <code>broken</code></a>,
     *		 {@link #connect(java.io.PipedOutputStream) unconnected},
     *		 closed, or if an I/O error occurs.
     */
    virtual int read(void *b, int len) THROWS(EIOException);
    
    /**
     * Returns the number of bytes that can be read from this input
     * stream without blocking.
     *
     * @return the number of bytes that can be read from this input stream
     *         without blocking, or {@code 0} if this input stream has been
     *         closed by invoking its {@link #close()} method, or if the pipe
     *	       is {@link #connect(java.io.PipedOutputStream) unconnected}, or
     *		<a href=#BROKEN> <code>broken</code></a>.
     *
     * @exception  IOException  if an I/O error occurs.
     * @since   JDK1.0.2
     */
    virtual long available() THROWS(EIOException);

    /**
     * Closes this piped input stream and releases any system resources
     * associated with the stream.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    virtual void close() THROWS(EIOException);
    
private:
	es_pipe_t *mPipe;
	boolean connected;// = false;
	
	es_file_t* getWriter() THROWS(EIOException);
};

} /* namespace efc */
#endif //!EPipedInputStream_HH_
