/*
 * EGZIPOutputStream.hh
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#ifndef EGZIPOUTPUTSTREAM_HH_
#define EGZIPOUTPUTSTREAM_HH_

#include "EDeflaterOutputStream.hh"
#include "ECRC32.hh"

namespace efc {

/**
 * This class implements a stream filter for writing compressed data in
 * the GZIP file format.
 *
 */

class EGZIPOutputStream: public EDeflaterOutputStream {
public:
	virtual ~EGZIPOutputStream();

	/**
	 * Creates a new output stream with the specified buffer size.
	 * @param out the output stream
	 * @param size the output buffer size
	 * @param syncFlush
     *        if {@code true} invocation of the inherited
     *        {@link DeflaterOutputStream#flush() flush()} method of
     *        this instance flushes the compressor with flush mode
     *        {@link Deflater#SYNC_FLUSH} before flushing the output
     *        stream, otherwise only flushes the output stream
     * @exception IOException If an I/O error has occurred.
     * @exception IllegalArgumentException if size is <= 0
     *
     * @since 1.7
	 */
	EGZIPOutputStream(EOutputStream* out, int size=512, boolean syncFlush=false) THROWS(EIOException);

	//TODO:
	EGZIPOutputStream(const EGZIPOutputStream& that);
	EGZIPOutputStream& operator= (const EGZIPOutputStream& that);

	/**
	 * Writes array of bytes to the compressed output stream. This method
	 * will block until all the bytes are written.
	 * @param buf the data to be written
	 * @param len the length of the data
	 * @exception IOException If an I/O error has occurred.
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

protected:
    /**
     * CRC-32 of uncompressed data.
     */
    ECRC32 _crc;

    /*
     * Writes GZIP member header.
     */
    void writeHeader() THROWS(EIOException);

	/*
	 * Writes GZIP member trailer to a byte array, starting at a given
	 * offset.
	 */
	void writeTrailer(byte buf[8]) THROWS(EIOException);

private:
	EDeflater* _def;
};

} /* namespace efc */
#endif /* EGZIPOUTPUTSTREAM_HH_ */
