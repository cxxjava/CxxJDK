/*
 * EFileChannel.hh
 *
 *  Created on: 2014-2-15
 *      Author: cxxjava@163.com
 */

#ifndef EFILECHANNEL_HH_
#define EFILECHANNEL_HH_

#include "EA.hh"
#include "EFileLock.hh"
#include "EReentrantLock.hh"
#include "EMappedByteBuffer.hh"
#include "EByteChannel.hh"
#include "ENativeThreadSet.hh"
#include "EInterruptibleChannel.hh"

namespace efc {
namespace nio {

/**
 * A channel for reading, writing, mapping, and manipulating a file.
 *
 * <p> A file channel has a current <i>position</i> within its file which can
 * be both {@link #position() </code>queried<code>} and {@link #position(long)
 * </code>modified<code>}.  The file itself contains a variable-length sequence
 * of bytes that can be read and written and whose current {@link #size
 * </code><i>size</i><code>} can be queried.  The size of the file increases
 * when bytes are written beyond its current size; the size of the file
 * decreases when it is {@link #truncate </code><i>truncated</i><code>}.  The
 * file may also have some associated <i>metadata</i> such as access
 * permissions, content type, and last-modification time; this class does not
 * define methods for metadata access.
 *
 * <p> In addition to the familiar read, write, and close operations of byte
 * channels, this class defines the following file-specific operations: </p>
 *
 * <ul>
 *
 *   <li><p> Bytes may be {@link #read(ByteBuffer, long) </code>read<code>} or
 *   {@link #write(ByteBuffer, long) </code>written<code>} at an absolute
 *   position in a file in a way that does not affect the channel's current
 *   position.  </p></li>
 *
 *   <li><p> A region of a file may be {@link #map </code>mapped<code>}
 *   directly into memory; for large files this is often much more efficient
 *   than invoking the usual <tt>read</tt> or <tt>write</tt> methods.
 *   </p></li>
 *
 *   <li><p> Updates made to a file may be {@link #force </code>forced
 *   out<code>} to the underlying storage device, ensuring that data are not
 *   lost in the event of a system crash.  </p></li>
 *
 *   <li><p> Bytes can be transferred from a file {@link #transferTo </code>to
 *   some other channel<code>}, and {@link #transferFrom </code>vice
 *   versa<code>}, in a way that can be optimized by many operating systems
 *   into a very fast transfer directly to or from the filesystem cache.
 *   </p></li>
 *
 *   <li><p> A region of a file may be {@link FileLock </code>locked<code>}
 *   against access by other programs.  </p></li>
 *
 * </ul>
 *
 * <p> File channels are safe for use by multiple concurrent threads.  The
 * {@link Channel#close close} method may be invoked at any time, as specified
 * by the {@link Channel} interface.  Only one operation that involves the
 * channel's position or can change its file's size may be in progress at any
 * given time; attempts to initiate a second such operation while the first is
 * still in progress will block until the first operation completes.  Other
 * operations, in particular those that take an explicit position, may proceed
 * concurrently; whether they in fact do so is dependent upon the underlying
 * implementation and is therefore unspecified.
 *
 * <p> The view of a file provided by an instance of this class is guaranteed
 * to be consistent with other views of the same file provided by other
 * instances in the same program.  The view provided by an instance of this
 * class may or may not, however, be consistent with the views seen by other
 * concurrently-running programs due to caching performed by the underlying
 * operating system and delays induced by network-filesystem protocols.  This
 * is true regardless of the language in which these other programs are
 * written, and whether they are running on the same machine or on some other
 * machine.  The exact nature of any such inconsistencies are system-dependent
 * and are therefore unspecified.
 *
 * <p> This class does not define methods for opening existing files or for
 * creating new ones; such methods may be added in a future release.  In this
 * release a file channel can be obtained from an existing {@link
 * java.io.FileInputStream#getChannel FileInputStream}, {@link
 * java.io.FileOutputStream#getChannel FileOutputStream}, or {@link
 * java.io.RandomAccessFile#getChannel RandomAccessFile} object by invoking
 * that object's <tt>getChannel</tt> method, which returns a file channel that
 * is connected to the same underlying file.
 *
 * <p> The state of a file channel is intimately connected to that of the
 * object whose <tt>getChannel</tt> method returned the channel.  Changing the
 * channel's position, whether explicitly or by reading or writing bytes, will
 * change the file position of the originating object, and vice versa.
 * Changing the file's length via the file channel will change the length seen
 * via the originating object, and vice versa.  Changing the file's content by
 * writing bytes will change the content seen by the originating object, and
 * vice versa.
 *
 * <a name="open-mode"><p> At various points this class specifies that an
 * instance that is "open for reading," "open for writing," or "open for
 * reading and writing" is required.  A channel obtained via the {@link
 * java.io.FileInputStream#getChannel getChannel} method of a {@link
 * java.io.FileInputStream} instance will be open for reading.  A channel
 * obtained via the {@link java.io.FileOutputStream#getChannel getChannel}
 * method of a {@link java.io.FileOutputStream} instance will be open for
 * writing.  Finally, a channel obtained via the {@link
 * java.io.RandomAccessFile#getChannel getChannel} method of a {@link
 * java.io.RandomAccessFile} instance will be open for reading if the instance
 * was created with mode <tt>"r"</tt> and will be open for reading and writing
 * if the instance was created with mode <tt>"rw"</tt>.
 *
 * <a name="append-mode"><p> A file channel that is open for writing may be in
 * <i>append mode</i>, for example if it was obtained from a file-output stream
 * that was created by invoking the {@link
 * java.io.FileOutputStream#FileOutputStream(java.io.File,boolean)
 * FileOutputStream(File,boolean)} constructor and passing <tt>true</tt> for
 * the second parameter.  In this mode each invocation of a relative write
 * operation first advances the position to the end of the file and then writes
 * the requested data.  Whether the advancement of the position and the writing
 * of the data are done in a single atomic operation is system-dependent and
 * therefore unspecified.
 *
 *
 * @see java.io.FileInputStream#getChannel()
 * @see java.io.FileOutputStream#getChannel()
 * @see java.io.RandomAccessFile#getChannel()
 *
 * @since 1.4
 */

class EFileChannel : public EInterruptibleChannel, virtual public EByteChannel {
public:
	virtual ~EFileChannel();

