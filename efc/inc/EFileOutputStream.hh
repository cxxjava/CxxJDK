/*
 * EFileOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EFileOutputStream_HH_
#define EFileOutputStream_HH_

#include "EOutputStream.hh"
#include "EFile.hh"
#include "ESynchronizeable.hh"
#include "./concurrent/EReentrantLock.hh"
#include "EFileNotFoundException.hh"

namespace efc {

/**
 * A file output stream is an output stream for writing data to a 
 * <code>File</code> or to a <code>FileDescriptor</code>. Whether or not
 * a file is available or may be created depends upon the underlying
 * platform.  Some platforms, in particular, allow a file to be opened
 * for writing by only one <tt>FileOutputStream</tt> (or other
 * file-writing object) at a time.  In such situations the constructors in
 * this class will fail if the file involved is already open.
 *
 * <p><code>FileOutputStream</code> is meant for writing streams of raw bytes
 * such as image data. For writing streams of characters, consider using
 * <code>FileWriter</code>.
 *
 * @version 1.58, 11/17/05
 * @see     java.io.File
 * @see     java.io.FileDescriptor
 * @see     java.io.FileInputStream
 * @since   JDK1.0
 */

namespace nio {
class EFileChannel;
}

class EFileOutputStream : public EOutputStream, public ESynchronizeable
{
public:
	virtual ~EFileOutputStream();
	
	/**
     * Creates an output file stream to write to the file with the 
     * specified name. A new <code>FileDescriptor</code> object is 
     * created to represent this file connection.
     * <p>
     * First, if there is a security manager, its <code>checkWrite</code> 
     * method is called with <code>name</code> as its argument.
     * <p>
     * If the file exists but is a directory rather than a regular file, does
     * not exist but cannot be created, or cannot be opened for any other
     * reason then a <code>FileNotFoundException</code> is thrown.
     *
     * @param      name   the system-dependent filename
     * @exception  FileNotFoundException  if the file exists but is a directory
     *                   rather than a regular file, does not exist but cannot
     *                   be created, or cannot be opened for any other reason
     * @exception  SecurityException  if a security manager exists and its
     *               <code>checkWrite</code> method denies write access
     *               to the file.
     * @see        java.lang.SecurityManager#checkWrite(java.lang.String)
     */
    EFileOutputStream(const char *name) THROWS(EFileNotFoundException);

	/**
     * Creates an output file stream to write to the file with the specified
     * <code>name</code>.  If the second argument is <code>true</code>, then
     * bytes will be written to the end of the file rather than the beginning.
     * A new <code>FileDescriptor</code> object is created to represent this
     * file connection.
     * <p>
     * First, if there is a security manager, its <code>checkWrite</code> 
     * method is called with <code>name</code> as its argument.
     * <p>
     * If the file exists but is a directory rather than a regular file, does
     * not exist but cannot be created, or cannot be opened for any other
     * reason then a <code>FileNotFoundException</code> is thrown.
     * 
     * @param     name        the system-dependent file name
     * @param     append      if <code>true</code>, then bytes will be written
     *                   to the end of the file rather than the beginning
     * @exception  FileNotFoundException  if the file exists but is a directory
     *                   rather than a regular file, does not exist but cannot
     *                   be created, or cannot be opened for any other reason.
     * @exception  SecurityException  if a security manager exists and its
     *               <code>checkWrite</code> method denies write access
     *               to the file.
     * @see        java.lang.SecurityManager#checkWrite(java.lang.String)
     * @since     JDK1.1
     */
    EFileOutputStream(const char *name, boolean append) THROWS(EFileNotFoundException);

	/**
     * Creates a file output stream to write to the file represented by 
     * the specified <code>File</code> object. A new 
     * <code>FileDescriptor</code> object is created to represent this 
     * file connection.
     * <p>
     * First, if there is a security manager, its <code>checkWrite</code> 
     * method is called with the path represented by the <code>file</code> 
     * argument as its argument.
     * <p>
     * If the file exists but is a directory rather than a regular file, does
     * not exist but cannot be created, or cannot be opened for any other
     * reason then a <code>FileNotFoundException</code> is thrown.
     *
     * @param      file               the file to be opened for writing.
     * @exception  FileNotFoundException  if the file exists but is a directory
     *                   rather than a regular file, does not exist but cannot
     *                   be created, or cannot be opened for any other reason
     * @exception  SecurityException  if a security manager exists and its
     *               <code>checkWrite</code> method denies write access
     *               to the file.
     * @see        java.io.File#getPath()
     * @see        java.lang.SecurityException
     * @see        java.lang.SecurityManager#checkWrite(java.lang.String)
     */
    EFileOutputStream(EFile *file) THROWS(EFileNotFoundException);
    
