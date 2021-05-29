/*
 * EFileInputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EFileInputStream.hh"
#include "ESystem.hh"
#include "../nio/inc/EFileChannel.hh"

#include <sys/stat.h>
#include <limits.h>
#ifdef WIN32
#include <io.h>
#else
#include <sys/ioctl.h>
#endif

namespace efc {

#define DEFAULT_STAT_BLKSIZE 4096

inline size_t restartable_read(int fd, void *buf, unsigned int nBytes) {
  size_t res;
  RESTARTABLE( (size_t) ::read(fd, buf, (size_t) nBytes), res);
  return res;
}

#ifdef WIN32
//@see: openjdk-7/jdk/src/windows/native/java/io/io_util_md.c
static llong handleLseek(int fd, llong offset, int whence)
{
    LARGE_INTEGER pos, distance;
    DWORD lowPos = 0;
    long highPos = 0;
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
        return -1;
    }
    return pos.QuadPart;
}

//@see: openjdk-7/hotspot/src/os/windows/vm/os_windows.cpp
static int nonSeekAvailable(int, long *);
static int stdinAvailable(int, long *);

#define S_ISCHR(mode)   (((mode) & _S_IFCHR) == _S_IFCHR)
#define S_ISFIFO(mode)  (((mode) & _S_IFIFO) == _S_IFIFO)

#define MAX_INPUT_EVENTS 2000

static int stdinAvailable(int fd, long *pbytes) {
	HANDLE han;
	DWORD numEventsRead = 0; /* Number of events read from buffer */
	DWORD numEvents = 0; /* Number of events in buffer */
	DWORD i = 0; /* Loop index */
	DWORD curLength = 0; /* Position marker */
	DWORD actualLength = 0; /* Number of bytes readable */
	BOOL error = FALSE; /* Error holder */
	INPUT_RECORD *lpBuffer; /* Pointer to records of input events */

	if ((han = ::GetStdHandle(STD_INPUT_HANDLE)) == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

	/* Construct an array of input records in the console buffer */
	error = ::GetNumberOfConsoleInputEvents(han, &numEvents);
	if (error == 0) {
		return nonSeekAvailable(fd, pbytes);
	}

	/* lpBuffer must fit into 64K or else PeekConsoleInput fails */
	if (numEvents > MAX_INPUT_EVENTS) {
		numEvents = MAX_INPUT_EVENTS;
	}

	lpBuffer = (INPUT_RECORD *)::malloc(numEvents * sizeof(INPUT_RECORD));
	if (lpBuffer == NULL) {
		return FALSE;
	}

	error = ::PeekConsoleInput(han, lpBuffer, numEvents, &numEventsRead);
	if (error == 0) {
		::free(lpBuffer);
		return FALSE;
	}

	/* Examine input records for the number of bytes available */
	for(i=0; i<numEvents; i++) {
		if (lpBuffer[i].EventType == KEY_EVENT) {

			KEY_EVENT_RECORD *keyRecord = (KEY_EVENT_RECORD *)
			&(lpBuffer[i].Event);
			if (keyRecord->bKeyDown == TRUE) {
				CHAR *keyPressed = (CHAR *) &(keyRecord->uChar);
				curLength++;
				if (*keyPressed == '\r') {
					actualLength = curLength;
				}
			}
		}
	}

	if(lpBuffer != NULL) {
		::free(lpBuffer);
	}

	*pbytes = (long) actualLength;
	return TRUE;
}

static int os_available(int fd, long *bytes) {
	llong cur, end;
	struct _stati64 stbuf64;

	if (::_fstati64(fd, &stbuf64) >= 0) {
		int mode = stbuf64.st_mode;
		if (S_ISCHR(mode) || S_ISFIFO(mode)) {
			int ret;
			long lpbytes;
			if (fd == 0) {
				ret = stdinAvailable(fd, &lpbytes);
			} else {
				ret = nonSeekAvailable(fd, &lpbytes);
			}
			(*bytes) = (long)(lpbytes);
			return ret;
		}
		if ((cur = ::_lseeki64(fd, 0L, SEEK_CUR)) == -1) {
			return FALSE;
		} else if ((end = ::_lseeki64(fd, 0L, SEEK_END)) == -1) {
			return FALSE;
		} else if (::_lseeki64(fd, cur, SEEK_SET) == -1) {
			return FALSE;
		}
		*bytes = (long)(end - cur);
		return TRUE;
	} else {
		return FALSE;
	}
}