	/**
	 * Opens a file channel.
	 *
	 * @return  A new file channel
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 */
	static EFileChannel* open(int fd, boolean readable, boolean writable, boolean append=false);
	static EFileChannel* open(const char* filename, boolean readable, boolean writable, boolean append=false);

	// -- Channel operations --

	/**
	 * Reads a sequence of bytes from this channel into the given buffer.
	 *
	 * <p> Bytes are read starting at this channel's current file position, and
	 * then the file position is updated with the number of bytes actually
	 * read.  Otherwise this method behaves exactly as specified in the {@link
	 * ReadableByteChannel} interface. </p>
	 */
	virtual int read(EIOByteBuffer* dst) THROWS(EIOException);

	/**
	 * Reads a sequence of bytes from this channel into a subsequence of the
	 * given buffers.
	 *
	 * <p> Bytes are read starting at this channel's current file position, and
	 * then the file position is updated with the number of bytes actually
	 * read.  Otherwise this method behaves exactly as specified in the {@link
	 * ScatteringByteChannel} interface.  </p>
	 */
	virtual long read(EA<EIOByteBuffer*>* dsts, int offset, int length) THROWS(EIOException);

	/**
	 * Reads a sequence of bytes from this channel into the given buffers.
	 *
	 * <p> Bytes are read starting at this channel's current file position, and
	 * then the file position is updated with the number of bytes actually
	 * read.  Otherwise this method behaves exactly as specified in the {@link
	 * ScatteringByteChannel} interface.  </p>
	 */
	virtual long read(EA<EIOByteBuffer*>* dsts) THROWS(EIOException);

	/**
	 * Writes a sequence of bytes to this channel from the given buffer.
	 *
	 * <p> Bytes are written starting at this channel's current file position
	 * unless the channel is in append mode, in which case the position is
	 * first advanced to the end of the file.  The file is grown, if necessary,
	 * to accommodate the written bytes, and then the file position is updated
	 * with the number of bytes actually written.  Otherwise this method
	 * behaves exactly as specified by the {@link WritableByteChannel}
	 * interface. </p>
	 */
	virtual int write(EIOByteBuffer* src) THROWS(EIOException);

	/**
	 * Writes a sequence of bytes to this channel from a subsequence of the
	 * given buffers.
	 *
	 * <p> Bytes are written starting at this channel's current file position
	 * unless the channel is in append mode, in which case the position is
	 * first advanced to the end of the file.  The file is grown, if necessary,
	 * to accommodate the written bytes, and then the file position is updated
	 * with the number of bytes actually written.  Otherwise this method
	 * behaves exactly as specified in the {@link GatheringByteChannel}
	 * interface.  </p>
	 */
	virtual long write(EA<EIOByteBuffer*>* srcs, int offset, int length) THROWS(EIOException);

