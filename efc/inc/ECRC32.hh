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

class ECRC32: public EObject, virtual public EChecksum {
public:
	~ECRC32();

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
	long getValue();

	/**
	 * Resets CRC-32 to initial value.
	 */
	void reset();

private:
	es_uint32_t _crc;
};

} /* namespace efc */
#endif /* ECRC32_HH_ */
