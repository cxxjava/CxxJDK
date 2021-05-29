/*
 * EInetAddress.cpp
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EInetAddress.hh"
#include "EIPAddressUtil.hh"
#include "ECharacter.hh"
#include "ESharedPtr.hh"
#include "EEnumeration.hh"
#include "ENetWrapper.hh"
#include "ENetworkInterface.hh"
#include "EUnknownHostException.hh"

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <sys/types.h>
#else
#include <netdb.h>
#include <fcntl.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/time.h>
#endif

/* the initial size of our hostent buffers */
#define HENT_BUF_SIZE 1024
#define BIG_HENT_BUF_SIZE 10240  /* a jumbo-sized one */

namespace efc {

#ifdef WIN32

static boolean isReachable0(int addr, int timeout, int ifaddr, int ttl) {
	int fd;
	struct sockaddr_in him;
	struct sockaddr_in* netif = NULL;
	struct sockaddr_in inf;
	int len = 0;
	WSAEVENT hEvent;
	int connect_rv = -1;
	int sz;

	memset((char *) &him, 0, sizeof(him));

	/**
	 * Socket address
	 */
	him.sin_addr.s_addr = addr;
	him.sin_family = AF_INET;
	len = sizeof(him);

	/**
	 * If a network interface was specified, let's convert its address
	 * as well.
	 */
	if (ifaddr != 0) {
		inf.sin_addr.s_addr = ifaddr;
		inf.sin_family = AF_INET;
		inf.sin_port = 0;
		netif = &inf;
	}

#if 0
    /*
     * Windows implementation of ICMP & RAW sockets is too unreliable for now.
     * Therefore it's best not to try it at all and rely only on TCP
     * We may revisit and enable this code in the future.
     */

    /*
     * Let's try to create a RAW socket to send ICMP packets
     * This usually requires "root" privileges, so it's likely to fail.
     */
    fd = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd != -1) {
      /*
       * It didn't fail, so we can use ICMP_ECHO requests.
       */
        return ping4(env, fd, &him, timeout, netif, ttl);
    }
#endif

    /*
	 * Can't create a raw socket, so let's try a TCP socket
	 */
	fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		/* note: if you run out of fds, you may not be able to load
		 * the exception class, and get a NoClassDefFoundError
		 * instead.
		 */
		return false;
	}
	if (ttl > 0) {
		setsockopt(fd, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
	}

	/*
	 * A network interface was specified, so let's bind to it.
	 */
	if (netif != NULL) {
		if (::bind(fd, (struct sockaddr*)netif, sizeof(struct sockaddr_in)) < 0) {
			::closesocket(fd);
			return false;
		}
	}

	/*
	 * Make the socket non blocking so we can use select/poll.
	 */
	ENetWrapper::configureBlocking(fd, false);

	/* no need to use NET_Connect as non-blocking */
	him.sin_port = htons(7);    /* Echo */
	connect_rv = ::connect(fd, (struct sockaddr *)&him, len);

	/**
	 * connection established or refused immediately, either way it means
	 * we were able to reach the host!
	 */
	if (connect_rv == 0 || WSAGetLastError() == WSAECONNREFUSED) {
		::closesocket(fd);
		return true;
	} else {
		int optlen;

		switch (WSAGetLastError()) {
		case WSAEHOSTUNREACH: /* Host Unreachable */
		case WSAENETUNREACH: /* Network Unreachable */
		case WSAENETDOWN: /* Network is down */
		case WSAEPFNOSUPPORT: /* Protocol Family unsupported */
			::closesocket(fd);
			return false;
		}

		if (WSAGetLastError() != WSAEWOULDBLOCK) {
			::closesocket(fd);
			return false;
		}

		timeout = eso_net_wait(fd, NET_WAIT_CONNECT, timeout);

		/* has connection been established */

		if (timeout > 0) {
			optlen = sizeof(connect_rv);
			if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*) &connect_rv,
					&optlen) < 0) {
				connect_rv = WSAGetLastError();
			}

			if (connect_rv == 0 || connect_rv == WSAECONNREFUSED) {
				::closesocket(fd);
				return true;
			}
		}
	}
	::closesocket(fd);
	return false;
}

#else //!

