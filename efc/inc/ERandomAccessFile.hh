/*
 * ERandomAccessFile.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef ERandomAccessFile_HH_
#define ERandomAccessFile_HH_

#include "EDataOutput.hh"
#include "EDataInput.hh"
#include "EFile.hh"
#include "EFileNotFoundException.hh"
#include "EIOException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

/**
 * Instances of this class support both reading and writing to a 
 * random access file. A random access file behaves like a large 
 * array of bytes stored in the file system. There is a kind of cursor, 
 * or index into the implied array, called the <em>file pointer</em>; 
 * input operations read bytes starting at the file pointer and advance 
 * the file pointer past the bytes read. If the random access file is 
 * created in read/write mode, then output operations are also available; 
 * output operations write bytes starting at the file pointer and advance 
 * the file pointer past the bytes written. Output operations that write 
 * past the current end of the implied array cause the array to be 
 * extended. The file pointer can be read by the 
 * <code>getFilePointer</code> method and set by the <code>seek</code> 
 * method. 
 * <p>
 * It is generally true of all the reading routines in this class that 
 * if end-of-file is reached before the desired number of bytes has been 
 * read, an <code>EOFException</code> (which is a kind of 
 * <code>IOException</code>) is thrown. If any byte cannot be read for 
 * any reason other than end-of-file, an <code>IOException</code> other 
 * than <code>EOFException</code> is thrown. In particular, an 
 * <code>IOException</code> may be thrown if the stream has been closed.
 *
 * @version 1.81, 04/10/06
 * @since   JDK1.0
 */

class ERandomAccessFile : public EDataOutput, public EDataInput {
public:
	virtual ~ERandomAccessFile();
	
    /**
     * Creates a random access file stream to read from, and optionally 
     * to write to, a file with the specified name. A new 
     * {@link FileDescriptor} object is created to represent the 
     * connection to the file.
     * 
     * <p> The <tt>mode</tt> argument specifies the access mode with which the
     * file is to be opened.  The permitted values and their meanings are as
     * specified for the <a
     * href="#mode"><tt>RandomAccessFile(File,String)</tt></a> constructor.
     *
     * <p>
     * If there is a security manager, its <code>checkRead</code> method
     * is called with the <code>name</code> argument
     * as its argument to see if read access to the file is allowed.
     * If the mode allows writing, the security manager's
     * <code>checkWrite</code> method
     * is also called with the <code>name</code> argument
     * as its argument to see if write access to the file is allowed.
     *
     * @param      name   the system-dependent filename
     * @param      mode   the access <a href="#mode">mode</a>
     * @exception  IllegalArgumentException  if the mode argument is not equal
     *               to one of <tt>"r"</tt>, <tt>"rw"</tt>, <tt>"rws"</tt>, or
     *               <tt>"rwd"</tt>
     * @exception FileNotFoundException
     *            if the mode is <tt>"r"</tt> but the given string does not
     *            denote an existing regular file, or if the mode begins with
     *            <tt>"rw"</tt> but the given string does not denote an
     *            existing, writable regular file and a new regular file of
     *            that name cannot be created, or if some other error occurs
     *            while opening or creating the file
     * @exception  SecurityException         if a security manager exists and its
     *               <code>checkRead</code> method denies read access to the file
     *               or the mode is "rw" and the security manager's
     *               <code>checkWrite</code> method denies write access to the file
     * @see        java.lang.SecurityException
     * @see        java.lang.SecurityManager#checkRead(java.lang.String)
     * @see        java.lang.SecurityManager#checkWrite(java.lang.String)
     * @revised 1.4
     * @spec JSR-51
     */
    ERandomAccessFile(const char *name, const char *mode) THROWS(EFileNotFoundException);

