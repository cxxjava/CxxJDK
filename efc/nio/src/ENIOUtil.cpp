/*
 * ENIOUtil.cpp
 *
 *  Created on: 2014-1-7
 *      Author: cxxjava@163.com
 */

#include "./ENIOUtil.hh"
#include "../../inc/EIOStatus.hh"
#include "./ESocketDispatcher.hh"

#ifdef WIN32
#include <io.h> //read
#else
#include <sys/resource.h>
#endif

namespace efc {
namespace nio {

long ENIOUtil::convertReturnVal(long n, boolean reading) {
	if (n > 0) /* Number of bytes written */
		return n;
	if (n == 0) {
		if (reading) {
			return EIOStatus::EOF_; /* EOF is -1 in javaland */
		} else {
			return 0;
		}
	}
#ifndef WIN32
	else if (errno == EAGAIN)
		return EIOStatus::UNAVAILABLE;
	else if (errno == EINTR)
		return EIOStatus::INTERRUPTED;
#endif
	throw EIOException(__FILE__, __LINE__, "Read/write failed");
	return EIOStatus::THROWN;
}

#ifdef WIN32
boolean ENIOUtil::drain(int fd) {
	char buf[128];
	int tn = 0;

	for (;;) {
		int n = ESocketDispatcher::read(fd, buf, sizeof(buf));
		tn += n;
		if ((n < 0) && (errno != EAGAIN))
			throw EIOException(__FILE__, __LINE__, "Drain");
		if (n == (int) sizeof(buf))
			continue;
		return (tn > 0) ? TRUE : FALSE;
	}
	//never reach here.
	return FALSE;
}
#else
boolean ENIOUtil::drain(int fd) {
	char buf[128];
	int tn = 0;

	for (;;) {
		int n = ::read(fd, buf, sizeof(buf));
		tn += n;
		if ((n < 0) && (errno != EAGAIN))
			throw EIOException(__FILE__, __LINE__, "Drain");
		if (n == (int) sizeof(buf))
			continue;
		return (tn > 0) ? TRUE : FALSE;
	}
	//never reach here.
	return FALSE;
}
#endif //!

int ENIOUtil::remaining(EA<EIOByteBuffer*>* bufs) {
	int numBufs = bufs->length();
	for (int i = 0; i < numBufs; i++) {
		if ((*bufs)[i]->hasRemaining()) {
			return i;
		}
	}
	return -1;
}

#ifdef WIN32
	//
#else
int ENIOUtil::fdLimit() {
	struct rlimit rlp;
	if (getrlimit(RLIMIT_NOFILE, &rlp) < 0) {
		throw EIOException(__FILE__, __LINE__, "getrlimit failed");
		//return -1;
	}
	if (rlp.rlim_max < 0 || rlp.rlim_max > ES_INT32_MAX_VALUE) {
		return ES_INT32_MAX_VALUE;
	} else {
		return (int)rlp.rlim_max;
	}
}
#endif //!

} /* namespace nio */
} /* namespace efc */
