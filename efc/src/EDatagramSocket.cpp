/*
 * EDatagramSocket.cpp
 *
 *  Created on: 2016-8-8
 *      Author: cxxjava@163.com
 */

#include "EDatagramSocket.hh"
#include "ESocket.hh"
#include "ESystem.hh"
#include "ENetWrapper.hh"
#include "EOutOfMemoryError.hh"
#include "ERuntimeException.hh"
#include "ESocketTimeoutException.hh"
#include "EInterruptedIOException.hh"
#include "EPortUnreachableException.hh"

//@see: openjdk-8/src/share/classes/java/net/DatagramSocket.java
//@see: openjdk-8/src/share/classes/java/net/DatagramSocketImpl.java
//@see: openjdk-8/src/share/classes/java/net/AbstractPlainDatagramSocketImpl.java
/*
 * On Unix systems we simply delegate to native methods.
 *
 */
//@see: openjdk-8/src/solaris/classes/java/net/PlainDatagramSocketImpl.java
//@see: openjdk-8/src/solaris/native/java/net/PlainDatagramSocketImpl.c
/**
 * This class defines the plain DatagramSocketImpl that is used for all
 * Windows versions lower than Vista. It adds support for IPv6 on
 * these platforms where available.
 *
 * For backward compatibility windows platforms that do not have IPv6
 * support also use this implementation, and fd1 gets set to null
 * during socket creation.
 *
 */
//@see: openjdk-8/src/windows/classes/java/net/TwoStacksPlainDatagramSocketImpl.java
//@see: openjdk-8/src/windows/classes/java/net/TwoStacksPlainDatagramSocketImpl.c
/**
 * `exclusiveBind` is only for WIN32 when using both IPv4 and IPv6.
 *
 * @see: http://www.oracle.com/technetwork/java/javase/7u25-relnotes-1955741.html
 * Change in Networking API Implementation on Windows platforms
 * The implementation of the networking APIs has been changed on Windows to use
 * the SO_EXCLUSIVEADDRUSE socket option by default. This change is necessary to
 * address anomalies that arise when using both IPv4 and IPv6 applications
 * that require to bind to the same port.
 */

namespace efc {

#ifdef __linux__
/**
 * IP_MULTICAST_ALL has been supported since kernel version 2.6.31
 * but we may be building on a machine that is older than that.
 */
#ifndef IP_MULTICAST_ALL
#define IP_MULTICAST_ALL      49
#endif
#endif //!

#ifdef WIN32
#define MAX_BUFFER_LEN 2048
#else //!
/*
 * On 64-bit JDKs we use a much larger stack and heap buffer.
 */
#ifdef AMD64 //@see: _LP64
#define MAX_BUFFER_LEN 65536
#else
#define MAX_BUFFER_LEN 8192
#endif
#endif //!WIN32

#define MAX_PACKET_LEN 65536

EDatagramSocket::~EDatagramSocket() {
	//
}

EDatagramSocket::EDatagramSocket():
		connectState(ST_NOT_CONNECTED),
		connectedPort(-1),
		localPort(0),
		socket(0),
		created(false),
		bound(false),
		closed(false),
		timeout(0),
		connected(false) {
	EInetSocketAddress bindaddr(0);
	init(&bindaddr);
}

EDatagramSocket::EDatagramSocket(EInetSocketAddress* bindaddr):
				connectState(ST_NOT_CONNECTED),
				connectedPort(-1),
				localPort(0),
				socket(0),
				created(false),
				bound(false),
				closed(false),
				timeout(0),
				connected(false) {
	init(bindaddr);
}

EDatagramSocket::EDatagramSocket(const char* hostname, int port):
				connectState(ST_NOT_CONNECTED),
				connectedPort(-1),
				localPort(0),
				socket(0),
				created(false),
				bound(false),
				closed(false),
				timeout(0),
				connected(false) {
	EInetSocketAddress bindaddr(hostname, port);
	init(&bindaddr);
}

EDatagramSocket::EDatagramSocket(int port):
				connectState(ST_NOT_CONNECTED),
				connectedPort(-1),
				localPort(0),
				socket(0),
				created(false),
				bound(false),
				closed(false),
				timeout(0),
				connected(false) {
	EInetSocketAddress bindaddr(port);
	init(&bindaddr);
}

EDatagramSocket::EDatagramSocket(int port, EInetAddress* laddr):
				connectState(ST_NOT_CONNECTED),
				connectedPort(-1),
				localPort(0),
				socket(0),
				created(false),
				bound(false),
				closed(false),
				timeout(0),
				connected(false) {
	EInetSocketAddress bindaddr(laddr, port);
	init(&bindaddr);
}

EDatagramSocket::EDatagramSocket(void* dummy1, void* dummy2):
				connectState(ST_NOT_CONNECTED),
				connectedPort(-1),
				localPort(0),
				socket(0),
				created(false),
				bound(false),
				closed(false),
				timeout(0),
				connected(false) {
	//
}

void EDatagramSocket::init(EInetSocketAddress* bindaddr) {
#ifdef WIN32
	//@see: TwoStacksPlainDatagramSocketImpl.cpp#L57
	fduse = -1;
	lastfd = -1;
#endif

	// create a datagram socket.
	datagramSocketCreate();

	if (bindaddr != null) {
		try {
			bind(bindaddr);
		} catch (...) {
			finally {
				if (!isBound())
					close();
			}
			throw; //!
		} finally {
			if (!isBound())
				close();
		}
	}
}

//@see: void createImpl() throws SocketException
//@see: Java_java_net_PlainDatagramSocketImpl_datagramSocketCreate
void EDatagramSocket::datagramSocketCreate() {
	int arg, t = 1;

	socket = ENetWrapper::socket(AF_INET, SOCK_DGRAM, 0);

#ifdef __APPLE__
	{
		arg = 65507;

		setOption(_SO_SNDBUF, &arg, sizeof(int));
		setOption(_SO_RCVBUF, &arg, sizeof(int));
	}
#endif /* __APPLE__ */

	setOption(_SO_BROADCAST, (char*) &t, sizeof(int));

#if defined(__linux__)
	{
		arg = 0;
		int level = IPPROTO_IP;
		if ((setsockopt(socket, level, IP_MULTICAST_ALL, (char*)&arg, sizeof(arg)) < 0) &&
				(errno != ENOPROTOOPT)) {
			::close(socket);
			throw ESocketException(__FILE__, __LINE__, errno);
		}
	}
#endif

	created = true;
}

//@see: DatagramSocket.java::bind(SocketAddress addr)
void EDatagramSocket::bind(EInetSocketAddress* epoint) {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		if (isBound())
			throw ESocketException(__FILE__, __LINE__, "already bound");
		/*
		if (addr == null)
			addr = new InetSocketAddress(0);
		if (!(addr instanceof InetSocketAddress))
			throw EIllegalArgumentException(__FILE__, __LINE__, "Unsupported address type!");
		InetSocketAddress epoint = (InetSocketAddress) addr;
		*/
		ES_ASSERT(epoint);
		if (epoint->isUnresolved())
			throw ESocketException(__FILE__, __LINE__, "Unresolved address");
		EInetAddress* iaddr = epoint->getAddress();
		int port = epoint->getPort();
		checkAddress(iaddr, "bind");
		/*
		SecurityManager sec = System.getSecurityManager();
		if (sec != null) {
			sec.checkListen(port);
		}
		*/
		try {
			//@see: getImpl().bind(port, iaddr);
			//@see: DefaultDatagramSocketImplFactory.java::createDatagramSocketImpl
			bindImpl(iaddr, port);
		} catch (ESocketException& e) {
			close();
			throw e;
		}

		bound = true;
    }}
}

