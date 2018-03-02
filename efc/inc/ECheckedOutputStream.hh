/*
 * ECheckedOutputStream.hh
 *
 *  Created on: 2017-12-8
 *      Author: cxxjava@163.com
 */

#ifndef ECHECKEDOUTPUTSTREAM_HH_
#define ECHECKEDOUTPUTSTREAM_HH_

#include "EFilterOutputStream.hh"
#include "EChecksum.hh"

namespace efc {

/**
 * An output stream that also maintains a checksum of the data being
 * written. The checksum can then be used to verify the integrity of
 * the output data.
 *
 * @see         Checksum
 */

class ECheckedOutputStream: public EFilterOutputStream {
public:
	virtual ~ECheckedOutputStream();

	/**
	 * Creates an output stream with the specified Checksum.
	 * @param out the output stream
	 * @param cksum the checksum
	 */
	ECheckedOutputStream(EOutputStream* out, EChecksum* cksum, boolean owned=false);

	//TODO...
	ECheckedOutputStream(const ECheckedOutputStream& that);
	ECheckedOutputStream& operator= (const ECheckedOutputStream& that);

	/**
	 * Writes a byte. Will block until the byte is actually written.
	 * @param b the byte to be written
	 * @exception IOException if an I/O error has occurred
	 */
	virtual void write(int b) THROWS(EIOException);

	/**
	 * Writes an array of bytes. Will block until the bytes are
	 * actually written.
	 * @param b the data to be written
	 * @param off the start offset of the data
	 * @param len the number of bytes to be written
	 * @exception IOException if an I/O error has occurred
	 */
	virtual void write(const void *b, int len) THROWS(EIOException);

	/**
	 * Returns the Checksum for this output stream.
	 * @return the Checksum
	 */
	EChecksum* getChecksum();

private:
	EChecksum* cksum;
};

} /* namespace efc */
#endif /* ECHECKEDOUTPUTSTREAM_HH_ */