	/**
	 * Writes a sequence of bytes to this channel from the given buffers.
	 *
	 * <p> Bytes are written starting at this channel's current file position
	 * unless the channel is in append mode, in which case the position is
	 * first advanced to the end of the file.  The file is grown, if necessary,
	 * to accommodate the written bytes, and then the file position is updated
	 * with the number of bytes actually written.  Otherwise this method
	 * behaves exactly as specified in the {@link GatheringByteChannel}
	 * interface.  </p>
	 */
	virtual long write(EA<EIOByteBuffer*>* srcs) THROWS(EIOException);

	// -- Other operations --

	/**
	 * Returns this channel's file position.  </p>
	 *
	 * @return  This channel's file position,
	 *          a non-negative integer counting the number of bytes
	 *          from the beginning of the file to the current position
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual long position() THROWS(EIOException);

	/**
	 * Sets this channel's file position.
	 *
	 * <p> Setting the position to a value that is greater than the file's
	 * current size is legal but does not change the size of the file.  A later
	 * attempt to read bytes at such a position will immediately return an
	 * end-of-file indication.  A later attempt to write bytes at such a
	 * position will cause the file to be grown to accommodate the new bytes;
	 * the values of any bytes between the previous end-of-file and the
	 * newly-written bytes are unspecified.  </p>
	 *
	 * @param  newPosition
	 *         The new position, a non-negative integer counting
	 *         the number of bytes from the beginning of the file
	 *
	 * @return  This file channel
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  IllegalArgumentException
	 *          If the new position is negative
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	EFileChannel* position(long newPosition) THROWS(EIOException);

	/**
	 * Returns the current size of this channel's file. </p>
	 *
	 * @return  The current size of this channel's file,
	 *          measured in bytes
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual long size() THROWS(EIOException);

	/**
	 * Truncates this channel's file to the given size.
	 *
	 * <p> If the given size is less than the file's current size then the file
	 * is truncated, discarding any bytes beyond the new end of the file.  If
	 * the given size is greater than or equal to the file's current size then
	 * the file is not modified.  In either case, if this channel's file
	 * position is greater than the given size then it is set to that size.
	 * </p>
	 *
	 * @param  size
	 *         The new size, a non-negative byte count
	 *
	 * @return  This file channel
	 *
	 * @throws  NonWritableChannelException
	 *          If this channel was not opened for writing
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  IllegalArgumentException
	 *          If the new size is negative
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	EFileChannel* truncate(long size) THROWS(EIOException);

	/**
	 * Forces any updates to this channel's file to be written to the storage
	 * device that contains it.
	 *
	 * <p> If this channel's file resides on a local storage device then when
	 * this method returns it is guaranteed that all changes made to the file
	 * since this channel was created, or since this method was last invoked,
	 * will have been written to that device.  This is useful for ensuring that
	 * critical information is not lost in the event of a system crash.
	 *
	 * <p> If the file does not reside on a local device then no such guarantee
	 * is made.
	 *
	 * <p> The <tt>metaData</tt> parameter can be used to limit the number of
	 * I/O operations that this method is required to perform.  Passing
	 * <tt>false</tt> for this parameter indicates that only updates to the
	 * file's content need be written to storage; passing <tt>true</tt>
	 * indicates that updates to both the file's content and metadata must be
	 * written, which generally requires at least one more I/O operation.
	 * Whether this parameter actually has any effect is dependent upon the
	 * underlying operating system and is therefore unspecified.
	 *
	 * <p> Invoking this method may cause an I/O operation to occur even if the
	 * channel was only opened for reading.  Some operating systems, for
	 * example, maintain a last-access time as part of a file's metadata, and
	 * this time is updated whenever the file is read.  Whether or not this is
	 * actually done is system-dependent and is therefore unspecified.
	 *
	 * <p> This method is only guaranteed to force changes that were made to
	 * this channel's file via the methods defined in this class.  It may or
	 * may not force changes that were made by modifying the content of a
	 * {@link MappedByteBuffer </code>mapped byte buffer<code>} obtained by
	 * invoking the {@link #map map} method.  Invoking the {@link
	 * MappedByteBuffer#force force} method of the mapped byte buffer will
	 * force changes made to the buffer's content to be written.  </p>
	 *
	 * @param   metaData
	 *          If <tt>true</tt> then this method is required to force changes
	 *          to both the file's content and metadata to be written to
	 *          storage; otherwise, it need only force content changes to be
	 *          written
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	void force(boolean metaData) THROWS(EIOException);

	/**
	 * Transfers bytes from this channel's file to the given writable byte
	 * channel.
	 *
	 * <p> An attempt is made to read up to <tt>count</tt> bytes starting at
	 * the given <tt>position</tt> in this channel's file and write them to the
	 * target channel.  An invocation of this method may or may not transfer
	 * all of the requested bytes; whether or not it does so depends upon the
	 * natures and states of the channels.  Fewer than the requested number of
	 * bytes are transferred if this channel's file contains fewer than
	 * <tt>count</tt> bytes starting at the given <tt>position</tt>, or if the
	 * target channel is non-blocking and it has fewer than <tt>count</tt>
	 * bytes free in its output buffer.
	 *
	 * <p> This method does not modify this channel's position.  If the given
	 * position is greater than the file's current size then no bytes are
	 * transferred.  If the target channel has a position then bytes are
	 * written starting at that position and then the position is incremented
	 * by the number of bytes written.
	 *
	 * <p> This method is potentially much more efficient than a simple loop
	 * that reads from this channel and writes to the target channel.  Many
	 * operating systems can transfer bytes directly from the filesystem cache
	 * to the target channel without actually copying them.  </p>
	 *
	 * @param  position
	 *         The position within the file at which the transfer is to begin;
	 *         must be non-negative
	 *
	 * @param  count
	 *         The maximum number of bytes to be transferred; must be
	 *         non-negative
	 *
	 * @param  target
	 *         The target channel
	 *
	 * @return  The number of bytes, possibly zero,
	 *          that were actually transferred
	 *
	 * @throws IllegalArgumentException
	 *         If the preconditions on the parameters do not hold
	 *
	 * @throws  NonReadableChannelException
	 *          If this channel was not opened for reading
	 *
	 * @throws  NonWritableChannelException
	 *          If the target channel was not opened for writing
	 *
	 * @throws  ClosedChannelException
	 *          If either this channel or the target channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes either channel
	 *          while the transfer is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread while the
	 *          transfer is in progress, thereby closing both channels and
	 *          setting the current thread's interrupt status
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	long transferTo(long position, long count, EByteChannel* target) THROWS(EIOException);

	/**
	 * Transfers bytes into this channel's file from the given readable byte
	 * channel.
	 *
	 * <p> An attempt is made to read up to <tt>count</tt> bytes from the
	 * source channel and write them to this channel's file starting at the
	 * given <tt>position</tt>.  An invocation of this method may or may not
	 * transfer all of the requested bytes; whether or not it does so depends
	 * upon the natures and states of the channels.  Fewer than the requested
	 * number of bytes will be transferred if the source channel has fewer than
	 * <tt>count</tt> bytes remaining, or if the source channel is non-blocking
	 * and has fewer than <tt>count</tt> bytes immediately available in its
	 * input buffer.
	 *
	 * <p> This method does not modify this channel's position.  If the given
	 * position is greater than the file's current size then no bytes are
	 * transferred.  If the source channel has a position then bytes are read
	 * starting at that position and then the position is incremented by the
	 * number of bytes read.
	 *
	 * <p> This method is potentially much more efficient than a simple loop
	 * that reads from the source channel and writes to this channel.  Many
	 * operating systems can transfer bytes directly from the source channel
	 * into the filesystem cache without actually copying them.  </p>
	 *
	 * @param  src
	 *         The source channel
	 *
	 * @param  position
	 *         The position within the file at which the transfer is to begin;
	 *         must be non-negative
	 *
	 * @param  count
	 *         The maximum number of bytes to be transferred; must be
	 *         non-negative
	 *
	 * @return  The number of bytes, possibly zero,
	 *          that were actually transferred
	 *
	 * @throws IllegalArgumentException
	 *         If the preconditions on the parameters do not hold
	 *
	 * @throws  NonReadableChannelException
	 *          If the source channel was not opened for reading
	 *
	 * @throws  NonWritableChannelException
	 *          If this channel was not opened for writing
	 *
	 * @throws  ClosedChannelException
	 *          If either this channel or the source channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes either channel
	 *          while the transfer is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread while the
	 *          transfer is in progress, thereby closing both channels and
	 *          setting the current thread's interrupt status
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	long transferFrom(EByteChannel* src, long position, long count) THROWS(EIOException);

	/**
	 * Reads a sequence of bytes from this channel into the given buffer,
	 * starting at the given file position.
	 *
	 * <p> This method works in the same manner as the {@link
	 * #read(ByteBuffer)} method, except that bytes are read starting at the
	 * given file position rather than at the channel's current position.  This
	 * method does not modify this channel's position.  If the given position
	 * is greater than the file's current size then no bytes are read.  </p>
	 *
	 * @param  dst
	 *         The buffer into which bytes are to be transferred
	 *
	 * @param  position
	 *         The file position at which the transfer is to begin;
	 *         must be non-negative
	 *
	 * @return  The number of bytes read, possibly zero, or <tt>-1</tt> if the
	 *          given position is greater than or equal to the file's current
	 *          size
	 *
	 * @throws  IllegalArgumentException
	 *          If the position is negative
	 *
	 * @throws  NonReadableChannelException
	 *          If this channel was not opened for reading
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the read operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the read operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual int read(EIOByteBuffer* dst, long position) THROWS(EIOException);

	/**
	 * Writes a sequence of bytes to this channel from the given buffer,
	 * starting at the given file position.
	 *
	 * <p> This method works in the same manner as the {@link
	 * #write(ByteBuffer)} method, except that bytes are written starting at
	 * the given file position rather than at the channel's current position.
	 * This method does not modify this channel's position.  If the given
	 * position is greater than the file's current size then the file will be
	 * grown to accommodate the new bytes; the values of any bytes between the
	 * previous end-of-file and the newly-written bytes are unspecified.  </p>
	 *
	 * @param  src
	 *         The buffer from which bytes are to be transferred
	 *
	 * @param  position
	 *         The file position at which the transfer is to begin;
	 *         must be non-negative
	 *
	 * @return  The number of bytes written, possibly zero
	 *
	 * @throws  IllegalArgumentException
	 *          If the position is negative
	 *
	 * @throws  NonWritableChannelException
	 *          If this channel was not opened for writing
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the write operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the write operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual int write(EIOByteBuffer* src, long position) THROWS(EIOException);

	/**
	 *
	 */
	virtual int getFDVal();