void EDatagramSocket::bind(const char* hostname, int port) {
	EInetSocketAddress isa(hostname, port);
	this->bind(&isa);
}

#ifdef WIN32
//@see: openjdk-8/src/windows/native/java/net/net_util_md.c#L493
/*
 * Wrapper for bind winsock call - transparent converts an
 * error related to binding to a port that has exclusive access
 * into an error indicating the port is in use (facilitates
 * better error reporting).
 *
 * Should be only called by the wrapper method NET_WinBind
 */
static int
NET_Bind(int s, struct sockaddr *him, int len)
{
    int rv;
    rv = ::bind(s, him, len);

    if (rv == SOCKET_ERROR) {
        /*
         * If bind fails with WSAEACCES it means that a privileged
         * process has done an exclusive bind (NT SP4/2000/XP only).
         */
        if (WSAGetLastError() == WSAEACCES) {
            WSASetLastError(WSAEADDRINUSE);
        }
    }

    return rv;
}

//@see: Java_java_net_TwoStacksPlainDatagramSocketImpl_bind0
void EDatagramSocket::bindImpl(EInetAddress* iaddr, int port) {
	int fd;
	union sockaddr_union lcladdr;
	int lcladdrlen;
	int address;

	fd = this->socket;
	address = iaddr->getAddress();

	lcladdrlen = SIZEOF_SOCKADDR(lcladdr);
	ENetWrapper::InetAddressToSockaddr(iaddr, port, &lcladdr);

	if (NET_Bind(fd, (struct sockaddr *)&lcladdr, lcladdrlen) == -1) {
		if (WSAGetLastError() == WSAEACCES) {
			WSASetLastError(WSAEADDRINUSE);
		}
		throw ESocketException(__FILE__, __LINE__, "Cannot bind");
	}

	if (port == 0) {
		//if (fd == -1) {
		//	/* must be an IPV6 only socket. */
		//	fd = fd1;
		//}
		if (::getsockname(fd, (struct sockaddr *)&lcladdr, &lcladdrlen) == -1) {
			throw ESocketException(__FILE__, __LINE__, "GetSockName");
		}
		port = ntohs((u_short)lcladdr.sin.sin_port);
	}
	this->localPort = port;
}

#else //!

//@see: Java_java_net_PlainDatagramSocketImpl_bind0
void EDatagramSocket::bindImpl(EInetAddress* iaddr, int port) {

	setDefaultScopeID(iaddr, port);

	ENetWrapper::bind(socket, iaddr, port);

	/* initialize the local port */
	if (port == 0) {
		/* Now that we're a connected socket, let's extract the port number
		 * that the system chose for us and store it in the Socket object.
		 */
		int lport;
		ENetWrapper::localAddress(socket, NULL, &lport);
		this->localPort = lport;
	} else {
		this->localPort = port;
	}
}
#endif //!WIN32

void EDatagramSocket::connect(EInetAddress* address, int port) {
	try {
		SYNCHRONIZED(this) {
			connectInternal(address, port);
        }}
	} catch (ESocketException& se) {
		throw ERuntimeException(__FILE__, __LINE__, "connect failed");
	}
}

