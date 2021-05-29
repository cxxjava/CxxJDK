/*
 * ENetWrapper.cpp
 *
 *  Created on: 2014-1-2
 *      Author: cxxjava@163.com
 */


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <winsock2.h>
#include <errno.h>

#define SET_BLOCKING 0
#define SET_NONBLOCKING 1

#else //

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>

#define HAVE_POLL

#endif //!WIN32

#include "ENetWrapper.hh"


namespace efc {

#ifndef IPTOS_TOS_MASK
#define IPTOS_TOS_MASK 0x1e
#endif
#ifndef IPTOS_PREC_MASK
#define IPTOS_PREC_MASK 0xe0
#endif

static int net_set_cloexec(const int fd)
{
#ifdef WIN32
	//@see: openjdk-6/jdk/src/windows/native/sun/nio/ch/Net.c
	SetHandleInformation((HANDLE)fd, HANDLE_FLAG_INHERIT, 0);
#else
	int flags;

	if ((flags = fcntl(fd, F_GETFD)) == -1)
		return errno;

	flags |= FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, flags) == -1)
		return errno;
#endif

	return 0;
}

#ifdef WIN32
//@see: openjdk-8/src/windows/native/java/net/net_util_md.c

/*
 * Return the default TOS value
 */
static int NET_GetDefaultTOS() {
    static int default_tos = -1;
    OSVERSIONINFO ver;
    HKEY hKey;
    LONG ret;

    /*
     * If default ToS already determined then return it
     */
    if (default_tos >= 0) {
        return default_tos;
    }

    /*
     * Assume default is "normal service"
     */
    default_tos = 0;

    /*
     * Which OS is this?
     */
    ver.dwOSVersionInfoSize = sizeof(ver);
    GetVersionEx(&ver);

    /*
     * If 2000 or greater then no default ToS in registry
     */
    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        if (ver.dwMajorVersion >= 5) {
            return default_tos;
        }
    }

    /*
     * Query the registry to see if a Default ToS has been set.
     * Different registry entry for NT vs 95/98/ME.
     */
    if (ver.dwPlatformId == VER_PLATFORM_WIN32_NT) {
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           "SYSTEM\\CurrentControlSet\\Services\\Tcp\\Parameters",
                           0, KEY_READ, (PHKEY)&hKey);
    } else {
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           "SYSTEM\\CurrentControlSet\\Services\\VxD\\MSTCP\\Parameters",
                           0, KEY_READ, (PHKEY)&hKey);
    }
    if (ret == ERROR_SUCCESS) {
        DWORD dwLen;
        DWORD dwDefaultTOS;
        ULONG ulType;
        dwLen = sizeof(dwDefaultTOS);

        ret = RegQueryValueEx(hKey, "DefaultTOS",  NULL, &ulType,
                             (LPBYTE)&dwDefaultTOS, &dwLen);
        RegCloseKey(hKey);
        if (ret == ERROR_SUCCESS) {
            default_tos = (int)dwDefaultTOS;
        }
    }
    return default_tos;
}

/*
 * Wrapper for setsockopt dealing with Windows specific issues :-
 *
 * IP_TOS is not supported on some versions of Windows so
 * instead return the default value for the OS.
 */
static int NET_GetSockOpt(int s, int level, int optname, void *optval, int *optlen)
{
    int rv;

    rv = getsockopt(s, level, optname, (char*)optval, optlen);

    /*
     * IPPROTO_IP/IP_TOS is not supported on some Windows
     * editions so return the default type-of-service
     * value.
     */
    if (rv == SOCKET_ERROR) {

        if (WSAGetLastError() == WSAENOPROTOOPT &&
            level == IPPROTO_IP && optname == IP_TOS) {

            int *tos;
            tos = (int *)optval;
            *tos = NET_GetDefaultTOS();

            rv = 0;
        }
    }

    return rv;
}

/*
 * Wrapper for setsockopt dealing with Windows specific issues :-
 *
 * IP_TOS and IP_MULTICAST_LOOP can't be set on some Windows
 * editions.
 *
 * The value for the type-of-service (TOS) needs to be masked
 * to get consistent behaviour with other operating systems.
 */