    /**
     * Creates a random access file stream to read from, and optionally to
     * write to, the file specified by the {@link File} argument.  A new {@link
     * FileDescriptor} object is created to represent this file connection.
     *
     * <a name="mode"><p> The <tt>mode</tt> argument specifies the access mode
     * in which the file is to be opened.  The permitted values and their
     * meanings are:
     *
     * <blockquote><table summary="Access mode permitted values and meanings">
     * <tr><th><p align="left">Value</p></th><th><p align="left">Meaning</p></th></tr>
     * <tr><td valign="top"><tt>"r"</tt></td>
     *     <td> Open for reading only.  Invoking any of the <tt>write</tt>
     *     methods of the resulting object will cause an {@link
     *     java.io.IOException} to be thrown. </td></tr>
     * <tr><td valign="top"><tt>"rw"</tt></td>
     *     <td> Open for reading and writing.  If the file does not already
     *     exist then an attempt will be made to create it. </td></tr>
     * <tr><td valign="top"><tt>"rws"</tt></td>
     *     <td> Open for reading and writing, as with <tt>"rw"</tt>, and also
     *     require that every update to the file's content or metadata be
     *     written synchronously to the underlying storage device.  </td></tr>
     * <tr><td valign="top"><tt>"rwd"&nbsp;&nbsp;</tt></td>
     *     <td> Open for reading and writing, as with <tt>"rw"</tt>, and also
     *     require that every update to the file's content be written
     *     synchronously to the underlying storage device. </td></tr>
     * </table></blockquote>
     *
     * The <tt>"rws"</tt> and <tt>"rwd"</tt> modes work much like the {@link
     * java.nio.channels.FileChannel#force(boolean) force(boolean)} method of
     * the {@link java.nio.channels.FileChannel} class, passing arguments of
     * <tt>true</tt> and <tt>false</tt>, respectively, except that they always
     * apply to every I/O operation and are therefore often more efficient.  If
     * the file resides on a local storage device then when an invocation of a
     * method of this class returns it is guaranteed that all changes made to
     * the file by that invocation will have been written to that device.  This
     * is useful for ensuring that critical information is not lost in the
     * event of a system crash.  If the file does not reside on a local device
     * then no such guarantee is made.
     *
     * <p> The <tt>"rwd"</tt> mode can be used to reduce the number of I/O
     * operations performed.  Using <tt>"rwd"</tt> only requires updates to the
     * file's content to be written to storage; using <tt>"rws"</tt> requires
     * updates to both the file's content and its metadata to be written, which
     * generally requires at least one more low-level I/O operation.
     *
     * <p> If there is a security manager, its <code>checkRead</code> method is
     * called with the pathname of the <code>file</code> argument as its
     * argument to see if read access to the file is allowed.  If the mode
     * allows writing, the security manager's <code>checkWrite</code> method is
     * also called with the path argument to see if write access to the file is
     * allowed.
     *
     * @param      file   the file object
     * @param      mode   the access mode, as described
     *                    <a href="#mode">above</a>
     * @exception  IllegalArgumentException  if the mode argument is not equal
     *               to one of <tt>"r"</tt>, <tt>"rw"</tt>, <tt>"rws"</tt>, or
     *               <tt>"rwd"</tt>
     * @exception FileNotFoundException
     *            if the mode is <tt>"r"</tt> but the given file object does
     *            not denote an existing regular file, or if the mode begins
     *            with <tt>"rw"</tt> but the given file object does not denote
     *            an existing, writable regular file and a new regular file of
     *            that name cannot be created, or if some other error occurs
     *            while opening or creating the file
     * @exception  SecurityException         if a security manager exists and its
     *               <code>checkRead</code> method denies read access to the file
     *               or the mode is "rw" and the security manager's
     *               <code>checkWrite</code> method denies write access to the file
     * @see        java.lang.SecurityManager#checkRead(java.lang.String)
     * @see        java.lang.SecurityManager#checkWrite(java.lang.String)
     * @see        java.nio.channels.FileChannel#force(boolean)
     * @revised 1.4
     * @spec JSR-51
     */
    ERandomAccessFile(EFile *file, const char *mode) THROWS(EFileNotFoundException);

    // TODO:
    ERandomAccessFile(const ERandomAccessFile& that);
    ERandomAccessFile& operator= (const ERandomAccessFile& that);

    /**
     * Returns the opaque file descriptor object associated with this
     * stream. </p>
     *
     * @return     the file descriptor object associated with this stream.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.FileDescriptor
     */
    es_file_t* getFD() THROWS(EIOException);

    // 'Read' primitives