#define SET_NONBLOCKING(fd) {           \
        int flags = fcntl(fd, F_GETFL); \
        flags |= O_NONBLOCK;            \
        fcntl(fd, F_SETFL, flags);      \
}

static unsigned short
in_cksum(unsigned short *addr, int len) {
	int nleft = len;
	int sum = 0;
	unsigned short *w = addr;
	unsigned short answer = 0;
	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&answer) = *(unsigned char *) w;
		sum += answer;
	}

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);
	answer = ~sum;
	return (answer);
}

/**
 * ping implementation.
 * Send a ICMP_ECHO_REQUEST packet every second until either the timeout
 * expires or a answer is received.
 * Returns true is an ECHO_REPLY is received, otherwise, false.
 */
static boolean ping4(int fd, struct sockaddr_in* him, int timeout,
      struct sockaddr_in* netif, int ttl) {
	int size;
	int n, hlen1, icmplen;
	socklen_t len;
	char sendbuf[1500];
	char recvbuf[1500];
	struct icmp *icmp;
	struct ip *ip;
	struct sockaddr_in sa_recv;
	unsigned short pid;
	int tmout2, seq = 1;
	struct timeval tv;
	size_t plen;

	/* icmp_id is a 16 bit data type, therefore down cast the pid */
	pid = (unsigned short)getpid();
	size = 60 * 1024;
	::setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));

	/*
	 * sets the ttl (max number of hops)
	 */
	if (ttl > 0) {
		::setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	}

	/*
	 * a specific interface was specified, so let's bind the socket
	 * to that interface to ensure the requests are sent only through it.
	 */
	if (netif != NULL) {
		if (::bind(fd, (struct sockaddr*) netif, sizeof(struct sockaddr_in)) < 0) {
			return false;
		}
	}

	/*
	 * Make the socket non blocking so we can use select
	 */
	SET_NONBLOCKING(fd);

	do {
		/*
		* create the ICMP request
		*/
		icmp = (struct icmp *) sendbuf;
		icmp->icmp_type = ICMP_ECHO;
		icmp->icmp_code = 0;
		icmp->icmp_id = htons(pid);
		icmp->icmp_seq = htons(seq);
		seq++;
		gettimeofday(&tv, NULL);
		memcpy(icmp->icmp_data, &tv, sizeof(tv));
		plen = ICMP_ADVLENMIN + sizeof(tv);
		icmp->icmp_cksum = 0;
		icmp->icmp_cksum = in_cksum((u_short *)icmp, plen);

		/*
		 * send it
		 */
		n = ::sendto(fd, sendbuf, plen, 0, (struct sockaddr *) him,
				sizeof(struct sockaddr));
		if (n < 0 && errno != EINPROGRESS) {
#ifdef __linux__
			if (errno != EINVAL && errno != EHOSTUNREACH)
			/*
			 * On some Linux versions, when a socket is bound to the loopback
			 * interface, sendto will fail and errno will be set to
			 * EINVAL or EHOSTUNREACH. When that happens, don't throw an
			 * exception, just return false.
			 */
#endif /*__linux__ */
			return false;
		}

		tmout2 = timeout > 1000 ? 1000 : timeout;
		int ret = eso_net_wait(fd, NET_WAIT_READ, tmout2);
		if (ret > 0) {
			len = sizeof(sa_recv);
			n = ::recvfrom(fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&sa_recv, &len);
			ip = (struct ip*) recvbuf;
			hlen1 = (ip->ip_hl) << 2;
			icmp = (struct icmp *) (recvbuf + hlen1);
			icmplen = n - hlen1;
			/*
			 * We did receive something, but is it what we were expecting?
			 * I.E.: A ICMP_ECHOREPLY packet with the proper PID.
			 */
			if (icmplen >= 8 && icmp->icmp_type == ICMP_ECHOREPLY
					&& (ntohs(icmp->icmp_id) == pid)) {
				if ((him->sin_addr.s_addr == sa_recv.sin_addr.s_addr)) {
					return true;
				}

				if (him->sin_addr.s_addr == 0) {
					return true;
				}
			}
		}

		timeout -= 1000;
	} while (timeout >0);

    return false;
}

