/*
 * EFileInputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EFileInputStream_HH_
#define EFileInputStream_HH_

#include "EInputStream.hh"
#include "EFile.hh"
#include "EFileNotFoundException.hh"

namespace efc {

/**
 * A <code>FileInputStream</code> obtains input bytes
 * from a file in a file system. What files
 * are  available depends on the host environment.
 *
 * <p><code>FileInputStream</code> is meant for reading streams of raw bytes
 * such as image data. For reading streams of characters, consider using
 * <code>FileReader</code>.
 *
 * @version 1.68, 04/07/06
 * @see     java.io.File
 * @see     java.io.FileDescriptor
 * @see	    java.io.FileOutputStream
 * @since   JDK1.0
 */

class EFileInputStream : public EInputStream
{
public:
	virtual ~EFileInputStream();
	
	/**
     * Creates a <code>FileInputStream</code> by
     * opening a connection to an actual file,
     * the file named by the path name <code>name</code>
     * in the file system.  A new <code>FileDescriptor</code>
     * object is created to represent this file
     * connection.
     * <p>
     * First, if there is a security
     * manager, its <code>checkRead</code> method
     * is called with the <code>name</code> argument
     * as its argument.
     * <p>
     * If the named file does not exist, is a directory rather than a regular
     * file, or for some other reason cannot be opened for reading then a
     * <code>FileNotFoundException</code> is thrown.
     *
     * @param      name   the system-dependent file name.
     * @exception  FileNotFoundException  if the file does not exist,
     *                   is a directory rather than a regular file,
     *                   or for some other reason cannot be opened for
     *                   reading.
     * @exception  SecurityException      if a security manager exists and its
     *               <code>checkRead</code> method denies read access
     *               to the file.
     * @see        java.lang.SecurityManager#checkRead(java.lang.String)
     */
	EFileInputStream(const char *name) THROWS(EFileNotFoundException);

    /**
     * Creates a <code>FileInputStream</code> by
     * opening a connection to an actual file,
     * the file named by the <code>File</code>
     * object <code>file</code> in the file system.
     * A new <code>FileDescriptor</code> object
     * is created to represent this file connection.
     * <p>
     * First, if there is a security manager,
     * its <code>checkRead</code> method  is called
     * with the path represented by the <code>file</code>
     * argument as its argument.
     * <p>
     * If the named file does not exist, is a directory rather than a regular
     * file, or for some other reason cannot be opened for reading then a
     * <code>FileNotFoundException</code> is thrown.
     *
     * @param      file   the file to be opened for reading.
     * @exception  FileNotFoundException  if the file does not exist,
     *                   is a directory rather than a regular file,
     *                   or for some other reason cannot be opened for
     *                   reading.
     * @exception  SecurityException      if a security manager exists and its
     *               <code>checkRead</code> method denies read access to the file.
     * @see        java.io.File#getPath()
     * @see        java.lang.SecurityManager#checkRead(java.lang.String)
     */
    EFileInputStream(EFile *file) THROWS(EFileNotFoundException);

    /**
     * Creates a <code>FileInputStream</code> by using the file descriptor
     * <code>fdObj</code>, which represents an existing connection to an
     * actual file in the file system.
     * <p>
     * If there is a security manager, its <code>checkRead</code> method is
     * called with the file descriptor <code>fdObj</code> as its argument to
     * see if it's ok to read the file descriptor. If read access is denied
     * to the file descriptor a <code>SecurityException</code> is thrown.
     * <p>
     * If <code>fdObj</code> is null then a <code>NullPointerException</code>
     * is thrown.
     *
     * @param      fdObj   the file descriptor to be opened for reading.
     * @throws     SecurityException      if a security manager exists and its
     *                 <code>checkRead</code> method denies read access to the
     *                 file descriptor.
     * @see        SecurityManager#checkRead(java.io.FileDescriptor)
     */
    EFileInputStream(es_file_t* file);
    EFileInputStream(es_os_file_t hfile);

    //TODO:
    EFileInputStream(const EFileInputStream& that);
    EFileInputStream& operator= (const EFileInputStream& that);

    /**
	 * Reads a byte of data from this input stream. This method blocks
	 * if no input is yet available.
	 *
	 * @return     the next byte of data, or <code>-1</code> if the end of the
	 *             file is reached.
	 * @exception  IOException  if an I/O error occurs.
	 */
    virtual int read() THROWS(EIOException);

