/*
 * EDataInput.hh
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#ifndef EDATAINPUT_HH_
#define EDATAINPUT_HH_

#include "ESharedPtr.hh"
#include "EIOException.hh"

namespace efc {

/**
 * The <code>DataInput</code> interface provides
 * for reading bytes from a binary stream and
 * reconstructing from them data in any of
 * the Java primitive types. There is also
 * a
 * facility for reconstructing a <code>String</code>
 * from data in
 * <a href="#modified-utf-8">modified UTF-8</a>
 * format.
 * <p>
 * It is generally true of all the reading
 * routines in this interface that if end of
 * file is reached before the desired number
 * of bytes has been read, an <code>EOFException</code>
 * (which is a kind of <code>IOException</code>)
 * is thrown. If any byte cannot be read for
 * any reason other than end of file, an <code>IOException</code>
 * other than <code>EOFException</code> is
 * thrown. In particular, an <code>IOException</code>
 * may be thrown if the input stream has been
 * closed.
 *
 * <h4><a name="modified-utf-8">Modified UTF-8</a></h4>
 * <p>
 * Implementations of the DataInput and DataOutput interfaces represent
 * Unicode strings in a format that is a slight modification of UTF-8.
 * (For information regarding the standard UTF-8 format, see section
 * <i>3.9 Unicode Encoding Forms</i> of <i>The Unicode Standard, Version
 * 4.0</i>).
 * Note that in the following tables, the most significant bit appears in the
 * far left-hand column.
 * <p>
 * All characters in the range <code>'&#92;u0001'</code> to
 * <code>'&#92;u007F'</code> are represented by a single byte:
 *
 * <blockquote>
 *   <table border="1" cellspacing="0" cellpadding="8" width="50%"
 *          summary="Bit values and bytes">
 *     <tr>
 *       <td></td>
 *       <th id="bit">Bit Values</th>
 *     </tr>
 *     <tr>
 *       <th id="byte1">Byte 1</th>
 *       <td>
 *         <table border="1" cellspacing="0" width="100%">
 *           <tr>
 *             <td width="12%"><center>0</center>
 *             <td colspan="7"><center>bits 6-0</center>
 *           </tr>
 *         </table>
 *       </td>
 *     </tr>
 *   </table>
 * </blockquote>
 *
 * <p>
 * The null character <code>'&#92;u0000'</code> and characters in the
 * range <code>'&#92;u0080'</code> to <code>'&#92;u07FF'</code> are
 * represented by a pair of bytes:
 *
 * <blockquote>
 *   <table border="1" cellspacing="0" cellpadding="8" width="50%"
 *          summary="Bit values and bytes">
 *     <tr>
 *       <td></td>
 *       <th id="bit">Bit Values</th>
 *     </tr>
 *     <tr>
 *       <th id="byte1">Byte 1</th>
 *       <td>
 *         <table border="1" cellspacing="0" width="100%">
 *           <tr>
 *             <td width="12%"><center>1</center>
 *             <td width="13%"><center>1</center>
 *             <td width="12%"><center>0</center>
 *             <td colspan="5"><center>bits 10-6</center>
 *           </tr>
 *         </table>
 *       </td>
 *     </tr>
 *     <tr>
 *       <th id="byte2">Byte 2</th>
 *       <td>
 *         <table border="1" cellspacing="0" width="100%">
 *           <tr>
 *             <td width="12%"><center>1</center>
 *             <td width="13%"><center>0</center>
 *             <td colspan="6"><center>bits 5-0</center>
 *           </tr>
 *         </table>
 *       </td>
 *     </tr>
 *   </table>
 *  </blockquote>
 *
 * <br>
 * <code>char</code> values in the range <code>'&#92;u0800'</code> to
 * <code>'&#92;uFFFF'</code> are represented by three bytes:
 *
 * <blockquote>
 *   <table border="1" cellspacing="0" cellpadding="8" width="50%"
 *          summary="Bit values and bytes">
 *     <tr>
 *       <td></td>
 *       <th id="bit">Bit Values</th>
 *     </tr>
 *     <tr>
 *       <th id="byte1">Byte 1</th>
 *       <td>
 *         <table border="1" cellspacing="0" width="100%">
 *           <tr>
 *             <td width="12%"><center>1</center>
 *             <td width="13%"><center>1</center>
 *             <td width="12%"><center>1</center>
 *             <td width="13%"><center>0</center>
 *             <td colspan="4"><center>bits 15-12</center>
 *           </tr>
 *         </table>
 *       </td>
 *     </tr>
 *     <tr>
 *       <th id="byte2">Byte 2</th>
 *       <td>
 *         <table border="1" cellspacing="0" width="100%">
 *           <tr>
 *             <td width="12%"><center>1</center>
 *             <td width="13%"><center>0</center>
 *             <td colspan="6"><center>bits 11-6</center>
 *           </tr>
 *         </table>
 *       </td>
 *     </tr>
 *     <tr>
 *       <th id="byte3">Byte 3</th>
 *       <td>
 *         <table border="1" cellspacing="0" width="100%">
 *           <tr>
 *             <td width="12%"><center>1</center>
 *             <td width="13%"><center>0</center>
 *             <td colspan="6"><center>bits 5-0</center>
 *           </tr>
 *         </table>
 *       </td>
 *     </tr>
 *   </table>
 *  </blockquote>
 *
 * <p>
 * The differences between this format and the
 * standard UTF-8 format are the following:
 * <ul>
 * <li>The null byte <code>'&#92;u0000'</code> is encoded in 2-byte format
 *     rather than 1-byte, so that the encoded strings never have
 *     embedded nulls.
 * <li>Only the 1-byte, 2-byte, and 3-byte formats are used.
 * <li><a href="../lang/Character.html#unicode">Supplementary characters</a>
 *     are represented in the form of surrogate pairs.
 * </ul>
 * @author  Frank Yellin
 * @version 1.25, 04/10/06
 * @see     java.io.DataInputStream
 * @see     java.io.DataOutput
 * @since   JDK1.0
 */