	// -- Memory-mapped buffers --

	/**
	 * Maps a region of this channel's file directly into memory.
	 *
	 * <p> A region of a file may be mapped into memory in one of three modes:
	 * </p>
	 *
	 * <ul type=disc>
	 *
	 *   <li><p> <i>Read-only:</i> Any attempt to modify the resulting buffer
	 *   will cause a {@link java.nio.ReadOnlyBufferException} to be thrown.
	 *   ({@link MapMode#READ_ONLY MapMode.READ_ONLY}) </p></li>
	 *
	 *   <li><p> <i>Read/write:</i> Changes made to the resulting buffer will
	 *   eventually be propagated to the file; they may or may not be made
	 *   visible to other programs that have mapped the same file.  ({@link
	 *   MapMode#READ_WRITE MapMode.READ_WRITE}) </p></li>
	 *
	 *   <li><p> <i>Private:</i> Changes made to the resulting buffer will not
	 *   be propagated to the file and will not be visible to other programs
	 *   that have mapped the same file; instead, they will cause private
	 *   copies of the modified portions of the buffer to be created.  ({@link
	 *   MapMode#PRIVATE MapMode.PRIVATE}) </p></li>
	 *
	 * </ul>
	 *
	 * <p> For a read-only mapping, this channel must have been opened for
	 * reading; for a read/write or private mapping, this channel must have
	 * been opened for both reading and writing.
	 *
	 * <p> The {@link MappedByteBuffer </code>mapped byte buffer<code>}
	 * returned by this method will have a position of zero and a limit and
	 * capacity of <tt>size</tt>; its mark will be undefined.  The buffer and
	 * the mapping that it represents will remain valid until the buffer itself
	 * is garbage-collected.
	 *
	 * <p> A mapping, once established, is not dependent upon the file channel
	 * that was used to create it.  Closing the channel, in particular, has no
	 * effect upon the validity of the mapping.
	 *
	 * <p> Many of the details of memory-mapped files are inherently dependent
	 * upon the underlying operating system and are therefore unspecified.  The
	 * behavior of this method when the requested region is not completely
	 * contained within this channel's file is unspecified.  Whether changes
	 * made to the content or size of the underlying file, by this program or
	 * another, are propagated to the buffer is unspecified.  The rate at which
	 * changes to the buffer are propagated to the file is unspecified.
	 *
	 * <p> For most operating systems, mapping a file into memory is more
	 * expensive than reading or writing a few tens of kilobytes of data via
	 * the usual {@link #read read} and {@link #write write} methods.  From the
	 * standpoint of performance it is generally only worth mapping relatively
	 * large files into memory.  </p>
	 *
	 * @param  mode
	 *         One of the constants {@link MapMode#READ_ONLY READ_ONLY}, {@link
	 *         MapMode#READ_WRITE READ_WRITE}, or {@link MapMode#PRIVATE
	 *         PRIVATE} defined in the {@link MapMode} class, according to
	 *         whether the file is to be mapped read-only, read/write, or
	 *         privately (copy-on-write), respectively
	 *
	 * @param  position
	 *         The position within the file at which the mapped region
	 *         is to start; must be non-negative
	 *
	 * @param  size
	 *         The size of the region to be mapped; must be non-negative and
	 *         no greater than {@link java.lang.Integer#MAX_VALUE}
	 *
	 * @throws NonReadableChannelException
	 *         If the <tt>mode</tt> is {@link MapMode#READ_ONLY READ_ONLY} but
	 *         this channel was not opened for reading
	 *
	 * @throws NonWritableChannelException
	 *         If the <tt>mode</tt> is {@link MapMode#READ_WRITE READ_WRITE} or
	 *         {@link MapMode#PRIVATE PRIVATE} but this channel was not opened
	 *         for both reading and writing
	 *
	 * @throws IllegalArgumentException
	 *         If the preconditions on the parameters do not hold
	 *
	 * @throws IOException
	 *         If some other I/O error occurs
	 *
	 * @see java.nio.channels.FileChannel.MapMode
	 * @see java.nio.MappedByteBuffer
	 */
	EMappedByteBuffer* map(EMapMode mode, long position, long size) THROWS(EIOException);

