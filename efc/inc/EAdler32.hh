/*
 * EADLER32.hh
 *
 *  Created on: 2017-12-24
 *      Author: cxxjava@163.com
 */

#ifndef EADLER32_HH_
#define EADLER32_HH_

#include "EObject.hh"
#include "EChecksum.hh"

namespace efc {

/**
 * A class that can be used to compute the Adler-32 checksum of a data
 * stream. An Adler-32 checksum is almost as reliable as a CRC-32 but
 * can be computed much faster.
 *
 * <p> Passing a {@code null} argument to a method in this class will cause
 * a {@link NullPointerException} to be thrown.
 *
 * @see         Checksum
 */

class EAdler32: virtual public EChecksum {
public:
	virtual ~EAdler32();

	/**
	 * Creates a new Adler32 object.
	 */
	EAdler32();

	/**
	 * Updates the checksum with the specified byte (the low eight
	 * bits of the argument b).
	 *
	 * @param b the byte to update the checksum with
	 */
	void update(byte b);

	/**
	 * Updates the checksum with the specified array of bytes.
	 *
	 * @param b the byte array to update the checksum with
	 */
	void update(byte* b, int len);

	/**
	 * Resets the checksum to initial value.
	 */
	llong getValue();

	/**
	 * Resets CRC-32 to initial value.
	 */
	void reset();

private:
	ulong adler;// = 1;
};

} /* namespace efc */
#endif /* EADLER32_HH_ */
