/*
 * ECheckedInputStream.hh
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#ifndef ECHECKEDINPUTSTREAM_HH_
#define ECHECKEDINPUTSTREAM_HH_

#include "EFilterInputStream.hh"
#include "EChecksum.hh"

namespace efc {

class ECheckedInputStream: public EFilterInputStream {
public:
	virtual ~ECheckedInputStream();

	/**
	 * Creates an input stream using the specified Checksum.
	 * @param in the input stream
	 * @param cksum the Checksum
	 */
	ECheckedInputStream(EInputStream *in, EChecksum *cksum);

	//TODO...
	ECheckedInputStream(const ECheckedInputStream& that);
	ECheckedInputStream& operator= (const ECheckedInputStream& that);

	/**
	 * Reads a byte. Will block if no input is available.
	 * @return the byte read, or -1 if the end of the stream is reached.
	 * @exception IOException if an I/O error has occurred
	 */
	int read() THROWS(EIOException);

	/**
	 * Reads into an array of bytes. If <code>len</code> is not zero, the method
	 * blocks until some input is available; otherwise, no
	 * bytes are read and <code>0</code> is returned.
	 * @param buf the buffer into which the data is read
	 * @param len the maximum number of bytes read
	 * @return    the actual number of bytes read, or -1 if the end
	 *		  of the stream is reached.
	 * @exception  NullPointerException If <code>buf</code> is <code>null</code>.
	 * @exception  IndexOutOfBoundsException If <code>off</code> is negative,
	 * <code>len</code> is negative, or <code>len</code> is greater than
	 * <code>buf.length - off</code>
	 * @exception IOException if an I/O error has occurred
	 */
	int read(void *b, int len) THROWS(EIOException);

	/**
	 * Skips specified number of bytes of input.
	 * @param n the number of bytes to skip
	 * @return the actual number of bytes skipped
	 * @exception IOException if an I/O error has occurred
	 */
	long skip(long n) THROWS(EIOException);

	/**
	 * Returns the Checksum for this input stream.
	 * @return the Checksum value
	 */
	EChecksum* getChecksum();

private:
	EChecksum* _cksum;
};

} /* namespace efc */
#endif /* ECHECKEDINPUTSTREAM_HH_ */