static int NET_SetSockOpt(int s, int level, int optname, const void *optval, int optlen)
{
    int rv;
    int parg;
    int plen = sizeof(parg);

    if (level == IPPROTO_IP && optname == IP_TOS) {
        int *tos = (int *)optval;
        *tos &= (IPTOS_TOS_MASK | IPTOS_PREC_MASK);
    }

    if (optname == SO_REUSEADDR) {
        /*
         * Do not set SO_REUSEADDE if SO_EXCLUSIVEADDUSE is already set
         */
        rv = NET_GetSockOpt(s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char *)&parg, &plen);
        if (rv == 0 && parg == 1) {
            return rv;
        }
    }

    rv = setsockopt(s, level, optname, (const char*)optval, optlen);

    if (rv == SOCKET_ERROR) {
        /*
         * IP_TOS & IP_MULTICAST_LOOP can't be set on some versions
         * of Windows.
         */
        if ((WSAGetLastError() == WSAENOPROTOOPT) &&
            (level == IPPROTO_IP) &&
            (optname == IP_TOS || optname == IP_MULTICAST_LOOP)) {
            rv = 0;
        }

        /*
         * IP_TOS can't be set on unbound UDP sockets.
         */
        if ((WSAGetLastError() == WSAEINVAL) &&
            (level == IPPROTO_IP) &&
            (optname == IP_TOS)) {
            rv = 0;
        }
    }

    return rv;
}

#else //!

//@see: openjdk-8/src/solaris/native/java/net/net_util_md.c

/*
 * Wrapper for getsockopt system routine - does any necessary
 * pre/post processing to deal with OS specific oddities :-
 *
 * IP_TOS is a no-op with IPv6 sockets as it's setup when
 * the connection is established.
 *
 * On Linux the SO_SNDBUF/SO_RCVBUF values must be post-processed
 * to compensate for an incorrect value returned by the kernel.
 */
static int NET_GetSockOpt(int fd, int level, int opt, void *result, int *len)
{
    int rv;

#ifdef __solaris__
    rv = getsockopt(fd, level, opt, result, len);
#else
    {
        socklen_t socklen = *len;
        rv = getsockopt(fd, level, opt, result, &socklen);
        *len = socklen;
    }
#endif

    if (rv < 0) {
        return rv;
    }

#ifdef __linux__
    /*
     * On Linux SO_SNDBUF/SO_RCVBUF aren't symmetric. This
     * stems from additional socket structures in the send
     * and receive buffers.
     */
    if ((level == SOL_SOCKET) && ((opt == SO_SNDBUF)
                                  || (opt == SO_RCVBUF))) {
        int n = *((int *)result);
        n /= 2;
        *((int *)result) = n;
    }
#endif

/* Workaround for Mac OS treating linger value as
 *  signed integer
 */
#ifdef __APPLE__
    if (level == SOL_SOCKET && opt == SO_LINGER) {
        struct linger* to_cast = (struct linger*)result;
        to_cast->l_linger = (unsigned short)to_cast->l_linger;
    }
#endif
    return rv;
}

/*
 * Wrapper for setsockopt system routine - performs any
 * necessary pre/post processing to deal with OS specific
 * issue :-
 *
 * On Solaris need to limit the suggested value for SO_SNDBUF
 * and SO_RCVBUF to the kernel configured limit
 *
 * For IP_TOS socket option need to mask off bits as this
 * aren't automatically masked by the kernel and results in
 * an error. In addition IP_TOS is a NOOP with IPv6 as it
 * should be setup as connection time.
 */
