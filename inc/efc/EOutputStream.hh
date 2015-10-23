/*
 * EOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EOutputStream_HH_
#define EOutputStream_HH_

#include "EObject.hh"
#include "EIOException.hh"

namespace efc {

/**
 * This abstract class is the superclass of all classes representing 
 * an output stream of bytes. An output stream accepts output bytes 
 * and sends them to some sink.
 * <p>
 * Applications that need to define a subclass of 
 * <code>OutputStream</code> must always provide at least a method 
 * that writes one byte of output.
 *
 * @author  Arthur van Hoff
 * @version 1.30, 11/17/05
 * @see     java.io.BufferedOutputStream
 * @see     java.io.ByteArrayOutputStream
 * @see     java.io.DataOutputStream
 * @see     java.io.FilterOutputStream
 * @see     java.io.InputStream
 * @see     java.io.OutputStream#write(int)
 * @since   JDK1.0
 */

abstract class EOutputStream : virtual public EObject {
public:
	virtual ~EOutputStream(){}

	/**
     * Writes <code>b.length</code> bytes from the specified byte array 
     * to this output stream. The general contract for <code>write(b)</code> 
     * is that it should have exactly the same effect as the call 
     * <code>write(b, 0, b.length)</code>.
     *
     * @param      b   the data.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.OutputStream#write(byte[], int, int)
     */
    virtual void write(const void *b, int len) THROWS(EIOException) = 0;
    
    /**
     * Writes the specified byte to this output stream. The general 
     * contract for <code>write</code> is that one byte is written 
     * to the output stream. The byte to be written is the eight 
     * low-order bits of the argument <code>b</code>. The 24 
     * high-order bits of <code>b</code> are ignored.
     * <p>
     * Subclasses of <code>OutputStream</code> must provide an 
     * implementation for this method. 
     *
     * @param      b   the <code>byte</code>.
     * @exception  IOException  if an I/O error occurs. In particular, 
     *             an <code>IOException</code> may be thrown if the 
     *             output stream has been closed.
     */
    virtual void write(int b) THROWS(EIOException);
    virtual void write(const char* s) THROWS(EIOException);
    
    /**
     * Flushes this output stream and forces any buffered output bytes 
     * to be written out. The general contract of <code>flush</code> is 
     * that calling it is an indication that, if any bytes previously 
     * written have been buffered by the implementation of the output 
     * stream, such bytes should immediately be written to their 
     * intended destination.
     * <p>
     * If the intended destination of this stream is an abstraction provided by
     * the underlying operating system, for example a file, then flushing the
     * stream guarantees only that bytes previously written to the stream are
     * passed to the operating system for writing; it does not guarantee that
     * they are actually written to a physical device such as a disk drive.
     * <p>
     * The <code>flush</code> method of <code>OutputStream</code> does nothing.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    virtual void flush() THROWS(EIOException);
    
    /**
     * Closes this output stream and releases any system resources 
     * associated with this stream. The general contract of <code>close</code> 
     * is that it closes the output stream. A closed stream cannot perform 
     * output operations and cannot be reopened.
     * <p>
     * The <code>close</code> method of <code>OutputStream</code> does nothing.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    virtual void close() THROWS(EIOException);
};

} /* namespace efc */
#endif //!EOutputStream_HH_
