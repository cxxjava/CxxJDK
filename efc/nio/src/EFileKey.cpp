/*
 * EFileKey.cpp
 *
 *  Created on: 2014-2-23
 *      Author: cxxjava@163.com
 */

#include "../inc/EFileKey.hh"
#include "../../inc/EIOException.hh"
#include "../../inc/ERuntimeException.hh"

namespace efc {
namespace nio {

#ifdef WIN32

/*
 * Represents a key to a specific file on Windows
 */

#include <Windows.h>

EFileKey* EFileKey::create(int fd) {
	EFileKey* fk = new EFileKey();
	try {
		HANDLE fileHandle = (HANDLE)fd;
		BOOL result;
		BY_HANDLE_FILE_INFORMATION fileInfo;

		result = GetFileInformationByHandle(fileHandle, &fileInfo);
		if (result) {
			fk->dwVolumeSerialNumber = fileInfo.dwVolumeSerialNumber;
			fk->nFileIndexHigh = fileInfo.nFileIndexHigh;
			fk->nFileIndexLow = fileInfo.nFileIndexLow;
		} else {
			throw EIOException(__FILE__, __LINE__, "GetFileInformationByHandle failed");
		}
	} catch (EIOException& ioe) {
		throw ERuntimeException(__FILE__, __LINE__);
	}
	return fk;
}

int EFileKey::hashCode() {
	return (int)(dwVolumeSerialNumber ^ (((ullong)dwVolumeSerialNumber) >> 32)) +
		   (int)(nFileIndexHigh ^ (((ullong)nFileIndexHigh) >> 32)) +
		   (int)(nFileIndexLow ^ (((ullong)nFileIndexHigh) >> 32));
}

boolean EFileKey::equals(EFileKey* other) {
	if (other == this)
		return true;
	if ((this->dwVolumeSerialNumber != other->dwVolumeSerialNumber) ||
		(this->nFileIndexHigh != other->nFileIndexHigh) ||
		(this->nFileIndexLow != other->nFileIndexLow)) {
		return false;
	}
	return true;
}

#else

/*
 * Represents a key to a specific file on Solaris or Linux
 */

#include <sys/stat.h>

#if !defined(__APPLE__)
	#if SIZEOF_VOID_P == 8
		#define stat stat64
		#define fstat fstat64
	#endif //!SIZEOF_VOID_P
#endif

EFileKey* EFileKey::create(int fd) {
	EFileKey* fk = new EFileKey();
	try {
		struct stat fbuf;
		int res;

		RESTARTABLE(fstat(fd, &fbuf), res);
		if (res < 0) {
			throw EIOException(__FILE__, __LINE__, "fstat64 failed");
		} else {
			fk->st_dev = (long)fbuf.st_dev;
			fk->st_ino = (long)fbuf.st_ino;
		}
	} catch (EIOException& ioe) {
		throw ERuntimeException(__FILE__, __LINE__);
	}
	return fk;
}

int EFileKey::hashCode() {
	return (int)(st_dev ^ (((unsigned long)st_dev) >> 32)) +
		   (int)(st_ino ^ (((unsigned long)st_ino) >> 32));
}

boolean EFileKey::equals(EFileKey* other) {
	if (!other)
		return false;
	if ((this->st_dev != other->st_dev) ||
		(this->st_ino != other->st_ino)) {
		return false;
	}
	return true;
}

#endif

} /* namespace nio */
} /* namespace efc */