static int NET_SetSockOpt(int fd, int level, int  opt, const void *arg, int len)
{
//@see: #if defined(_ALLBSD_SOURCE)
#if defined(__bsd__)
#if defined(KIPC_MAXSOCKBUF)
    int mib[3];
    size_t rlen;
#endif

    int *bufsize;

#ifdef __APPLE__
    static int maxsockbuf = -1;
#else
    static long maxsockbuf = -1;
#endif
#endif

    /*
     * IPPROTO/IP_TOS :-
     * 1. IPv6 on Solaris/Mac OS: NOOP and will be set
     *    in flowinfo field when connecting TCP socket,
     *    or sending UDP packet.
     * 2. IPv6 on Linux: By default Linux ignores flowinfo
     *    field so enable IPV6_FLOWINFO_SEND so that flowinfo
     *    will be examined.
     * 3. IPv4: set socket option based on ToS and Precedence
     *    fields (otherwise get invalid argument)
     */
    if (level == IPPROTO_IP && opt == IP_TOS) {
        int *iptos;

        iptos = (int *)arg;
        *iptos &= (IPTOS_TOS_MASK | IPTOS_PREC_MASK);
    }

    /*
     * SOL_SOCKET/{SO_SNDBUF,SO_RCVBUF} - On Solaris we may need to clamp
     * the value when it exceeds the system limit.
     */
#ifdef __solaris__
    if (level == SOL_SOCKET) {
        if (opt == SO_SNDBUF || opt == SO_RCVBUF) {
            int sotype=0, arglen;
            int *bufsize, maxbuf;
            int ret;

            /* Attempt with the original size */
            ret = setsockopt(fd, level, opt, arg, len);
            if ((ret == 0) || (ret == -1 && errno != ENOBUFS))
                return ret;

            /* Exceeded system limit so clamp and retry */

            arglen = sizeof(sotype);
            if (getsockopt(fd, SOL_SOCKET, SO_TYPE, (void *)&sotype,
                           &arglen) < 0) {
                return -1;
            }

            /*
             * We try to get tcp_maxbuf (and udp_max_buf) using
             * an ioctl() that isn't available on all versions of Solaris.
             * If that fails, we use the search algorithm in findMaxBuf()
             */
            if (!init_tcp_max_buf && sotype == SOCK_STREAM) {
                tcp_max_buf = net_getParam("/dev/tcp", "tcp_max_buf");
                if (tcp_max_buf == -1) {
                    tcp_max_buf = findMaxBuf(fd, opt, SOCK_STREAM);
                    if (tcp_max_buf == -1) {
                        return -1;
                    }
                }
                init_tcp_max_buf = 1;
            } else if (!init_udp_max_buf && sotype == SOCK_DGRAM) {
                udp_max_buf = net_getParam("/dev/udp", "udp_max_buf");
                if (udp_max_buf == -1) {
                    udp_max_buf = findMaxBuf(fd, opt, SOCK_DGRAM);
                    if (udp_max_buf == -1) {
                        return -1;
                    }
                }
                init_udp_max_buf = 1;
            }

            maxbuf = (sotype == SOCK_STREAM) ? tcp_max_buf : udp_max_buf;
            bufsize = (int *)arg;
            if (*bufsize > maxbuf) {
                *bufsize = maxbuf;
            }
        }
    }
#endif

#ifdef _AIX
    if (level == SOL_SOCKET) {
        if (opt == SO_SNDBUF || opt == SO_RCVBUF) {
            /*
             * Just try to set the requested size. If it fails we will leave the
             * socket option as is. Setting the buffer size means only a hint in
             * the jse2/java software layer, see javadoc. In the previous
             * solution the buffer has always been truncated to a length of
             * 0x100000 Byte, even if the technical limit has not been reached.
             * This kind of absolute truncation was unexpected in the jck tests.
             */
            int ret = setsockopt(fd, level, opt, arg, len);
            if ((ret == 0) || (ret == -1 && errno == ENOBUFS)) {
                // Accept failure because of insufficient buffer memory resources.
                return 0;
            } else {
                // Deliver all other kinds of errors.
                return ret;
            }
        }
    }
#endif

    /*
     * On Linux the receive buffer is used for both socket
     * structures and the the packet payload. The implication
     * is that if SO_RCVBUF is too small then small packets
     * must be discard.
     */
#ifdef __linux__
    if (level == SOL_SOCKET && opt == SO_RCVBUF) {
        int *bufsize = (int *)arg;
        if (*bufsize < 1024) {
            *bufsize = 1024;
        }
    }
#endif

//@see: #if defined(_ALLBSD_SOURCE)
#if defined(__bsd__)
    /*
     * SOL_SOCKET/{SO_SNDBUF,SO_RCVBUF} - On FreeBSD need to
     * ensure that value is <= kern.ipc.maxsockbuf as otherwise we get
     * an ENOBUFS error.
     */
    if (level == SOL_SOCKET) {
        if (opt == SO_SNDBUF || opt == SO_RCVBUF) {
#ifdef KIPC_MAXSOCKBUF
            if (maxsockbuf == -1) {
               mib[0] = CTL_KERN;
               mib[1] = KERN_IPC;
               mib[2] = KIPC_MAXSOCKBUF;
               rlen = sizeof(maxsockbuf);
               if (sysctl(mib, 3, &maxsockbuf, &rlen, NULL, 0) == -1)
                   maxsockbuf = 1024;

#if 1
               /* XXXBSD: This is a hack to workaround mb_max/mb_max_adj
                  problem.  It should be removed when kern.ipc.maxsockbuf
                  will be real value. */
               maxsockbuf = (maxsockbuf/5)*4;
#endif
           }
#elif defined(__OpenBSD__)
           maxsockbuf = SB_MAX;
#else
           maxsockbuf = 64 * 1024;      /* XXX: NetBSD */
#endif

           bufsize = (int *)arg;
           if (*bufsize > maxsockbuf) {
               *bufsize = maxsockbuf;
           }

           if (opt == SO_RCVBUF && *bufsize < 1024) {
                *bufsize = 1024;
           }

        }
    }
#endif

//@see: #if defined(_ALLBSD_SOURCE) || defined(_AIX)
#if defined(__bsd__) || defined(_AIX)
    /*
     * On Solaris, SO_REUSEADDR will allow multiple datagram
     * sockets to bind to the same port. The network jck tests check
     * for this "feature", so we need to emulate it by turning on
     * SO_REUSEPORT as well for that combination.
     */
    if (level == SOL_SOCKET && opt == SO_REUSEADDR) {
        int sotype;
        socklen_t arglen;

        arglen = sizeof(sotype);
        if (getsockopt(fd, SOL_SOCKET, SO_TYPE, (void *)&sotype, &arglen) < 0) {
            return -1;
        }

        if (sotype == SOCK_DGRAM) {
            setsockopt(fd, level, SO_REUSEPORT, arg, len);
        }
    }
#endif

    return setsockopt(fd, level, opt, arg, len);
}
#endif //!WIN32

