/*
 * EDatagramDispatcher.cpp
 *
 *  Created on: 2016-8-24
 *      Author: cxxjava@163.com
 */

#include "./EDatagramDispatcher.hh"
#include "./ENIOUtil.hh"
#include "../../inc/EInteger.hh"
#include "../../inc/ENetWrapper.hh"
#include "./ESocketDispatcher.hh"
#include "../../inc/EPortUnreachableException.hh"
#include "../../inc/EIndexOutOfBoundsException.hh"

namespace efc {
namespace nio {

#define MAX_PACKET_LEN 65536

#ifdef WIN32
//@see: openjdk-8/src/windows/native/sun/nio/ch/DatagramDispatcher.c

struct iovec {
    void *iov_base;
    size_t iov_len;
};

/*
 * Return JNI_TRUE if this Windows edition supports ICMP Port Unreachable
 */
__inline static boolean supportPortUnreachable() {
    static boolean initDone;
    static boolean portUnreachableSupported;

    if (!initDone) {
        OSVERSIONINFO ver;
        ver.dwOSVersionInfoSize = sizeof(ver);
        GetVersionEx(&ver);
        if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT && ver.dwMajorVersion >= 5) {
            portUnreachableSupported = true;
        } else {
            portUnreachableSupported = false;
        }
        initDone = true;
    }
    return portUnreachableSupported;
}

/* This function "purges" all outstanding ICMP port unreachable packets
* outstanding on a socket and returns JNI_TRUE if any ICMP messages
* have been purged. The rational for purging is to emulate normal BSD
* behaviour whereby receiving a "connection reset" status resets the
* socket.
*/
//@see: openjdk-8/src/windows/native/java/net/TwoStacksPlainDatagramSocketImpl.c#L329
static boolean purgeOutstandingICMP(int fd)
{
   boolean got_icmp = false;
   char buf[1];
   fd_set tbl;
   struct timeval t = { 0, 0 };
   struct sockaddr_in rmtaddr;
   int addrlen = sizeof(rmtaddr);

   /*
    * A no-op if this OS doesn't support it.
    */
   if (!supportPortUnreachable()) {
       return false;
   }

   /*
    * Peek at the queue to see if there is an ICMP port unreachable. If there
    * is then receive it.
    */
   FD_ZERO(&tbl);
   FD_SET(fd, &tbl);
   while(1) {
       if (::select(/*ignored*/fd+1, &tbl, 0, 0, &t) <= 0) {
           break;
       }
       if (::recvfrom(fd, buf, 1, MSG_PEEK,
                        (struct sockaddr *)&rmtaddr, &addrlen) != -1) {
           break;
       }
       if (WSAGetLastError() != WSAECONNRESET) {
           /* some other error - we don't care here */
           break;
       }

       ::recvfrom(fd, buf, 1, 0,  (struct sockaddr *)&rmtaddr, &addrlen);
       got_icmp = true;
   }

   return got_icmp;
}

int EDatagramDispatcher::read(int fd, void* address, int len) {
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
		if (theErr == WSAECONNRESET) {
			purgeOutstandingICMP(fd);
			throw EPortUnreachableException(__FILE__, __LINE__);
			return EIOStatus::THROWN;
		}
		throw EIOException(__FILE__, __LINE__, "Read failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(read, TRUE);
}

long EDatagramDispatcher::readv(int fd, void* address, int len) {
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
		if (theErr == WSAECONNRESET) {
			purgeOutstandingICMP(fd);
			throw EPortUnreachableException(__FILE__, __LINE__);
			return EIOStatus::THROWN;
		}
		EIOException(__FILE__, __LINE__, "Vector read failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(read, TRUE);
}

int EDatagramDispatcher::write(int fd, void* address, int len) {
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
		if (theErr == WSAECONNRESET) {
			purgeOutstandingICMP(fd);
			throw EPortUnreachableException(__FILE__, __LINE__);
			return EIOStatus::THROWN;
		}
		throw EIOException(__FILE__, __LINE__, "Write failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(written, FALSE);
}

long EDatagramDispatcher::writev(int fd, void* address, int len) {
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
		if (theErr == WSAECONNRESET) {
			purgeOutstandingICMP(fd);
			throw EPortUnreachableException(__FILE__, __LINE__);
			return EIOStatus::THROWN;
		}
		throw EIOException(__FILE__, __LINE__, "Vector write failed");
		return EIOStatus::THROWN;
	}

	return ENIOUtil::convertReturnVal(written, FALSE);
}

int EDatagramDispatcher::recvfrom(int fd, void* address, int len, boolean connected, int* raddr, int* rport) {
	union sockaddr_union rmtaddr;
	socklen_t addrlen = SIZEOF_SOCKADDR(rmtaddr);
	boolean retry = false;
	int n = 0;

	do {
		retry = false;
		n = ::recvfrom((SOCKET)fd, (char *)address, len, 0, &rmtaddr.sa, &addrlen);
		if (n == SOCKET_ERROR) {
			int theErr = WSAGetLastError();
			if (theErr == WSAEMSGSIZE) {
				/* Spec says the rest of the data will be discarded... */
				n = len;
			} else if (theErr == WSAECONNRESET) {
				purgeOutstandingICMP(fd);
				if (connected == false) {
					retry = true;
				} else {
					throw EPortUnreachableException(__FILE__, __LINE__);
					return EIOStatus::THROWN;
				}
			} else if (theErr == WSAEWOULDBLOCK) {
				return EIOStatus::UNAVAILABLE;
			} else {
				return ENetWrapper::handleSocketError(theErr);
			}
		}
	} while (retry == true);

	if (raddr) *raddr = rmtaddr.sin.sin_addr.s_addr;
	if (rport) *rport = ntohs((u_short)rmtaddr.sin.sin_port);

	return n;
}

int EDatagramDispatcher::sendto(int fd, void* address, int len, int raddr, int rport) {
	union sockaddr_union rmtaddr;
	socklen_t addrlen = SIZEOF_SOCKADDR(rmtaddr);
	int n = 0;

	memset(&rmtaddr, 0, sizeof(rmtaddr));
	rmtaddr.sin.sin_family = AF_INET;
	rmtaddr.sin.sin_addr.s_addr = raddr;
	rmtaddr.sin.sin_port = htons((unsigned short) rport);

	n = ::sendto((SOCKET)fd, (const char*)address, len, 0, &rmtaddr.sa, addrlen);
	if (n == SOCKET_ERROR) {
		int theErr = (int)WSAGetLastError();
		if (theErr == WSAEWOULDBLOCK) {
			return EIOStatus::UNAVAILABLE;
		}
		return ENetWrapper::handleSocketError(theErr);
	}

	return n;
}

#else //!

//@see: openjdk-8/src/solaris/native/sun/nio/ch/DatagramDispatcher.c

#include <limits.h>

#ifndef IOV_MAX
#if defined(__FreeBSD__) || defined(__APPLE__)
# define IOV_MAX 1024
#endif
#endif

int EDatagramDispatcher::read(int fd, void* address, int len) {
	int result = ::recv(fd, address, len, 0);
	if (result < 0 && errno == ECONNREFUSED) {
		throw EPortUnreachableException(__FILE__, __LINE__);
		return -2;
	}
	return (int)ENIOUtil::convertReturnVal(result, TRUE);
}

long EDatagramDispatcher::readv(int fd, void* address, int len) {
	ssize_t result = 0;
	struct iovec *iov = (struct iovec *)address;
	struct msghdr m;
	if (len > IOV_MAX) {
		len = IOV_MAX;
	}

	// initialize the message
	memset(&m, 0, sizeof(m));
	m.msg_iov = iov;
	m.msg_iovlen = len;

	result = recvmsg(fd, &m, 0);
	if (result < 0 && errno == ECONNREFUSED) {
		throw EPortUnreachableException(__FILE__, __LINE__);
		return -2;
	}
	return ENIOUtil::convertReturnVal((long)result, TRUE);
}

int EDatagramDispatcher::write(int fd, void* address, int len) {
	int result = ::send(fd, address, len, 0);
	if (result < 0 && errno == ECONNREFUSED) {
		throw EPortUnreachableException(__FILE__, __LINE__);
		return -2;
	}
	return (int)ENIOUtil::convertReturnVal(result, FALSE);
}

long EDatagramDispatcher::writev(int fd, void* address, int len) {
	struct iovec *iov = (struct iovec *)address;
	struct msghdr m;
	ssize_t result = 0;
	if (len > IOV_MAX) {
		len = IOV_MAX;
	}

	// initialize the message
	memset(&m, 0, sizeof(m));
	m.msg_iov = iov;
	m.msg_iovlen = len;

	result = ::sendmsg(fd, &m, 0);
	if (result < 0 && errno == ECONNREFUSED) {
		throw EPortUnreachableException(__FILE__, __LINE__);
		return -2;
	}
	return ENIOUtil::convertReturnVal((long)result, FALSE);
}

//@see: Java_sun_nio_ch_DatagramChannelImpl_receive0
int EDatagramDispatcher::recvfrom(int fd, void* address, int len, boolean connected, int* raddr, int* rport) {
	union sockaddr_union rmtaddr;
	socklen_t addrlen = SIZEOF_SOCKADDR(rmtaddr);
	boolean retry = false;
	int n = 0;

	if (len > MAX_PACKET_LEN) {
		len = MAX_PACKET_LEN;
	}

	do {
		retry = false;
		n = ::recvfrom(fd, address, len, 0, &rmtaddr.sa, &addrlen);
		if (n < 0) {
			if (errno == EWOULDBLOCK) {
				return EIOStatus::UNAVAILABLE;
			}
			if (errno == EINTR) {
				return EIOStatus::INTERRUPTED;
			}
			if (errno == ECONNREFUSED) {
				if (connected == false) {
					retry = true;
				} else {
					throw EPortUnreachableException(__FILE__, __LINE__);
					return EIOStatus::THROWN;
				}
			} else {
				return ENetWrapper::handleSocketError(errno);
			}
		}
	} while (retry == true);

	if (raddr) *raddr = rmtaddr.sin.sin_addr.s_addr;
	if (rport) *rport = ntohs((u_short)rmtaddr.sin.sin_port);

	return n;
}

//@see: Java_sun_nio_ch_DatagramChannelImpl_send0
int EDatagramDispatcher::sendto(int fd, void* address, int len, int raddr, int rport) {
	union sockaddr_union rmtaddr;
	socklen_t addrlen = SIZEOF_SOCKADDR(rmtaddr);
	int n = 0;

	if (len > MAX_PACKET_LEN) {
		len = MAX_PACKET_LEN;
	}

	memset(&rmtaddr, 0, sizeof(rmtaddr));
	rmtaddr.sin.sin_family = AF_INET;
	rmtaddr.sin.sin_addr.s_addr = raddr;
	rmtaddr.sin.sin_port = htons((unsigned short) rport);

	n = ::sendto(fd, address, len, 0, &rmtaddr.sa, addrlen);
	if (n < 0) {
		if (errno == EAGAIN) {
			return EIOStatus::UNAVAILABLE;
		}
		if (errno == EINTR) {
			return EIOStatus::INTERRUPTED;
		}
		if (errno == ECONNREFUSED) {
			throw EPortUnreachableException(__FILE__, __LINE__);
			return EIOStatus::THROWN;
		}
		return ENetWrapper::handleSocketError(errno);
	}

	return n;
}
#endif //!WIN32

int EDatagramDispatcher::read(int fd, EIOByteBuffer* bb) {
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

int EDatagramDispatcher::read(int fd, EA<EIOByteBuffer*>* dsts, int offset, int length) {
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

int EDatagramDispatcher::write(int fd, EIOByteBuffer* bb) {
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

int EDatagramDispatcher::write(int fd, EA<EIOByteBuffer*>* srcs, int offset, int length) {
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

void EDatagramDispatcher::close(int fd) {
	ESocketDispatcher::close(fd);
}

void EDatagramDispatcher::preClose(int fd) {
	ESocketDispatcher::preClose(fd);
}

} /* namespace nio */
} /* namespace efc */