void EDatagramSocket::connect(EInetSocketAddress* addr) {
	if (addr == null)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Address can't be null");
	//if (!(addr instanceof InetSocketAddress))
	//	throw new IllegalArgumentException(__FILE__, __LINE__, "Unsupported address type");
	//InetSocketAddress epoint = (InetSocketAddress) addr;
	//if (epoint.isUnresolved())
	if (addr->isUnresolved())
		throw ESocketException(__FILE__, __LINE__, "Unresolved address");

	SYNCHRONIZED(this) {
		connectInternal(addr->getAddress(), addr->getPort());
    }}
}

void EDatagramSocket::connect(const char *hostname, int port) {
	if (hostname == null)
		throw EIllegalArgumentException(__FILE__, __LINE__, "connect: The address can't be null");

	EInetSocketAddress isa(hostname, port);
	this->connect(&isa);
}

void EDatagramSocket::connectInternal(EInetAddress* address, int port) {
	if (port < 0 || port > 0xFFFF) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("connect: %d", port).c_str());
	}
	if (address == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "connect: null address");
	}
	checkAddress (address, "connect");
	if (isClosed())
		return;
	/* @see:
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
		if (address.isMulticastAddress()) {
			security.checkMulticast(address);
		} else {
			security.checkConnect(address.getHostAddress(), port);
			security.checkAccept(address.getHostAddress(), port);
		}
	}
	*/

	if (!isBound()) {
		EInetSocketAddress isa(0);
		bind(&isa);
	}

	/* @see:
	// old impls do not support connect/disconnect
	if (oldImpl || (impl instanceof AbstractPlainDatagramSocketImpl &&
		 ((AbstractPlainDatagramSocketImpl)impl).nativeConnectDisabled())) {
		connectState = ST_CONNECTED_NO_IMPL;
	} else {
	*/
#ifdef __APPLE__
		connectState = ST_CONNECTED_NO_IMPL;
#else
		try {
			//@see: getImpl().connect(address, port);
			connectImpl(address, port);

			// socket is now connected by the impl
			connectState = ST_CONNECTED;
		} catch (ESocketException& se) {

			// connection will be emulated by DatagramSocket
			connectState = ST_CONNECTED_NO_IMPL;
		}
#endif
	//}

	connectedAddress = new EInetAddress(*address);
	connectedPort = port;
}

#ifdef WIN32
//@see: Java_java_net_TwoStacksPlainDatagramSocketImpl_connect0
void EDatagramSocket::connectImpl(EInetAddress* address, int port) {
	/* The fdObj'fd */
	int fdc = this->socket;

	if (true /*xp_or_later*/) {
		/* SIO_UDP_CONNRESET fixes a bug introduced in Windows 2000, which
		 * returns connection reset errors on connected UDP sockets (as well
		 * as connected sockets). The solution is to only enable this feature
		 * when the socket is connected
		 */
		DWORD x1, x2; /* ignored result codes */
		int res, t = TRUE;
		res = WSAIoctl(fdc,SIO_UDP_CONNRESET,&t,sizeof(t),&x1,sizeof(x1),&x2,0,0);
	}

	ENetWrapper::connect(fdc, address, port, 0);

	connected = true;
}

#else //!

//@see: Java_java_net_PlainDatagramSocketImpl_connect0
void EDatagramSocket::connectImpl(EInetAddress* address, int port) {

	setDefaultScopeID(address, port);

	ENetWrapper::connect(socket, address, port, 0);

	connected = true;
}
#endif //!WIN32

void EDatagramSocket::disconnect() {
	SYNCHRONIZED (this) {
		if (isClosed())
			return;
		if (connectState == ST_CONNECTED) {
			//@see: impl.disconnect ();
			disconnectImpl();
		}
		connected = false;
		connectedAddress = null;
		connectedPort = -1;
		connectState = ST_NOT_CONNECTED;
    }}
}

#ifdef WIN32
//@see: Java_java_net_TwoStacksPlainDatagramSocketImpl_disconnect0
void EDatagramSocket::disconnectImpl() {
	union sockaddr_union addr;
	int len = SIZEOF_SOCKADDR(addr);

	memset(&addr, 0, len);
	::connect(socket, &addr.sa, len);

	/*
	 * use SIO_UDP_CONNRESET
	 * to disable ICMP port unreachable handling here.
	 */
	if (true /*xp_or_later*/) {
		DWORD x1, x2; /* ignored result codes */
		int t = FALSE;
		WSAIoctl(socket, SIO_UDP_CONNRESET, &t, sizeof(t), &x1, sizeof(x1), &x2, 0, 0);
	}
}

#else //!

//@see: Java_java_net_PlainDatagramSocketImpl_disconnect0
void EDatagramSocket::disconnectImpl() {
	int ret = -1;

#if defined(__linux__) || defined(__bsd__)
	union sockaddr_union addr;
	int len = SIZEOF_SOCKADDR(addr);

	{
		struct sockaddr_in *him4 = (struct sockaddr_in*)&addr;
		him4->sin_family = AF_UNSPEC;
	}
	RESTARTABLE(::connect(socket, (struct sockaddr *)&addr, len), ret);

#ifdef __linux__
	int lport;
	ENetWrapper::localAddress(socket, NULL, &lport);
	if (lport == 0) {
		lport = localPort;
		ENetWrapper::bind(socket, connectedAddress.get(), lport);
	}
#endif
#else //!
	//@see: JVM_Connect(fd, 0, 0);
	RESTARTABLE(::connect(socket, 0, 0), ret);
#endif
}
#endif //!WIN32

