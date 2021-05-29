/*
 * ESocketDispatcher.cpp
 *
 *  Created on: 2013-12-31
 *      Author: cxxjava@163.com
 */

#include "./ESocketDispatcher.hh"
#include "../../inc/EIOStatus.hh"
#include "./ENIOUtil.hh"

namespace efc {
namespace nio {

#ifdef WIN32

struct iovec {
    void *iov_base;
    size_t iov_len;
};

//@see: openjdk-6/jdk/src/windows/native/sun/nio/ch/SocketDispatcher.c

int ESocketDispatcher::read(int fd, void* address, int len) {
	/* set up */
	int i = 0;
	DWORD read = 0;
	DWORD flags = 0;
	WSABUF buf;

	/* destination buffer and size */
	buf.buf = (char *)address;
	buf.len = (u_long)len;

	/* read into the buffers */
	i = WSARecv((SOCKET)fd, /* Socket */
			&buf,           /* pointers to the buffers */
			(DWORD)1,       /* number of buffers to process */
			&read,          /* receives number of bytes read */
			&flags,         /* no flags */
			0,              /* no overlapped sockets */
			0);             /* no completion routine */

	if (i == SOCKET_ERROR) {
		int theErr = (int)WSAGetLastError();
		if (theErr == WSAEWOULDBLOCK) {
			return EIOStatus::UNAVAILABLE;
		}
		throw EIOException(__FILE__, __LINE__, "Read failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(read, TRUE);
}

long ESocketDispatcher::readv(int fd, void* address, int len) {
	/* set up */
	int i = 0;
	DWORD read = 0;
	DWORD flags = 0;
	struct iovec *iovp = (struct iovec *)address;
	WSABUF *bufs = (WSABUF*)eso_malloc(len * sizeof(WSABUF));

	ES_ASSERT(bufs);

	/* copy iovec into WSABUF */
	for(i=0; i<len; i++) {
		bufs[i].buf = (char *)iovp[i].iov_base;
		bufs[i].len = (u_long)iovp[i].iov_len;
	}

	/* read into the buffers */
	i = WSARecv((SOCKET)fd, /* Socket */
			bufs,           /* pointers to the buffers */
			(DWORD)len,     /* number of buffers to process */
			&read,          /* receives number of bytes read */
			&flags,         /* no flags */
			0,              /* no overlapped sockets */
			0);             /* no completion routine */

	/* clean up */
	eso_free(bufs);

	if (i != 0) {
		int theErr = (int)WSAGetLastError();
		if (theErr == WSAEWOULDBLOCK) {
			return EIOStatus::UNAVAILABLE;
		}
		EIOException(__FILE__, __LINE__, "Vector read failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(read, TRUE);
}

int ESocketDispatcher::write(int fd, void* address, int len) {
	/* set up */
	int i = 0;
	DWORD written = 0;
	WSABUF buf;

	/* copy iovec into WSABUF */
	buf.buf = (char *)address;
	buf.len = (u_long)len;

	/* read into the buffers */
	i = WSASend((SOCKET)fd, /* Socket */
			&buf,           /* pointers to the buffers */
			(DWORD)1,       /* number of buffers to process */
			&written,       /* receives number of bytes written */
			0,              /* no flags */
			0,              /* no overlapped sockets */
			0);             /* no completion routine */

	if (i == SOCKET_ERROR) {
		int theErr = (int)WSAGetLastError();
		if (theErr == WSAEWOULDBLOCK) {
			return EIOStatus::UNAVAILABLE;
		}
		throw EIOException(__FILE__, __LINE__, "Write failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(written, FALSE);
}

long ESocketDispatcher::writev(int fd, void* address, int len) {
	/* set up */
	int i = 0;
	DWORD written = 0;
	struct iovec *iovp = (struct iovec *)address;
	WSABUF *bufs = (WSABUF*)eso_malloc(len * sizeof(WSABUF));

	ES_ASSERT(bufs);

	/* copy iovec into WSABUF */
	for(i=0; i<len; i++) {
		bufs[i].buf = (char *)iovp[i].iov_base;
		bufs[i].len = (u_long)iovp[i].iov_len;
	}

	/* read into the buffers */
	i = WSASend((SOCKET)fd, /* Socket */
			bufs,           /* pointers to the buffers */
			(DWORD)len,     /* number of buffers to process */
			&written,       /* receives number of bytes written */
			0,              /* no flags */
			0,              /* no overlapped sockets */
			0);             /* no completion routine */

	/* clean up */
	eso_free(bufs);

	if (i != 0) {
		int theErr = (int)WSAGetLastError();
		if (theErr == WSAEWOULDBLOCK) {
			return EIOStatus::UNAVAILABLE;
		}
		throw EIOException(__FILE__, __LINE__, "Vector write failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(written, FALSE);
}

int ESocketDispatcher::read(int fd, EIOByteBuffer* bb) {
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

int ESocketDispatcher::read(int fd, EA<EIOByteBuffer*>* dsts, int offset, int length) {
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

int ESocketDispatcher::write(int fd, EIOByteBuffer* bb) {
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

int ESocketDispatcher::write(int fd, EA<EIOByteBuffer*>* srcs, int offset, int length) {
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

void ESocketDispatcher::close(int fd) {
	struct linger l;
	int len = sizeof(l);

	if (fd != -1) {
		int result = 0;
		if (getsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&l, &len) == 0) {
			if (l.l_onoff == 0) {
				WSASendDisconnect(fd, NULL);
			}
		}
		result = closesocket(fd);
		if (result == SOCKET_ERROR) {
			throw EIOException(__FILE__, __LINE__, "Socket close failed");
		}
	}
}

void ESocketDispatcher::preClose(int fd) {
	//nothing to do.
}

#else //

int ESocketDispatcher::read(int fd, void* address, int len) {
	return EFileDispatcher::read(fd, address, len);
}

long ESocketDispatcher::readv(int fd, void* address, int len) {
	return EFileDispatcher::readv(fd, address, len);
}

int ESocketDispatcher::write(int fd, void* address, int len) {
	return EFileDispatcher::write(fd, address, len);
}

long ESocketDispatcher::writev(int fd, void* address, int len) {
	return EFileDispatcher::writev(fd, address, len);
}

int ESocketDispatcher::read(int fd, EIOByteBuffer* bb) {
	return EFileDispatcher::read(fd, bb);
}

int ESocketDispatcher::read(int fd, EA<EIOByteBuffer*>* dsts, int offset, int length) {
	return EFileDispatcher::read(fd, dsts, offset, length);
}

int ESocketDispatcher::write(int fd, EIOByteBuffer* bb) {
	return EFileDispatcher::write(fd, bb);
}

int ESocketDispatcher::write(int fd, EA<EIOByteBuffer*>* srcs, int offset, int length) {
	return EFileDispatcher::write(fd, srcs, offset, length);
}

void ESocketDispatcher::close(int fd) {
	return EFileDispatcher::close(fd);
}

void ESocketDispatcher::preClose(int fd) {
	EFileDispatcher::preClose(fd);
}

#endif //!WIN32

} /* namespace nio */
} /* namespace efc */