    /**
     * Reads a byte of data from this file. The byte is returned as an
     * integer in the range 0 to 255 (<code>0x00-0x0ff</code>). This
     * method blocks if no input is yet available.
     * <p>
     * Although <code>RandomAccessFile</code> is not a subclass of
     * <code>InputStream</code>, this method behaves in exactly the same
     * way as the {@link InputStream#read()} method of
     * <code>InputStream</code>.
     *
     * @return     the next byte of data, or <code>-1</code> if the end of the
     *             file has been reached.
     * @exception  IOException  if an I/O error occurs. Not thrown if
     *                          end-of-file has been reached.
     */
    int read() THROWS(EIOException);

    /**
     * Reads up to <code>b.length</code> bytes of data from this file 
     * into an array of bytes. This method blocks until at least one byte 
     * of input is available. 
     * <p>
     * Although <code>RandomAccessFile</code> is not a subclass of 
     * <code>InputStream</code>, this method behaves in exactly the 
     * same way as the {@link InputStream#read(byte[])} method of 
     * <code>InputStream</code>.
     *
     * @param      b   the buffer into which the data is read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             this file has been reached.
     * @exception  IOException If the first byte cannot be read for any reason
     * other than end of file, or if the random access file has been closed, or if
     * some other I/O error occurs.
     * @exception  NullPointerException If <code>b</code> is <code>null</code>.
     */
    int read(void *b, int len) THROWS(EIOException);

    /**
     * Reads <code>b.length</code> bytes from this file into the byte 
     * array, starting at the current file pointer. This method reads 
     * repeatedly from the file until the requested number of bytes are 
     * read. This method blocks until the requested number of bytes are 
     * read, the end of the stream is detected, or an exception is thrown. 
     *
     * @param      b   the buffer into which the data is read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             this file has been reached.
     * @exception  EOFException  if this file reaches the end before reading
     *               all the bytes.
     * @exception  IOException   if an I/O error occurs.       
     */
    void readFully(byte *b, int len) THROWS(EIOException);

    /**
     * Attempts to skip over <code>n</code> bytes of input discarding the 
     * skipped bytes. 
     * <p>
     * 
     * This method may skip over some smaller number of bytes, possibly zero. 
     * This may result from any of a number of conditions; reaching end of 
     * file before <code>n</code> bytes have been skipped is only one 
     * possibility. This method never throws an <code>EOFException</code>. 
     * The actual number of bytes skipped is returned.  If <code>n</code> 
     * is negative, no bytes are skipped.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     */
    int skipBytes(int n) THROWS(EIOException);

    // 'Write' primitives

	/**
     * Writes the specified byte to this file. The write starts at 
     * the current file pointer.
     *
     * @param      b   the <code>byte</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    void write(int b) THROWS(EIOException);
    
    /**
     * Writes <code>b.length</code> bytes from the specified byte array 
     * to this file, starting at the current file pointer. 
     *
     * @param      b   the data.
     * @exception  IOException  if an I/O error occurs.
     */
    void write(const void *b, int len) THROWS(EIOException);

    // 'Random access' stuff

    /**
     * Returns the current offset in this file. 
     *
     * @return     the offset from the beginning of the file, in bytes,
     *             at which the next read or write occurs.
     * @exception  IOException  if an I/O error occurs.
     */
    llong getFilePointer() THROWS(EIOException);

    /**
     * Sets the file-pointer offset, measured from the beginning of this 
     * file, at which the next read or write occurs.  The offset may be 
     * set beyond the end of the file. Setting the offset beyond the end 
     * of the file does not change the file length.  The file length will 
     * change only by writing after the offset has been set beyond the end 
     * of the file. 
     *
     * @param      pos   the offset position, measured in bytes from the 
     *                   beginning of the file, at which to set the file 
     *                   pointer.
     * @exception  IOException  if <code>pos</code> is less than 
     *                          <code>0</code> or if an I/O error occurs.
     */
    void seek(llong pos, int whence=ES_SEEK_SET) THROWS(EIOException);

    /**
     * Returns the length of this file.
     *
     * @return     the length of this file, measured in bytes.
     * @exception  IOException  if an I/O error occurs.
     */
    llong length() THROWS(EIOException);

