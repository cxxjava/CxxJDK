/*
 * ERandomAccessFile.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "ERandomAccessFile.hh"
#include "EStream.hh"
#include "EFloat.hh"
#include "EDouble.hh"
#include "EEOFException.hh"
#include "../nio/inc/EFileChannel.hh"

#include <sys/stat.h>
#include <limits.h>
#ifdef WIN32
#include <io.h>
#else
#include <sys/ioctl.h>
#endif

namespace efc {

inline size_t restartable_read(int fd, void *buf, unsigned int nBytes) {
  size_t res;
  RESTARTABLE( (size_t) ::read(fd, buf, (size_t) nBytes), res);
  return res;
}

inline size_t restartable_write(int fd, const void *buf, unsigned int nBytes) {
  size_t res;
  RESTARTABLE((size_t) ::write(fd, buf, (size_t) nBytes), res);
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

ERandomAccessFile::~ERandomAccessFile()
{
	try {
		close();
	} catch (...) {
	}
}

void ERandomAccessFile::open0(const char *name, const char *mode)
{
	ES_ASSERT(name);
	ES_ASSERT(mode);
	
	mSync = false;
	char *cmode = NULL;
	if (eso_strncmp(mode, "rw", 2) == 0) { //rwd | rws
		if (eso_strcmp(mode, "rwd") == 0 || eso_strcmp(mode, "rws") == 0) {
			mSync = true;
		}
		cmode = (char*)"w+";
		rw = true;
	}
	else {
		cmode = (char*)mode;
	}
	
	mFile = eso_fopen(name, cmode);
	if (!mFile) {
		throw EFileNotFoundException(__FILE__, __LINE__);
	}

	mFD = eso_fileno(mFile);
}

void ERandomAccessFile::fsync0() {
	if (mFile == null || mFD == -1) {
		throw EIOException(__FILE__, __LINE__);
	}

	if (mSync) {
		eso_fflush(mFile); //!

		int ret;
#ifdef WIN32
		ret = ::_commit(mFD);
#else
		ret = ::fsync(mFD);
#endif
		if (ret == -1) {
			throw EIOException(__FILE__, __LINE__);
		}
	}
}

ERandomAccessFile::ERandomAccessFile(const char *name, const char *mode) :
		rw(false), closed(false), channel(null)
{
	open0(name, mode);
}

ERandomAccessFile::ERandomAccessFile(EFile *file, const char *mode) :
		rw(false), closed(false), channel(null)
{
	ES_ASSERT(file);
	open0(file->getPath().c_str(), mode);
}

es_file_t* ERandomAccessFile::getFD()
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	return (mFile);
}

int ERandomAccessFile::read()
{
	unsigned char c;
	int r = read((byte*)&c, 1);
	return (r == -1) ? -1 : (int)c;
}

int ERandomAccessFile::read(void *b, int len)
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}

	ES_ASSERT(b);

	if (len <= 0) {
		return 0;
	}

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

void ERandomAccessFile::readFully(byte *b, int len)
{
	int n = 0;
	do {
		int count = read((char*)b + n, len - n);
		if (count < 0)
			throw EEOFException(__FILE__, __LINE__);
		n += count;
	} while (n < len);
}

int ERandomAccessFile::skipBytes(int n)
{
	long pos;
	long len;
	long newpos; 

	if (n <= 0) {
	    return 0;
	}
	pos = getFilePointer();
	len = length();
	newpos = pos + n;
	if (newpos > len) {
	    newpos = len;
	}
	seek(newpos);

	/* return the actual number of bytes skipped */
	return (int) (newpos - pos);
}

void ERandomAccessFile::write(int b)
{
	char c = (char)b;
	write((byte*)&c, 1);
}

void ERandomAccessFile::write(const void *b, int len)
{
	if (mFile == null || mFD == -1) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	ES_ASSERT(b);
	
	if (len <= 0) {
		return;
	}
	
	//@see: openjdk-7/jdk/src/share/native/java/io/io_util.c :: writeBytes()
	int off = 0;
	while (len > 0) {
		if (mFD == -1) {
			throw EIOException(__FILE__, __LINE__, "Stream Closed");
			break;
		}
		int n = (int)restartable_write(mFD, (byte*)b+off, len);
		if (n == -1) {
			throw EIOException(__FILE__, __LINE__, "Write error");
			break;
		}
		off += n;
		len -= n;
	}

	if (mSync) {
		fsync0();
	}
}

llong ERandomAccessFile::getFilePointer()
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	llong pos = handleLseek(mFD, 0L, ES_SEEK_CUR);
	if (pos == -1) {
		throw EIOException(__FILE__, __LINE__);
	}
	return pos;
}

void ERandomAccessFile::seek(llong pos, int whence)
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	int ret = handleLseek(mFD, pos, whence);
	if (ret == -1) {
		throw EIOException(__FILE__, __LINE__, "Seek failed");
	}
}

