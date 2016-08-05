/*
 * EPipedOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EPipedOutputStream_HH_
#define EPipedOutputStream_HH_

#include "EOutputStream.hh"
#include "EIOException.hh"
#include "ENullPointerException.hh"

namespace efc {

/**
 * A piped output stream can be connected to a piped input stream 
 * to create a communications pipe. The piped output stream is the 
 * sending end of the pipe. Typically, data is written to a 
 * <code>PipedOutputStream</code> object by one thread and data is 
 * read from the connected <code>PipedInputStream</code> by some 
 * other thread. Attempting to use both objects from a single thread 
 * is not recommended as it may deadlock the thread.
 * The pipe is said to be <a name=BROKEN> <i>broken</i> </a> if a
 * thread that was reading data bytes from the connected piped input 
 * stream is no longer alive.
 *
 * @version 1.28, 06/07/06
 * @see     java.io.PipedInputStream
 * @since   JDK1.0
 */

class EPipedInputStream;

class EPipedOutputStream : public EOutputStream {
public:
	virtual ~EPipedOutputStream();
	
    /**
     * Creates a piped output stream connected to the specified piped 
     * input stream. Data bytes written to this stream will then be 
     * available as input from <code>snk</code>.
     *
     * @param      snk   The piped input stream to connect to.
     * @exception  IOException  if an I/O error occurs.
     */
    EPipedOutputStream(EPipedInputStream *snk) THROWS(EIOException);
    
    /**
     * Creates a piped output stream that is not yet connected to a 
     * piped input stream. It must be connected to a piped input stream, 
     * either by the receiver or the sender, before being used. 
     *
     * @see     java.io.PipedInputStream#connect(java.io.PipedOutputStream)
     * @see     java.io.PipedOutputStream#connect(java.io.PipedInputStream)
     */
    EPipedOutputStream();
    
    // TODO:
    EPipedOutputStream(const EPipedOutputStream& that);
    EPipedOutputStream& operator= (const EPipedOutputStream& that);

    /**
     * Connects this piped output stream to a receiver. If this object
     * is already connected to some other piped input stream, an 
     * <code>IOException</code> is thrown.
     * <p>
     * If <code>snk</code> is an unconnected piped input stream and 
     * <code>src</code> is an unconnected piped output stream, they may 
     * be connected by either the call:
     * <blockquote><pre>
     * src.connect(snk)</pre></blockquote>
     * or the call:
     * <blockquote><pre>
     * snk.connect(src)</pre></blockquote>
     * The two calls have the same effect.
     *
     * @param      snk   the piped input stream to connect to.
     * @exception  IOException  if an I/O error occurs.
     */
    void connect(EPipedInputStream *snk) THROWS(EIOException);

    /**
     * Writes <code>len</code> bytes from the specified byte array 
     * starting at offset <code>off</code> to this piped output stream. 
     * This method blocks until all the bytes are written to the output
     * stream.
     *
     * @param      b     the data.
     * @param      len   the number of bytes to write.
     * @exception IOException if the pipe is <a href=#BROKEN> broken</a>,
     *          {@link #connect(java.io.PipedInputStream) unconnected},
     *		closed, or if an I/O error occurs.
     */
    virtual void write(const void *b, int len) THROWS(EIOException);
    virtual void write(const char *s) THROWS(EIOException);
	virtual void write(int b) THROWS(EIOException);

    /**
     * Closes this piped output stream and releases any system resources 
     * associated with this stream. This stream may no longer be used for 
     * writing bytes.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    virtual void close() THROWS(EIOException);
	
private:
	/* REMIND: identification of the read and write sides needs to be
	   more sophisticated.  Either using thread groups (but what about
	   pipes within a thread?) or using finalization (but it may be a
	   long time until the next GC). */
    EPipedInputStream *sink;
};

} /* namespace efc */
#endif //!EPipedOutputStream_HH_