//=============================================================================

EInetSocketAddress* ENetWrapper::checkAddress(EInetSocketAddress* sa) {
	//@see: openjdk-6/jdk/src/share/classes/sun/nio/ch/Net.java
	if (sa == null)
		throw EILLEGALARGUMENTEXCEPTION;
	if (sa->isUnresolved())
		throw EUNRESOLVEDADDRESSEXCEPTION; // ## needs arg
	return sa;
}

int ENetWrapper::handleSocketError(int errorValue)
{
#ifdef WIN32
	//@see: openjdk-6/jdk/src/windows/native/java/net/net_util_md.c
	switch (errorValue) {
        case WSAEADDRINUSE:
            throw EBindException(__FILE__, __LINE__, "Address already in use");
		case WSAEADDRNOTAVAIL:
            throw EBindException(__FILE__, __LINE__, "Cannot assign requested address");
		case WSAEAFNOSUPPORT:
			throw ESocketException(__FILE__, __LINE__, "Address family not supported by protocol family");
		case WSAEALREADY:
			throw ESocketException(__FILE__, __LINE__, "Operation already in progress");
		case WSAECONNABORTED:
			throw ESocketException(__FILE__, __LINE__, "Software caused connection abort");
		case WSAECONNREFUSED:
			throw EConnectException(__FILE__, __LINE__, "Connection refused");
		case WSAECONNRESET:
			throw ESocketException(__FILE__, __LINE__, "Connection reset by peer");
		case WSAEDESTADDRREQ:
			throw ESocketException(__FILE__, __LINE__, "Destination address required");
		case WSAEFAULT:
			throw ESocketException(__FILE__, __LINE__, "Bad address");
		case WSAEHOSTDOWN:
			throw ESocketException(__FILE__, __LINE__, "Host is down");
		case WSAEHOSTUNREACH:
			throw ENoRouteToHostException(__FILE__, __LINE__, "No route to host");
		case WSAEINPROGRESS:
			throw ESocketException(__FILE__, __LINE__, "Operation now in progress");
		case WSAEINTR:
			throw ESocketException(__FILE__, __LINE__, "Interrupted function call");
		case WSAEINVAL:
			throw ESocketException(__FILE__, __LINE__, "Invalid argument");
		case WSAEISCONN:
			throw ESocketException(__FILE__, __LINE__, "Socket is already connected");
		case WSAEMFILE:
			throw ESocketException(__FILE__, __LINE__, "Too many open files");
		case WSAEMSGSIZE:
			throw ESocketException(__FILE__, __LINE__, "The message is larger than the maximum supported by the underlying transport");
		case WSAENETDOWN:
			throw ESocketException(__FILE__, __LINE__, "Network is down");
		case WSAENETRESET:
			throw ESocketException(__FILE__, __LINE__, "Network dropped connection on reset");
		case WSAENETUNREACH:
			throw ESocketException(__FILE__, __LINE__, "Network is unreachable");
		case WSAENOBUFS:
			throw ESocketException(__FILE__, __LINE__, "No buffer space available (maximum connections reached?)");
		case WSAENOPROTOOPT:
			throw ESocketException(__FILE__, __LINE__, "Bad protocol option");
		case WSAENOTCONN:
			throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
		case WSAENOTSOCK:
			throw ESocketException(__FILE__, __LINE__, "Socket operation on nonsocket");
		case WSAEOPNOTSUPP:
			throw ESocketException(__FILE__, __LINE__, "Operation not supported");
		case WSAEPFNOSUPPORT:
			throw ESocketException(__FILE__, __LINE__, "Protocol family not supported");
		case WSAEPROCLIM:
			throw ESocketException(__FILE__, __LINE__, "Too many processes");
		case WSAEPROTONOSUPPORT:
			throw ESocketException(__FILE__, __LINE__, "Protocol not supported");
		case WSAEPROTOTYPE:
			throw ESocketException(__FILE__, __LINE__, "Protocol wrong type for socket");
		case WSAESHUTDOWN:
			throw ESocketException(__FILE__, __LINE__, "Cannot send after socket shutdown");
		case WSAESOCKTNOSUPPORT:
			throw ESocketException(__FILE__, __LINE__, "Socket type not supported");
		case WSAETIMEDOUT:
			throw EConnectException(__FILE__, __LINE__, "Connection timed out");
		case WSATYPE_NOT_FOUND:
			throw ESocketException(__FILE__, __LINE__, "Class type not found");
		case WSAEWOULDBLOCK:
			throw ESocketException(__FILE__, __LINE__, "Resource temporarily unavailable");
		case WSAHOST_NOT_FOUND:
			throw ESocketException(__FILE__, __LINE__, "Host not found");
		case WSA_NOT_ENOUGH_MEMORY:
			throw ESocketException(__FILE__, __LINE__, "Insufficient memory available");
		case WSANOTINITIALISED:
			throw ESocketException(__FILE__, __LINE__, "Successful WSAStartup not yet performed");
		case WSANO_DATA:
			throw ESocketException(__FILE__, __LINE__, "Valid name, no data record of requested type");
		case WSANO_RECOVERY:
			throw ESocketException(__FILE__, __LINE__, "This is a nonrecoverable error");
		case WSASYSNOTREADY:
			throw ESocketException(__FILE__, __LINE__, "Network subsystem is unavailable");
		case WSATRY_AGAIN:
			throw ESocketException(__FILE__, __LINE__, "Nonauthoritative host not found");
		case WSAVERNOTSUPPORTED:
			throw ESocketException(__FILE__, __LINE__, "Winsock.dll version out of range");
		case WSAEDISCON:
			throw ESocketException(__FILE__, __LINE__, "Graceful shutdown in progress");
		case WSA_OPERATION_ABORTED:
			throw ESocketException(__FILE__, __LINE__, "Overlapped operation aborted");
	};
	//default
	throw ESOCKETEXCEPTION;
#else
    switch (errorValue) {
        case EINPROGRESS:       /* Non-blocking connect */
            return 0;
#ifdef EPROTO
        case EPROTO:
            throw EPROTOCOLEXCEPTION;
#endif
        case ECONNREFUSED:
        	throw ECONNECTEXCEPTION;
        case ETIMEDOUT:
        	throw ECONNECTEXCEPTION;
        case EHOSTUNREACH:
            throw ENOROUTETOHOSTEXCEPTION;
        case EADDRINUSE:  /* Fall through */
        case EADDRNOTAVAIL:
            throw EBINDEXCEPTION;
        default:
            throw ESOCKETEXCEPTION;
    }
#endif
}

