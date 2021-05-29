/*
 * ESocketDispatcher.hh
 *
 *  Created on: 2013-12-31
 *      Author: cxxjava@163.com
 */

#ifndef ESOCKETDISPATCHER_HH_
#define ESOCKETDISPATCHER_HH_

#include "../../inc/EA.hh"
#include "../inc/EFileDispatcher.hh"

namespace efc {
namespace nio {

/**
 * Allows different platforms to call different native methods
 * for read and write operations.
 */
class ESocketDispatcher {
public:
	static int read(int fd, void* address, int len) THROWS(EIOException);
	static long readv(int fd, void* address, int len) THROWS(EIOException);
	static int write(int fd, void* address, int len) THROWS(EIOException);
	static long writev(int fd, void* address, int len) THROWS(EIOException);

	static int read(int fd, EIOByteBuffer* bb) THROWS(EIOException);
	static int read(int fd, EA<EIOByteBuffer*>* dsts, int offset, int length) THROWS(EIOException);
	static int write(int fd, EIOByteBuffer* bb) THROWS(EIOException);
	static int write(int fd, EA<EIOByteBuffer*>* srcs, int offset, int length) THROWS(EIOException);

	static void close(int fd) THROWS(EIOException);
	static void preClose(int fd) THROWS(EIOException);
};

} /* namespace nio */
} /* namespace efc */

#endif /* ESOCKETDISPATCHER_HH_ */
