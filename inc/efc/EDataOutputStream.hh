/*
 * EDataOutputStream.hh
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#ifndef EDATAOUTPUTSTREAM_HH_
#define EDATAOUTPUTSTREAM_HH_

#include "EFilterOutputStream.hh"
#include "EDataOutput.hh"

namespace efc {

/**
 * A data output stream lets an application write primitive Java data
 * types to an output stream in a portable way. An application can
 * then use a data input stream to read the data back in.
 *
 * @version 1.44, 11/17/05
 * @see     java.io.DataInputStream
 * @since   JDK1.0
 */

class EDataOutputStream: public EFilterOutputStream, public virtual EDataOutput {
public:
	virtual ~EDataOutputStream();

	/**
	 * Creates a new data output stream to write data to the specified
	 * underlying output stream. The counter <code>written</code> is
	 * set to zero.
	 *
	 * @param   out   the underlying output stream, to be saved for later
	 *                use.
	 * @see     java.io.FilterOutputStream#out
	 */
	EDataOutputStream(EOutputStream* out);

	//TODO:
	EDataOutputStream(const EDataOutputStream& that);
	EDataOutputStream& operator= (const EDataOutputStream& that);

	/**
	 * Writes the specified byte (the low eight bits of the argument
	 * <code>b</code>) to the underlying output stream. If no exception
	 * is thrown, the counter <code>written</code> is incremented by
	 * <code>1</code>.
	 * <p>
	 * Implements the <code>write</code> method of <code>OutputStream</code>.
	 *
	 * @param      b   the <code>byte</code> to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	virtual void write(int b) THROWS(EIOException);
	virtual void write(const char* s) THROWS(EIOException);

	/**
	 * Writes <code>len</code> bytes from the specified byte array
	 * starting at offset <code>off</code> to the underlying output stream.
	 * If no exception is thrown, the counter <code>written</code> is
	 * incremented by <code>len</code>.
	 *
	 * @param      b     the data.
	 * @param      len   the number of bytes to write.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	virtual void write(const void *b, int len) THROWS(EIOException);

	/**
	 * Flushes this data output stream. This forces any buffered output
	 * bytes to be written out to the stream.
	 * <p>
	 * The <code>flush</code> method of <code>DataOutputStream</code>
	 * calls the <code>flush</code> method of its underlying output stream.
	 *
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 * @see        java.io.OutputStream#flush()
	 */
	virtual void flush() THROWS(EIOException);

	/**
	 * Writes a <code>boolean</code> to the underlying output stream as
	 * a 1-byte value. The value <code>true</code> is written out as the
	 * value <code>(byte)1</code>; the value <code>false</code> is
	 * written out as the value <code>(byte)0</code>. If no exception is
	 * thrown, the counter <code>written</code> is incremented by
	 * <code>1</code>.
	 *
	 * @param      v   a <code>boolean</code> value to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	void writeBoolean(boolean v) THROWS(EIOException);

	/**
	 * Writes out a <code>byte</code> to the underlying output stream as
	 * a 1-byte value. If no exception is thrown, the counter
	 * <code>written</code> is incremented by <code>1</code>.
	 *
	 * @param      v   a <code>byte</code> value to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	void writeByte(int v) THROWS(EIOException);

	/**
	 * Writes a <code>short</code> to the underlying output stream as two
	 * bytes, high byte first. If no exception is thrown, the counter
	 * <code>written</code> is incremented by <code>2</code>.
	 *
	 * @param      v   a <code>short</code> to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	void writeShort(int v) THROWS(EIOException);

	/**
	 * Writes an <code>int</code> to the underlying output stream as four
	 * bytes, high byte first. If no exception is thrown, the counter
	 * <code>written</code> is incremented by <code>4</code>.
	 *
	 * @param      v   an <code>int</code> to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	void writeInt(int v) THROWS(EIOException);

	/**
	 * Writes a <code>long</code> to the underlying output stream as eight
	 * bytes, high byte first. In no exception is thrown, the counter
	 * <code>written</code> is incremented by <code>8</code>.
	 *
	 * @param      v   a <code>long</code> to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	void writeLLong(llong v) THROWS(EIOException);

	/**
	 * Converts the float argument to an <code>int</code> using the
	 * <code>floatToIntBits</code> method in class <code>Float</code>,
	 * and then writes that <code>int</code> value to the underlying
	 * output stream as a 4-byte quantity, high byte first. If no
	 * exception is thrown, the counter <code>written</code> is
	 * incremented by <code>4</code>.
	 *
	 * @param      v   a <code>float</code> value to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 * @see        java.lang.Float#floatToIntBits(float)
	 */
	void writeFloat(float v) THROWS(EIOException);

	/**
	 * Converts the double argument to a <code>long</code> using the
	 * <code>doubleToLongBits</code> method in class <code>Double</code>,
	 * and then writes that <code>long</code> value to the underlying
	 * output stream as an 8-byte quantity, high byte first. If no
	 * exception is thrown, the counter <code>written</code> is
	 * incremented by <code>8</code>.
	 *
	 * @param      v   a <code>double</code> value to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 * @see        java.lang.Double#doubleToLongBits(double)
	 */
	void writeDouble(double v) THROWS(EIOException);

	/**
	 * Writes out the string to the underlying output stream as a
	 * sequence of bytes. Each character in the string is written out, in
	 * sequence, by discarding its high eight bits. If no exception is
	 * thrown, the counter <code>written</code> is incremented by the
	 * length of <code>s</code>.
	 *
	 * @param      s   a string of bytes to be written.
	 * @exception  IOException  if an I/O error occurs.
	 * @see        java.io.FilterOutputStream#out
	 */
	void writeBytes(const char* s) THROWS(EIOException);

	/**
	 * Returns the current value of the counter <code>written</code>,
	 * the number of bytes written to this data output stream so far.
	 * If the counter overflows, it will be wrapped to Integer.MAX_VALUE.
	 *
	 * @return  the value of the <code>written</code> field.
	 * @see     java.io.DataOutputStream#written
	 */
	int size();

protected:
	/**
	 * The number of bytes written to the data output stream so far.
	 * If this counter overflows, it will be wrapped to Integer.MAX_VALUE.
	 */
	int written;

private:
	/**
	 * Increases the written counter by the specified value
	 * until it reaches Integer.MAX_VALUE.
	 */
	void incCount(int value);
};

} /* namespace efc */
#endif /* EDATAOUTPUTSTREAM_HH_ */