static boolean isReachable0(int addr, int timeout, int ifaddr, int ttl) {
	int fd;
	struct sockaddr_in him;
	struct sockaddr_in* netif = NULL;
	struct sockaddr_in inf;
	int len = 0;
	int connect_rv = -1;

	memset((char *) &him, 0, sizeof(him));
	memset((char *) &inf, 0, sizeof(inf));
	him.sin_addr.s_addr = addr;
	him.sin_family = AF_INET;
	len = sizeof(him);
	/*
	 * If a network interface was specified, let's create the address
	 * for it.
	 */
	if (ifaddr != 0) {
		inf.sin_addr.s_addr = ifaddr;
		inf.sin_family = AF_INET;
		inf.sin_port = 0;
		netif = &inf;
	}

	/*
	 * Let's try to create a RAW socket to send ICMP packets
	 * This usually requires "root" privileges, so it's likely to fail.
	 */
	fd = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (fd != -1) {
		/*
		 * It didn't fail, so we can use ICMP_ECHO requests.
		 */
		boolean ret = ping4(fd, &him, timeout, netif, ttl);
		::close(fd);
		return ret;
	}

	/*
	 * Can't create a raw socket, so let's try a TCP socket
	 */
	fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		/* note: if you run out of fds, you may not be able to load
		 * the exception class, and get a NoClassDefFoundError
		 * instead.
		 */
		throw false;
	}
	if (ttl > 0) {
		::setsockopt(fd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	}

	/*
	 * A network interface was specified, so let's bind to it.
	 */
	if (netif != NULL) {
		if (::bind(fd, (struct sockaddr*) netif, sizeof(struct sockaddr_in)) < 0) {
			::close(fd);
			return false;
		}
	}

	/*
	 * Make the socket non blocking so we can use select/poll.
	 */
	SET_NONBLOCKING(fd);

	/* no need to use NET_Connect as non-blocking */
	him.sin_port = htons(7);    /* Echo */
	connect_rv = ::connect(fd, (struct sockaddr *)&him, len);

	/**
	 * connection established or refused immediately, either way it means
	 * we were able to reach the host!
	 */
	if (connect_rv == 0 || errno == ECONNREFUSED) {
		::close(fd);
		return true;
	} else {
		int optlen;

		switch (errno) {
		case ENETUNREACH: /* Network Unreachable */
		case EAFNOSUPPORT: /* Address Family not supported */
		case EADDRNOTAVAIL: /* address is not available on  the  remote machine */
#ifdef __linux__
		case EINVAL:
		case EHOSTUNREACH:
		  /*
		   * On some Linux versions, when a socket is bound to the loopback
		   * interface, connect will fail and errno will be set to EINVAL
		   * or EHOSTUNREACH.  When that happens, don't throw an exception,
		   * just return false.
		   */
#endif /* __linux__ */
		  ::close(fd);
		  return false;
		}

		if (errno != EINPROGRESS) {
		  ::close(fd);
		  return false;
		}

		int ret = eso_net_wait(fd, NET_WAIT_CONNECT, timeout);
		if (ret > 0) {
			/* has connection been established? */
			optlen = sizeof(connect_rv);
			if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, (void*) &connect_rv,
					(socklen_t*)&optlen) < 0) {
				connect_rv = errno;
			}
			if (connect_rv == 0 || connect_rv == ECONNREFUSED) {
				::close(fd);
				return true;
			}
		}
		::close(fd);
		return false;
	}
}

#endif

//=============================================================================

EInetAddress::~EInetAddress() {
 //
}

EInetAddress::EInetAddress()
{
	eso_memset(_address, 0, sizeof(_address));
}

EInetAddress::EInetAddress(const char* hostName, byte addr[4])
{
	this->_hostName = new EString(hostName);
	
	setAddress(addr);
}

EInetAddress::EInetAddress(const char* hostName, int address)
{
    if (hostName) {
        _hostName = new EString(hostName);
    }
	
	setAddress(address);
}

void EInetAddress::setAddress(byte addr[4])
{
	if (addr != null) {
		eso_memcpy(_address, addr, sizeof(_address));
	}
	else {
		eso_memset(_address, 0, sizeof(_address));
	}
}

void EInetAddress::setAddress(int address)
{
    eso_llong2array(address, (es_byte_t*)_address, 4);
}

boolean EInetAddress::isMulticastAddress()
{
	return ((_address[0] & 0xf0) == 0xe0);
}

boolean EInetAddress::isAnyLocalAddress()
{
	int addr;
	eso_memcpy(&addr, _address, sizeof(int));
	return addr == 0;
}

