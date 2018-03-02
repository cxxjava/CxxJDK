/*
 * EChecksum.hh
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#ifndef ECHECKSUM_HH_
#define ECHECKSUM_HH_

#include "EBase.hh"

namespace efc {

/**
 * An interface representing a data checksum.
 *
 * @version 	1.17, 11/17/05
 */
interface EChecksum : virtual public EObject {
	virtual ~EChecksum() {
	}

	/**
	 * Updates the current checksum with the specified byte.
	 *
	 * @param b the byte to update the checksum with
	 */
	virtual void update(byte b) = 0;

	/**
	 * Updates the current checksum with the specified array of bytes.
	 * @param b the byte array to update the checksum with
	 * @param off the start offset of the data
	 * @param len the number of bytes to use for the update
	 */
	virtual void update(byte* b, int len) = 0;

	/**
	 * Returns the current checksum value.
	 * @return the current checksum value
	 */
	virtual llong getValue() = 0;

	/**
	 * Resets the checksum to its initial value.
	 */
	virtual void reset() = 0;
};

} /* namespace efc */
#endif /* ECHECKSUM_HH_ */
