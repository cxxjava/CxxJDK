/*
 * EMulticastSocket.cpp
 *
 *  Created on: 2016-8-11
 *      Author: cxxjava@163.com
 */

#include "EMulticastSocket.hh"
#include "ESocketException.hh"
#include "ENullPointerException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

EMulticastSocket::~EMulticastSocket() {
	//
}

EMulticastSocket::EMulticastSocket() :
		EDatagramSocket((EInetSocketAddress*) null), interfaceSet(false) {
	EInetSocketAddress isa(0);
	init(&isa);
}

EMulticastSocket::EMulticastSocket(int port) :
		EDatagramSocket((EInetSocketAddress*) null), interfaceSet(false) {
	EInetSocketAddress isa(port);
	init(&isa);
}

EMulticastSocket::EMulticastSocket(EInetSocketAddress* bindaddr) :
		EDatagramSocket((EInetSocketAddress*) null), interfaceSet(false) {
	init(bindaddr);
}

EMulticastSocket::EMulticastSocket(const char* hostname, int port) {
	EInetSocketAddress isa(hostname, port);
	init(&isa);
}

void EMulticastSocket::init(EInetSocketAddress* bindaddr) {
	// Enable SO_REUSEADDR before binding
	setReuseAddress(true);

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

void EMulticastSocket::setTimeToLive(int ttl) {
	if (ttl < 0 || ttl > 255) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "ttl out of range");
	}
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	//@see: getImpl().setTimeToLive(ttl);
	{
		//@see: setTTL(socket, ttl);
		char ittl = (char)ttl;
		setOption(_IP_MULTICAST_TTL, (char*)&ittl, sizeof(ittl));
	}
}

int EMulticastSocket::getTimeToLive() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	//@see: return getImpl().getTimeToLive();
	{
		/* getsockopt of TTL */
		{
			u_char ttl = 0;
			int len = sizeof(ttl);
			getOption(_IP_MULTICAST_TTL, (char*)&ttl, &len);
			return (int)ttl;
		}
	}
}

void EMulticastSocket::joinGroup(EInetAddress* mcastaddr) {
	joinGroup(mcastaddr, null);
}

void EMulticastSocket::leaveGroup(EInetAddress* mcastaddr) {
	leaveGroup(mcastaddr, null);
}

void EMulticastSocket::joinGroup(EInetSocketAddress* mcastaddr,
		ENetworkInterface* netIf) {
	if (!mcastaddr) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	joinGroup(mcastaddr->getAddress(), netIf);
}

void EMulticastSocket::joinGroup(const char* hostname, int port, ENetworkInterface* netIf) {
	EInetSocketAddress isa(hostname, port);
	this->joinGroup(&isa, netIf);
}

void EMulticastSocket::leaveGroup(EInetSocketAddress* mcastaddr,
		ENetworkInterface* netIf) {
	if (!mcastaddr) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	leaveGroup(mcastaddr->getAddress(), netIf);
}

void EMulticastSocket::leaveGroup(const char* hostname, int port, ENetworkInterface* netIf) {
	EInetSocketAddress isa(hostname, port);
	this->leaveGroup(&isa, netIf);
}

void EMulticastSocket::joinGroup(EInetAddress* mcastaddr,
		ENetworkInterface* netIf) {
	if (!mcastaddr) {
		throw ENullPointerException(__FILE__, __LINE__);
	}

	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}

	checkAddress(mcastaddr, "joinGroup");
	/*
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
		security.checkMulticast(mcastaddr);
	}
	*/
	if (!mcastaddr->isMulticastAddress()) {
		throw ESocketException(__FILE__, __LINE__, "Not a multicast address");
	}

	/**
	 * required for some platforms where it's not possible to join
	 * a group without setting the interface first.
	 */
	ENetworkInterface* defaultInterface = ENetworkInterface::getDefault();

	if (!interfaceSet && defaultInterface != null) {
		setNetworkInterface(defaultInterface);
	}

	//@see: getImpl().join(mcastaddr);
	{
		//@see: mcast_join_leave(env, this, iaObj, niObj, JNI_TRUE);
		mcast_join_leave(mcastaddr, netIf, true);
	}
}

void EMulticastSocket::joinGroup(const char* hostname, ENetworkInterface* netIf) {
	EInetAddress ia = EInetAddress::getByName(hostname);
	this->joinGroup(&ia, netIf);
}

void EMulticastSocket::leaveGroup(EInetAddress* mcastaddr,
		ENetworkInterface* netIf) {
	if (!mcastaddr) {
		throw ENullPointerException(__FILE__, __LINE__);
	}

	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}

	checkAddress(mcastaddr, "leaveGroup");
	/*
	SecurityManager security = System.getSecurityManager();
	if (security != null) {
		security.checkMulticast(mcastaddr);
	}
	*/

	if (!mcastaddr->isMulticastAddress()) {
		throw ESocketException(__FILE__, __LINE__, "Not a multicast address");
	}

	//@see :getImpl().leave(mcastaddr);
	{
		//@see: mcast_join_leave(env, this, iaObj, niObj, JNI_FALSE);
		mcast_join_leave(mcastaddr, netIf, false);
	}
}