boolean EInetAddress::isLoopbackAddress()
{
	/* 127.x.x.x */
 	return _address[0] == 127;
}

boolean EInetAddress::isLinkLocalAddress()
{
	// link-local unicast in IPv4 (169.254.0.0/16)
	// defined in "Documenting Special Use IPv4 Address Blocks
	// that have been Registered with IANA" by Bill Manning
	// draft-manning-dsua-06.txt
	return ((_address[0] & 0xFF) == 169) 
	    && ((_address[1] & 0xFF) == 254);
}

boolean EInetAddress::isSiteLocalAddress()
{
	// refer to RFC 1918
	// 10/8 prefix
	// 172.16/12 prefix
	// 192.168/16 prefix
	return ((_address[0] & 0xFF) == 10)
	    || (((_address[0] & 0xFF) == 172) 
		&& ((_address[1] & 0xF0) == 16))
	    || (((_address[0] & 0xFF) == 192) 
		&& ((_address[1] & 0xFF) == 168));
}

boolean EInetAddress::isMCGlobal()
{
	// 224.0.1.0 to 238.255.255.255
 	return ((_address[0] & 0xff) >= 224 && (_address[0] & 0xff) <= 238 ) &&
 	    !((_address[0] & 0xff) == 224 && _address[1] == 0 &&
 	      _address[2] == 0);
}

boolean EInetAddress::isMCNodeLocal()
{
	// unless ttl == 0
	return false;
}

boolean EInetAddress::isMCLinkLocal()
{
	// 224.0.0/24 prefix and ttl == 1
	return ((_address[0] & 0xFF) == 224) 
	    && ((_address[1] & 0xFF) == 0)
	    && ((_address[2] & 0xFF) == 0);
}

boolean EInetAddress::isMCSiteLocal()
{
	// 239.255/16 prefix or ttl < 32
	return ((_address[0] & 0xFF) == 239) 
	    && ((_address[1] & 0xFF) == 255);
}

boolean EInetAddress::isMCOrgLocal()
{
	// 239.192 - 239.195
	return ((_address[0] & 0xFF) == 239) 
	    && ((_address[1] & 0xFF) >= 192)
	    && ((_address[1] & 0xFF) <= 195);
}

