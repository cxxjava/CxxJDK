/*
 * EByteChannel.hh
 *
 *  Created on: 2014-2-8
 *      Author: cxxjava@163.com
 */

#ifndef EBYTECHANNEL_HH_
#define EBYTECHANNEL_HH_

#include "EChannel.hh"
#include "EIOByteBuffer.hh"
#include "EIOException.hh"

namespace efc {
namespace nio {

/**
 * A channel that can read and write bytes.  This interface simply unifies
 * {@link ReadableByteChannel} and {@link WritableByteChannel}; it does not
 * specify any new operations.
 *
 * @author Mark Reinhold
 * @author JSR-51 Expert Group
 * @since 1.4
 */

//ByteChannel extends ReadableByteChannel, WritableByteChannel
//ReadableByteChannel extends Channel
//WritableByteChannel extends Channel
//Channel extends Closeable

#define _EByteChannel_	_S(EByteChannel) "," _EChannel_

interface EByteChannel : virtual public EChannel {
public:
	virtual ~EByteChannel(){}

	/**
	 * Reads a sequence of bytes from this channel into the given buffer.
	 *
	 * <p> An attempt is made to read up to <i>r</i> bytes from the channel,
	 * where <i>r</i> is the number of bytes remaining in the buffer, that is,
	 * <tt>dst.remaining()</tt>, at the moment this method is invoked.
	 *
	 * <p> Suppose that a byte sequence of length <i>n</i> is read, where
	 * <tt>0</tt>&nbsp;<tt>&lt;=</tt>&nbsp;<i>n</i>&nbsp;<tt>&lt;=</tt>&nbsp;<i>r</i>.
	 * This byte sequence will be transferred into the buffer so that the first
	 * byte in the sequence is at index <i>p</i> and the last byte is at index
	 * <i>p</i>&nbsp;<tt>+</tt>&nbsp;<i>n</i>&nbsp;<tt>-</tt>&nbsp;<tt>1</tt>,
	 * where <i>p</i> is the buffer's position at the moment this method is
	 * invoked.  Upon return the buffer's position will be equal to
	 * <i>p</i>&nbsp;<tt>+</tt>&nbsp;<i>n</i>; its limit will not have changed.
	 *
	 * <p> A read operation might not fill the buffer, and in fact it might not
	 * read any bytes at all.  Whether or not it does so depends upon the
	 * nature and state of the channel.  A socket channel in non-blocking mode,
	 * for example, cannot read any more bytes than are immediately available
	 * from the socket's input buffer; similarly, a file channel cannot read
	 * any more bytes than remain in the file.  It is guaranteed, however, that
	 * if a channel is in blocking mode and there is at least one byte
	 * remaining in the buffer then this method will block until at least one
	 * byte is read.
	 *
	 * <p> This method may be invoked at any time.  If another thread has
	 * already initiated a read operation upon this channel, however, then an
	 * invocation of this method will block until the first operation is
	 * complete. </p>
	 *
	 * @param  dst
	 *         The buffer into which bytes are to be transferred
	 *
	 * @return  The number of bytes read, possibly zero, or <tt>-1</tt> if the
	 *          channel has reached end-of-stream
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
	virtual int read(EIOByteBuffer* dst) THROWS(EIOException) = 0;

	/**
	 * Writes a sequence of bytes to this channel from the given buffer.
	 *
	 * <p> An attempt is made to write up to <i>r</i> bytes to the channel,
	 * where <i>r</i> is the number of bytes remaining in the buffer, that is,
	 * <tt>src.remaining()</tt>, at the moment this method is invoked.
	 *
	 * <p> Suppose that a byte sequence of length <i>n</i> is written, where
	 * <tt>0</tt>&nbsp;<tt>&lt;=</tt>&nbsp;<i>n</i>&nbsp;<tt>&lt;=</tt>&nbsp;<i>r</i>.
	 * This byte sequence will be transferred from the buffer starting at index
	 * <i>p</i>, where <i>p</i> is the buffer's position at the moment this
	 * method is invoked; the index of the last byte written will be
	 * <i>p</i>&nbsp;<tt>+</tt>&nbsp;<i>n</i>&nbsp;<tt>-</tt>&nbsp;<tt>1</tt>.
	 * Upon return the buffer's position will be equal to
	 * <i>p</i>&nbsp;<tt>+</tt>&nbsp;<i>n</i>; its limit will not have changed.
	 *
	 * <p> Unless otherwise specified, a write operation will return only after
	 * writing all of the <i>r</i> requested bytes.  Some types of channels,
	 * depending upon their state, may write only some of the bytes or possibly
	 * none at all.  A socket channel in non-blocking mode, for example, cannot
	 * write any more bytes than are free in the socket's output buffer.
	 *
	 * <p> This method may be invoked at any time.  If another thread has
	 * already initiated a write operation upon this channel, however, then an
	 * invocation of this method will block until the first operation is
	 * complete. </p>
	 *
	 * @param  src
	 *         The buffer from which bytes are to be retrieved
	 *
	 * @return The number of bytes written, possibly zero
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
	virtual int write(EIOByteBuffer* src) THROWS(EIOException) = 0;
};

} /* namespace nio */
} /* namespace efc */
#endif /* EBYTECHANNEL_HH_ */