boolean EDatagramSocket::isBound() {
	return bound;
}

boolean EDatagramSocket::isConnected() {
	return connectState != ST_NOT_CONNECTED;
}

EInetAddress* EDatagramSocket::getInetAddress() {
	return connectedAddress.get();
}

int EDatagramSocket::getPort() {
	return connectedPort;
}

sp<EInetSocketAddress> EDatagramSocket::getRemoteSocketAddress() {
	if (!isConnected())
		return null;
	return new EInetSocketAddress(getInetAddress(), getPort());
}

sp<EInetSocketAddress> EDatagramSocket::getLocalSocketAddress() {
	if (isClosed())
		return null;
	if (!isBound())
		return null;
	return new EInetSocketAddress(getLocalAddress().get(), getLocalPort());
}

void EDatagramSocket::send(EDatagramPacket* p) {
	SYNCHRONIZED(p) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		checkAddress (p->getAddress(), "send");
		if (connectState == ST_NOT_CONNECTED) {
			/* @see:
			// check the address is ok wiht the security manager on every send.
			SecurityManager security = System.getSecurityManager();

			// The reason you want to synchronize on datagram packet
			// is because you don't want an applet to change the address
			// while you are trying to send the packet for example
			// after the security check but before the send.
			if (security != null) {
				if (p.getAddress().isMulticastAddress()) {
					security.checkMulticast(p.getAddress());
				} else {
					security.checkConnect(p.getAddress().getHostAddress(),
										  p.getPort());
				}
			}
			*/
		} else {
			// we're connected
			EInetAddress* packetAddress = p->getAddress();
			if (packetAddress == null) {
				p->setAddress(connectedAddress.get());
				p->setPort(connectedPort);
			} else if ((!connectedAddress->equals(packetAddress)) ||
					   p->getPort() != connectedPort) {
				throw EIllegalArgumentException(__FILE__, __LINE__, "connected address and packet address differ");
			}
		}

		// Check whether the socket is bound
		if (!isBound()) {
			EInetSocketAddress isa(0);
			bind(&isa);
		}
		// call the  method to send
		//@see: getImpl().send(p);
		sendImpl(p);
    }}
}

#ifdef WIN32

//@see: Java_java_net_TwoStacksPlainDatagramSocketImpl_send
void EDatagramSocket::sendImpl(EDatagramPacket* p) {
	EA<byte>* packetBuffer = p->getData();
	int packetBufferLen = p->getLength();
	char* packetBufferAddr = (char*)((packetBuffer != null) ? packetBuffer->address() : NULL);
	EInetAddress* packetAddress = p->getAddress();
	int packetPort = p->getPort();

    union sockaddr_union rmtaddr;
    struct sockaddr *rmtaddrP=&rmtaddr.sa;
	socklen_t addrlen = 0;

	if (connected) {
		/* arg to JVM_Sendto () null in this case */
		addrlen = 0;
		rmtaddrP = 0;
	} else {
		addrlen = SIZEOF_SOCKADDR(rmtaddr);
		ENetWrapper::InetAddressToSockaddr(packetAddress, packetPort, &rmtaddr);
	}

	/* Note: the buffer needn't be greater than 65,536 (0xFFFF)...
	 * the maximum size of an IP packet. Anything bigger is truncated anyway.
	 */
	if (packetBufferLen > MAX_PACKET_LEN) {
		packetBufferLen = MAX_PACKET_LEN;
	}

	switch (::sendto(socket, packetBufferAddr, packetBufferLen, 0,
					   (struct sockaddr *)rmtaddrP, addrlen)) {
		case -1 : //@see: jvm.h::JVM_IO_ERR == -1
			throw EIOException(__FILE__, __LINE__, "sendto failed");
			break;

		case -2: //@see: jvm.h::JVM_IO_INTR == -2
			throw EInterruptedIOException(__FILE__, __LINE__, "operation interrupted");
			break;
	}
}

#else //!

//@see: Java_java_net_PlainDatagramSocketImpl_send
void EDatagramSocket::sendImpl(EDatagramPacket* p) {
	EA<byte>* packetBuffer = p->getData();
	int packetBufferLen = p->getLength();
	void* packetBufferAddr = (packetBuffer != null) ? packetBuffer->address() : NULL;
	EInetAddress* packetAddress = p->getAddress();
	int packetPort = p->getPort();

    union sockaddr_union rmtaddr;
    struct sockaddr *rmtaddrP=&rmtaddr.sa;
	socklen_t addrlen = 0;

	if (connected) {
		/* arg to NET_Sendto () null in this case */
		addrlen = 0;
		rmtaddrP = 0;
	} else {
		addrlen = SIZEOF_SOCKADDR(rmtaddr);
		ENetWrapper::InetAddressToSockaddr(packetAddress, packetPort, &rmtaddr);
	}

	setDefaultScopeID(packetAddress, packetPort);

	if (packetBufferLen > MAX_BUFFER_LEN) {
		/* When JNI-ifying the JDK's IO routines, we turned
		 * reads and writes of byte arrays of size greater
		 * than 2048 bytes into several operations of size 2048.
		 * This saves a malloc()/memcpy()/free() for big
		 * buffers.  This is OK for file IO and TCP, but that
		 * strategy violates the semantics of a datagram protocol.
		 * (one big send) != (several smaller sends).  So here
		 * we *must* allocate the buffer.  Note it needn't be bigger
		 * than 65,536 (0xFFFF), the max size of an IP packet.
		 * Anything bigger should be truncated anyway.
		 *
		 * We may want to use a smarter allocation scheme at some
		 * point.
		 */
		if (packetBufferLen > MAX_PACKET_LEN) {
			packetBufferLen = MAX_PACKET_LEN;
		}
	}

	long ret;
	RESTARTABLE(::sendto(socket, packetBufferAddr, packetBufferLen, 0, rmtaddrP, addrlen), ret);
	if (ret < 0) {
		switch (ret) {
			case -1 : //@see: jvm.h::JVM_IO_ERR == -1
				if (errno == ECONNREFUSED) {
					throw EPortUnreachableException(__FILE__, __LINE__, "ICMP Port Unreachable");
				} else {
					throw EIOException(__FILE__, __LINE__, "sendto failed", errno);
				}
				break;

			case -2: //@see: jvm.h::JVM_IO_INTR == -2
				//cxxjava: always not reached ?
				throw EInterruptedIOException(__FILE__, __LINE__, "operation interrupted");
				break;
		}
	}
}
#endif //!WIN32