void EMulticastSocket::leaveGroup(const char* hostname, ENetworkInterface* netIf) {
	EInetAddress ia = EInetAddress::getByName(hostname);
	this->leaveGroup(&ia, netIf);
}

void EMulticastSocket::setInterface(EInetAddress* inf) {
	if (!inf) {
		throw ENullPointerException(__FILE__, __LINE__);
	}

	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
	checkAddress(inf, "setInterface");
	SYNCBLOCK(&infLock) {
		//@see: openjdk-8/src/solaris/native/java/net/PlainDatagramSocketImpl.c#L1202
		//@see: mcast_set_if_by_addr_v4
		struct in_addr in;
		in.s_addr = inf->getAddress();
		setOption(_IP_MULTICAST_IF, &in, sizeof(in));
		infAddress = new EInetAddress(*inf);
		interfaceSet = true;
    }}
}

sp<EInetAddress> EMulticastSocket::getInterface() {
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
	SYNCBLOCK(&infLock) {
		struct in_addr in;
		struct in_addr *inP = &in;
		int len = sizeof(struct in_addr);

		getOption(_IP_MULTICAST_IF, (char *)inP, &len);

		sp<EInetAddress> ia = new EInetAddress(NULL, inP->s_addr);

		/**
		 * No previous setInterface or interface can be
		 * set using setNetworkInterface
		 */
		if (infAddress == null) {
			return ia;
		}

		/**
		 * Same interface set with setInterface?
		 */
		if (ia->equals(infAddress.get())) {
			return ia;
		}

		/**
		 * Different InetAddress from what we set with setInterface
		 * so enumerate the current interface to see if the
		 * address set by setInterface is bound to this interface.
		 */
		try {
			sp<ENetworkInterface> ni = ENetworkInterface::getByInetAddress(ia.get());
			sp<EEnumeration<EInetAddress*> > addrs = ni->getInetAddresses();
			while (addrs->hasMoreElements()) {
				EInetAddress* addr = addrs->nextElement();
				if (addr->equals(infAddress.get())) {
					return infAddress;
				}
			}

			/**
			 * No match so reset infAddress to indicate that the
			 * interface has changed via means
			 */
			infAddress = null;
			return ia;
		} catch (EException& e) {
			return ia;
		}
    }}
}

void EMulticastSocket::setNetworkInterface(ENetworkInterface* netIf) {
	if (!netIf) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	SYNCBLOCK(&infLock) {
		//@see: openjdk-8/src/solaris/native/java/net/PlainDatagramSocketImpl.c#L1202
		//@see: mcast_set_if_by_if_v4
		sp<EEnumeration<EInetAddress*> > ei = netIf->getInetAddresses();
		EInetAddress* addr = ei->hasMoreElements() ? ei->nextElement() : null;

		if (!addr) {
			throw ESocketException(__FILE__, __LINE__,
				"bad argument for IP_MULTICAST_IF2: "
				"No IP addresses bound to interface");
			return;
		}

		struct in_addr in;
		in.s_addr = addr->getAddress();
		setOption(_IP_MULTICAST_IF2, &in, sizeof(in));
		infAddress = null;
		interfaceSet = true;
    }}
}

sp<ENetworkInterface> EMulticastSocket::getNetworkInterface() {
	//@see: getMulticastInterface
	//@see: Java_java_net_NetworkInterface_getByInetAddress0

	/*
	 * For IP_MULTICAST_IF2 we get the NetworkInterface for
	 * this address and return it
	 */
	struct in_addr in;
	struct in_addr *inP = &in;
	int len = sizeof(struct in_addr);
	getOption(_IP_MULTICAST_IF2, (char *)inP, &len);
	sp<EInetAddress> ia = new EInetAddress(NULL, inP->s_addr);
	sp<ENetworkInterface> nif = ENetworkInterface::getByInetAddress(ia.get());
	if (nif != null) {
		return nif;
	}

	/*
	 * The address doesn't appear to be bound at any known
	 * NetworkInterface. Therefore we construct a NetworkInterface
	 * with this address.
	 */
	EA<EInetAddress*> addrs(1);
	addrs[0] = ia.dismiss();
	return new ENetworkInterface(NULL, -1, &addrs);
}

void EMulticastSocket::setLoopbackMode(boolean disable) {
	int opt = 1;
	setOption(_IP_MULTICAST_LOOP, &opt, sizeof(opt));
}

boolean EMulticastSocket::getLoopbackMode() {
	int opt, len;
    len = sizeof(int);
	getOption(_IP_MULTICAST_LOOP, &opt, &len);
	return opt ? true : false;
}