    /**
     * Sets the length of this file.
     *
     * <p> If the present length of the file as returned by the
     * <code>length</code> method is greater than the <code>newLength</code>
     * argument then the file will be truncated.  In this case, if the file
     * offset as returned by the <code>getFilePointer</code> method is greater
     * than <code>newLength</code> then after this method returns the offset
     * will be equal to <code>newLength</code>.
     *
     * <p> If the present length of the file as returned by the
     * <code>length</code> method is smaller than the <code>newLength</code>
     * argument then the file will be extended.  In this case, the contents of
     * the extended portion of the file are not defined.
     *
     * @param      newLength    The desired length of the file
     * @exception  IOException  If an I/O error occurs
     * @since      1.2
     */
    void setLength(llong newLength) THROWS(EIOException);

    /**
     * Closes this random access file stream and releases any system 
     * resources associated with the stream. A closed random access 
     * file cannot perform input or output operations and cannot be 
     * reopened.
     *
     * <p> If this file has an associated channel then the channel is closed
     * as well.
     *
     * @exception  IOException  if an I/O error occurs.
     *
     * @revised 1.4
     * @spec JSR-51
     */
    void close() THROWS(EIOException);
    
    //
    //  Some "reading/writing Java data types" methods stolen from
    //  DataInputStream and DataOutputStream.
    //

    /**
     * Reads a <code>boolean</code> from this file. This method reads a 
     * single byte from the file, starting at the current file pointer. 
     * A value of <code>0</code> represents 
     * <code>false</code>. Any other value represents <code>true</code>. 
     * This method blocks until the byte is read, the end of the stream 
     * is detected, or an exception is thrown. 
     *
     * @return     the <code>boolean</code> value read.
     * @exception  EOFException  if this file has reached the end.
     * @exception  IOException   if an I/O error occurs.
     */
    boolean readBoolean() THROWS(EIOException);

    /**
     * Reads a signed eight-bit value from this file. This method reads a 
     * byte from the file, starting from the current file pointer. 
     * If the byte read is <code>b</code>, where 
     * <code>0&nbsp;&lt;=&nbsp;b&nbsp;&lt;=&nbsp;255</code>, 
     * then the result is:
     * <blockquote><pre>
     *     (byte)(b)
     * </pre></blockquote>
     * <p>
     * This method blocks until the byte is read, the end of the stream 
     * is detected, or an exception is thrown. 
     *
     * @return     the next byte of this file as a signed eight-bit
     *             <code>byte</code>.
     * @exception  EOFException  if this file has reached the end.
     * @exception  IOException   if an I/O error occurs.
     */
    byte readByte() THROWS(EIOException);

    /**
     * Reads an unsigned eight-bit number from this file. This method reads 
     * a byte from this file, starting at the current file pointer, 
     * and returns that byte. 
     * <p>
     * This method blocks until the byte is read, the end of the stream 
     * is detected, or an exception is thrown. 
     *
     * @return     the next byte of this file, interpreted as an unsigned
     *             eight-bit number.
     * @exception  EOFException  if this file has reached the end.
     * @exception  IOException   if an I/O error occurs.
     */
    ubyte readUnsignedByte() THROWS(EIOException);