	// -- Locks --

	/**
	 * Acquires a lock on the given region of this channel's file.
	 *
	 * <p> An invocation of this method will block until the region can be
	 * locked, this channel is closed, or the invoking thread is interrupted,
	 * whichever comes first.
	 *
	 * <p> If this channel is closed by another thread during an invocation of
	 * this method then an {@link AsynchronousCloseException} will be thrown.
	 *
	 * <p> If the invoking thread is interrupted while waiting to acquire the
	 * lock then its interrupt status will be set and a {@link
	 * FileLockInterruptionException} will be thrown.  If the invoker's
	 * interrupt status is set when this method is invoked then that exception
	 * will be thrown immediately; the thread's interrupt status will not be
	 * changed.
	 *
	 * <p> The region specified by the <tt>position</tt> and <tt>size</tt>
	 * parameters need not be contained within, or even overlap, the actual
	 * underlying file.  Lock regions are fixed in size; if a locked region
	 * initially contains the end of the file and the file grows beyond the
	 * region then the new portion of the file will not be covered by the lock.
	 * If a file is expected to grow in size and a lock on the entire file is
	 * required then a region starting at zero, and no smaller than the
	 * expected maximum size of the file, should be locked.  The zero-argument
	 * {@link #lock()} method simply locks a region of size {@link
	 * Long#MAX_VALUE}.
	 *
	 * <p> Some operating systems do not support shared locks, in which case a
	 * request for a shared lock is automatically converted into a request for
	 * an exclusive lock.  Whether the newly-acquired lock is shared or
	 * exclusive may be tested by invoking the resulting lock object's {@link
	 * FileLock#isShared() isShared} method.
	 *
	 * <p> File locks are held on behalf of the entire Java virtual machine.
	 * They are not suitable for controlling access to a file by multiple
	 * threads within the same virtual machine.  </p>
	 *
	 * @param  position
	 *         The position at which the locked region is to start; must be
	 *         non-negative
	 *
	 * @param  size
	 *         The size of the locked region; must be non-negative, and the sum
	 *         <tt>position</tt>&nbsp;+&nbsp;<tt>size</tt> must be non-negative
	 *
	 * @param  shared
	 *         <tt>true</tt> to request a shared lock, in which case this
	 *         channel must be open for reading (and possibly writing);
	 *         <tt>false</tt> to request an exclusive lock, in which case this
	 *         channel must be open for writing (and possibly reading)
	 *
	 * @return  A lock object representing the newly-acquired lock
	 *
	 * @throws  IllegalArgumentException
	 *          If the preconditions on the parameters do not hold
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel while the invoking
	 *          thread is blocked in this method
	 *
	 * @throws  FileLockInterruptionException
	 *          If the invoking thread is interrupted while blocked in this
	 *          method
	 *
	 * @throws  OverlappingFileLockException
	 *          If a lock that overlaps the requested region is already held by
	 *          this Java virtual machine, or if another thread is already
	 *          blocked in this method and is attempting to lock an overlapping
	 *          region
	 *
	 * @throws  NonReadableChannelException
	 *          If <tt>shared</tt> is <tt>true</tt> this channel was not
	 *          opened for reading
	 *
	 * @throws  NonWritableChannelException
	 *          If <tt>shared</tt> is <tt>false</tt> but this channel was not
	 *          opened for writing
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 *
	 * @see     #lock()
	 * @see     #tryLock()
	 * @see     #tryLock(long,long,boolean)
	 */
	EFileLock* lock(long position, long size, boolean shared) THROWS(EIOException);