int ENetWrapper::socket(int domain, int type, int protocol) {
	int fd = ::socket(domain, type, protocol);
	if (fd < 0) {
		return handleSocketError(errno);
	}

	if (net_set_cloexec(fd) != 0) {
		return handleSocketError(errno);
	}

	return fd;
}

void ENetWrapper::close(const int fd) {
#ifdef WIN32
    (void)::closesocket(fd);
#else
    (void)::close(fd);
#endif
}

int ENetWrapper::connect(const int fd, EInetAddress* addr, int port, int trafficClass) {
	ES_ASSERT(addr);

#ifdef HAVE_IPV6
	//TODO...
	return EIOStatus::UNAVAILABLE;
#else
	struct sockaddr_in sin;
	eso_memset((char *) &sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = htons((unsigned short) port);
	if (addr->isAnyLocalAddress()) {
		EInetAddress ia = EInetAddress::getLocalHost();
		sin.sin_addr.s_addr = ia.getAddress();
	}
	else {
		sin.sin_addr.s_addr = addr->getAddress();
	}

	//@see: openjdk-6/jdk/src/solaris/native/sun/nio/ch/Net.c
	//@see: openjdk-6/jdk/src/windows/native/sun/nio/ch/Net.c

	int rv = ::connect(fd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
    if (rv != 0) {
#ifdef WIN32
		int err = WSAGetLastError();
        if (err == WSAEINPROGRESS || err == WSAEWOULDBLOCK) {
            return EIOStatus::UNAVAILABLE;
        }
#else
        if (errno == EINPROGRESS) {
            return EIOStatus::UNAVAILABLE;
        } else if (errno == EINTR) {
            return EIOStatus::INTERRUPTED;
        }
#endif
        return handleSocketError(errno);
    }
    return 1;
#endif //!HAVE_IPV6
}

int ENetWrapper::bind(const int fd, EInetAddress* addr, int port) {
	union sockaddr_union so;

	eso_memset(&so, 0, sizeof(so));
	so.sin.sin_family = AF_INET;
	so.sin.sin_addr.s_addr = addr ? addr->getAddress() : INADDR_ANY;
	so.sin.sin_port = htons((unsigned short) port);

	/* specify the address/port we want to listen in */
	if (::bind(fd, &so.sa, SIZEOF_SOCKADDR(so)) == -1) {
		return handleSocketError(errno);
	}

	return 0;
}

int ENetWrapper::listen(const int fd, int backlog) {
	if (backlog < 1) {
		backlog = 50;
	}
	if (::listen(fd, backlog) == -1) {
		return handleSocketError(errno);
	}
	return 0;
}

int ENetWrapper::accept(const int fd, int *newfd, EA<EInetSocketAddress*>* isaa) {
	//@see: openjdk-6-src-b27-26_oct_2012.tar/jdk/src/solaris/native/sun/nio/ch/ServerSocketChannelImpl.c accept0()

	ES_ASSERT(newfd && isaa);

	struct sockaddr cliaddr;
	int addrlen = sizeof(cliaddr);

	/*
	 * accept connection but ignore ECONNABORTED indicating that
	 * a connection was eagerly accepted but was reset before
	 * accept() was called.
	 */
	do {
#ifdef WIN32
		*newfd = ::accept(fd, &cliaddr, &addrlen);
#else
#ifdef HAVE_ACCEPT4
		*newfd = ::accept4(fd, &cliaddr, (socklen_t*)&addrlen, SOCK_CLOEXEC);
#else
		*newfd = ::accept(fd, &cliaddr, (socklen_t*)&addrlen);
#endif
#endif
		if (*newfd >= 0) {
			break;
		}
#ifdef WIN32
	}while (0);

	if (*newfd == INVALID_SOCKET) {
		int theErr = (int)WSAGetLastError();
		if (theErr == WSAEWOULDBLOCK) {
			return EIOStatus::UNAVAILABLE;
		}__FILE__, __LINE__,
		throw EIOException(__FILE__, __LINE__, "Accept failed");
	}
#else
	} while (errno == ECONNABORTED);
	/* ECONNABORTED => restart accept */

	if (*newfd < 0) {
		if (errno == EAGAIN)
			return EIOStatus::UNAVAILABLE;
		if (errno == EINTR)
			return EIOStatus::INTERRUPTED;
		throw EIOException(__FILE__, __LINE__, "Accept failed");
	}
#endif

	union sockaddr_union *pso = (union sockaddr_union*)&cliaddr;
    (*isaa)[0] = new EInetSocketAddress(pso->sin.sin_addr.s_addr, ntohs((u_short)pso->sin.sin_port));

#ifndef HAVE_ACCEPT4
	if (net_set_cloexec(*newfd) != 0)
		throw EIOEXCEPTION;
#endif

	return 1;
}

int ENetWrapper::configureBlocking(const int fd, boolean blocking) {
	//@see1: openjdk-6-src-b27-26_oct_2012.tar/jdk/src/solaris/native/sun/nio/ch/IOUtil.c configureBlocking()
	//@see2: openjdk-6-src-b27-26_oct_2012.tar/jdk/src/windows/native/sun/nio/ch/IOUtil.c configureBlocking()

#ifdef WIN32
	unsigned long argp;
	int result = 0;

	if (blocking == FALSE) {
		argp = SET_NONBLOCKING;
	} else {
		argp = SET_BLOCKING;
		/* Blocking fd cannot be registered with EventSelect */
		WSAEventSelect(fd, NULL, 0);
	}
	result = ioctlsocket(fd, FIONBIO, &argp);
	if (result == SOCKET_ERROR) {
		int error = WSAGetLastError();
		return handleSocketError(error);
	}
#else
	int flags = ::fcntl(fd, F_GETFL);

	if ((blocking == FALSE) && !(flags & O_NONBLOCK))
		return ::fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	else if ((blocking == TRUE) && (flags & O_NONBLOCK))
		return ::fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
#endif
	return 0;
}

int ENetWrapper::localAddress(const int fd, int *laddr, int *lport)
{
	struct sockaddr addr;
	int addrlen = sizeof(addr);
#ifdef WIN32
	if (::getsockname(fd, &addr, &addrlen) == 0) {
#else
	if (::getsockname(fd, &addr, (socklen_t*)&addrlen) == 0) {
#endif
		union sockaddr_union *pso = (union sockaddr_union*)&addr;
		if (laddr != NULL)
			*laddr = pso->sin.sin_addr.s_addr;
		if (lport != NULL)
			*lport = ntohs((u_short)pso->sin.sin_port);
		return 0;
	}
	return handleSocketError(errno);
}

/*
 * Map the Java level socket option to the platform specific
 * level and option name.
 */
int ENetWrapper::mapSocketOption(int cmd, int *level, int *optname) {
    static struct {
        int cmd;
        int level;
        int optname;
    } const opts[] = {
    		{ ESocketOptions::_TCP_NODELAY,           IPPROTO_TCP,    TCP_NODELAY },
			{ ESocketOptions::_SO_OOBINLINE,          SOL_SOCKET,     SO_OOBINLINE },
			{ ESocketOptions::_SO_LINGER,             SOL_SOCKET,     SO_LINGER },
			{ ESocketOptions::_SO_SNDBUF,             SOL_SOCKET,     SO_SNDBUF },
			{ ESocketOptions::_SO_RCVBUF,             SOL_SOCKET,     SO_RCVBUF },
			{ ESocketOptions::_SO_KEEPALIVE,          SOL_SOCKET,     SO_KEEPALIVE },
			{ ESocketOptions::_SO_REUSEADDR,          SOL_SOCKET,     SO_REUSEADDR },
			{ ESocketOptions::_SO_BROADCAST,          SOL_SOCKET,     SO_BROADCAST },
			{ ESocketOptions::_SO_TIMEOUT,            SOL_SOCKET,     SO_RCVTIMEO },
			{ ESocketOptions::_IP_TOS,                IPPROTO_IP,     IP_TOS },
			{ ESocketOptions::_IP_MULTICAST_IF,       IPPROTO_IP,     IP_MULTICAST_IF },
			{ ESocketOptions::_IP_MULTICAST_IF2,      IPPROTO_IP,     IP_MULTICAST_IF },
			{ ESocketOptions::_IP_MULTICAST_LOOP,     IPPROTO_IP,     IP_MULTICAST_LOOP },
			{ ESocketOptions::_IP_ADD_MEMBERSHIP,     IPPROTO_IP,     IP_ADD_MEMBERSHIP },
			{ ESocketOptions::_IP_DROP_MEMBERSHIP,    IPPROTO_IP,     IP_DROP_MEMBERSHIP },
			{ ESocketOptions::_IP_MULTICAST_TTL,      IPPROTO_IP,     IP_MULTICAST_TTL },
    };

    int i;

    /*
     * Map the Java level option to the native level
     */
    for (i=0; i<(int)(sizeof(opts) / sizeof(opts[0])); i++) {
        if (cmd == opts[i].cmd) {
            *level = opts[i].level;
            *optname = opts[i].optname;
            return 0;
        }
    }

    /* not found */
    return -1;
}

void ENetWrapper::setOption(const int fd, int optID, const void* optval, int optlen)
		THROWS(EIOException) {
	int level, optname, rv;

	rv = mapSocketOption(optID, &level, &optname);
	if (rv != 0) {
		throw ESocketException(__FILE__, __LINE__, "Invalid option");
	}

	rv = NET_SetSockOpt(fd, level, optname, (char*)optval, optlen);
	if (rv != 0) {
		throw ESocketException(__FILE__, __LINE__, "Error setting socket option");
	}
}

void ENetWrapper::getOption(const int fd, int optID, void* optval, int* optlen)
		THROWS(EIOException) {
	int level, optname, rv;

	rv = ENetWrapper::mapSocketOption(optID, &level, &optname);
	if (rv != 0) {
		throw ESocketException(__FILE__, __LINE__, "Invalid option");
	}

	rv = NET_GetSockOpt(fd, level, optname, (char*)optval,  optlen);
	if (rv != 0) {
		throw ESocketException(__FILE__, __LINE__, "Error setting socket option");
	}
}

int ENetWrapper::checkConnect(int fd, boolean block, boolean ready) {
#ifdef HAVE_POLL
	int error = 0;
	int n = sizeof(int);
	int result = 0;
	struct pollfd poller;

	poller.revents = 1;
	if (!ready) {
		poller.fd = fd;
		poller.events = POLLOUT;
		poller.revents = 0;
		result = ::poll(&poller, 1, block ? -1 : 0);
		if (result < 0) {
			throw EIOException(__FILE__, __LINE__, "Poll failed");
		}
		if (!block && (result == 0))
			return EIOStatus::UNAVAILABLE;
	}

	if (poller.revents) {
		errno = 0;
		result = ::getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&error, (socklen_t*)&n);
		if (result < 0) {
			handleSocketError(errno);
			return EIOStatus::THROWN;
		} else if (error) {
			handleSocketError(error);
			return EIOStatus::THROWN;
		}
		return 1;
	}
	return 0;
#else
	int optError = 0;
	int lastError = 0;
	int result = 0;
	int retry = 0;
	int n = sizeof(int);
	fd_set wr, ex;
	struct timeval t = {0, 0};

	FD_ZERO(&wr);
	FD_ZERO(&ex);
	FD_SET((u_int)fd, &wr);
	FD_SET((u_int)fd, &ex);

	result = ::select(fd+1, 0, &wr, &ex, block ? NULL : &t);

	/* save last winsock error */
	if (result == SOCKET_ERROR) {
		lastError = WSAGetLastError();
	}

	if (block) { /* must configure socket back to blocking state */
		u_long argp = 0;
		int r = ::ioctlsocket(fd, FIONBIO, &argp);
		if (r == SOCKET_ERROR) {
			handleSocketError(WSAGetLastError());
		}
	}

	if (result == 0) { /* timeout */
		return block ? 0 : EIOStatus::UNAVAILABLE;
	} else {
		if (result == SOCKET_ERROR) { /* select failed */
			handleSocketError(lastError);
			return EIOStatus::THROWN;
		}
	}

	/*
	 * Socket is writable or error occured. On some Windows editions
	 * the socket will appear writable when the connect fails so we
	 * check for error rather than writable.
	 */
	if (!FD_ISSET(fd, &ex)) {
		return 1; /* connection established */
	}

	/*
	 * A getsockopt( SO_ERROR ) may indicate success on NT4 even
	 * though the connection has failed. The workaround is to allow
	 * winsock to be scheduled and this is done via by yielding.
	 * As the yield approach is problematic in heavy load situations
	 * we attempt up to 3 times to get the failure reason.
	 */
	for (retry=0; retry<3; retry++) {
		result = ::getsockopt((SOCKET)fd,
				SOL_SOCKET,
				SO_ERROR,
				(char *)&optError,
				&n);
		if (result == SOCKET_ERROR) {
			int lastError = WSAGetLastError();
			if (lastError == WSAEINPROGRESS) {
				return EIOStatus::UNAVAILABLE;
			}
			throw EIOException(__FILE__, __LINE__, "getsockopt", lastError);
			return EIOStatus::THROWN;
		}
		if (optError) {
			break;
		}
		Sleep(0);
	}

	if (optError != NO_ERROR) {
		handleSocketError(optError);
		return EIOStatus::THROWN;
	}

	return 0;
#endif
}

void ENetWrapper::shutdown(const int fd, int how) {
#ifdef WIN32
	if (::shutdown(fd, how) == SOCKET_ERROR) {
		throw ESocketException(__FILE__, __LINE__, "shutdown", WSAGetLastError());
	}
#else
	if ((::shutdown(fd, how) < 0)  && (errno != ENOTCONN))
		handleSocketError(errno);
#endif
}

int ENetWrapper::InetAddressToSockaddr(EInetAddress* address, int port, union sockaddr_union* so) {
	if (!so) {
		return -1;
	}
	eso_memset(so, 0, sizeof(union sockaddr_union));
	so->sin.sin_family = AF_INET;
	so->sin.sin_addr.s_addr = address ? address->getAddress() : INADDR_ANY;
	so->sin.sin_port = htons((unsigned short) port);
	return 0;
}

//@see: openjdk-8/src/solaris/native/sun/nio/ch/Net.c#L394
EInetSocketAddress* ENetWrapper::SocketToInetSocketAddress(int fd) {
	union sockaddr_union sa;
	socklen_t sa_len = SIZEOF_SOCKADDR(sa);

	if (::getsockname(fd, (struct sockaddr *)&sa, &sa_len) < 0) {
#ifdef __bsd__
		/*
		 * XXXBSD:
		 * ECONNRESET is specific to the BSDs. We can not return an error,
		 * as the calling Java code with raise a java.lang.Error with the expectation
		 * that getsockname() will never fail. According to the Single UNIX Specification,
		 * it shouldn't fail. As such, we just fill in generic Linux-compatible values.
		 */
		if (errno == ECONNRESET) {
			struct sockaddr_in *sin;
			sin = (struct sockaddr_in *) &sa;
			bzero(sin, sizeof(*sin));
			sin->sin_len  = sizeof(struct sockaddr_in);
			sin->sin_family = AF_INET;
			sin->sin_port = htonl(0);
			sin->sin_addr.s_addr = INADDR_ANY;
		} else {
			handleSocketError(errno);
			return null;
		}
#else //!
		handleSocketError(errno);
		return null;
#endif //!__bsd__
	}

	return new EInetSocketAddress(sa.sin.sin_addr.s_addr, ntohs(sa.sin.sin_port));
}

} /* namespace efc */