    /**
     * Reads a signed 16-bit number from this file. The method reads two 
     * bytes from this file, starting at the current file pointer. 
     * If the two bytes read, in order, are 
     * <code>b1</code> and <code>b2</code>, where each of the two values is 
     * between <code>0</code> and <code>255</code>, inclusive, then the 
     * result is equal to:
     * <blockquote><pre>
     *     (short)((b1 &lt;&lt; 8) | b2)
     * </pre></blockquote>
     * <p>
     * This method blocks until the two bytes are read, the end of the 
     * stream is detected, or an exception is thrown. 
     *
     * @return     the next two bytes of this file, interpreted as a signed
     *             16-bit number.
     * @exception  EOFException  if this file reaches the end before reading
     *               two bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    short readShort() THROWS(EIOException);

    /**
     * Reads an unsigned 16-bit number from this file. This method reads 
     * two bytes from the file, starting at the current file pointer. 
     * If the bytes read, in order, are 
     * <code>b1</code> and <code>b2</code>, where 
     * <code>0&nbsp;&lt;=&nbsp;b1, b2&nbsp;&lt;=&nbsp;255</code>, 
     * then the result is equal to:
     * <blockquote><pre>
     *     (b1 &lt;&lt; 8) | b2
     * </pre></blockquote>
     * <p>
     * This method blocks until the two bytes are read, the end of the 
     * stream is detected, or an exception is thrown. 
     *
     * @return     the next two bytes of this file, interpreted as an unsigned
     *             16-bit integer.
     * @exception  EOFException  if this file reaches the end before reading
     *               two bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    ushort readUnsignedShort() THROWS(EIOException);

    /**
     * Reads a signed 32-bit integer from this file. This method reads 4 
     * bytes from the file, starting at the current file pointer. 
     * If the bytes read, in order, are <code>b1</code>,
     * <code>b2</code>, <code>b3</code>, and <code>b4</code>, where 
     * <code>0&nbsp;&lt;=&nbsp;b1, b2, b3, b4&nbsp;&lt;=&nbsp;255</code>, 
     * then the result is equal to:
     * <blockquote><pre>
     *     (b1 &lt;&lt; 24) | (b2 &lt;&lt; 16) + (b3 &lt;&lt; 8) + b4
     * </pre></blockquote>
     * <p>
     * This method blocks until the four bytes are read, the end of the 
     * stream is detected, or an exception is thrown. 
     *
     * @return     the next four bytes of this file, interpreted as an
     *             <code>int</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *               four bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    int readInt() THROWS(EIOException);

    /**
     * Reads a signed 64-bit integer from this file. This method reads eight
     * bytes from the file, starting at the current file pointer. 
     * If the bytes read, in order, are 
     * <code>b1</code>, <code>b2</code>, <code>b3</code>, 
     * <code>b4</code>, <code>b5</code>, <code>b6</code>, 
     * <code>b7</code>, and <code>b8,</code> where:
     * <blockquote><pre>
     *     0 &lt;= b1, b2, b3, b4, b5, b6, b7, b8 &lt;=255,
     * </pre></blockquote>
     * <p>
     * then the result is equal to:
     * <p><blockquote><pre>
     *     ((long)b1 &lt;&lt; 56) + ((long)b2 &lt;&lt; 48)
     *     + ((long)b3 &lt;&lt; 40) + ((long)b4 &lt;&lt; 32)
     *     + ((long)b5 &lt;&lt; 24) + ((long)b6 &lt;&lt; 16)
     *     + ((long)b7 &lt;&lt; 8) + b8
     * </pre></blockquote>
     * <p>
     * This method blocks until the eight bytes are read, the end of the 
     * stream is detected, or an exception is thrown. 
     *
     * @return     the next eight bytes of this file, interpreted as a
     *             <code>long</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *               eight bytes.
     * @exception  IOException   if an I/O error occurs.
     */
    llong readLLong() THROWS(EIOException);

    /**
     * Reads a <code>float</code> from this file. This method reads an 
     * <code>int</code> value, starting at the current file pointer, 
     * as if by the <code>readInt</code> method 
     * and then converts that <code>int</code> to a <code>float</code> 
     * using the <code>intBitsToFloat</code> method in class 
     * <code>Float</code>. 
     * <p>
     * This method blocks until the four bytes are read, the end of the 
     * stream is detected, or an exception is thrown. 
     *
     * @return     the next four bytes of this file, interpreted as a
     *             <code>float</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *             four bytes.
     * @exception  IOException   if an I/O error occurs.
     * @see        java.io.RandomAccessFile#readInt()
     * @see        java.lang.Float#intBitsToFloat(int)
     */
    float readFloat() THROWS(EIOException);

    /**
     * Reads a <code>double</code> from this file. This method reads a 
     * <code>long</code> value, starting at the current file pointer, 
     * as if by the <code>readLong</code> method 
     * and then converts that <code>long</code> to a <code>double</code> 
     * using the <code>longBitsToDouble</code> method in 
     * class <code>Double</code>.
     * <p>
     * This method blocks until the eight bytes are read, the end of the 
     * stream is detected, or an exception is thrown. 
     *
     * @return     the next eight bytes of this file, interpreted as a
     *             <code>double</code>.
     * @exception  EOFException  if this file reaches the end before reading
     *             eight bytes.
     * @exception  IOException   if an I/O error occurs.
     * @see        java.io.RandomAccessFile#readLong()
     * @see        java.lang.Double#longBitsToDouble(long)
     */
    double readDouble() THROWS(EIOException);

