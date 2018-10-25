/**
 * @file  eso_net.c
 * @brief ES Network library
 */


#include "es_status.h"
#include "eso_net.h"
#include "eso_libc.h"
#include "eso_datetime.h"

#include <errno.h>

#ifdef WIN32

#undef EWOULDBLOCK
#define EWOULDBLOCK	WSAEWOULDBLOCK

#undef EINPROGRESS
#define EINPROGRESS	WSAEINPROGRESS

#undef ENOTSOCK
#define ENOTSOCK WSAENOTSOCK

#undef EOPNOTSUPP
#define EOPNOTSUPP WSAEOPNOTSUPP

#undef EINTR
#define EINTR WSAEINTR

#undef EALREADY
#define EALREADY WSAEALREADY

#undef EISCONN
#define EISCONN WSAEISCONN

#undef EBADF
#define EBADF WSAEBADF

#undef EFAULT
#define EFAULT WSAEFAULT

#undef EINVAL
#define EINVAL WSAEINVAL

#undef ETIMEDOUT
#define ETIMEDOUT WSAETIMEDOUT

#undef ECONNRESET
#define ECONNRESET WSAECONNRESET

#undef EPIPE
#define EPIPE WSAESHUTDOWN  /* good enough? */

#else //!

#include <fcntl.h>
#include <sys/time.h>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>

#define HAVE_POLL

#endif //!WIN32


/**
 * set socket block mode: 1 nonblock, 0 block.
 */
static int net_nonblock(int fd, int nonblock)
{
#ifdef WIN32
	u_long flags = (nonblock != 0) ? 1 : 0;
	if (ioctlsocket(fd, FIONBIO, &flags) != 0) {
		return -1;
	}
	return 0;
#else
	int flags = fcntl(fd, F_GETFL, 0);

	if ((nonblock != 0) && !(flags & O_NONBLOCK))
		return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	else if ((nonblock == 0) && (flags & O_NONBLOCK))
		return fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
	return 0;
#endif
}

/*
 * only for ip address, not support hostname.
 */
static int net_addr2ip(const char *addr, struct ip_addr *ip)
{
	if (eso_strchr(addr, ':') != NULL) {
		/* IPv6 */
#ifdef HAVE_IPV6
		ip->family = AF_INET6;
		if (inet_pton(AF_INET6, addr, &ip->ip) == 0)
			return -1;
#else
		ip->ip.s_addr = 0;
#endif
 	}
 	else {
		/* IPv4 */
		ip->family = AF_INET;
#ifdef WIN32
		ip->ip.s_addr = inet_addr(addr);
		if (ip->ip.s_addr == INADDR_NONE)
			return -1;
#else
		if (inet_aton(addr, (struct in_addr *) &ip->ip) == 0)
			return -1;
#endif
	}

	return 0;
}

/* copy IP to sockaddr */
static void sin_set_ip(union sockaddr_union *so, const struct ip_addr *ip)
{
	if (ip == NULL) {
#ifdef HAVE_IPV6
		so->sin6.sin6_family = AF_INET6;
		so->sin6.sin6_addr = in6addr_any;
#else
		so->sin.sin_family = AF_INET;
		so->sin.sin_addr.s_addr = INADDR_ANY;
#endif
		return;
	}

	so->sin.sin_family = ip->family;
#ifdef HAVE_IPV6
	if (ip->family == AF_INET6)
		eso_memcpy(&so->sin6.sin6_addr, &ip->ip, sizeof(ip->ip));
	else
#endif
		eso_memcpy(&so->sin.sin_addr, &ip->ip, sizeof(ip->ip)/*4*/);
}

static void sin_set_port(union sockaddr_union *so, unsigned int port)
{
#ifdef HAVE_IPV6
	if (so->sin.sin_family == AF_INET6)
		so->sin6.sin6_port = htons((unsigned short) port);
	else
#endif
		so->sin.sin_port = htons((unsigned short) port);
}

static void sin_get_ip(const union sockaddr_union *so, struct ip_addr *ip)
{
	ip->family = so->sin.sin_family;

#ifdef HAVE_IPV6
	if (ip->family == AF_INET6)
		eso_memcpy(&ip->ip, &so->sin6.sin6_addr, sizeof(ip->ip));
	else
#endif
	if (ip->family == AF_INET)
		eso_memcpy(&ip->ip, &so->sin.sin_addr, sizeof(ip->ip)/*4*/);
	else
		eso_memset(&ip->ip, 0, sizeof(ip->ip));
}