interface EDataInput : virtual public EObject {
	virtual ~EDataInput() {
	}

	/**
	 *
	 * Reads <code>len</code>
	 * bytes from
	 * an input stream.
	 * <p>
	 * This method
	 * blocks until one of the following conditions
	 * occurs:<p>
	 * <ul>
	 * <li><code>len</code> bytes
	 * of input data are available, in which case
	 * a normal return is made.
	 *
	 * <li>End of file
	 * is detected, in which case an <code>EOFException</code>
	 * is thrown.
	 *
	 * <li>An I/O error occurs, in
	 * which case an <code>IOException</code> other
	 * than <code>EOFException</code> is thrown.
	 * </ul>
	 * <p>
	 * If <code>b</code> is <code>null</code>,
	 * a <code>NullPointerException</code> is thrown.
	 * If <code>off</code> is negative, or <code>len</code>
	 * is negative, or <code>off+len</code> is
	 * greater than the length of the array <code>b</code>,
	 * then an <code>IndexOutOfBoundsException</code>
	 * is thrown.
	 * If <code>len</code> is zero,
	 * then no bytes are read. Otherwise, the first
	 * byte read is stored into element <code>b[off]</code>,
	 * the next one into <code>b[off+1]</code>,
	 * and so on. The number of bytes read is,
	 * at most, equal to <code>len</code>.
	 *
	 * @param     b   the buffer into which the data is read.
	 * @param len  an int specifying the number of bytes to read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual void readFully(byte* b, int len) THROWS(EIOException) = 0;

	/**
	 * Makes an attempt to skip over
	 * <code>n</code> bytes
	 * of data from the input
	 * stream, discarding the skipped bytes. However,
	 * it may skip
	 * over some smaller number of
	 * bytes, possibly zero. This may result from
	 * any of a
	 * number of conditions; reaching
	 * end of file before <code>n</code> bytes
	 * have been skipped is
	 * only one possibility.
	 * This method never throws an <code>EOFException</code>.
	 * The actual
	 * number of bytes skipped is returned.
	 *
	 * @param      n   the number of bytes to be skipped.
	 * @return     the number of bytes actually skipped.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual int skipBytes(int n) THROWS(EIOException) = 0;

	/**
	 * Reads one input byte and returns
	 * <code>true</code> if that byte is nonzero,
	 * <code>false</code> if that byte is zero.
	 * This method is suitable for reading
	 * the byte written by the <code>writeBoolean</code>
	 * method of interface <code>DataOutput</code>.
	 *
	 * @return     the <code>boolean</code> value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual boolean readBoolean() THROWS(EIOException) = 0;

	/**
	 * Reads and returns one input byte.
	 * The byte is treated as a signed value in
	 * the range <code>-128</code> through <code>127</code>,
	 * inclusive.
	 * This method is suitable for
	 * reading the byte written by the <code>writeByte</code>
	 * method of interface <code>DataOutput</code>.
	 *
	 * @return     the 8-bit value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual byte readByte() THROWS(EIOException) = 0;

	/**
	 * Reads one input byte, zero-extends
	 * it to type <code>int</code>, and returns
	 * the result, which is therefore in the range
	 * <code>0</code>
	 * through <code>255</code>.
	 * This method is suitable for reading
	 * the byte written by the <code>writeByte</code>
	 * method of interface <code>DataOutput</code>
	 * if the argument to <code>writeByte</code>
	 * was intended to be a value in the range
	 * <code>0</code> through <code>255</code>.
	 *
	 * @return     the unsigned 8-bit value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual ubyte readUnsignedByte() THROWS(EIOException) = 0;

	/**
	 * Reads two input bytes and returns
	 * a <code>short</code> value. Let <code>a</code>
	 * be the first byte read and <code>b</code>
	 * be the second byte. The value
	 * returned
	 * is:
	 * <p><pre><code>(short)((a &lt;&lt; 8) | (b &amp; 0xff))
	 * </code></pre>
	 * This method
	 * is suitable for reading the bytes written
	 * by the <code>writeShort</code> method of
	 * interface <code>DataOutput</code>.
	 *
	 * @return     the 16-bit value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual short readShort() THROWS(EIOException) = 0;

	/**
	 * Reads two input bytes and returns
	 * an <code>int</code> value in the range <code>0</code>
	 * through <code>65535</code>. Let <code>a</code>
	 * be the first byte read and
	 * <code>b</code>
	 * be the second byte. The value returned is:
	 * <p><pre><code>(((a &amp; 0xff) &lt;&lt; 8) | (b &amp; 0xff))
	 * </code></pre>
	 * This method is suitable for reading the bytes
	 * written by the <code>writeShort</code> method
	 * of interface <code>DataOutput</code>  if
	 * the argument to <code>writeShort</code>
	 * was intended to be a value in the range
	 * <code>0</code> through <code>65535</code>.
	 *
	 * @return     the unsigned 16-bit value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual ushort readUnsignedShort() THROWS(EIOException) = 0;

	/**
	 * Reads four input bytes and returns an
	 * <code>int</code> value. Let <code>a-d</code>
	 * be the first through fourth bytes read. The value returned is:
	 * <p><pre>
	 * <code>
	 * (((a &amp; 0xff) &lt;&lt; 24) | ((b &amp; 0xff) &lt;&lt; 16) |
	 * &#32;((c &amp; 0xff) &lt;&lt; 8) | (d &amp; 0xff))
	 * </code></pre>
	 * This method is suitable
	 * for reading bytes written by the <code>writeInt</code>
	 * method of interface <code>DataOutput</code>.
	 *
	 * @return     the <code>int</code> value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual int readInt() THROWS(EIOException) = 0;

	/**
	 * Reads eight input bytes and returns
	 * a <code>long</code> value. Let <code>a-h</code>
	 * be the first through eighth bytes read.
	 * The value returned is:
	 * <p><pre> <code>
	 * (((long)(a &amp; 0xff) &lt;&lt; 56) |
	 *  ((long)(b &amp; 0xff) &lt;&lt; 48) |
	 *  ((long)(c &amp; 0xff) &lt;&lt; 40) |
	 *  ((long)(d &amp; 0xff) &lt;&lt; 32) |
	 *  ((long)(e &amp; 0xff) &lt;&lt; 24) |
	 *  ((long)(f &amp; 0xff) &lt;&lt; 16) |
	 *  ((long)(g &amp; 0xff) &lt;&lt;  8) |
	 *  ((long)(h &amp; 0xff)))
	 * </code></pre>
	 * <p>
	 * This method is suitable
	 * for reading bytes written by the <code>writeLong</code>
	 * method of interface <code>DataOutput</code>.
	 *
	 * @return     the <code>long</code> value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual llong readLLong() THROWS(EIOException) = 0;

	/**
	 * Reads four input bytes and returns
	 * a <code>float</code> value. It does this
	 * by first constructing an <code>int</code>
	 * value in exactly the manner
	 * of the <code>readInt</code>
	 * method, then converting this <code>int</code>
	 * value to a <code>float</code> in
	 * exactly the manner of the method <code>Float.intBitsToFloat</code>.
	 * This method is suitable for reading
	 * bytes written by the <code>writeFloat</code>
	 * method of interface <code>DataOutput</code>.
	 *
	 * @return     the <code>float</code> value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual float readFloat() THROWS(EIOException) = 0;

	/**
	 * Reads eight input bytes and returns
	 * a <code>double</code> value. It does this
	 * by first constructing a <code>long</code>
	 * value in exactly the manner
	 * of the <code>readlong</code>
	 * method, then converting this <code>long</code>
	 * value to a <code>double</code> in exactly
	 * the manner of the method <code>Double.longBitsToDouble</code>.
	 * This method is suitable for reading
	 * bytes written by the <code>writeDouble</code>
	 * method of interface <code>DataOutput</code>.
	 *
	 * @return     the <code>double</code> value read.
	 * @exception  EOFException  if this stream reaches the end before reading
	 *               all the bytes.
	 * @exception  IOException   if an I/O error occurs.
	 */
	virtual double readDouble() THROWS(EIOException) = 0;