    /**
     * Reads up to <code>b.length</code> bytes of data from this input
     * stream into an array of bytes. This method blocks until some input
     * is available.
     *
     * @param      b   the buffer into which the data is read.
     * @return     the total number of bytes read into the buffer, or
     *             <code>-1</code> if there is no more data because the end of
     *             the file has been reached.
     * @exception  IOException  if an I/O error occurs.
     */
    virtual int read(void *b, int len) THROWS(EIOException);

    /**
     * Skips over and discards <code>n</code> bytes of data from the
     * input stream.
     *
     * <p>The <code>skip</code> method may, for a variety of
     * reasons, end up skipping over some smaller number of bytes,
     * possibly <code>0</code>. If <code>n</code> is negative, an
     * <code>IOException</code> is thrown, even though the <code>skip</code>
     * method of the {@link InputStream} superclass does nothing in this case.
     * The actual number of bytes skipped is returned.
     *
     * <p>This method may skip more bytes than are remaining in the backing
     * file. This produces no exception and the number of bytes skipped
     * may include some number of bytes that were beyond the EOF of the
     * backing file. Attempting to read from the stream after skipping past
     * the end will result in -1 indicating the end of the file.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if n is negative, if the stream does not
     *                   support seek, or if an I/O error occurs.
     */
    virtual long skip(long n) THROWS(EIOException);

    /**
     * Returns an estimate of the number of remaining bytes that can be read (or
     * skipped over) from this input stream without blocking by the next
     * invocation of a method for this input stream. The next invocation might be
     * the same thread or another thread.  A single read or skip of this
     * many bytes will not block, but may read or skip fewer bytes.
     *
     * <p> In some cases, a non-blocking read (or skip) may appear to be
     * blocked when it is merely slow, for example when reading large
     * files over slow networks.
     *
     * @return     an estimate of the number of remaining bytes that can be read
     *             (or skipped over) from this input stream without blocking.
     * @exception  IOException  if this file input stream has been closed by calling
     *             {@code close} or an I/O error occurs.
     */
    virtual long available() THROWS(EIOException);

    /**
     * Closes this file input stream and releases any system resources
     * associated with the stream.
     *
     * <p> If this stream has an associated channel then the channel is closed
     * as well.
     *
     * @exception  IOException  if an I/O error occurs.
     *
     * @revised 1.4
     * @spec JSR-51
     */
    virtual void close() THROWS(EIOException);

    /**
     * Returns the <code>FileDescriptor</code>
     * object  that represents the connection to
     * the actual file in the file system being
     * used by this <code>FileInputStream</code>.
     *
     * @return     the file descriptor object associated with this stream.
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.FileDescriptor
     */
    es_file_t* getFD() THROWS(EIOException);

    /**
	 * Use buffered.
	 */
	boolean isIOBuffered();
	void setIOBuffered(boolean onoff);

private:
    es_os_file_t mFD;
	es_file_t *mFile;
	boolean needClose;

	/**
	 * The internal buffer array where the data is stored. When necessary,
	 * it may be replaced by another array of
	 * a different size.
	 */
	EA<byte>* buf;

	/**
	 * The index one greater than the index of the last valid byte in
	 * the buffer.
	 * This value is always
	 * in the range <code>0</code> through <code>buf.length</code>;
	 * elements <code>buf[0]</code>  through <code>buf[count-1]
	 * </code>contain buffered input data obtained
	 * from the underlying  input stream.
	 */
	long count;

	/**
	 * The current position in the buffer. This is the index of the next
	 * character to be read from the <code>buf</code> array.
	 * <p>
	 * This value is always in the range <code>0</code>
	 * through <code>count</code>. If it is less
	 * than <code>count</code>, then  <code>buf[pos]</code>
	 * is the next byte to be supplied as input;
	 * if it is equal to <code>count</code>, then
	 * the  next <code>read</code> or <code>skip</code>
	 * operation will require more bytes to be
	 * read from the contained  input stream.
	 *
	 * @see     java.io.BufferedInputStream#buf
	 */
	long pos;

	/**
	 * Fills the buffer with more data, taking into account
	 * shuffling and other tricks for dealing with marks.
	 * Assumes that it is being called by a synchronized method.
	 * This method also assumes that all data has already been read in,
	 * hence pos > count.
	 */
	void fill() THROWS(EIOException);

	/**
	 * Read characters into a portion of an array, reading from the underlying
	 * stream at most once if necessary.
	 */
	int read1(void* b, int off, int len) THROWS(EIOException);

	int read0(void* b, int len) THROWS(EIOException);
	long skip0(long n) THROWS(EIOException);
	long available0() THROWS(EIOException);
};

} /* namespace efc */
#endif //!EFileInputStream_HH_