static int sin_get_port(union sockaddr_union *so)
{
#ifdef HAVE_IPV6
	if (so->sin.sin_family == AF_INET6)
		return ntohs((u_short)so->sin6.sin6_port);
#endif
	if (so->sin.sin_family == AF_INET)
		return ntohs((u_short)so->sin.sin_port);

	return 0;
}

static int net_set_cloexec(const int fd)
{
#ifdef WIN32
	//TODO: see apr-1.4.6\network_io\win32\sockets.c
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

//==============================================================================

#ifdef WIN32

//init socket env
int eso_net_startup(void)
{
	WSADATA wsaData; 
	return WSAStartup(MAKEWORD(2,2), &wsaData);
}

//clearup socket env
int eso_net_cleanup(void)
{
	return WSACleanup();
}

#endif

/**
 * create socket
 * Returns -1 = failure
 * @see socket
 */
int eso_net_socket(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);
	if (fd < 0) {
		return -1;
	}

	if (net_set_cloexec(fd) != 0)
		return -1;

	return fd;
}

/**
 * close socket
 */
void eso_net_close(const int fd)
{
#ifdef WIN32
	struct linger l;
	int len = sizeof(l);

	if (fd != -1) {
		if (getsockopt(fd, SOL_SOCKET, SO_LINGER, (char *)&l, &len) == 0) {
			if (l.l_onoff == 0) {
				WSASendDisconnect(fd, NULL);
			}
		}
		(void)closesocket(fd);
	}
#else
	(void)close(fd);
#endif
}

/**
 * bind a name to a socket
 * Returns 0 = success, -1 = failure
 */
int eso_net_bind(const int fd, const char *ip, int port)
{
	if (ip || port) {
		union sockaddr_union so;
		struct ip_addr addr;

		if (ip && net_addr2ip(ip, &addr) < 0) {
			return -1;
		}

		eso_memset(&so, 0, sizeof(so));
		sin_set_ip(&so, ip ? &addr : NULL);
		sin_set_port(&so, port);

		/* if using IPv6, bind both on the IPv4 and IPv6 addresses */
#ifdef IPV6_V6ONLY
		if (so.sin.sin_family == AF_INET6) {
			int opt = 0;
			setsockopt(fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&opt, sizeof(opt));
		}
#endif
		/* specify the address/port we want to listen in */
		if (bind(fd, &so.sa, SIZEOF_SOCKADDR(so)) == -1) {
			return -1;
		}
	}

	return 0;
}

/**
 * client connect
 * Returns socket handle, -1 = failure, -2 = timeout
 */
