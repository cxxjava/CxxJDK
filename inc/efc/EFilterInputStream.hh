/*
 * EFilterInputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EFilterInputStream_HH_
#define EFilterInputStream_HH_

#include "EInputStream.hh"
#include "EIOException.hh"

namespace efc {

/**
 * A <code>FilterInputStream</code> contains
 * some other input stream, which it uses as
 * its  basic source of data, possibly transforming
 * the data along the way or providing  additional
 * functionality. The class <code>FilterInputStream</code>
 * itself simply overrides all  methods of
 * <code>InputStream</code> with versions that
 * pass all requests to the contained  input
 * stream. Subclasses of <code>FilterInputStream</code>
 * may further override some of  these methods
 * and may also provide additional methods
 * and fields.
 *
 * @version 1.33, 04/07/06
 * @since   JDK1.0
 */

class EFilterInputStream : public EInputStream {
public:
	virtual ~EFilterInputStream();

    /**
     * Reads up to <code>byte.length</code> bytes of data from this 
     * input stream into an array of bytes. This method blocks until some 
     * input is available. 
     * <p>
     * This method simply performs the call
     * <code>read(b, 0, b.length)</code> and returns
     * the  result. It is important that it does
     * <i>not</i> do <code>in.read(b)</code> instead;
     * certain subclasses of  <code>FilterInputStream</code>
     * depend on the implementation strategy actually
     * used.
     *
     * @param      b   the buffer into which the data is read.
     * @param      len   the maximum number of bytes read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the stream has been reached.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.FilterInputStream#read(byte[], int, int)
     */
	virtual int read(void *b, int len) THROWS(EIOException);

    /**
     * {@inheritDoc} 
     * <p>
     * This method simply performs <code>in.skip(n)</code>.
     */
	virtual long skip(long n) THROWS(EIOException);

    /**
     * Returns an estimate of the number of bytes that can be read (or
     * skipped over) from this input stream without blocking by the next
     * caller of a method for this input stream. The next caller might be
     * the same thread or another thread.  A single read or skip of this
     * many bytes will not block, but may read or skip fewer bytes.
     * <p>
     * This method returns the result of {@link #in in}.available().
     *
     * @return     an estimate of the number of bytes that can be read (or skipped
     *             over) from this input stream without blocking.
     * @exception  IOException  if an I/O error occurs.
     */
	virtual long available() THROWS(EIOException);

    /**
     * Closes this input stream and releases any system resources 
     * associated with the stream. 
     * This
     * method simply performs <code>in.close()</code>.
     *
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.FilterInputStream#in
     */
	virtual void close() THROWS(EIOException);
    
protected:
    /**
     * The input stream to be filtered. 
     */
	EInputStream* volatile _in;

    /**
     * Creates a <code>FilterInputStream</code>
     * by assigning the  argument <code>in</code>
     * to the field <code>this.in</code> so as
     * to remember it for later use.
     *
     * @param   in   the underlying input stream, or <code>null</code> if 
     *          this instance is to be created without an underlying stream.
     */
    EFilterInputStream(EInputStream *in);
};

} /* namespace efc */
#endif //!EFilterInputStream_HH_