void EDatagramSocket::receive(EDatagramPacket* p) {
	SYNCHRONIZED(p) {
		if (!isBound()) {
			EInetSocketAddress isa(0);
			bind(&isa);
		}

		if (connectState == ST_NOT_CONNECTED) {
			// check the address is ok with the security manager before every recv.
			/* @see:
			SecurityManager security = System.getSecurityManager();
			if (security != null) {
				while(true) {
					String peekAd = null;
					int peekPort = 0;
					// peek at the packet to see who it is from.
					if (!oldImpl) {
						// We can use the new peekData() API
						DatagramPacket peekPacket = new DatagramPacket(new byte[1], 1);
						peekPort = getImpl().peekData(peekPacket);
						peekAd = peekPacket.getAddress().getHostAddress();
					} else {
						InetAddress adr = new InetAddress();
						peekPort = getImpl().peek(adr);
						peekAd = adr.getHostAddress();
					}
					try {
						security.checkAccept(peekAd, peekPort);
						// security check succeeded - so now break
						// and recv the packet.
						break;
					} catch (SecurityException se) {
						// Throw away the offending packet by consuming
						// it in a tmp buffer.
						DatagramPacket tmp = new DatagramPacket(new byte[1], 1);
						getImpl().receive(tmp);

						// silently discard the offending packet
						// and continue: unknown/malicious
						// entities on nets should not make
						// runtime throw security exception and
						// disrupt the applet by sending random
						// datagram packets.
						continue;
					}
				} // end of while
			}
			*/
		}

		if (connectState == ST_CONNECTED_NO_IMPL) {
			// We have to do the filtering the old fashioned way since
			// the native impl doesn't support connect or the connect
			// via the impl failed.
			boolean stop = false;
			while (!stop) {
				int peekAddress = 0;
				int peekPort = -1;
				byte b[1] = {0};
				// peek at the packet to see who it is from.
				//if (!oldImpl)
				{
					// We can use the new peekData() API
					/* @see:
					DatagramPacket peekPacket = new DatagramPacket(new byte[1], 1);
					peekPort = getImpl().peekData(peekPacket);
					*/
					receiveImpl(true, b, 1, &peekAddress, &peekPort);
				}
				//else {
				//	// this api only works for IPv4
				//	peekAddress = new InetAddress();
				//	peekPort = getImpl().peek(peekAddress);
				//}
				if ((connectedAddress->getAddress() != peekAddress) ||
					(connectedPort != peekPort)) {
					// throw the packet away and silently continue
					/* @see:
					 DatagramPacket tmp = new DatagramPacket(new byte[1], 1);
					 getImpl().receive(tmp);
					 */
					receiveImpl(false, b, 1, &peekAddress, &peekPort);
				} else {
					stop = true;
				}
			}
		}

		// If the security check succeeds, or the datagram is
		// connected then receive the packet

		//@see: getImpl().receive(p);
		EA<byte>* packetBuffer = p->getData();
		int packetBufferLen = p->getLength();
		byte* packetBufferAddr = (packetBuffer != null) ? (byte*)packetBuffer->address() : NULL;
		int remoteAddress = 0;
		int remotePort = -1;
		int n = receiveImpl(false, packetBufferAddr, packetBufferLen, &remoteAddress, &remotePort);

		//@see: PlainDatagramSocketImpl.c#L897
		/*
		 * success - fill in received address...
		 *
		 * REMIND: Fill in an int on the packet, and create inetadd
		 * object in Java, as a performance improvement. Also
		 * construct the inetadd object lazily.
		 */

		/*
		 * Check if there is an InetAddress already associated with this
		 * packet. If so we check if it is the same source address. We
		 * can't update any existing InetAddress because it is immutable
		 */
		EInetAddress* packetAddress = p->getAddress();
		if (packetAddress != null) {
			if (packetAddress->getAddress() != remoteAddress) {
				/* force a new InetAddress to be created */
				packetAddress = NULL;
			}
		}
		if (packetAddress == NULL) {
			/* stuff the new Inetaddress in the packet */
			EInetAddress isa(NULL, remoteAddress);
			p->setAddress(&isa);
		}
		p->setPort(remotePort);
		p->setLength(n); //!
    }}
}

#ifdef WIN32
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

