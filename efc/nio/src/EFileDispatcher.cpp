/*
 * EFileDispatcher.cpp
 *
 *  Created on: 2013-12-31
 *      Author: cxxjava@163.com
 */

#include "./ENIOUtil.hh"
#include "../inc/EFileDispatcher.hh"
#include "../../inc/EIOStatus.hh"
#include "../../inc/EInteger.hh"

#include "../../inc/ESynchronizeable.hh"

#ifdef WIN32
#include <windows.h>
#include <io.h>
#else
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#if !defined(__APPLE__)
	#include <sys/sendfile.h>
	#if SIZEOF_VOID_P == 8
		#define lseek lseek64
		#define ftruncate ftruncate64
		#define stat stat64
		#define fstat fstat64
		#define flock flock64
		#define F_SETLKW F_SETLKW64
		#define F_SETLK F_SETLK64
		#ifdef __solaris__
			#define sendfilev sendfilev64
		#else
			#define sendfile sendfile64
		#endif
	#else //
		#ifdef __solaris__
			#define sendfilev sendfilev
		#endif
	#endif //!SIZEOF_VOID_P
#endif
#endif //!WIN32


namespace efc {
namespace nio {

#ifdef WIN32

struct iovec {
    void *iov_base;
    size_t iov_len;
};

//@see: openjdk-6/jdk/src/windows/native/sun/nio/ch/FileDispatcher.c

int EFileDispatcher::read(int fd, void* address, int len) {
	DWORD reads = 0;
	BOOL result = 0;
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	if (h == INVALID_HANDLE_VALUE) {
		throw EIOException(__FILE__, __LINE__, "Invalid handle");
		return EIOStatus::THROWN;
	}
	result = ReadFile(h,          /* File handle to read */
					  (LPVOID)address,    /* address to put data */
					  len,        /* number of bytes to read */
					  &reads,      /* number of bytes read */
					  NULL);      /* no overlapped struct */
	if (result == 0) {
		int error = GetLastError();
		if (error == ERROR_BROKEN_PIPE) {
			return EIOStatus::EOF_;
		}
		if (error == ERROR_NO_DATA) {
			return EIOStatus::UNAVAILABLE;
		}
		throw EIOException(__FILE__, __LINE__, "Read failed");
		return EIOStatus::THROWN;
	}

	return (int)ENIOUtil::convertReturnVal(reads, TRUE);
}

int EFileDispatcher::pread(int fd, void* address, int len, long position, ELock* lock) {
	SYNCBLOCK(lock) {
		DWORD reads = 0;
		BOOL result = 0;
		HANDLE h = (HANDLE)_get_osfhandle(fd);
		DWORD lowPos = 0;
		long highPos = 0;
		DWORD lowOffset = 0;
		long highOffset = 0;

		if (h == INVALID_HANDLE_VALUE) {
			throw EIOException(__FILE__, __LINE__, "Invalid handle");
			return EIOStatus::THROWN;
		}

		lowPos = SetFilePointer(h, 0, &highPos, FILE_CURRENT);
		if (lowPos == ((DWORD)-1)) {
			if (GetLastError() != ERROR_SUCCESS) {
				throw EIOException(__FILE__, __LINE__, "Seek failed");
				return EIOStatus::THROWN;
			}
		}

		lowOffset = (DWORD)position;
		highOffset = (DWORD)(position >> 32);
		lowOffset = SetFilePointer(h, lowOffset, &highOffset, FILE_BEGIN);
		if (lowOffset == ((DWORD)-1)) {
			if (GetLastError() != ERROR_SUCCESS) {
				throw EIOException(__FILE__, __LINE__, "Seek failed");
				return EIOStatus::THROWN;
			}
		}

		result = ReadFile(h,                /* File handle to read */
						  (LPVOID)address,  /* address to put data */
						  len,              /* number of bytes to read */
						  &reads,            /* number of bytes read */
						  NULL);              /* struct with offset */

		if (result == 0) {
			int error = GetLastError();
			if (error == ERROR_BROKEN_PIPE) {
				return EIOStatus::EOF_;
			}
			if (error == ERROR_NO_DATA) {
				return EIOStatus::UNAVAILABLE;
			}
			throw EIOException(__FILE__, __LINE__, "Read failed");
			return EIOStatus::THROWN;
		}

		lowPos = SetFilePointer(h, lowPos, &highPos, FILE_BEGIN);
		if (lowPos == ((DWORD)-1)) {
			if (GetLastError() != ERROR_SUCCESS) {
				throw EIOException(__FILE__, __LINE__, "Seek failed");
				return EIOStatus::THROWN;
			}
		}
		return ENIOUtil::convertReturnVal(reads, TRUE);
    }}
}

long EFileDispatcher::readv(int fd, void* address, int len) {
	DWORD reads = 0;
	BOOL result = 0;
	long totalRead = 0;
	LPVOID loc;
	int i = 0;
	DWORD num = 0;
	struct iovec *iovecp = (struct iovec *)address;
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	if (h == INVALID_HANDLE_VALUE) {
		throw EIOException(__FILE__, __LINE__, "Invalid handle");
		return EIOStatus::THROWN;
	}

	for(i=0; i<len; i++) {
		loc = (LPVOID)(iovecp[i].iov_base);
		num = iovecp[i].iov_len;
		result = ReadFile(h,                /* File handle to read */
						  loc,              /* address to put data */
						  num,              /* number of bytes to read */
						  &reads,            /* number of bytes read */
						  NULL);            /* no overlapped struct */
		if (reads > 0) {
			totalRead += reads;
		}
		if (reads < num) {
			break;
		}
	}

	if (result == 0) {
		int error = GetLastError();
		if (error == ERROR_BROKEN_PIPE) {
			return EIOStatus::EOF_;
		}
		if (error == ERROR_NO_DATA) {
			return EIOStatus::UNAVAILABLE;
		}
		throw EIOException(__FILE__, __LINE__, "Read failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(totalRead, TRUE);
}

int EFileDispatcher::write(int fd, void* address, int len) {
	BOOL result = 0;
	DWORD written = 0;
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	if (h != INVALID_HANDLE_VALUE) {
		result = WriteFile(h,           /* File handle to write */
					  (LPCVOID)address, /* pointers to the buffers */
					  len,              /* number of bytes to write */
					  &written,         /* receives number of bytes written */
					  NULL);            /* no overlapped struct */
	}

	if ((h == INVALID_HANDLE_VALUE) || (result == 0)) {
		throw EIOException(__FILE__, __LINE__, "Write failed");
		return EIOStatus::THROWN;
	}

	return (int)ENIOUtil::convertReturnVal(written, FALSE);
}

int EFileDispatcher::pwrite(int fd, void* address, int len, long position, ELock* lock) {
	SYNCBLOCK(lock) {
		BOOL result = 0;
	    DWORD written = 0;
	    HANDLE h = (HANDLE)_get_osfhandle(fd);
	    DWORD lowPos = 0;
	    long highPos = 0;
	    DWORD lowOffset = 0;
	    long highOffset = 0;

	    lowPos = SetFilePointer(h, 0, &highPos, FILE_CURRENT);
	    if (lowPos == ((DWORD)-1)) {
	        if (GetLastError() != ERROR_SUCCESS) {
	        	throw EIOException(__FILE__, __LINE__, "Seek failed");
	        	return EIOStatus::THROWN;
	        }
	    }

	    lowOffset = (DWORD)position;
	    highOffset = (DWORD)(position >> 32);
	    lowOffset = SetFilePointer(h, lowOffset, &highOffset, FILE_BEGIN);
	    if (lowOffset == ((DWORD)-1)) {
	        if (GetLastError() != ERROR_SUCCESS) {
	        	throw EIOException(__FILE__, __LINE__, "Seek failed");
	        	return EIOStatus::THROWN;
	        }
	    }

	    result = WriteFile(h,               /* File handle to write */
	                      (LPCVOID)address, /* pointers to the buffers */
	                      len,              /* number of bytes to write */
	                      &written,         /* receives number of bytes written */
	                      NULL);            /* no overlapped struct */

	    if ((h == INVALID_HANDLE_VALUE) || (result == 0)) {
	    	throw EIOException(__FILE__, __LINE__, "Write failed");
	    	return EIOStatus::THROWN;
	    }

	    lowPos = SetFilePointer(h, lowPos, &highPos, FILE_BEGIN);
	    if (lowPos == ((DWORD)-1)) {
	        if (GetLastError() != ERROR_SUCCESS) {
	        	throw EIOException(__FILE__, __LINE__, "Seek failed");
	        	return EIOStatus::THROWN;
	        }
	    }

	    return ENIOUtil::convertReturnVal(written, FALSE);
    }}
}

long EFileDispatcher::writev(int fd, void* address, int len) {
	BOOL result = 0;
	DWORD written = 0;
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	long totalWritten = 0;

	if (h != INVALID_HANDLE_VALUE) {
		LPVOID loc;
		int i = 0;
		DWORD num = 0;
		struct iovec *iovecp = (struct iovec *)address;

		for(i=0; i<len; i++) {
			loc = (LPVOID)(iovecp[i].iov_base);
			num = iovecp[i].iov_len;
			result = WriteFile(h,       /* File handle to write */
							   loc,     /* pointers to the buffers */
							   num,     /* number of bytes to write */
							   &written,/* receives number of bytes written */
							   NULL);   /* no overlapped struct */
			if (written > 0) {
				totalWritten += written;
			}
			if (written < num) {
				break;
			}
		}
	}

	if ((h == INVALID_HANDLE_VALUE) || (result == 0)) {
		throw EIOException(__FILE__, __LINE__, "Write failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(totalWritten, FALSE);
}

void EFileDispatcher::close(int fd) {
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	if (h != INVALID_HANDLE_VALUE) {
        int result = CloseHandle(h);
        if (result < 0)
		throw EIOException(__FILE__, __LINE__, "Close failed");
	}
}

void EFileDispatcher::preClose(int fd) {
	//
}

long EFileDispatcher::position(int fd, long offset, int whence) {
	LARGE_INTEGER pos, distance;
	DWORD op = FILE_CURRENT;
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	if (whence == SEEK_END) {
		op = FILE_END;
	}
	if (whence == SEEK_CUR) {
		op = FILE_CURRENT;
	}
	if (whence == SEEK_SET) {
		op = FILE_BEGIN;
	}

	distance.QuadPart = offset;
	if (SetFilePointerEx(h, distance, &pos, op) == 0) {
		throw EIOException(__FILE__, __LINE__, "Seek failed", GetLastError());
		return EIOStatus::THROWN;
	}
	return pos.QuadPart;
}

long EFileDispatcher::size(int fd) {
	DWORD sizeLow = 0;
	DWORD sizeHigh = 0;
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	sizeLow = GetFileSize(h, &sizeHigh);
	if (sizeLow == ((DWORD)-1)) {
		if (GetLastError() != ERROR_SUCCESS) {
			throw EIOException(__FILE__, __LINE__, "Size failed");
			return EIOStatus::THROWN;
		}
	}
	return (((long)sizeHigh) << 32) | sizeLow;
}

int EFileDispatcher::truncate(int fd, long size) {
	DWORD lowPos = 0;
	long highPos = 0;
	BOOL result = 0;
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	lowPos = (DWORD)size;

	#if SIZEOF_VOID_P == 8
		highPos = (long)(size >> 32);
	#endif
	lowPos = SetFilePointer(h, lowPos, &highPos, FILE_BEGIN);
	if (lowPos == ((DWORD)-1)) {
		if (GetLastError() != ERROR_SUCCESS) {
			throw EIOException(__FILE__, __LINE__, "Truncation failed");
			return EIOStatus::THROWN;
		}
	}
	result = SetEndOfFile(h);
	if (result == 0) {
		throw EIOException(__FILE__, __LINE__, "Truncation failed");
		return EIOStatus::THROWN;
	}
	return 0;
}

int EFileDispatcher::force(int fd, boolean md) {
	int result = 0;
	HANDLE h = (HANDLE)_get_osfhandle(fd);

	if (h != INVALID_HANDLE_VALUE) {
		result = FlushFileBuffers(h);
		if (result == 0) {
			int error = GetLastError();
			if (error != ERROR_ACCESS_DENIED) {
				throw EIOException(__FILE__, __LINE__, "Force failed");
				return EIOStatus::THROWN;
			}
		}
	} else {
		throw EIOException(__FILE__, __LINE__, "Force failed");
		return EIOStatus::THROWN;
	}
	return 0;
}

int EFileDispatcher::lock(int fd, boolean blocking, long pos, long size, boolean shared) {
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	DWORD lowPos = (DWORD)pos;
	long highPos = 0;
	DWORD lowNumBytes = (DWORD)size;
	DWORD highNumBytes = 0;
	int result = 0;

	#if SIZEOF_VOID_P == 8
	highPos = (long)(pos >> 32);
	highNumBytes = (DWORD)(size >> 32);
	#endif

	//if (onNT)
	{
		DWORD flags = 0;
		OVERLAPPED o;
		o.hEvent = 0;
		o.Offset = lowPos;
		o.OffsetHigh = highPos;
		if (blocking == false) {
			flags |= LOCKFILE_FAIL_IMMEDIATELY;
		}
		if (shared == false) {
			flags |= LOCKFILE_EXCLUSIVE_LOCK;
		}
		result = LockFileEx(h, flags, 0, lowNumBytes, highNumBytes, &o);
		if (result == 0) {
			int error = GetLastError();
			if (error != ERROR_LOCK_VIOLATION) {
				throw EIOException(__FILE__, __LINE__, "Lock failed");
				return NO_LOCK;
			}
			if (flags & LOCKFILE_FAIL_IMMEDIATELY) {
				return NO_LOCK;
			}
			throw EIOException(__FILE__, __LINE__, "Lock failed");
			return NO_LOCK;
		}
		return LOCKED;
	}
	return NO_LOCK;
}

void EFileDispatcher::release(int fd, long pos, long size) {
	HANDLE h = (HANDLE)_get_osfhandle(fd);
	DWORD lowPos = (DWORD)pos;
	long highPos = 0;
	DWORD lowNumBytes = (DWORD)size;
	DWORD highNumBytes = 0;
	int result = 0;

	#if SIZEOF_VOID_P == 8
	highPos = (long)(pos >> 32);
	highNumBytes = (DWORD)(size >> 32);
	#endif

	//if (onNT)
	{
		OVERLAPPED o;
		o.hEvent = 0;
		o.Offset = lowPos;
		o.OffsetHigh = highPos;
		result = UnlockFileEx(h, 0, lowNumBytes, highNumBytes, &o);
	}
	if (result == 0) {
		throw EIOException(__FILE__, __LINE__, "Release failed");
	}
}

long EFileDispatcher::transferTo(int src, long position, long count, int dst) {
	return EIOStatus::UNSUPPORTED;
}

#else //!

//@see: openjdk-6/jdk/src/solaris/native/sun/nio/ch/FileDispatcher.c

#if !defined(__APPLE__)
	#if SIZEOF_VOID_P == 8
		#define PREAD pread64
		#define PWRITE pwrite64
	#else
		#define PREAD pread
		#define PWRITE pwrite
	#endif
#else
	#define PREAD pread
	#define PWRITE pwrite
#endif

static long
handle(long rv, const char *msg)
{
    if (rv >= 0)
        return rv;
    if (errno == EINTR)
        return EIOStatus::INTERRUPTED;
    throw EIOException(__FILE__, __LINE__, msg);
    return EIOStatus::THROWN;
}

int EFileDispatcher::preCloseFD = -1;

void EFileDispatcher::init() {
	if (preCloseFD == -1) {
		int sp[2];
		if (socketpair(PF_UNIX, SOCK_STREAM, 0, sp) < 0) {
			throw EIOException(__FILE__, __LINE__, "socketpair failed");
			return;
		}
		preCloseFD = sp[0];
		close(sp[1]);
	}
}

int EFileDispatcher::read(int fd, void* address, int len) {
	return (int)ENIOUtil::convertReturnVal(::read(fd, address, len), TRUE);
}

int EFileDispatcher::pread(int fd, void* address, int len,
		long position, ELock* lock) {
	return ENIOUtil::convertReturnVal(::PREAD(fd, address, len, position), TRUE);
}

long EFileDispatcher::readv(int fd, void* address, int len) {
	struct iovec *iov = (struct iovec *)address;
	return ENIOUtil::convertReturnVal(::readv(fd, iov, len), TRUE);
}

int EFileDispatcher::write(int fd, void* address, int len) {
	return (int)ENIOUtil::convertReturnVal(::write(fd, address, len), FALSE);
}

int EFileDispatcher::pwrite(int fd, void* address, int len,
		long position, ELock* lock) {
	return ENIOUtil::convertReturnVal(::PWRITE(fd, address, len, position), TRUE);
}

long EFileDispatcher::writev(int fd, void* address, int len) {
	struct iovec *iov = (struct iovec *)address;
	return ENIOUtil::convertReturnVal(::writev(fd, iov, len), FALSE);
}

void EFileDispatcher::close(int fd) {
	if (fd != -1) {
		int result = ::close(fd);
		if (result < 0)
			throw EIOException(__FILE__, __LINE__, "Close failed");
	}
}

void EFileDispatcher::preClose(int fd) {
	init();

	if (preCloseFD >= 0) {
		if (dup2(preCloseFD, fd) < 0)
			throw EIOException(__FILE__, __LINE__, "dup2 failed");
	}
}

long EFileDispatcher::position(int fd, long offset, int whence) {
	long result = lseek(fd, offset, whence);
	return handle(result, "Position failed");
}

long EFileDispatcher::size(int fd) {
	struct stat fbuf;

	if (fstat(fd, &fbuf) < 0)
		return handle(-1, "Size failed");
	return fbuf.st_size;
}

int EFileDispatcher::truncate(int fd, long size) {
	int rv = ftruncate(fd, size);
	return handle(rv, "Truncation failed");
}

int EFileDispatcher::force(int fd, boolean md) {
	int result = 0;

	if (md == false) {
#ifdef __APPLE__
        //@see: openjdk-8/src/solaris/native/sun/nio/ch/FileDispatcherImpl.c#L52
        #define fdatasync fsync
#endif
		result = fdatasync(fd);
	} else {
		result = fsync(fd);
	}
	return handle(result, "Force failed");
}

int EFileDispatcher::lock(int fd, boolean blocking, long pos, long size, boolean shared) {
	int lockResult = 0;
	int cmd = 0;
	struct flock fl;

	fl.l_whence = SEEK_SET;
	fl.l_len = size;
	fl.l_start = pos;
	if (shared == true) {
		fl.l_type = F_RDLCK;
	} else {
		fl.l_type = F_WRLCK;
	}
	if (blocking == true) {
		cmd = F_SETLKW;
	} else {
		cmd = F_SETLK;
	}
	lockResult = fcntl(fd, cmd, &fl);
	if (lockResult < 0) {
		if ((cmd == F_SETLK) && (errno == EAGAIN))
			return NO_LOCK;
		if (errno == EINTR)
			return INTERRUPTED;
		throw EIOException(__FILE__, __LINE__, "Lock failed");
	}
	return 0;
}

void EFileDispatcher::release(int fd, long pos, long size) {
	int lockResult = 0;
	struct flock fl;
	int cmd = F_SETLK;

	fl.l_whence = SEEK_SET;
	fl.l_len = size;
	fl.l_start = pos;
	fl.l_type = F_UNLCK;
	lockResult = fcntl(fd, cmd, &fl);
	if (lockResult < 0) {
		throw EIOException(__FILE__, __LINE__, "Release failed");
	}
}

long EFileDispatcher::transferTo(int srcFD, long position, long count, int dstFD) {
#ifdef __solaris__
	typedef struct sendfilevec64 {
	    int     sfv_fd;         /* input fd */
	    uint_t  sfv_flag;       /* Flags. see below */
	    off64_t sfv_off;        /* offset to start reading from */
	    size_t  sfv_len;        /* amount of data */
	} sendfilevec_t;

    sendfilevec_t sfv;
	size_t numBytes = 0;
	long result;

	sfv.sfv_fd = srcFD;
	sfv.sfv_flag = 0;
	sfv.sfv_off = (off64_t)position;
	sfv.sfv_len = count;

	result = sendfilev(dstFD, &sfv, 1, &numBytes);

	/* Solaris sendfilev() will return -1 even if some bytes have been
	 * transferred, so we check numBytes first.
	 */
	if (numBytes > 0)
		return numBytes;
	if (result < 0) {
		if (errno == EAGAIN)
			return EIOStatus::UNAVAILABLE;
		if ((errno == EINVAL) && ((ssize_t)count >= 0))
			return EIOStatus::UNSUPPORTED_CASE;
		if (errno == EINTR)
			return EIOStatus::INTERRUPTED;
		throw EIOException(__FILE__, __LINE__, "Transfer failed");
		return EIOStatus::THROWN;
	}
	return result;
#elif defined(__linux__)
    off_t offset;
	long n = sendfile(dstFD, srcFD, &offset, (size_t)count);
	if (n < 0) {
		if (errno == EAGAIN)
			return EIOStatus::UNAVAILABLE;
		if ((errno == EINVAL) && ((ssize_t)count >= 0))
			return EIOStatus::UNSUPPORTED_CASE;
		if (errno == EINTR) {
			return EIOStatus::INTERRUPTED;
		}
		throw EIOException(__FILE__, __LINE__, "Transfer failed");
		return EIOStatus::THROWN;
	}
	return n;
#elif defined(__APPLE__)
	off_t numBytes;
	int result;

	numBytes = count;

	result = sendfile(srcFD, dstFD, position, &numBytes, NULL, 0);

	if (numBytes > 0)
		return numBytes;

	if (result == -1) {
		if (errno == EAGAIN)
			return EIOStatus::UNAVAILABLE;
		if (errno == EOPNOTSUPP || errno == ENOTSOCK || errno == ENOTCONN)
			return EIOStatus::UNSUPPORTED_CASE;
		if ((errno == EINVAL) && ((ssize_t)count >= 0))
			return EIOStatus::UNSUPPORTED_CASE;
		if (errno == EINTR)
			return EIOStatus::INTERRUPTED;
		throw EIOException(__FILE__, __LINE__, "Transfer failed");
		return EIOStatus::THROWN;
	}

	return result;
#else
    return EIOStatus::UNSUPPORTED_CASE;
#endif
}

#endif //!WIN32

int EFileDispatcher::read(int fd, EIOByteBuffer* bb) {
	int pos = bb->position();
	int lim = bb->limit();
	ES_ASSERT(pos <= lim);
	int rem = (pos <= lim ? lim - pos : 0);

	if (rem == 0)
		return 0;
	int n = read(fd, (char*)(bb->address()) + pos, rem);
	if (n > 0)
		bb->position(pos + n);
	return n;
}

int EFileDispatcher::write(int fd, EIOByteBuffer* bb) {
	int pos = bb->position();
	int lim = bb->limit();
	ES_ASSERT(pos <= lim);
	int rem = (pos <= lim ? lim - pos : 0);

	if (rem == 0)
		return 0;
	int n = write(fd, (char*)(bb->address()) + pos, rem);
	if (n > 0)
		bb->position(pos + (int)n);
	return n;
}

long EFileDispatcher::read(int fd, EA<EIOByteBuffer*>* dsts, int offset, int length) {
	//@see: openjdk-6/jdk/src/share/classes/sun/nio/ch/IOUtil.java

	if ((offset < 0) || (length < 0) || (offset > dsts->length() - length))
		throw EINDEXOUTOFBOUNDSEXCEPTION;

	struct iovec *iovBase = (struct iovec *)eso_calloc(sizeof(struct iovec));
    struct iovec *iov = iovBase;
	long bytesRead = 0;
	try {
		for (int i=0; i<length; i++) {
			EIOByteBuffer* bb = (*dsts)[i + offset];
			iov->iov_base = bb->current();
			iov->iov_len = bb->remaining();
			iov++;
		}
		bytesRead = readv(fd, iovBase, length);
	} catch (...) {
		finally {
			eso_free(iovBase);
		}
		throw;
	} finally {
		eso_free(iovBase);
	}
	return bytesRead;
}

long EFileDispatcher::write(int fd, EA<EIOByteBuffer*>* srcs, int offset, int length) {
	//@see: openjdk-6/jdk/src/share/classes/sun/nio/ch/IOUtil.java

	if ((offset < 0) || (length < 0) || (offset > srcs->length() - length))
		throw EINDEXOUTOFBOUNDSEXCEPTION;

	struct iovec *iovBase = (struct iovec *) eso_calloc(sizeof(struct iovec) * length);
    struct iovec *iov = iovBase;
	long bytesWritten = 0;
	try {
		for (int i = 0; i < length; i++) {
			EIOByteBuffer* bb = (*srcs)[i + offset];
			iov->iov_base = bb->current();
			iov->iov_len = bb->remaining();
			iov++;
		}
		bytesWritten = writev(fd, iovBase, length);
	} catch (...) {
		finally {
			eso_free(iovBase);
		}
		throw;
	} finally {
		eso_free(iovBase);
	}

	long returnVal = bytesWritten;
	// Notify the buffers how many bytes were taken
	for (int i=0; i<length; i++) {
		EIOByteBuffer* nextBuffer = (*srcs)[i];
		int pos = nextBuffer->position();
		int lim = nextBuffer->limit();
		ES_ASSERT(pos <= lim);
		int len = (pos <= lim ? lim - pos : lim);
		if (bytesWritten >= len) {
			bytesWritten -= len;
			int newPosition = pos + len;
			nextBuffer->position(newPosition);
		} else { // Buffers not completely filled
			if (bytesWritten > 0) {
				ES_ASSERT(pos + bytesWritten < (long)EInteger::MAX_VALUE);
				int newPosition = (int)(pos + bytesWritten);
				nextBuffer->position(newPosition);
			}
			break;
		}
	}
	return returnVal;
}

int EFileDispatcher::pread(int fd, long position, EIOByteBuffer* bb, ELock* lock) {
	int pos = bb->position();
	int lim = bb->limit();
	ES_ASSERT(pos <= lim);
	int rem = (pos <= lim ? lim - pos : 0);

	if (rem == 0)
		return 0;
	int n = pread(fd, (char*)(bb->address()) + pos, rem, position, lock);
	if (n > 0)
		bb->position(pos + n);
	return n;
}

int EFileDispatcher::pwrite(int fd, long position, EIOByteBuffer* bb, ELock* lock) {
	int pos = bb->position();
	int lim = bb->limit();
	ES_ASSERT(pos <= lim);
	int rem = (pos <= lim ? lim - pos : 0);

	if (rem == 0)
		return 0;
	int n = pwrite(fd, (char*)(bb->address()) + pos, rem, position, lock);
	if (n > 0)
		bb->position(pos + (int)n);
	return n;
}

} /* namespace nio */
} /* namespace efc */
