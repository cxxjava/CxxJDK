/*
 * ENIOUtil.hh
 *
 *  Created on: 2014-1-7
 *      Author: cxxjava@163.com
 */

#ifndef ENIOUTIL_HH_
#define ENIOUTIL_HH_

#include "../../inc/EA.hh"
#include "../inc/EIOByteBuffer.hh"
#include "../../inc/EIOException.hh"

namespace efc {
namespace nio {

class ENIOUtil {
public:
	static long convertReturnVal(long n, boolean reading) THROWS(EIOException);

	static boolean drain(int fd) THROWS(EIOException);

	/*
	 * Returns the index of first buffer in bufs with remaining,
	 * or -1 if there is nothing left
	 */
	static int remaining(EA<EIOByteBuffer*>* bufs);

#ifdef WIN32
	//
#else
	static int fdLimit();
#endif
};

} /* namespace nio */
} /* namespace efc */
#endif /* ENIOUTIL_HH_ */
