/**
 * @file  eso_net.h
 * @brief ES Network library (only support blocking mode)
 */

#ifndef __ESO_NETWORK_H__
#define __ESO_NETWORK_H__

#ifdef WIN32
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#endif

#include "es_comm.h"
#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct ip_addr {
	int family;
#ifdef HAVE_IPV6
	struct in6_addr ip;
#else
	struct in_addr ip;
#endif
};

union sockaddr_union {
	struct sockaddr sa;
	struct sockaddr_in sin;
#ifdef HAVE_IPV6
	struct sockaddr_in6 sin6;
#endif
};

#ifdef HAVE_IPV6
#  define SIZEOF_SOCKADDR(so) ((so).sa.sa_family == AF_INET6 ? \
	sizeof(so.sin6) : sizeof(so.sin))
#else
#  define SIZEOF_SOCKADDR(so) (sizeof(so.sin))
#endif

#define NET_WAIT_READ		0x01
#define NET_WAIT_WRITE		0x02
#define NET_WAIT_CONNECT	0x04

/* Chosen for us by eso_initialize */
#ifdef WIN32

//init socket env
int eso_net_startup(void);

//clearup socket env
int eso_net_cleanup(void);

#endif

/**
 * create socket
 * Returns -1 = failure
 * @see socket
 */
int eso_net_socket(int domain, int type, int protocol);

/**
 * close socket
 */
void eso_net_close(const int fd);

/**
 * client connect
 * Returns 0 = success, -1 = failure, -2 = timeout
 */
int eso_net_connect(int fd,
		            const char *ip, int port,
		            int timeout);

/**
 * bind a name to a socket
 * Returns 0 = success, -1 = failure
 */
int eso_net_bind(const int fd, const char *ip, int port);

/**
 * server listen
 * Returns 0 = success, -1 = failure
 * @mark before listen normal need to do:
 * setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
 * setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
 */
int eso_net_listen(int fd, int backlog);

/**
 * server accept
 * Returns >=0 = new socket handle, -1 = failure, -2 = timeout.
 */
int eso_net_accept(int fd, struct ip_addr *raddr, int *rport);

/**
 * read data
 * Returns number of bytes read, 0 = EOF, -1 = failure, -2 = timeout, -3 = disconnected
 */
int eso_net_read(const int fd, void *buf, int len);

/**
 * write data
 * Returns number of bytes write, -1 = failure, -2 = timeout, -3 = disconnected
 */
int eso_net_write(const int fd, const void *buf, int len);

/**
 * recv data
 * Returns number of bytes read, -1 = failure, -2 = timeout
 */
int eso_net_recvfrom(const int fd, void *buf, int len, int flags, struct ip_addr *raddr, int *rport);

/**
 * send data
 * Returns number of bytes write, -1 = failure, -2 = timeout
 */
int eso_net_sendto(const int fd, const void *buf, int len, int flags, const char *ip, int port);

/**
 * Enable/disable SO_TIMEOUT with the specified timeout.
 */
int eso_net_sotimeout(const int fd, int timeout);

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
int eso_net_wait(const int fd, int flags, int timeout);

/**
 * Get local bound address.
 * Return 0 = success, -1 = failure
 */
int eso_net_localaddr(const int fd, struct ip_addr *laddr, int *lport);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_NETWORK_H__ */