//@see: Java_java_net_TwoStacksPlainDatagramSocketImpl_peekData
//@see: Java_java_net_TwoStacksPlainDatagramSocketImpl_receive0
int EDatagramSocket::receiveImpl(boolean isPeek, byte* buf, int len, int* raddr, int* rport) {
	int fduse = socket;
	boolean retry = false;
	llong prevTime = 0;
	int n = 0;

	union sockaddr_union rmtaddr;
	socklen_t addrlen = SIZEOF_SOCKADDR(rmtaddr);

	if (raddr) *raddr = 0;
	if (rport) *rport = -1;

	/*
	 * If this Windows edition supports ICMP port unreachable and if we
	 * are not connected then we need to know if a timeout has been specified
	 * and if so we need to pick up the current time. These are required in
	 * order to implement the semantics of timeout, viz :-
	 * timeout set to t1 but ICMP port unreachable arrives in t2 where
	 * t2 < t1. In this case we must discard the ICMP packets and then
	 * wait for the next packet up to a maximum of t1 minus t2.
	 */
	if (supportPortUnreachable() && !connected && timeout /*&&!ipv6_supported*/) {
		prevTime = ESystem::currentTimeMillis();
	}

	if (timeout > 0) {
		int ret = eso_net_wait(fduse, NET_WAIT_READ, timeout);
		if (ret <= 0) {
			if (ret == 0) {
				throw ESocketTimeoutException(__FILE__, __LINE__, "Receive timed out");
			} else if (ret == -1) {
				throw ESocketException(__FILE__, __LINE__, "Socket closed");
			}
			/*else if (ret == EINTR) {
				throw EInterruptedIOException(__FILE__, __LINE__, "operation interrupted");
			}*/
			return 0;
		}
	}

	do {
		retry = false;

		n = ::recvfrom(fduse, (char*)buf, len, isPeek ? MSG_PEEK : 0, &rmtaddr.sa, &addrlen);
		if (n == -1 /*JVM_IO_ERR*/) {
			if (WSAGetLastError() == WSAECONNRESET) {
				/*
				 * An icmp port unreachable has been received - consume any other
				 * outstanding packets.
				 */
				purgeOutstandingICMP(fduse);

				/*
				 * If connected throw a PortUnreachableException
				 */
				if (connected) {
					throw EPortUnreachableException(__FILE__, __LINE__, "ICMP Port Unreachable");
				}

				/*
				 * If a timeout was specified then we need to adjust it because
				 * we may have used up some of the timeout before the icmp port
				 * unreachable arrived.
				 */
				if (timeout > 0) {
					int ret;
					llong newTime = ESystem::currentTimeMillis();
					timeout -= (int)(newTime - prevTime);
					prevTime = newTime;

					if (timeout <= 0) {
						ret = 0;
					} else {
						ret = eso_net_wait(fduse, NET_WAIT_READ, timeout);
					}

					if (ret <= 0) {
						if (ret == 0) {
							throw ESocketTimeoutException(__FILE__, __LINE__, "Receive timed out");
						} else if (ret == -1/*JVM_IO_ERR*/) {
							throw ESocketException(__FILE__, __LINE__, "Socket closed");
						} /*else if (ret == JVM_IO_INTR) {
							JNU_ThrowByName(env, JNU_JAVAIOPKG "InterruptedIOException",
											"operation interrupted");
						}*/
						//always not reach here.
						return 0;
					}
				}

				/*
				 * An ICMP port unreachable was received but we are
				 * not connected so ignore it.
				 */
				retry = true;
			}
		}
	} while (retry);

	/* truncate the data if the packet's length is too small */
	if (n > len) {
		n = len;
	}
	if (n < 0) {
		int errorCode = WSAGetLastError();
		/* check to see if it's because the buffer was too small */
		if (errorCode == WSAEMSGSIZE) {
			/* it is because the buffer is too small. It's UDP, it's
			 * unreliable, it's all good. discard the rest of the
			 * data..
			 */
			n = len;
		} else {
			/* failure */
			//@see: (*env)->SetIntField(env, packet, dp_lengthID, 0);
		}
	}
	if (n == -1) {
		throw ESocketException(__FILE__, __LINE__, "Socket closed");
	} /*else if (n == -2) {
		JNU_ThrowByName(env, JNU_JAVAIOPKG "InterruptedIOException",
						"operation interrupted");
	}*/ else if (n < 0) {
		throw ESocketException(__FILE__, __LINE__, "Datagram receive failed");
	}

	if (raddr) *raddr = rmtaddr.sin.sin_addr.s_addr;
	if (rport) *rport = ntohs((u_short)rmtaddr.sin.sin_port);

	return n;
}

#else //!

