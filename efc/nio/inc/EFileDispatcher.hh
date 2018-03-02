/*
 * EFileDispatcher.hh
 *
 *  Created on: 2013-12-31
 *      Author: cxxjava@163.com
 */

#ifndef EFILEDISPATCHER_HH_
#define EFILEDISPATCHER_HH_

#include "../../inc/EA.hh"
#include "../../inc/ELock.hh"
#include "./EIOByteBuffer.hh"
#include "../../inc/EIOException.hh"

namespace efc {
namespace nio {

/**
 * Allows different platforms to call different native methods
 * for read and write operations.
 */
class EFileDispatcher {
public:
	static const int NO_LOCK = -1;       // Failed to lock
    static const int LOCKED = 0;         // Obtained requested lock
    static const int RET_EX_LOCK = 1;    // Obtained exclusive lock
    static const int INTERRUPTED = 2;    // Request interrupted

	static int read(int fd, void* address, int len) THROWS(EIOException);
	static int pread(int fd, void* address, int len, long position, ELock* lock) THROWS(EIOException);
	static long readv(int fd, void* address, int len) THROWS(EIOException);
	static int write(int fd, void* address, int len) THROWS(EIOException);
	static int pwrite(int fd, void* address, int len, long position, ELock* lock) THROWS(EIOException);
	static long writev(int fd, void* address, int len) THROWS(EIOException);
	static void close(int fd) THROWS(EIOException);
	static void preClose(int fd) THROWS(EIOException);

	static int read(int fd, EIOByteBuffer* bb) THROWS(EIOException);
	static int pread(int fd, long position, EIOByteBuffer* bb, ELock* lock) THROWS(EIOException);
	static long read(int fd, EA<EIOByteBuffer*>* dsts, int offset, int length) THROWS(EIOException);
	static int write(int fd, EIOByteBuffer* bb) THROWS(EIOException);
	static int pwrite(int fd, long position, EIOByteBuffer* bb, ELock* lock) THROWS(EIOException);
	static long write(int fd, EA<EIOByteBuffer*>* srcs, int offset, int length) THROWS(EIOException);

	static long position(int fd, long offset, int whence) THROWS(EIOException);
	static long size(int fd) THROWS(EIOException);
	static int truncate(int fd, long size) THROWS(EIOException);
	static int force(int fd, boolean md) THROWS(EIOException);
	static int lock(int fd, boolean blocking, long pos, long size, boolean shared) THROWS(EIOException);
	static void release(int fd, long pos, long size) THROWS(EIOException);
	static long transferTo(int srcFD, long position, long count, int dstFD) THROWS(EIOException);

private:
#ifndef WIN32
	/* File descriptor to which we dup other fd's
	 * before closing them for real
	 */
	static int preCloseFD;// = -1;
	static void init() THROWS(EIOException);
#endif
};

} /* namespace nio */
} /* namespace efc */

#endif /* EFILEDISPATCHER_HH_ */