/*
 * mcast_join_leave: Join or leave a multicast group.
 *
 * For IPv4 sockets use IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP socket option
 * to join/leave multicast group.
 *
 * For IPv6 sockets use IPV6_ADD_MEMBERSHIP/IPV6_DROP_MEMBERSHIP socket option
 * to join/leave multicast group. If multicast group is an IPv4 address then
 * an IPv4-mapped address is used.
 *
 * On Linux with IPv6 if we wish to join/leave an IPv4 multicast group then
 * we must use the IPv4 socket options. This is because the IPv6 socket options
 * don't support IPv4-mapped addresses. This is true as per 2.2.19 and 2.4.7
 * kernel releases. In the future it's possible that IP_ADD_MEMBERSHIP
 * will be updated to return ENOPROTOOPT if uses with an IPv6 socket (Solaris
 * already does this). Thus to cater for this we first try with the IPv4
 * socket options and if they fail we use the IPv6 socket options. This
 * seems a reasonable failsafe solution.
 */
void EMulticastSocket::mcast_join_leave(EInetAddress* inetaddr,
		ENetworkInterface* netIf, boolean join) {
    /*
     * cxxjava: only for IPv4 join/leave.
     */
	boolean ipv6_join_leave = false;

    /*
     * For IPv4 join use IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP socket option
     *
     * On Linux if IPv4 or IPv6 use IP_ADD_MEMBERSHIP/IP_DROP_MEMBERSHIP
     */
    if (!ipv6_join_leave) {
#ifdef __linux__
        struct ip_mreqn mname;
#else
        struct ip_mreq mname;
#endif
        int mname_len;

        /*
         * joinGroup(InetAddress, NetworkInterface) implementation :-
         *
         * Linux/IPv6:  use ip_mreqn structure populated with multicast
         *              address and interface index.
         *
         * IPv4:        use ip_mreq structure populated with multicast
         *              address and first address obtained from
         *              NetworkInterface
         */
        if (netIf != NULL) {
        	sp<EEnumeration<EInetAddress*> > ei = netIf->getInetAddresses();
        	EInetAddress* addr = ei->hasMoreElements() ? ei->nextElement() : null;

			if (!addr) {
				throw ESocketException(__FILE__, __LINE__,
					"bad argument for IP_ADD_MEMBERSHIP: "
					"No IP addresses bound to interface");
				return;
			}

			mname.imr_multiaddr.s_addr = inetaddr->getAddress();
#ifdef __linux__
			mname.imr_address.s_addr = addr->getAddress();
#else
			mname.imr_interface.s_addr = addr->getAddress();
#endif
			mname_len = sizeof(struct ip_mreq);
        }


        /*
         * joinGroup(InetAddress) implementation :-
         *
         * Linux/IPv6:  use ip_mreqn structure populated with multicast
         *              address and interface index. index obtained
         *              from cached value or IPV6_MULTICAST_IF.
         *
         * IPv4:        use ip_mreq structure populated with multicast
         *              address and local address obtained from
         *              IP_MULTICAST_IF. On Linux IP_MULTICAST_IF
         *              returns different structure depending on
         *              kernel.
         */

        if (netIf == NULL) {
			struct in_addr in;
			struct in_addr *inP = &in;
			int len = sizeof(struct in_addr);

			getOption(_IP_MULTICAST_IF, (char *)inP, &len);

#ifdef __linux__
			mname.imr_address.s_addr = in.s_addr;

#else
			mname.imr_interface.s_addr = in.s_addr;
#endif
			mname.imr_multiaddr.s_addr = inetaddr->getAddress();
			mname_len = sizeof(struct ip_mreq);
        }


        /*
         * Join the multicast group.
         */
        if (::setsockopt(socket, IPPROTO_IP, (join ? _IP_ADD_MEMBERSHIP:_IP_DROP_MEMBERSHIP),
                           (char *) &mname, mname_len) < 0) {

            /*
             * If IP_ADD_MEMBERSHIP returns ENOPROTOOPT on Linux and we've got
             * IPv6 enabled then it's possible that the kernel has been fixed
             * so we switch to IPV6_ADD_MEMBERSHIP socket option.
             * As of 2.4.7 kernel IPV6_ADD_MEMBERSHIP can't handle IPv4-mapped
             * addresses so we have to use IP_ADD_MEMBERSHIP for IPv4 multicast
             * groups. However if the socket is an IPv6 socket then then setsockopt
             * should return ENOPROTOOPT. We assume this will be fixed in Linux
             * at some stage.
             */
            if (errno) {
                if (join) {
                	throw ESocketException(__FILE__, __LINE__, "setsockopt IP_ADD_MEMBERSHIP failed");
                } else {
                    if (errno == ENOENT)
                    	throw ESocketException(__FILE__, __LINE__, "Not a member of the multicast group");
                    else
                    	throw ESocketException(__FILE__, __LINE__, "setsockopt IP_DROP_MEMBERSHIP failed");
                }
            }
        }

        /*
         * If we haven't switched to IPv6 socket option then we're done.
         */
        if (!ipv6_join_leave) {
            return;
        }
    }
}

} /* namespace efc */
