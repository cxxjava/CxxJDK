/*
 * EDatagramDispatcher.hh
 *
 *  Created on: 2016-8-24
 *      Author: cxxjava@163.com
 */

#ifndef EDATAGRAMDISPATCHER_HH_
#define EDATAGRAMDISPATCHER_HH_

#include "../../inc/EA.hh"
#include "../inc/EIOByteBuffer.hh"
#include "../../inc/EIOException.hh"

namespace efc {
namespace nio {

/**
 * Allows different platforms to call different native methods
 * for read and write operations.
 */

class EDatagramDispatcher {
public:
	static int read(int fd, void* address, int len) THROWS(EIOException);
	static long readv(int fd, void* address, int len) THROWS(EIOException);
	static int write(int fd, void* address, int len) THROWS(EIOException);
	static long writev(int fd, void* address, int len) THROWS(EIOException);

	static int read(int fd, EIOByteBuffer* bb) THROWS(EIOException);
	static int read(int fd, EA<EIOByteBuffer*>* dsts, int offset, int length) THROWS(EIOException);
	static int write(int fd, EIOByteBuffer* bb) THROWS(EIOException);
	static int write(int fd, EA<EIOByteBuffer*>* srcs, int offset, int length) THROWS(EIOException);

	static int recvfrom(int fd, void* address, int len, boolean connected, int* raddr, int* rport) THROWS(EIOException);
	static int sendto(int fd, void* address, int len, int raddr, int rport) THROWS(EIOException);

	static void close(int fd) THROWS(EIOException);
	static void preClose(int fd) THROWS(EIOException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EDATAGRAMDISPATCHER_HH_ */