    /**
     * Reads the next line of text from this file.  This method successively
     * reads bytes from the file, starting at the current file pointer, 
     * until it reaches a line terminator or the end
     * of the file.  Each byte is converted into a character by taking the
     * byte's value for the lower eight bits of the character and setting the
     * high eight bits of the character to zero.  This method does not,
     * therefore, support the full Unicode character set.
     *
     * <p> A line of text is terminated by a carriage-return character
     * (<code>'&#92;r'</code>), a newline character (<code>'&#92;n'</code>), a
     * carriage-return character immediately followed by a newline character,
     * or the end of the file.  Line-terminating characters are discarded and
     * are not included as part of the string returned.
     *
     * <p> This method blocks until a newline character is read, a carriage
     * return and the byte following it are read (to see if it is a newline),
     * the end of the file is reached, or an exception is thrown.
     *
     * @return     the next line of text from this file, or null if end
     *             of file is encountered before even one byte is read.
     * @exception  IOException  if an I/O error occurs.
     */
    sp<EString> readLine() THROWS(EIOException);

    /**
     * Writes a <code>boolean</code> to the file as a one-byte value. The 
     * value <code>true</code> is written out as the value 
     * <code>(byte)1</code>; the value <code>false</code> is written out 
     * as the value <code>(byte)0</code>. The write starts at 
     * the current position of the file pointer.
     *
     * @param      v   a <code>boolean</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    void writeBoolean(boolean v) THROWS(EIOException);

    /**
     * Writes a <code>byte</code> to the file as a one-byte value. The 
     * write starts at the current position of the file pointer.
     *
     * @param      v   a <code>byte</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    void writeByte(int v) THROWS(EIOException);

    /**
     * Writes a <code>short</code> to the file as two bytes, high byte first. 
     * The write starts at the current position of the file pointer.
     *
     * @param      v   a <code>short</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    void writeShort(int v) THROWS(EIOException);

    /**
     * Writes an <code>int</code> to the file as four bytes, high byte first. 
     * The write starts at the current position of the file pointer.
     *
     * @param      v   an <code>int</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    void writeInt(int v) THROWS(EIOException);

    /**
     * Writes a <code>long</code> to the file as eight bytes, high byte first. 
     * The write starts at the current position of the file pointer.
     *
     * @param      v   a <code>long</code> to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    void writeLLong(llong v) THROWS(EIOException);

    /**
     * Converts the float argument to an <code>int</code> using the 
     * <code>floatToIntBits</code> method in class <code>Float</code>, 
     * and then writes that <code>int</code> value to the file as a 
     * four-byte quantity, high byte first. The write starts at the 
     * current position of the file pointer.
     *
     * @param      v   a <code>float</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.lang.Float#floatToIntBits(float)
     */
    void writeFloat(float v) THROWS(EIOException);

    /**
     * Converts the double argument to a <code>long</code> using the 
     * <code>doubleToLongBits</code> method in class <code>Double</code>, 
     * and then writes that <code>long</code> value to the file as an 
     * eight-byte quantity, high byte first. The write starts at the current 
     * position of the file pointer.
     *
     * @param      v   a <code>double</code> value to be written.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.lang.Double#doubleToLongBits(double)
     */
    void writeDouble(double v) THROWS(EIOException);

    /**
     * Writes the string to the file as a sequence of bytes. Each 
     * character in the string is written out, in sequence, by discarding 
     * its high eight bits. The write starts at the current position of 
     * the file pointer.
     *
     * @param      s   a string of bytes to be written.
     * @exception  IOException  if an I/O error occurs.
     */
    void writeBytes(const char* s) THROWS(EIOException);

private:
    es_os_file_t mFD;
    es_file_t *mFile;
    boolean mSync;
    
    void open0(const char *name, const char *mode) THROWS(EFileNotFoundException);

    void fsync0() THROWS(EIOException);
};

} /* namespace efc */
#endif //!ERandomAccessFile_HH_