#define BACK_TO_BLOCK(nerr) do { \
	if (timeout > 0) { \
		int old_errno = nerr; \
		net_nonblock(fd, 0); \
		errno = old_errno; \
	} \
} while(0);
int eso_net_connect(int fd,
		            const char *ip, int port,
		            int timeout)
{
	union sockaddr_union so;
	struct ip_addr host_ip;
	int ret;
	
	if (net_addr2ip(ip, &host_ip) < 0) {
		return -1;
	}
	
	if (timeout > 0) {
		/* set socket options */
		net_nonblock(fd, 1);
	}

	/* connect */
	eso_memset(&so, 0, sizeof(so));
	sin_set_ip(&so, &host_ip);
	sin_set_port(&so, port);
	do {
		ret = connect(fd, &so.sa, SIZEOF_SOCKADDR(so));
	} while (ret == -1 && errno == EINTR);

	if (ret == 0) {
		/* back to block mode */
		net_nonblock(fd, 0);
		return 0;
	}
	
	/* we can see EINPROGRESS the first time connect is called on a non-blocking
	 * socket; if called again, we can see EALREADY
	 */
#ifdef WIN32
	/* set the errno here */
	errno = WSAGetLastError();
	if (errno != EINPROGRESS && errno != EWOULDBLOCK
# ifdef __sparc__
		/*
	 	* HACK: it looks like the sparc does not propagate the errno
	 	* correctly in pthread mode.  Ugh.
		*/
	  	&& errno != 0
# endif
	) {
		BACK_TO_BLOCK(errno);
		return -1;
  	}
  	else { //!
  	
#else
	
	if ((ret == -1) && (errno == EINPROGRESS || errno == EALREADY)
					&& (timeout > 0)) { //!

#endif
		ret = eso_net_wait(fd, NET_WAIT_CONNECT, timeout);
		if (ret == 0) {
			BACK_TO_BLOCK(ES_ETIMEDOUT);
			return -2; //timeout!
		}
		else {
			int optlen = sizeof(int);
			getsockopt(fd, SOL_SOCKET, SO_ERROR, (void *)&ret, &optlen);
			if (ret != 0) {
				BACK_TO_BLOCK(errno);
				return -1;
			}
		}
#ifndef WIN32
		if (ret == -1 && errno != EISCONN) {
			BACK_TO_BLOCK(errno);
			return -1;
		}
#endif		

	} //!

	/* back to block mode */
	net_nonblock(fd, 0);

	return 0;
}

/**
 * server listen
 * Returns socket handle, -1 = failure
 */
int eso_net_listen(int fd, int backlog)
{
	if (backlog < 1) {
		backlog = 50;
	}
	return listen(fd, backlog);
}

/**
 * server accept
 * Returns >=0 = new socket handle, -1 = failure, -2 = timeout.
 */
int eso_net_accept(int fd, struct ip_addr *raddr, int *rport)
{
	union sockaddr_union *pso;
	struct sockaddr cliaddr;
	int newfd;
	int addrlen = sizeof(cliaddr);
	
	do {
#ifdef HAVE_ACCEPT4
		newfd = accept4(fd, &cliaddr, &addrlen, SOCK_CLOEXEC);
#else
		newfd = accept(fd, &cliaddr, &addrlen);
#endif
		if (newfd >= 0) {
			break;
		}
#ifdef WIN32
	} while (0);
#else
#ifdef	EPROTO
	} while (errno == EPROTO || errno == ECONNABORTED || errno == EWOULDBLOCK);
#else
	} while (errno == ECONNABORTED || errno == EWOULDBLOCK);
#endif
#endif //!WIN32
	
	if (newfd == -1) {
		if (errno == ETIMEDOUT || errno == EAGAIN || errno == EWOULDBLOCK) {
			return -2; //timeout
		} else {
			return -1; //failure
		}
	}

	pso = (union sockaddr_union*)&cliaddr;
	if (raddr != NULL) sin_get_ip(pso, raddr);
	if (rport != NULL) *rport = sin_get_port(pso);

#ifndef HAVE_ACCEPT4
	if (net_set_cloexec(newfd) != 0)
		return -1;
#endif

	return newfd;
}

/**
 * read data
 * Returns number of bytes read, -1 = failure, -2 = timeout, -3 = disconnected
 */
int eso_net_read(const int fd, void *buf, int len)
{
	int rv;
	
	do {
#ifdef WIN32
		rv = recv(fd, buf, len, 0);
		errno = WSAGetLastError();
#else
		rv = read(fd, buf, len);
#endif
	} while (rv == -1 && errno == EINTR);

	if (rv == -1) {
		if (errno == ETIMEDOUT || errno == EAGAIN || errno == EWOULDBLOCK) {
			return -2; //timeout
		} else if (errno == ECONNRESET || errno == EPIPE) {
			return -3; //disconnected
		} else {
			return -1; //failure
		}
	}
	
	return rv;
}

/**
 * write data
 * Returns number of bytes write, -1 = failure, -2 = timeout, -3 = disconnected
 */
int eso_net_write(const int fd, const void *buf, int len)
{
	int rv;
	
	do {
#ifdef WIN32
		rv = send(fd, buf, len, 0);
		errno = WSAGetLastError();
#else
		rv = write(fd, buf, len);
#endif

	} while (rv == -1 && errno == EINTR);
	
	if (rv == -1) {
		if (errno == ETIMEDOUT || errno == EAGAIN || errno == EWOULDBLOCK) {
			return -2; //timeout
		} else if (errno == ECONNRESET || errno == EPIPE) {
			return -3; //disconnected
		} else {
			return -1; //failure
		}
	}
	
	return rv;
}

/**
 * recv data
 * Returns number of bytes read, -1 = failure, -2 = timeout
 */
