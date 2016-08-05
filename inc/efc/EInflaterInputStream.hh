/*
 * EInflaterInputStream.hh
 *
 *  Created on: 2013-7-19
 *      Author: cxxjava@163.com
 */

#ifndef EINFLATERINPUTSTREAM_HH_
#define EINFLATERINPUTSTREAM_HH_

#include "EInflater.hh"
#include "EFilterInputStream.hh"

namespace efc {

/**
 * This class implements a stream filter for uncompressing data in the
 * "deflate" compression format. It is also used as the basis for other
 * decompression filters, such as GZIPInputStream.
 *
 * @see		Inflater
 * @version 	1.40, 04/07/06
 */

class EInflaterInputStream : public EFilterInputStream {
public:
	virtual ~EInflaterInputStream();

	/**
	 * Creates a new input stream with a default decompressor and buffer size.
	 * @param in the input stream
	 * @param size the input buffer size
	 * @param useDefault if false then need to call .setInflater(inf)
	 */
	EInflaterInputStream(EInputStream* in, int size=512, boolean useDefault=true);

	//TODO:
	EInflaterInputStream(const EInflaterInputStream& that);
	EInflaterInputStream& operator= (const EInflaterInputStream& that);

	/**
	 * Set the specified decompressor
	 * @param inf the decompressor ("inflater")
	 * @exception EIOException if _inf is not null
	 */
	void setInflater(EInflater *inf) THROWS(EIOException);

	/**
	 * Reads uncompressed data into an array of bytes. If <code>len</code> is not
	 * zero, the method will block until some input can be decompressed; otherwise,
	 * no bytes are read and <code>0</code> is returned.
	 * @param b the buffer into which the data is read
	 * @param len the maximum number of bytes read
	 * @return the actual number of bytes read, or -1 if the end of the
	 *         compressed input is reached or a preset dictionary is needed
	 * @exception  NullPointerException If <code>b</code> is <code>null</code>.
	 * @exception  IndexOutOfBoundsException If <code>off</code> is negative,
	 * <code>len</code> is negative, or <code>len</code> is greater than
	 * <code>b.length - off</code>
	 * @exception ZipException if a ZIP format error has occurred
	 * @exception IOException if an I/O error has occurred
	 */
	virtual int read(void *b, int len) THROWS(EIOException);

	/**
	 * Skips specified number of bytes of uncompressed data.
	 * @param n the number of bytes to skip
	 * @return the actual number of bytes skipped.
	 * @exception IOException if an I/O error has occurred
	 * @exception IllegalArgumentException if n < 0
	 */
	virtual long skip(long n) THROWS(EIOException);

	/**
	 * Returns 0 after EOF has been reached, otherwise always return 1.
	 * <p>
	 * Programs should not count on this method to return the actual number
	 * of bytes that could be read without blocking.
	 *
	 * @return     1 before EOF and 0 after EOF.
	 * @exception  IOException  if an I/O error occurs.
	 *
	 */
	virtual long available() THROWS(EIOException);

	/**
	 * Closes this input stream and releases any system resources associated
	 * with the stream.
	 * @exception IOException if an I/O error has occurred
	 */
	virtual void close() THROWS(EIOException);

protected:
	/**
	 * Input buffer for decompression.
	 */
	es_buffer_t *_buf;
	/**
	 * Length of input buffer.
	 */
	int _len;

    boolean _usesDefaultInflater;// = false;
    boolean _needFreeInflater;//=false;

	/**
	 * Fills input buffer with more data to decompress.
	 * @exception IOException if an I/O error has occurred
	 */
	void fill() THROWS(EIOException);

private:
	/**
	 * Decompressor for this stream.
	 */
	EInflater *_inf;

	boolean _closed;// = false;
    // this flag is set to true after EOF has reached
    boolean _reachEOF;// = false;

    /**
	 * Check to make sure that this stream has not been closed
	 */
	void ensureOpen() THROWS(EIOException);
};

} /* namespace efc */
#endif /* EINFLATERINPUTSTREAM_HH_ */
