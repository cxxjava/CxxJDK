/*
 * ENetWrapper.hh
 *
 *  Created on: 2014-1-2
 *      Author: cxxjava@163.com
 */

#ifndef ENETWRAPPER_HH_
#define ENETWRAPPER_HH_

#include "EA.hh"
#include "EIOStatus.hh"
#include "EInetAddress.hh"
#include "ESocketOptions.hh"
#include "EInetSocketAddress.hh"
#include "EIOException.hh"
#include "EProtocolException.hh"
#include "EConnectException.hh"
#include "ENoRouteToHostException.hh"
#include "EBindException.hh"
#include "ESocketException.hh"
#include "EUnresolvedAddressException.hh"
#include "ENoConnectionPendingException.hh"

//@see: openjdk-8/src/share/classes/sun/nio/ch/Net.java

namespace efc {

class ENetWrapper {
public:
	/**
	 * Event masks for the various poll system calls.
	 * They will be set platform dependant in the static initializer below.
	 */
	static const short POLLIN_       = 0x0001;
	static const short POLLOUT_      = 0x0004;
	static const short POLLERR_      = 0x0008;
	static const short POLLHUP_      = 0x0010;
	static const short POLLNVAL_     = 0x0020;
	static const short POLLREMOVE_   = 0x0800;
	static const short POLLCONN_     = POLLOUT_;

public:
	/**
	 *
	 */
	static EInetSocketAddress* checkAddress(EInetSocketAddress* sa) THROWS(EException);
	static int handleSocketError(int errorValue) THROWS5(EProtocolException,
			EConnectException,
			ENoRouteToHostException,
			EBindException,
			ESocketException);
	static int checkConnect(int fd, boolean block, boolean ready) THROWS(EIOException);

	/**
	 * create socket
	 * Returns -1 = failure
	 * @see socket
	 */
	static int socket(int domain, int type, int protocol) THROWS(EIOException);

	/**
	 * close socket
	 */
	static void close(const int fd) THROWS(EIOException);

	/**
	 * client connect
	 * Returns 0 = success, -1 = failure, -2 = timeout
	 */
	static int connect(const int fd,
			EInetAddress* addr, int port,
			int trafficClass) THROWS(EIOException);

	/**
	 * bind a name to a socket
	 * Returns 0 = success, -1 = failure
	 * @mark Only support IPv4!
	 */
	static int bind(const int fd, EInetAddress* addr, int port) THROWS(EIOException);

	/**
	 * server listen
	 * Returns 0 = success, -1 = failure
	 * @mark before listen normal need to do:
	 * setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, sizeof(opt));
	 * setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(opt));
	 */
	static int listen(const int fd, int backlog) THROWS(EIOException);

	/**
	 * Server accept
	 * Returns EIOStatus.
	 */
	static int accept(const int fd, int *newfd, EA<EInetSocketAddress*>* isaa) THROWS(EIOException);

	/**
	 * Socket blocking set
	 */
	static int configureBlocking(const int fd, boolean blocking) THROWS(EIOException);

	/**
	 * Get local address
	 */
	static int localAddress(const int fd, int *laddr, int *lport) THROWS(EIOException);

    /*
     * Map the Java level socket option to the platform specific
     * level and option name.
     */
    static int mapSocketOption(int cmd, int *level, int *optname);

	/**
	 * Set socket opetion
	 */
	static void setOption(const int fd, int optID, const void* optval, int optlen) THROWS(ESocketException);

	/**
	 * Get socket opetion
	 */
	static void getOption(const int fd, int optID, void* optval, int* optlen) THROWS(ESocketException);

	/**
	 * Shutdown socket
	 */
	static void shutdown(const int fd, int how) THROWS(EIOException);

	/**
	 * Convert EInetAddress to sockaddr_union
	 */
	static int InetAddressToSockaddr(EInetAddress* address, int port, union sockaddr_union* so);

	/**
	 * Convert socket handle to EInetSocketAddress
	 */
	static EInetSocketAddress* SocketToInetSocketAddress(int fd);
};

} /* namespace efc */
#endif /* ENETWRAPPER_HH_ */