	/**
	 * Acquires an exclusive lock on this channel's file.
	 *
	 * <p> An invocation of this method of the form <tt>fc.lock()</tt> behaves
	 * in exactly the same way as the invocation
	 *
	 * <pre>
	 *     fc.{@link #lock(long,long,boolean) lock}(0L, Long.MAX_VALUE, false) </pre>
	 *
	 * @return  A lock object representing the newly-acquired lock
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel while the invoking
	 *          thread is blocked in this method
	 *
	 * @throws  FileLockInterruptionException
	 *          If the invoking thread is interrupted while blocked in this
	 *          method
	 *
	 * @throws  OverlappingFileLockException
	 *          If a lock that overlaps the requested region is already held by
	 *          this Java virtual machine, or if another thread is already
	 *          blocked in this method and is attempting to lock an overlapping
	 *          region of the same file
	 *
	 * @throws  NonWritableChannelException
	 *          If this channel was not opened for writing
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 *
	 * @see     #lock(long,long,boolean)
	 * @see     #tryLock()
	 * @see     #tryLock(long,long,boolean)
	 */
	EFileLock* lock() THROWS(EIOException);

	/**
	 * Attempts to acquire a lock on the given region of this channel's file.
	 *
	 * <p> This method does not block.  An invocation always returns
	 * immediately, either having acquired a lock on the requested region or
	 * having failed to do so.  If it fails to acquire a lock because an
	 * overlapping lock is held by another program then it returns
	 * <tt>null</tt>.  If it fails to acquire a lock for any other reason then
	 * an appropriate exception is thrown.
	 *
	 * <p> The region specified by the <tt>position</tt> and <tt>size</tt>
	 * parameters need not be contained within, or even overlap, the actual
	 * underlying file.  Lock regions are fixed in size; if a locked region
	 * initially contains the end of the file and the file grows beyond the
	 * region then the new portion of the file will not be covered by the lock.
	 * If a file is expected to grow in size and a lock on the entire file is
	 * required then a region starting at zero, and no smaller than the
	 * expected maximum size of the file, should be locked.  The zero-argument
	 * {@link #tryLock()} method simply locks a region of size {@link
	 * Long#MAX_VALUE}.
	 *
	 * <p> Some operating systems do not support shared locks, in which case a
	 * request for a shared lock is automatically converted into a request for
	 * an exclusive lock.  Whether the newly-acquired lock is shared or
	 * exclusive may be tested by invoking the resulting lock object's {@link
	 * FileLock#isShared() isShared} method.
	 *
	 * <p> File locks are held on behalf of the entire Java virtual machine.
	 * They are not suitable for controlling access to a file by multiple
	 * threads within the same virtual machine.  </p>
	 *
	 * @param  position
	 *         The position at which the locked region is to start; must be
	 *         non-negative
	 *
	 * @param  size
	 *         The size of the locked region; must be non-negative, and the sum
	 *         <tt>position</tt>&nbsp;+&nbsp;<tt>size</tt> must be non-negative
	 *
	 * @param  shared
	 *         <tt>true</tt> to request a shared lock,
	 *         <tt>false</tt> to request an exclusive lock
	 *
	 * @return  A lock object representing the newly-acquired lock,
	 *          or <tt>null</tt> if the lock could not be acquired
	 *          because another program holds an overlapping lock
	 *
	 * @throws  IllegalArgumentException
	 *          If the preconditions on the parameters do not hold
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  OverlappingFileLockException
	 *          If a lock that overlaps the requested region is already held by
	 *          this Java virtual machine, or if another thread is already
	 *          blocked in this method and is attempting to lock an overlapping
	 *          region of the same file
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 *
	 * @see     #lock()
	 * @see     #lock(long,long,boolean)
	 * @see     #tryLock()
	 */
	EFileLock* tryLock(long position, long size, boolean shared) THROWS(EIOException);

