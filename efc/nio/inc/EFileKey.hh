/*
 * EFileKey.hh
 *
 *  Created on: 2014-2-23
 *      Author: cxxjava@163.com
 */

#ifndef EFILEKEY_HH_
#define EFILEKEY_HH_

#include "../../EBase.hh"

namespace efc {
namespace nio {

class EFileKey {
public:
	static EFileKey* create(int fd);

	virtual int hashCode();
	boolean equals(EFileKey* obj);

private:
#ifdef WIN32
	long dwVolumeSerialNumber;
	long nFileIndexHigh;
	long nFileIndexLow;
#else
	long st_dev;    // ID of device
	long st_ino;    // Inode number
#endif
	EFileKey() { }
};

} /* namespace nio */
} /* namespace efc */
#endif /* EFILEKEY_HH_ */