//@see: Java_java_net_PlainDatagramSocketImpl_peekData
//@see: Java_java_net_PlainDatagramSocketImpl_receive0
int EDatagramSocket::receiveImpl(boolean isPeek, byte* buf, int len, int* raddr, int* rport) {
	boolean retry = false;
	int n = 0;

	union sockaddr_union rmtaddr;
	socklen_t addrlen = SIZEOF_SOCKADDR(rmtaddr);

	if (raddr) *raddr = 0;
	if (rport) *rport = -1;

	do {
		retry = false;

		if (timeout > 0) {
			int ret = eso_net_wait(socket, NET_WAIT_READ, timeout);
			if (ret <= 0) {
				if (ret == 0) {
					throw ESocketTimeoutException(__FILE__, __LINE__, "Receive timed out");
				} else if (ret == -1) {
					if (errno == ENOMEM) {
						throw EOutOfMemoryError(__FILE__, __LINE__, "NET_Timeout native heap allocation failed");
#ifdef __linux__
					} else if (errno == EBADF) {
						throw ESocketTimeoutException(__FILE__, __LINE__, "Socket closed");
					} else {
						throw ESocketTimeoutException(__FILE__, __LINE__, "Receive failed");
#else
					} else {
						throw ESocketTimeoutException(__FILE__, __LINE__, "Socket closed");
#endif
					}
				}
				/*else if (ret == EINTR) {
					throw EInterruptedIOException(__FILE__, __LINE__, "operation interrupted");
				}*/
				return 0;
			}
		}

		/* When JNI-ifying the JDK's IO routines, we turned
		 * reads and writes of byte arrays of size greater
		 * than 2048 bytes into several operations of size 2048.
		 * This saves a malloc()/memcpy()/free() for big
		 * buffers.  This is OK for file IO and TCP, but that
		 * strategy violates the semantics of a datagram protocol.
		 * (one big send) != (several smaller sends).  So here
		 * we *must* allocate the buffer.  Note it needn't be bigger
		 * than 65,536 (0xFFFF) the max size of an IP packet,
		 * anything bigger is truncated anyway.
		 *
		 * We may want to use a smarter allocation scheme at some
		 * point.
		 */
		if (len > MAX_PACKET_LEN) {
			len = MAX_PACKET_LEN;
		}

		RESTARTABLE(::recvfrom(socket, buf, len, isPeek ? MSG_PEEK : 0, &rmtaddr.sa, &addrlen), n);
		if (n == -1/*JVM_IO_ERR*/) {
			if (errno == ECONNREFUSED) {
				throw EPortUnreachableException(__FILE__, __LINE__, "ICMP Port Unreachable");
			} else {
				if (errno == EBADF) {
					throw ESocketException(__FILE__, __LINE__, "Socket closed");
				 } else {
					throw ESocketException(__FILE__, __LINE__, "Receive failed", errno);
				 }
			}
		} /* else if (n == JVM_IO_INTR) {
			(*env)->SetIntField(env, packet, dp_offsetID, 0);
			(*env)->SetIntField(env, packet, dp_lengthID, 0);
			JNU_ThrowByName(env, JNU_JAVAIOPKG "InterruptedIOException",
                            "operation interrupted");
		} */

	} while (retry);

	/*
	 * success - fill in received address...
	 *
	 * REMIND: Fill in an int on the packet, and create inetadd
	 * object in Java, as a performance improvement. Also
	 * construct the inetadd object lazily.
	 */
	if (raddr) *raddr = rmtaddr.sin.sin_addr.s_addr;
	if (rport) *rport = ntohs((u_short)rmtaddr.sin.sin_port);

	return n;
}
#endif //!WIN32

sp<EInetAddress> EDatagramSocket::getLocalAddress() {
	if (isClosed())
		return null;
	sp<EInetAddress> in;// = null;
	try {
		//@see: PlainDatagramSocketImpl.c#L1705
		/*
		 * SO_BINDADDR implemented using getsockname
		 */
		int laddr;
		ENetWrapper::localAddress(socket, &laddr, NULL);
		in = new EInetAddress(NULL, laddr);
		/* @see:
		if (in.isAnyLocalAddress()) {
			in = InetAddress.anyLocalAddress();
		}
		SecurityManager s = System.getSecurityManager();
		if (s != null) {
			s.checkConnect(in.getHostAddress(), -1);
		}
		*/
	} catch (EException& e) {
		in = new EInetAddress("0.0.0.0", 0); // "0.0.0.0"
	}
	return in;
}

int EDatagramSocket::getLocalPort() {
	if (isClosed())
		return -1;
	return localPort;
}

void EDatagramSocket::setSoTimeout(int timeout) {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
#ifdef WIN32
		setOption(_SO_TIMEOUT, &timeout, sizeof(int));
#else
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;
		setOption(_SO_TIMEOUT, &tv, sizeof(tv));
#endif
    }}
}

int EDatagramSocket::getSoTimeout() {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		/* @see:
		if (getImpl() == null)
			return 0;
		*/
		int result, len;
#ifdef WIN32
        len = sizeof(int);
		getOption(_SO_TIMEOUT, &result, &len);
#else
		struct timeval tv;
        len = sizeof(tv);
		getOption(_SO_TIMEOUT, &tv, &len);
		result = tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
		return result;
    }}
}

void EDatagramSocket::setSendBufferSize(int size) {
	SYNCHRONIZED(this) {
		if (!(size > 0)) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "negative send size");
		}
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int opt = size;
		setOption(_SO_SNDBUF, &opt, sizeof(int));
    }}
}

int EDatagramSocket::getSendBufferSize() {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int result, len;
        len = sizeof(int);
		getOption(_SO_SNDBUF, &result, &len);
		return result;
    }}
}

void EDatagramSocket::setReceiveBufferSize(int size) {
	SYNCHRONIZED(this) {
		if (size <= 0) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "invalid receive size");
		}
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int opt = size;
		setOption(_SO_RCVBUF, &opt, sizeof(int));
    }}
}

int EDatagramSocket::getReceiveBufferSize() {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int result, len;
        len = sizeof(int);
		getOption(_SO_RCVBUF, &result, &len);
		return result;
    }}
}

void EDatagramSocket::setReuseAddress(boolean on) {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		// Integer instead of Boolean for compatibility with older DatagramSocketImpl
		/* @see:
		if (oldImpl)
			getImpl().setOption(SocketOptions.SO_REUSEADDR, new Integer(on?-1:0));
		else
			getImpl().setOption(SocketOptions.SO_REUSEADDR, Boolean.valueOf(on));
		*/
		int opt = on ? 1 : 0;
		setOption(_SO_REUSEADDR, &opt, sizeof(int));
    }}
}