	/**
	 * Attempts to acquire an exclusive lock on this channel's file.
	 *
	 * <p> An invocation of this method of the form <tt>fc.tryLock()</tt>
	 * behaves in exactly the same way as the invocation
	 *
	 * <pre>
	 *     fc.{@link #tryLock(long,long,boolean) tryLock}(0L, Long.MAX_VALUE, false) </pre>
	 *
	 * @return  A lock object representing the newly-acquired lock,
	 *          or <tt>null</tt> if the lock could not be acquired
	 *          because another program holds an overlapping lock
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  OverlappingFileLockException
	 *          If a lock that overlaps the requested region is already held by
	 *          this Java virtual machine, or if another thread is already
	 *          blocked in this method and is attempting to lock an overlapping
	 *          region
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 *
	 * @see     #lock()
	 * @see     #lock(long,long,boolean)
	 * @see     #tryLock(long,long,boolean)
	 */
	EFileLock* tryLock() THROWS(EIOException);

protected:
	/**
	 * Closes this channel.
	 *
	 * <p> This method is invoked by the {@link #close close} method in order
	 * to perform the actual work of closing the channel.  This method is only
	 * invoked if the channel has not yet been closed, and it is never invoked
	 * more than once.
	 *
	 * <p> An implementation of this method must arrange for any other thread
	 * that is blocked in an I/O operation upon this channel to return
	 * immediately, either by throwing an exception or by returning normally.
	 * </p>
	 *
	 * @throws  IOException
	 *          If an I/O error occurs while closing the channel
	 */
	virtual void implCloseChannel() THROWS(EIOException);

private:
	friend class EFileLock;