	/**
	 * Reads the next line of text from the input stream.
	 * It reads successive bytes, converting
	 * each byte separately into a character,
	 * until it encounters a line terminator or
	 * end of
	 * file; the characters read are then
	 * returned as a <code>String</code>. Note
	 * that because this
	 * method processes bytes,
	 * it does not support input of the full Unicode
	 * character set.
	 * <p>
	 * If end of file is encountered
	 * before even one byte can be read, then <code>null</code>
	 * is returned. Otherwise, each byte that is
	 * read is converted to type <code>char</code>
	 * by zero-extension. If the character <code>'\n'</code>
	 * is encountered, it is discarded and reading
	 * ceases. If the character <code>'\r'</code>
	 * is encountered, it is discarded and, if
	 * the following byte converts &#32;to the
	 * character <code>'\n'</code>, then that is
	 * discarded also; reading then ceases. If
	 * end of file is encountered before either
	 * of the characters <code>'\n'</code> and
	 * <code>'\r'</code> is encountered, reading
	 * ceases. Once reading has ceased, a <code>String</code>
	 * is returned that contains all the characters
	 * read and not discarded, taken in order.
	 * Note that every character in this string
	 * will have a value less than <code>&#92;u0100</code>,
	 * that is, <code>(char)256</code>.
	 *
	 * @return the next line of text from the input stream,
	 *         or <CODE>null</CODE> if the end of file is
	 *         encountered before a byte can be read.
	 * @exception  IOException  if an I/O error occurs.
	 */
	virtual sp<EString> readLine() THROWS(EIOException) = 0;
};

} /* namespace efc */
#endif /* EDATAINPUT_HH_ */