const char* EInetAddress::getHostName() THROWS(EUnknownHostException)
{
	if (_hostName == null) {
#ifdef WIN32
		//@see: openjdk-8/src/windows/native/java/net/Inet4AddressImpl.c
		struct hostent *hp;
		int addr = getAddress();

		hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
		if (hp == NULL || hp->h_name == NULL) {
			//throw EUnknownHostException(__FILE__, __LINE__);
			_hostName = new EString(getHostAddress());
		}
		else {
			_hostName = new EString(hp->h_name);
		}
#else
		//@see: openjdk-8/src/solaris/native/java/net/Inet4AddressImpl.c
		char host[NI_MAXHOST+1];
		struct sockaddr_in him4;
		struct sockaddr *sa;

		eso_memset((void *) &him4, 0, sizeof(him4));
		him4.sin_addr.s_addr = getAddress();
		him4.sin_family = AF_INET;
		sa = (struct sockaddr *) &him4;

		int len = sizeof(him4);
		int error = getnameinfo(sa, len, host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
		if (!error) {
			_hostName = new EString(host);
		}
		else {
			_hostName = new EString(getHostAddress());
		}
#endif
	}

	return _hostName->c_str();
}

const char* EInetAddress::getCanonicalHostName() THROWS(EUnknownHostException)
{
	if (_canonicalHostName == null) {
#ifdef WIN32
		struct hostent *hp;
		int addr = getAddress();

		hp = gethostbyaddr((char *) &addr, sizeof(addr), AF_INET);
		if (hp == NULL || !hp->h_aliases || !*(hp->h_aliases)) {
			//throw EUnknownHostException(__FILE__, __LINE__);
			_canonicalHostName = new EString(getHostAddress());
		}
		else {
			_canonicalHostName = new EString(*(hp->h_aliases));
		}
#else
		//@see: openjdk-8/src/solaris/native/java/net/Inet4AddressImpl.c
		char host[NI_MAXHOST+1];
		struct sockaddr_in him4;
		struct sockaddr *sa;

		eso_memset((void *) &him4, 0, sizeof(him4));
		him4.sin_addr.s_addr = getAddress();
		him4.sin_family = AF_INET;
		sa = (struct sockaddr *) &him4;

		int len = sizeof(him4);
		int error = getnameinfo(sa, len, host, NI_MAXHOST, NULL, 0, NI_NAMEREQD);
		if (!error) {
			_canonicalHostName = new EString(host);
		}
		else {
			_canonicalHostName = new EString(getHostAddress());
		}
#endif
	}

	return _canonicalHostName->c_str();
}

int EInetAddress::getAddress()
{
	return (int)eso_array2llong((es_byte_t *)_address, 4);
}

EString EInetAddress::getHostAddress()
{
	return EString::formatOf("%d.%d.%d.%d",
	                      (_address[0] & 0xff),
	                      (_address[1] & 0xff),
	                      (_address[2] & 0xff),
	                      (_address[3] & 0xff));
}

boolean EInetAddress::isReachable(int timeout) {
	return isReachable(null, 0, timeout);
}

boolean EInetAddress::isReachable(ENetworkInterface* netif, int ttl,
							   int timeout) {
	if (ttl < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "ttl can't be negative");
	if (timeout < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "timeout can't be negative");

	//@see: return impl.isReachable(this, timeout, netif, ttl);
	{
		//@see: openjdk-8/src/share/classes/java/net/Inet4AddressImpl.java
		int ifaddr = 0;
		if (netif != null) {
			/*
			 * Let's make sure we use an address of the proper family
			 */
			sp<EEnumeration<EInetAddress*> > it = netif->getInetAddresses();
			EInetAddress* inetaddr = null;
			while (!instanceof<EInetAddress>(inetaddr) && it->hasMoreElements())
				inetaddr = it->nextElement();
			if (inetaddr)
				ifaddr = inetaddr->getAddress();
		}
		//@see: return isReachable0(addr.getAddress(), timeout, ifaddr, ttl);
		return isReachable0(getAddress(), timeout, ifaddr, ttl);
	}
}

int EInetAddress::hashCode()
{
	return getAddress();
}

boolean EInetAddress::equals(EInetAddress *obj)
{
	return (obj != null) && (eso_memcmp(obj->_address, _address, sizeof(_address)) == 0);
}

boolean EInetAddress::equals(EObject* obj) {
	EInetAddress* that = dynamic_cast<EInetAddress*>(obj);
	return (that != null) && (eso_memcmp(that->_address, _address, sizeof(_address)) == 0);
}

EString EInetAddress::toString()
{
    return EString::formatOf("%s/%s", ((_hostName != null) ? _hostName->c_str() : "null"), getHostAddress().c_str());
}

EInetAddress EInetAddress::anyLocalAddress()
{
	return EInetAddress("0.0.0.0", 0); // {0x00,0x00,0x00,0x00}
}

EInetAddress EInetAddress::loopbackAddress()
{
	byte addr[4] = {0x7f,0x00,0x00,0x01};
	return EInetAddress("localhost", addr); // {0x7f,0x00,0x00,0x01}
}

EInetAddress EInetAddress::getByAddress(byte addr[4]) THROWS(EUnknownHostException)
{
	return EInetAddress::getByAddress(null, addr);
}

EInetAddress EInetAddress::getByAddress(const char* host, byte addr[4]) THROWS(EUnknownHostException)
{
	EString hostStr = host;
	if (hostStr.startsWith("[") && hostStr.endsWith("]")) {
		hostStr.erase(hostStr.length() - 1, 1);
		hostStr.erase(0, 1);
	}
	if (addr != null) {
		return EInetAddress(hostStr.c_str(), addr);
	}
	throw EUnknownHostException(__FILE__, __LINE__, "addr is of illegal length");
}

EInetAddress EInetAddress::getByName(const char* host) THROWS(EUnknownHostException)
{
	EArray<EInetAddress*> arrs =  EInetAddress::getAllByName(host);
	return * arrs[0];
}

EArray<EInetAddress*> EInetAddress::getAllByName(const char* host) THROWS(EUnknownHostException)
{
	EArray<EInetAddress*> array;

	EString hostStr = host;
	if (hostStr.length() == 0) {
		byte addr[4] = { 0x7f, 0x00, 0x00, 0x01 };
		EInetAddress* inetaddr = new EInetAddress("localhost", addr);
		array.add(inetaddr);
		return array;
	}

	boolean ipv6Expected = false;
	if (hostStr.startsWith("[")) {
		if (hostStr.length() > 2 && hostStr.endsWith("]")) {
			// This is supposed to be an IPv6 litteral
			hostStr.erase(hostStr.length() - 1, 1);
			hostStr.erase(0, 1);
			ipv6Expected = true;
		} else {
			// This was supposed to be a IPv6 address, but it's not!
			EString msg(hostStr + ": invalid IPv6 address");
			throw EUnknownHostException(__FILE__, __LINE__, msg.c_str());
		}
	}

	// if host is an IP address, we won't do further lookup
	if (ECharacter::digit(hostStr.charAt(0), 16) != -1 || (hostStr.charAt(0) == ':')) {
		if (ipv6Expected) {
			// We were expecting an IPv6 Litteral, but got something else
			EString msg("["+hostStr+"]");
			throw EUnknownHostException(__FILE__, __LINE__, msg.c_str());
		}
		else {
			byte addr4[4];
			int ret = EIPAddressUtil::textToNumericFormatV4(host, addr4);
			if (ret == 0) {
				EInetAddress *inetaddr = new EInetAddress(host, addr4);
				array.add(inetaddr);
				return array;
			} // else get host by name.
		}
	}

	//@see: return getAllByName0(host, reqAddr, true);

	struct hostent *hp = 0;

	/* temporary buffer, on the off chance we need to expand */
	char *tmp = NULL;

	/* Try once, with our static buffer. */
#if defined(WIN32) || defined(__CYGWIN32__) || defined(__APPLE__)
	// Darwin gethostbyname uses thread local storage, and is thus thread safe.
	// @see: http://lists.apple.com/archives/darwin-dev/2006/May/msg00004.html
	hp = gethostbyname(hostStr.c_str());
#else //linux
	struct hostent res;
	// this buffer must be pointer-aligned so is declared
	// with pointer type
	char *buf[HENT_BUF_SIZE/(sizeof (char *))];
	int h_error=0;
#ifdef __GLIBC__
	gethostbyname_r(hostStr.c_str(), &res, (char*)buf, sizeof(buf), &hp, &h_error);
#else
	hp = gethostbyname_r(hostStr.c_str(), &res, (char*)buf, sizeof(buf), &h_error);
#endif
	/* With the re-entrant system calls, it's possible that the buffer
	 * we pass to it is not large enough to hold an exceptionally
	 * large DNS entry.  This is signaled by errno->ERANGE.  We try once
	 * more, with a very big size.
	 */
	if (hp == NULL && errno == ERANGE) {
		tmp = (char*) malloc(BIG_HENT_BUF_SIZE);
#ifdef __GLIBC__
		gethostbyname_r(hostStr.c_str(), &res, tmp, BIG_HENT_BUF_SIZE,
		                            &hp, &h_error);
#else
		hp = gethostbyname_r(hostStr.c_str(), &res, tmp, BIG_HENT_BUF_SIZE,
		                                 &h_error);
#endif
	}
#endif
	if (hp != NULL) {
		struct in_addr **addrp = (struct in_addr **) hp->h_addr_list;
		int i = 0;

		while (*addrp) {
			EInetAddress *inetaddr = new EInetAddress(hostStr.c_str(),
					(*addrp)->s_addr);
			array.add(inetaddr);
			addrp++;
			i++;
		}
	}

	if (tmp) {
		free(tmp);
	}

	if (hp == NULL) {
		throw EUnknownHostException(__FILE__, __LINE__);
	}

	return array;
}

EInetAddress EInetAddress::getLocalHost() THROWS(EUnknownHostException)
{
	char hname[HENT_BUF_SIZE/(sizeof (char *))];

	if (gethostname(hname, sizeof(hname)) == 0) {
		struct hostent *hent = gethostbyname(hname);
		if (hent) {
			struct in_addr **addrp = (struct in_addr **) hent->h_addr_list;
			if (*addrp) {
				return EInetAddress(hent->h_name, (*addrp)->s_addr);
			}
		}
	}

	throw EUnknownHostException(__FILE__, __LINE__);
}

} /* namespace efc */