static int nonSeekAvailable(int fd, long *pbytes) {
	/* This is used for available on non-seekable devices
	 * (like both named and anonymous pipes, such as pipes
	 *  connected to an exec'd process).
	 * Standard Input is a special case.
	 *
	 */
	HANDLE han;

	if ((han = (HANDLE) ::_get_osfhandle(fd)) == (HANDLE)(-1)) {
		return FALSE;
	}

	if (! ::PeekNamedPipe(han, NULL, 0, NULL, (LPDWORD)pbytes, NULL)) {
		/* PeekNamedPipe fails when at EOF.  In that case we
		 * simply make *pbytes = 0 which is consistent with the
		 * behavior we get on Solaris when an fd is at EOF.
		 * The only alternative is to raise an Exception,
		 * which isn't really warranted.
		 */
		if (::GetLastError() != ERROR_BROKEN_PIPE) {
			return FALSE;
		}
		*pbytes = 0;
	}
	return TRUE;
}

#else //!

#if !defined(__APPLE__)
	#if SIZEOF_VOID_P == 8
		#define stat stat64
		#define fstat fstat64
	#endif //!SIZEOF_VOID_P
#endif

//@see: openjdk-7/hotspot/src/os/linux/vm/os_linux.inline.hpp
static llong handleLseek(int fd, llong offset, int whence)
{
	return ::lseek(fd, offset, whence);
}

//@see: openjdk-7/hotspot/src/os/linux/vm/os_linux.cpp
static int os_available(int fd, long *bytes) {
	long cur, end;
	int mode;
	struct stat buf;

	if (::fstat(fd, &buf) >= 0) {
		mode = buf.st_mode;
		if (S_ISCHR(mode) || S_ISFIFO(mode) || S_ISSOCK(mode)) {
			/*
			 * XXX: is the following call interruptible? If so, this might
			 * need to go through the INTERRUPT_IO() wrapper as for other
			 * blocking, interruptible calls in this file.
			 */
			int n;
			if (::ioctl(fd, FIONREAD, &n) >= 0) {
				*bytes = n;
				return 1;
			}
		}
	}
	if ((cur = ::lseek(fd, 0L, SEEK_CUR)) == -1) {
		return 0;
	} else if ((end = ::lseek(fd, 0L, SEEK_END)) == -1) {
		return 0;
	} else if (::lseek(fd, cur, SEEK_SET) == -1) {
		return 0;
	}
	*bytes = end - cur;
	return 1;
}
#endif //!WIN32

EFileInputStream::~EFileInputStream()
{
	try {
		close();
	} catch (...) {
	}

	delete buf;
	delete channel;
}

EFileInputStream::EFileInputStream(const char *name) :
		needClose(true), buf(null), count(0), pos(0), closed(false), channel(null) {
	ES_ASSERT(name);
	mFile = eso_fopen(name, "rb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__);
	}
	eso_file_setbuffer(mFile, NULL, 0);
	mFD = eso_fileno(mFile);
}

EFileInputStream::EFileInputStream(EFile *file) :
		needClose(true), buf(null), count(0), pos(0), closed(false), channel(null) {
	ES_ASSERT(file);
	mFile = eso_fopen(file->getPath().c_str(), "rb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__);
	}
	eso_file_setbuffer(mFile, NULL, 0);
	mFD = eso_fileno(mFile);
}

EFileInputStream::EFileInputStream(es_file_t* file) :
		needClose(false), buf(null), count(0), pos(0), closed(false), channel(null) {
	ES_ASSERT(file);
	mFile = file;
	eso_file_setbuffer(mFile, NULL, 0);
	mFD = eso_fileno(mFile);
}

EFileInputStream::EFileInputStream(es_os_file_t hfile) :
		needClose(true), buf(null), count(0), pos(0), closed(false), channel(null) {
	mFile = eso_fdopen(hfile, "rb");
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__);
	}
	eso_file_setbuffer(mFile, NULL, 0);
	mFD = eso_fileno(mFile);
}

int EFileInputStream::read() {
	if (buf) { //iobuffered.
		if (pos >= count) {
			fill();
			if (pos >= count)
				return -1;
		}
		return (*buf)[pos++] & 0xff;
	}
	else {
		return EInputStream::read();
	}
}

int EFileInputStream::read(void *b, int len)
{
	ES_ASSERT(b);

	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__, "Stream closed");
	}

	if (len < 0) {
		throw EIndexOutOfBoundsException(__FILE__, __LINE__);
	} else if (len == 0) {
		return 0;
	}

	if (buf) { //iobuffered.
		int n = 0;
		for (;;) {
			int nread = read1(b, n, len - n);
			if (nread <= 0)
				return (n == 0) ? nread : n;
			n += nread;
			if (n >= len)
				return n;
			// if not closed but no bytes available, return
			if (available() <= 0)
				return n;
		}
	}
	else {
		return read0(b, len);
	}
}

