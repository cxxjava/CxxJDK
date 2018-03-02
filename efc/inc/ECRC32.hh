/*
 * ECRC32.hh
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#ifndef ECRC32_HH_
#define ECRC32_HH_

#include "EObject.hh"
#include "EChecksum.hh"

namespace efc {

/**
 * A class that can be used to compute the CRC-32 of a data stream.
 *
 * <p> Passing a {@code null} argument to a method in this class will cause
 * a {@link NullPointerException} to be thrown.
 *
 * @see         Checksum
 */

class ECRC32: virtual public EChecksum {
public:
	virtual ~ECRC32();

	/**
	 * Creates a new CRC32 object.
	 */
	ECRC32();

	/**
	 * Updates CRC-32 with specified byte.
	 */
	void update(byte b);

	/**
	 * Updates CRC-32 with specified array of bytes.
	 */
	void update(byte* b, int len);

	/**
	 * Returns CRC-32 value.
	 */
	llong getValue();

	/**
	 * Resets CRC-32 to initial value.
	 */
	void reset();

private:
	es_uint32_t _crc;
};

} /* namespace efc */
#endif /* ECRC32_HH_ */