int eso_net_recvfrom(const int fd, void *buf, int len, int flags, struct ip_addr *raddr, int *rport)
{
	int rv;

	union sockaddr_union *pso;
	struct sockaddr cliaddr;
	int addrlen = sizeof(cliaddr);

	do {
		rv = recvfrom(fd, buf, len, flags, &cliaddr, &addrlen);
#ifdef WIN32
		errno = WSAGetLastError();
#endif
	} while (rv == -1 && errno == EINTR);

	if ((rv == -1) && (errno == ETIMEDOUT || errno == EAGAIN || errno == EWOULDBLOCK)) {
		return -2; //timeout
	}

	pso = (union sockaddr_union*)&cliaddr;
	if (raddr != NULL) sin_get_ip(pso, raddr);
	if (rport != NULL) *rport = sin_get_port(pso);

	return rv;
}

/**
 * send data
 * Returns number of bytes write, -1 = failure, -2 = timeout
 */
int eso_net_sendto(const int fd, const void *buf, int len, int flags, const char *ip, int port)
{
	int rv;

	union sockaddr_union so;
	struct ip_addr addr;

	if (ip && net_addr2ip(ip, &addr) < 0) {
		return -1;
	}

	eso_memset(&so, 0, sizeof(so));
	sin_set_ip(&so, ip ? &addr : NULL);
	sin_set_port(&so, port);

	do {
		rv = sendto(fd, buf, len, flags, &so.sa, SIZEOF_SOCKADDR(so));
#ifdef WIN32
		errno = WSAGetLastError();
#endif

	} while (rv == -1 && errno == EINTR);

	if (rv == -1 && (errno == ETIMEDOUT || errno == EAGAIN || errno == EWOULDBLOCK)) {
		return -2; //timeout
	}

	return rv;
}

/**
 * Enable/disable SO_TIMEOUT with the specified timeout.
 */
int eso_net_sotimeout(const int fd, int timeout)
{
	int rv;

#ifdef WIN32
	rv = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(int));
#else
	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;
	rv = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

	return rv;
}

/**
 * Wrapper for select/poll with timeout on a single file descriptor.
 *
 * flags can be any combination of
 * NET_WAIT_READ, NET_WAIT_WRITE & NET_WAIT_CONNECT.
 *
 * The function will return when either the socket is ready for one
 * of the specified operation or the timeout expired.
 *
 * If suceess then it returns >0, or 0 if it expired, or -1 if it failed.
 */
int eso_net_wait(const int fd, int flags, int timeout)
{
	es_int64_t curr_time = eso_dt_millis();
	es_int64_t prev_time = curr_time;
	int read_rv;

	for (;;) {
#ifdef HAVE_POLL
		struct pollfd pfd;

		pfd.fd = fd;
		pfd.events = POLLERR;

		if (flags & NET_WAIT_READ)
			pfd.events |= POLLIN;
		if (flags & NET_WAIT_WRITE)
			pfd.events |= POLLOUT;
		if (flags & NET_WAIT_CONNECT)
			pfd.events |= POLLOUT;

		errno = 0;
		read_rv = poll(&pfd, 1, timeout);
#else
		fd_set rd, wr, ex;
		struct timeval t;

		t.tv_sec = timeout / 1000;
		t.tv_usec = (timeout % 1000) * 1000;

		FD_ZERO(&rd);
		FD_ZERO(&wr);
		FD_ZERO(&ex);

		if (flags & NET_WAIT_READ) {
			FD_SET(fd, &rd);
		}
		if (flags & NET_WAIT_WRITE) {
			FD_SET(fd, &wr);
		}
		if (flags & NET_WAIT_CONNECT) {
			FD_SET(fd, &wr);
			FD_SET(fd, &ex);
		}

		errno = 0;
		read_rv = select(fd + 1, &rd, &wr, &ex, (timeout >= 0) ? &t : NULL);
#endif

#ifdef WIN32
		errno = WSAGetLastError();
#endif

		if (read_rv == -1 && errno == EINTR) {
			if (timeout > 0) {
				curr_time = eso_dt_millis();
				timeout -= curr_time - prev_time;
				if (timeout <= 0) {
					return 0;
				}
				prev_time = curr_time;
			}
		} else {
			break;
		}
	}

	return read_rv;
}

int eso_net_localaddr(const int fd, struct ip_addr *laddr, int *lport)
{
	struct sockaddr addr;
	int addrlen = sizeof(addr);
	if (getsockname(fd, &addr, &addrlen) == 0) {
		union sockaddr_union *pso = (union sockaddr_union*)&addr;
		if (laddr != NULL)
			sin_get_ip(pso, laddr);
		if (lport != NULL)
			*lport = sin_get_port(pso);
		return 0;
	}
	return -1;
}