long EFileInputStream::skip(long n)
{
	if (mFD == -1) {
		throw EIOException(__FILE__, __LINE__, "Stream closed");
	}

	if (n<= 0L) {
		return 0L;
	}

	if (buf) { //iobuffered.
		long avail = count - pos;

		if (avail <= 0) {
			return skip0(n);
		}

		long skipped = (avail < n) ? avail : n;
		pos += skipped;
		return skipped;
	}
	else {
		return skip0(n);
	}
}

long EFileInputStream::available()
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}

	if (buf) { //iobuffered.
		long n = count - pos;
		long avail = available0();
		return n > (LONG_MAX - avail)
					? LONG_MAX
					: n + avail;
	}
	else {
		return available0();
	}
}

void EFileInputStream::close()
{
	SYNCBLOCK(&closeLock) {
		if (closed) {
			return;
		}
		closed = true;
	}}

	if (channel != null) {
	   channel->close();
	}

	if (needClose && mFile && eso_fclose(mFile) != 0) {
		throw EIOException(__FILE__, __LINE__);
	}
}

es_file_t* EFileInputStream::getFD()
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	return (mFile);
}

nio::EFileChannel* EFileInputStream::getChannel() {
	SYNCHRONIZED(this) {
		if (channel == null) {
			channel = nio::EFileChannel::open(eso_fileno(mFile), true, false);
		}
		return channel;
	}}
}

boolean EFileInputStream::isIOBuffered() {
	return buf ? true : false;
}

void EFileInputStream::setIOBuffered(boolean onoff) {
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}

	if (onoff) { //iobuffered.
		if (!buf) {
			buf = new EA<byte>(8192);
			count = pos = 0;
		}
	}
	else {
		int avail = count - pos;
		if (avail > 0) {
			skip0(0 - avail);
		}
		delete buf;
		buf = null;
		count = pos = 0;
	}
}

void EFileInputStream::fill() {
	count = pos = 0;
	int n = read0(buf->address(), buf->length());
	if (n > 0)
		count = n + pos;
}

int EFileInputStream::read1(void* b, int off, int len) {
	int avail = count - pos;
	if (avail <= 0) {
		/* If the requested length is at least as large as the buffer, and
		   if there is no mark/reset activity, do not bother to copy the
		   bytes into the local buffer.  In this way buffered streams will
		   cascade harmlessly. */
		if (len >= buf->length()) {
			return read0((byte*)b + off, len);
		}
		fill();
		avail = count - pos;
		if (avail <= 0) return -1;
	}
	int cnt = (avail < len) ? avail : len;
	ESystem::arraycopy(buf->address(), pos, b, off, cnt);
	pos += cnt;
	return cnt;
}

int EFileInputStream::read0(void* b, int len) {
	//@see: openjdk-7/jdk/src/share/native/java/io/io_util.c :: readBytes()
	//@see: openjdk-7/jdk/src/windows/native/java/io/io_util_md.c :: handleRead()
	//@see: openjdk-7/hotspot/src/os/linux/vm/os_linux.inline.hpp :: restartable_read()
	//	if (nread > 0) {
	//		(*env)->SetByteArrayRegion(env, bytes, off, nread, (jbyte *)buf);
	//	} else if (nread == JVM_IO_ERR) {
	//		JNU_ThrowIOExceptionWithLastError(env, "Read error");
	//	} else if (nread == JVM_IO_INTR) {
	//		JNU_ThrowByName(env, "java/io/InterruptedIOException", NULL);
	//	} else { /* EOF */
	//		nread = -1;
	//	}
	int nread = (int)restartable_read(mFD, b, len);
	if (nread == -1) {
		throw EIOException(__FILE__, __LINE__, "Read error");
	}
	else if (nread == 0) { /* EOF */
		nread = -1;
	}
	return nread;
}

long EFileInputStream::skip0(long n) {
	long cur = 0L;
	long end = 0L;
	if ((cur = handleLseek(mFD, 0L, ES_SEEK_CUR)) == -1) {
		throw EIOException(__FILE__, __LINE__);
	} else if ((end = handleLseek(mFD, n, ES_SEEK_CUR)) == -1) {
		throw EIOException(__FILE__, __LINE__);
	}
	return (end - cur);
}

long EFileInputStream::available0()
{
	long ret;
	if (os_available(mFD, &ret)) {
		if (ret > LONG_MAX) {
			ret = LONG_MAX;
		}
		return ret;
	}
	throw EIOException(__FILE__, __LINE__);
}

} /* namespace efc */
