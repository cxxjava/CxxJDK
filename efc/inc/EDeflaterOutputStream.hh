/*
 * EDeflaterOutputStream.hh
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#ifndef EDEFLATEROUTPUTSTREAM_HH_
#define EDEFLATEROUTPUTSTREAM_HH_

#include "EFilterOutputStream.hh"
#include "EDeflater.hh"
#include "EA.hh"

namespace efc {

/**
 * This class implements an output stream filter for compressing data in
 * the "deflate" compression format. It is also used as the basis for other
 * types of compression filters, such as GZIPOutputStream.
 *
 * @see         Deflater
 */

class EDeflaterOutputStream: public EFilterOutputStream {
public:
	virtual ~EDeflaterOutputStream();

	/**
	 * Creates a new output stream with a default encompressor and buffer size.
	 * @param out the output stream
	 * @param size the input buffer size
	 * @param syncFlush
     *        if {@code true} invocation of the inherited
     *        {@link DeflaterOutputStream#flush() flush()} method of
     *        this instance flushes the compressor with flush mode
     *        {@link Deflater#SYNC_FLUSH} before flushing the output
     *        stream, otherwise only flushes the output stream
	 * @param useDefault if false then need to call .setDeflater(def)
	 * @exception IOException If an I/O error has occurred.
     * @exception IllegalArgumentException if size is <= 0
     *
     * @since 1.7
	 */
	EDeflaterOutputStream(EOutputStream* out, int size=512,
			boolean syncFlush=false, boolean useDefault=true, boolean owned=false);

	//TODO:
	EDeflaterOutputStream(const EDeflaterOutputStream& that);
	EDeflaterOutputStream& operator= (const EDeflaterOutputStream& that);

	/**
	 * Set the specified encompressor
	 * @param def the encompressor ("deflater")
	 * @exception EIOException if _def is not null
	 */
	virtual void setDeflater(EDeflater *def) THROWS(EIOException);

	/**
	 * Writes an array of bytes to the compressed output stream. This
	 * method will block until all the bytes are written.
	 * @param b the data to be written
	 * @param len the length of the data
	 * @exception IOException if an I/O error has occurred
	 */
	virtual void write(const void *b, int len) THROWS(EIOException);
	virtual void write(const char *s) THROWS(EIOException);
	virtual void write(int b) THROWS(EIOException);

	/**
	 * Finishes writing compressed data to the output stream without closing
	 * the underlying stream. Use this method when applying multiple filters
	 * in succession to the same output stream.
	 * @exception IOException if an I/O error has occurred
	 */
	virtual void finish() THROWS(EIOException);

	/**
	 * Writes remaining compressed data to the output stream and closes the
	 * underlying stream.
	 * @exception IOException if an I/O error has occurred
	 */
	virtual void close() THROWS(EIOException);

	/**
	 * Flushes the compressed output stream.
	 *
	 * If {@link #DeflaterOutputStream(OutputStream, Deflater, int, boolean)
	 * syncFlush} is {@code true} when this compressed output stream is
	 * constructed, this method first flushes the underlying {@code compressor}
	 * with the flush mode {@link Deflater#SYNC_FLUSH} to force
	 * all pending data to be flushed out to the output stream and then
	 * flushes the output stream. Otherwise this method only flushes the
	 * output stream without flushing the {@code compressor}.
	 *
	 * @throws IOException if an I/O error has occurred
	 *
	 * @since 1.7
	 */
	virtual void flush() THROWS(EIOException);

protected:
	/**
	 * Input buffer for decompression.
	 */
	EA<byte> *_buf;

	boolean _usesDefaultDeflater; // = false;

	/**
	 * Writes next block of compressed data to the output stream.
	 * @throws IOException if an I/O error has occurred
	 */
	virtual void deflate() THROWS(EIOException);

private:
	/**
	 * Decompressor for this stream.
	 */
	EDeflater *_def;

	boolean _needFreeDeflater; //=false;

	boolean _syncFlush;
};

} /* namespace efc */
#endif /* EDEFLATEROUTPUTSTREAM_HH_ */