	// File descriptor
	int fd_;
	es_file_t* pfile_;//= null;

	// File access mode (immutable)
	boolean readable_;
	boolean writable_;
	boolean appending_;

	// Thread-safe set of IDs of native threads, for signalling
	ENativeThreadSet* threads_;// = new NativeThreadSet(2);

	// Lock for operations involving position and size
	EReentrantLock positionLock_;

	// Assume at first that the underlying kernel supports sendfile();
	// set this to false if we find out later that it doesn't
	//
	static volatile boolean transferSupported_;// = true;

//	// Assume that the underlying kernel sendfile() will work if the target
//	// fd is a pipe; set this to false if we find out later that it doesn't
//	//
//	static volatile boolean pipeSupported_;// = true;

	// Assume that the underlying kernel sendfile() will work if the target
	// fd is a file; set this to false if we find out later that it doesn't
	//
	static volatile boolean fileSupported_;// = true;

	/**
	 * Initializes a new instance of this class.
	 */
	EFileChannel(int fd, boolean readable, boolean writable, boolean append);
	EFileChannel(const char* filename, boolean readable, boolean writable, boolean append);

	void release(EFileLock* fli) THROWS(EIOException);
	void ensureOpen() THROWS(EIOException);

	long transferToDirectly(long position, int icount, EByteChannel* target) THROWS(EIOException);
	long transferToTrustedChannel(long position, int icount, EByteChannel* target) THROWS(EIOException);
	long transferToArbitraryChannel(long position, int icount, EByteChannel* target) THROWS(EIOException);
	long transferFromFileChannel(EFileChannel* src, long position, long count) THROWS(EIOException);
	long transferFromArbitraryChannel(EByteChannel* src, long position, long count) THROWS(EIOException);

	// Sets or reports this file's position
	// If offset is -1, the current position is returned
	// otherwise the position is set to offset
	long position0(int fd, long offset);

	// Reports this file's size
	long size0(int fd);

	// Truncates a file
	int truncate0(int fd, long size);

	// Forces output to device
	int force0(int fd, boolean metaData);

	// Grabs a file lock
	int lock0(int fd, boolean blocking, long pos, long size,
	          boolean shared) THROWS(EIOException);

	// Releases a file lock
	void release0(int fd, long pos, long size) THROWS(EIOException);

	// Transfers from src to dst, or returns -2 if kernel can't do that
	long transferTo0(int src, long position, long count, int dst);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EFILECHANNEL_HH_ */