boolean EDatagramSocket::getReuseAddress() {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int opt, len;
        len = sizeof(int);
		getOption(_SO_REUSEADDR, &opt, &len);
		return opt;
    }}
}

void EDatagramSocket::setBroadcast(boolean on) {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int opt = on ? 1 : 0;
		setOption(_SO_BROADCAST, &opt, sizeof(int));
    }}
}

boolean EDatagramSocket::getBroadcast() {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int opt, len;
        len = sizeof(int);
		getOption(_SO_BROADCAST, &opt, &len);
		return opt;
    }}
}

void EDatagramSocket::setTrafficClass(int tc) {
	SYNCHRONIZED(this) {
		if (tc < 0 || tc > 255)
			throw EIllegalArgumentException(__FILE__, __LINE__, "tc is not in range 0 -- 255");

		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");
		int opt = tc;
		setOption(_IP_TOS, &opt, sizeof(int));
    }}
}

int EDatagramSocket::getTrafficClass() {
	SYNCHRONIZED(this) {
		if (isClosed())
			throw ESocketException(__FILE__, __LINE__, "Socket is closed");

		int opt, len;
        len = sizeof(int);
		getOption(_IP_TOS, &opt, &len);
		return opt;
    }}
}

void EDatagramSocket::close() {
	SYNCBLOCK(&closeLock) {
		if (isClosed())
			return;
		//@see: impl.close();
		ENetWrapper::close(socket);
		closed = true;
    }}
}

boolean EDatagramSocket::isClosed() {
	SYNCBLOCK(&closeLock) {
		return closed;
    }}
}

int EDatagramSocket::getFD() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	return socket;
}

void EDatagramSocket::setOption(int optID, const void* optval, int optlen) {
	ES_ASSERT(optval);

#if 0
	//@see: openjdk-8/src/solaris/native/java/net/PlainDatagramSocketImpl.c

	/*
	 * Setting the multicast interface handled separately
	 */
	// to use default logic @cxxjava
	if (optID == _IP_MULTICAST_IF ||
			optID == _IP_MULTICAST_IF2) {

		setMulticastInterface(env, this, fd, opt, value);
		return;
	}

	/*
	 * Setting the multicast loopback mode handled separately
	 */
	// to use default logic @cxxjava
	if (optID == _IP_MULTICAST_LOOP) {
		setMulticastLoopbackMode(env, this, fd, opt, value);
		return;
	}
#endif

	if (optID == _SO_TIMEOUT) {
		//@see: AbstractPlainDatagramSocketImpl.java#L266
		ES_ASSERT(optlen == sizeof(int));
		int* p = (int*)optval;
		timeout = (*p);
	}

	ENetWrapper::setOption(socket, optID, (char*)optval, optlen);
}

void EDatagramSocket::getOption(int optID, void* optval, int* optlen) {
#if 0
	//@see: openjdk-8/src/solaris/native/java/net/PlainDatagramSocketImpl.c
	/*
	 * Handle IP_MULTICAST_IF separately
	 */
	// to use default logic @cxxjava
	if (opt == java_net_SocketOptions_IP_MULTICAST_IF ||
		opt == java_net_SocketOptions_IP_MULTICAST_IF2) {
		return getMulticastInterface(env, this, fd, opt);

	}

	/*
	 * SO_BINDADDR implemented using getsockname
	 */
	// to use ENetWrapper::localAddress() @cxxjava
	if (opt == java_net_SocketOptions_SO_BINDADDR) {
		/* find out local IP address */
		SOCKADDR him;
		socklen_t len = 0;
		int port;
		jobject iaObj;

		len = SOCKADDR_LEN;

		if (getsockname(fd, (struct sockaddr *)&him, &len) == -1) {
			NET_ThrowByNameWithLastError(env, JNU_JAVANETPKG "SocketException",
						   "Error getting socket name");
			return NULL;
		}
		iaObj = NET_SockaddrToInetAddress(env, (struct sockaddr *)&him, &port);

		return iaObj;
	}
#endif

	ENetWrapper::getOption(socket, optID, optval, optlen);
}

void EDatagramSocket::checkAddress (EInetAddress* addr, const char* op) {
	/* @see:
	if (addr == null) {
		return;
	}
	if (!(addr instanceof Inet4Address || addr instanceof Inet6Address)) {
		throw new IllegalArgumentException(op + ": invalid address type");
	}
	*/
	return;
}

void EDatagramSocket::setDefaultScopeID(EInetAddress* address, int port) {
#ifdef MACOSX
	/*
	static jclass ni_class = NULL;
    static jfieldID ni_defaultIndexID;
    if (ni_class == NULL) {
        jclass c = (*env)->FindClass(env, "java/net/NetworkInterface");
        CHECK_NULL(c);
        c = (*env)->NewGlobalRef(env, c);
        CHECK_NULL(c);
        ni_defaultIndexID = (*env)->GetStaticFieldID(
            env, c, "defaultIndex", "I");
        ni_class = c;
    }
    int defaultIndex;
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)him;
    if (sin6->sin6_family == AF_INET6 && (sin6->sin6_scope_id == 0)) {
        defaultIndex = (*env)->GetStaticIntField(env, ni_class,
                                                 ni_defaultIndexID);
        sin6->sin6_scope_id = defaultIndex;
    }
	*/
#endif
}

} /* namespace efc */