llong ERandomAccessFile::length()
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	llong cur = 0L;
	llong end = 0L;
	if ((cur = handleLseek(mFD, 0L, ES_SEEK_CUR)) == -1) {
		throw EIOException(__FILE__, __LINE__, "Seek failed");
	} else if ((end = handleLseek(mFD, 0L, ES_SEEK_END)) == -1) {
		throw EIOException(__FILE__, __LINE__, "Seek failed");
	} else if (handleLseek(mFD, cur, ES_SEEK_SET) == -1) {
		throw EIOException(__FILE__, __LINE__, "Seek failed");
    }
	return end;
}

void ERandomAccessFile::setLength(llong newLength)
{
	if (mFile == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	
	if (newLength < 0) {
		throw EIOException(__FILE__, __LINE__);
	}
	int ret = eso_ftruncate(mFile, newLength);
	if (ret == -1) {
		throw EIOException(__FILE__, __LINE__);
	}
}

void ERandomAccessFile::close()
{
	SYNCBLOCK(&closeLock) {
		if (closed) {
			return;
		}
		closed = true;
	}}
	
	if (mSync) {
		fsync0();
	}

	if (channel != null) {
		channel->close();
	}

	if (eso_fclose(mFile) != 0) {
		throw EIOException(__FILE__, __LINE__);
	}
	mFile = null;
	mFD = -1;
}

boolean ERandomAccessFile::readBoolean()
{
	int ch = read();
	if (ch < 0)
	    throw EIOException(__FILE__, __LINE__);
	return (ch != 0);
}

byte ERandomAccessFile::readByte()
{
	int ch = read();
	if (ch < 0)
	    throw EIOException(__FILE__, __LINE__);
	return (byte)(ch);
}

ubyte ERandomAccessFile::readUnsignedByte()
{
	int ch = read();
	if (ch < 0)
	    throw EIOException(__FILE__, __LINE__);
	return ch;
}

short ERandomAccessFile::readShort()
{
	int ch1 = read();
	int ch2 = read();
	if ((ch1 | ch2) < 0)
	    throw EIOException(__FILE__, __LINE__);
	return (short)((ch1 << 8) + (ch2 << 0));
}

ushort ERandomAccessFile::readUnsignedShort()
{
	int ch1 = read();
	int ch2 = read();
	if ((ch1 | ch2) < 0)
	    throw EIOException(__FILE__, __LINE__);
	return (ch1 << 8) + (ch2 << 0);
}

int ERandomAccessFile::readInt()
{
	es_byte_t s[4];
	read((byte*)s, 4);
	return EStream::readInt(s);
}

llong ERandomAccessFile::readLLong()
{
	es_byte_t s[8];
	read((byte*)s, 8);
	return EStream::readLLong(s);
}

float ERandomAccessFile::readFloat()
{
	return EFloat::intBitsToFloat(readInt());
}

double ERandomAccessFile::readDouble()
{
	return EDouble::llongBitsToDouble(readLLong());
}

sp<EString> ERandomAccessFile::readLine()
{
	EString* input = new EString();
	int c = -1;
	boolean eol = false;

	while (!eol) {
		switch (c = read()) {
		case -1:
		case '\n':
			eol = true;
			break;
		case '\r':
			eol = true;
			if ((read()) != '\n') {
				eso_fseek(mFile, -1, ES_SEEK_CUR);
			}
			break;
		default:
			(*input) << (char)c;
			break;
		}
	}
	if ((c == -1) && (input->length() == 0)) {
		delete input;
		return null;
	}
	return input;
}

void ERandomAccessFile::writeBoolean(boolean v)
{
	write(v ? 1 : 0);
}

void ERandomAccessFile::writeByte(int v)
{
	write(v);
}

void ERandomAccessFile::writeShort(int v)
{
	es_byte_t s[2];
	EStream::writeShort(s, v);
	write((byte*)s, 2);
}

void ERandomAccessFile::writeInt(int v)
{
	es_byte_t s[4];
	EStream::writeInt(s, v);
	write((byte*)s, 4);
}

void ERandomAccessFile::writeLLong(llong v)
{
	es_byte_t s[8];
	EStream::writeLLong(s, v);
	write((byte*)s, 8);
}

void ERandomAccessFile::writeFloat(float v)
{
	writeInt(EFloat::floatToIntBits(v));
}

void ERandomAccessFile::writeDouble(double v)
{
	writeLLong(EDouble::doubleToLLongBits(v));
}

void ERandomAccessFile::writeBytes(const char* s)
{
	if (s == null) {
		throw EIOException(__FILE__, __LINE__);
	}
	write((byte*)s, eso_strlen(s));
}

nio::EFileChannel* ERandomAccessFile::getChannel() {
	SYNCHRONIZED (this) {
		if (channel == null) {
			channel = nio::EFileChannel::open(mFD, true, rw, false);
		}
		return channel;
	}}
}

} /* namespace efc */
