/*
 * EFilterOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EFilterOutputStream_HH_
#define EFilterOutputStream_HH_

#include "EOutputStream.hh"
#include "EIOException.hh"

namespace efc {

/**
 * This class is the superclass of all classes that filter output 
 * streams. These streams sit on top of an already existing output 
 * stream (the <i>underlying</i> output stream) which it uses as its 
 * basic sink of data, but possibly transforming the data along the 
 * way or providing additional functionality. 
 * <p>
 * The class <code>EFilterOutputStream</code> itself simply overrides 
 * all methods of <code>OutputStream</code> with versions that pass 
 * all requests to the underlying output stream. Subclasses of 
 * <code>EFilterOutputStream</code> may further override some of these 
 * methods as well as provide additional methods and fields. 
 *
 * @version 1.32, 11/17/05
 * @since   JDK1.0
 */

class EFilterOutputStream : public EOutputStream {
public:
	virtual ~EFilterOutputStream();

    /**
     * Creates an output stream filter built on top of the specified 
     * underlying output stream. 
     *
     * @param   out   the underlying output stream to be assigned to 
     *                the field <tt>this.out</tt> for later use, or 
     *                <code>null</code> if this instance is to be 
     *                created without an underlying stream.
     */
    EFilterOutputStream(EOutputStream *out);

    /**
     * Writes <code>b.length</code> bytes to this output stream. 
     * <p>
     * The <code>write</code> method of <code>EFilterOutputStream</code> 
     * calls its <code>write</code> method of three arguments with the 
     * arguments <code>b</code>, <code>0</code>, and 
     * <code>b.length</code>. 
     * <p>
     * Note that this method does not call the one-argument 
     * <code>write</code> method of its underlying stream with the single 
     * argument <code>b</code>. 
     *
     * @param      b   the data to be written.
     * @param      len   the number of bytes to write.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.EFilterOutputStream#write(byte[], int, int)
     */
    virtual void write(const void *b, int len) THROWS(EIOException);
    virtual void write(const char *s) THROWS(EIOException);
    virtual void write(int b) THROWS(EIOException);

    /**
     * Flushes this output stream and forces any buffered output bytes 
     * to be written out to the stream. 
     * <p>
     * The <code>flush</code> method of <code>EFilterOutputStream</code> 
     * calls the <code>flush</code> method of its underlying output stream. 
     *
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.EFilterOutputStream#out
     */
    virtual void flush() THROWS(EIOException);

    /**
     * Closes this output stream and releases any system resources 
     * associated with the stream. 
     * <p>
     * The <code>close</code> method of <code>EFilterOutputStream</code> 
     * calls its <code>flush</code> method, and then calls the 
     * <code>close</code> method of its underlying output stream. 
     *
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.EFilterOutputStream#flush()
     * @see        java.io.EFilterOutputStream#out
     */
    virtual void close() THROWS(EIOException);
    
protected:
    /**
     * The underlying output stream to be filtered. 
     */
    EOutputStream *_out;
};

} /* namespace efc */
#endif //!EFilterOutputStream_HH_
