/*
 * EGZIPInputStream.hh
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#ifndef EGZIPINPUTSTREAM_HH_
#define EGZIPINPUTSTREAM_HH_

#include "EInflaterInputStream.hh"
#include "ECRC32.hh"
#include "EEOFException.hh"

namespace efc {

class EGZIPInputStream: public EInflaterInputStream {
public:
	virtual ~EGZIPInputStream();

	/**
	 * Creates a new input stream with the specified buffer size.
	 * @param in the input stream
	 * @param size the input buffer size
	 * @exception IOException if an I/O error has occurred
	 * @exception IllegalArgumentException if size is <= 0
	 */
	EGZIPInputStream(EInputStream* in, int size = 512) THROWS(EIOException);

	/**
	 * Reads uncompressed data into an array of bytes. If <code>len</code> is not
	 * zero, the method will block until some input can be decompressed; otherwise,
	 * no bytes are read and <code>0</code> is returned.
	 * @param buf the buffer into which the data is read
	 * @param off the start offset in the destination array <code>b</code>
	 * @param len the maximum number of bytes read
	 * @return	the actual number of bytes read, or -1 if the end of the
	 *		compressed input stream is reached
	 * @exception  NullPointerException If <code>buf</code> is <code>null</code>.
	 * @exception  IndexOutOfBoundsException If <code>off</code> is negative,
	 * <code>len</code> is negative, or <code>len</code> is greater than
	 * <code>buf.length - off</code>
	 * @exception IOException if an I/O error has occurred or the compressed
	 *			      input data is corrupt
	 */
	virtual int read(void *b, int len) THROWS(EIOException);

	/**
	 * Closes this input stream and releases any system resources associated
	 * with the stream.
	 * @exception IOException if an I/O error has occurred
	 */
	virtual void close() THROWS(EIOException);

protected:
	/**
	 * CRC-32 for uncompressed data.
	 */
	ECRC32 _crc;

	/**
	 * Indicates end of input stream.
	 */
	boolean _eos;

private:
	boolean _closed; // = false;
	EInflater* _inf;

	/**
	 * Check to make sure that this stream has not been closed
	 */
	void ensureOpen() THROWS(EIOException);

	/*
	 * Reads GZIP member header.
	 */
	void readHeader() THROWS(EIOException);

	/*
	 * Reads GZIP member trailer.
	 */
	void readTrailer() THROWS(EIOException);

	/*
	 * Reads unsigned integer in Intel byte order.
	 */
	uint readUInt(EInputStream& in) THROWS(EIOException);

	/*
	 * Reads unsigned short in Intel byte order.
	 */
	ushort readUShort(EInputStream& in) THROWS(EIOException);

	/*
	 * Reads unsigned byte.
	 */
	ubyte readUByte(EInputStream& in) THROWS(EIOException);

	/*
	 * Skips bytes of input data blocking until all bytes are skipped.
	 * Does not assume that the input stream is capable of seeking.
	 */
	void skipBytes(EInputStream& in, int n) THROWS(EIOException);
};

} /* namespace efc */
#endif /* EGZIPINPUTSTREAM_HH_ */