    /**
     * Creates a file output stream to write to the file represented by 
     * the specified <code>File</code> object. If the second argument is
     * <code>true</code>, then bytes will be written to the end of the file
     * rather than the beginning. A new <code>FileDescriptor</code> object is
     * created to represent this file connection.
     * <p>
     * First, if there is a security manager, its <code>checkWrite</code> 
     * method is called with the path represented by the <code>file</code> 
     * argument as its argument.
     * <p>
     * If the file exists but is a directory rather than a regular file, does
     * not exist but cannot be created, or cannot be opened for any other
     * reason then a <code>FileNotFoundException</code> is thrown.
     *
     * @param      file               the file to be opened for writing.
     * @param     append      if <code>true</code>, then bytes will be written
     *                   to the end of the file rather than the beginning
     * @exception  FileNotFoundException  if the file exists but is a directory
     *                   rather than a regular file, does not exist but cannot
     *                   be created, or cannot be opened for any other reason
     * @exception  SecurityException  if a security manager exists and its
     *               <code>checkWrite</code> method denies write access
     *               to the file.
     * @see        java.io.File#getPath()
     * @see        java.lang.SecurityException
     * @see        java.lang.SecurityManager#checkWrite(java.lang.String)
     * @since 1.4
     */
    EFileOutputStream(EFile *file, boolean append) THROWS(EFileNotFoundException);
	
	/**
     * Creates an output file stream to write to the specified file 
     * descriptor, which represents an existing connection to an actual 
     * file in the file system.
     * <p>
     * First, if there is a security manager, its <code>checkWrite</code> 
     * method is called with the file descriptor <code>fdObj</code> 
     * argument as its argument.
     *
     * @param      fdObj   the file descriptor to be opened for writing
     * @exception  SecurityException  if a security manager exists and its
     *               <code>checkWrite</code> method denies
     *               write access to the file descriptor
     * @see        java.lang.SecurityManager#checkWrite(java.io.FileDescriptor)
     */
    EFileOutputStream(es_file_t* file);
    EFileOutputStream(es_os_file_t hfile);
	
    //TODO:
    EFileOutputStream(const EFileOutputStream& that);
    EFileOutputStream& operator= (const EFileOutputStream& that);

	/**
     * Writes <code>b.length</code> bytes from the specified byte array 
     * to this file output stream. 
     *
     * @param      b   the data.
     * @exception  IOException  if an I/O error occurs.
     */
    virtual void write(const void *b, int len) THROWS(EIOException);
    virtual void write(const char *s) THROWS(EIOException);
    virtual void write(int b) THROWS(EIOException);
    
	/**
     * Closes this file output stream and releases any system resources 
     * associated with this stream. This file output stream may no longer 
     * be used for writing bytes. 
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
     * Returns the file descriptor associated with this stream.
     *
     * @return  the <code>FileDescriptor</code> object that represents 
     *          the connection to the file in the file system being used 
     *          by this <code>FileOutputStream</code> object. 
     * 
     * @exception  IOException  if an I/O error occurs.
     * @see        java.io.FileDescriptor
     */
    es_file_t* getFD() THROWS(EIOException);

    /**
	 * Returns the unique {@link java.nio.channels.FileChannel FileChannel}
	 * object associated with this file output stream.
	 *
	 * <p> The initial {@link java.nio.channels.FileChannel#position()
	 * position} of the returned channel will be equal to the
	 * number of bytes written to the file so far unless this stream is in
	 * append mode, in which case it will be equal to the size of the file.
	 * Writing bytes to this stream will increment the channel's position
	 * accordingly.  Changing the channel's position, either explicitly or by
	 * writing, will change this stream's file position.
	 *
	 * @return  the file channel associated with this file output stream
	 *
	 * @since 1.4
	 * @spec JSR-51
	 */
    nio::EFileChannel* getChannel();

    /**
     * Use buffered.
     */
    boolean isIOBuffered();
    void setIOBuffered(boolean onoff);
	
private:
	es_file_t *mFile;
	boolean needClose;
	boolean mBuffered;

	EReentrantLock closeLock;
	volatile boolean closed;// = false;

	/**
	 * True if the file is opened for append.
	 */
	boolean append;

	/**
	 * The associated channel, initialized lazily.
	 */
	nio::EFileChannel* channel;
};

} /* namespace efc */
#endif //!EFileOutputStream_HH_
