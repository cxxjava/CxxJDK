/*
 * ENetworkInterface.cpp
 *
 *  Created on: 2016-8-12
 *      Author: cxxjava@163.com
 */

#ifdef WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#endif

#include "ENetworkInterface.hh"
#include "ENetWrapper.hh"
#include "ENullPointerException.hh"
#include "ENoSuchElementException.hh"
#include "EIllegalArgumentException.hh"

#ifdef WIN32

#include <windows.h>
#include <WS2tcpip.h>

#else //!

#if defined(__OpenBSD__)
#include <sys/types.h>
#endif
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <net/if_arp.h>

#ifdef __solaris__
#include <sys/dlpi.h>
#include <fcntl.h>
#include <stropts.h>
#include <sys/sockio.h>
#endif

#ifdef __linux__
#include <sys/ioctl.h>
#include <bits/ioctls.h>
#include <sys/utsname.h>
#include <stdio.h>

#include <ifaddrs.h>
//    Match Linux to FreeBSD
#    define AF_LINK AF_PACKET
#endif

#if defined(_AIX)
#include <sys/ioctl.h>
#include <netinet/in6_var.h>
#include <sys/ndd_var.h>
#include <sys/kinfo.h>
#endif

#if defined(__bsd__)
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#if defined(__APPLE__)
#include <net/ethernet.h>
#include <net/if_var.h>
#include <net/if_dl.h>
#include <netinet/in_var.h>
#include <ifaddrs.h>
#endif
#endif

#endif //!WIN32

namespace efc {

#ifdef WIN32

typedef union {
    struct sockaddr     him;
    struct sockaddr_in  him4;
    //struct SOCKADDR_IN6 him6;
} SOCKETADDRESS;

/*
 * Structures used when enumerating interfaces and addresses
 */
typedef struct _netaddr  {
    SOCKETADDRESS    addr;                  /* IPv4 or IPv6 address */
    SOCKETADDRESS    brdcast;
    short            mask;
    struct _netaddr *next;
} netaddr;

typedef struct _netif {
    char *name;
    char *displayName;
    DWORD dwIndex;              /* Internal index */
    DWORD ifType;               /* Interface type */
    int index;                  /* Friendly index */
    struct _netif *next;

    /* Following fields used on Windows XP when IPv6 is used only */
    es_bool_t hasIpv6Address;    /* true when following fields valid */
    es_bool_t dNameIsUnicode;    /* Display Name is Unicode */
    int naddrs;                 /* Number of addrs */
    DWORD ipv6Index;
    struct _netaddr *addrs;     /* addr list for interfaces */
} netif;

static void free_netaddr(netaddr *netaddrP) {
    netaddr *curr = netaddrP;
    while (curr != NULL) {
        netaddrP = netaddrP->next;
        free(curr);
        curr = netaddrP;
    }
}

/*
* Support routines to free netif and netaddr lists
*/
static void freeif(netif *netifP) {
   netif *curr = netifP;
   while (curr != NULL) {
       if (curr->name != NULL)
           free(curr->name);
       if (curr->displayName != NULL)
           free(curr->displayName);
       if (curr->addrs != NULL)
           free_netaddr (curr->addrs);
       netifP = netifP->next;
       free(curr);
       curr = netifP;
   }
}

/*
 * Returns the interface structure from the table with the matching index.
 */
static MIB_IFROW *getIF(int index) {
	MIB_IFTABLE *tableP;
	MIB_IFROW *ifrowP, *ret = NULL;
	ULONG size;
	DWORD i, count;
	int ifindex;

	/*
	 * Ask the IP Helper library to enumerate the adapters
	 */
	size = sizeof(MIB_IFTABLE);
	tableP = (MIB_IFTABLE *)malloc(size);
	if(tableP == NULL) {
		return NULL;
	}

	count = GetIfTable(tableP, &size, TRUE);
	if (count == ERROR_INSUFFICIENT_BUFFER || count == ERROR_BUFFER_OVERFLOW) {
		MIB_IFTABLE* newTableP = (MIB_IFTABLE *)realloc(tableP, size);
		if (newTableP == NULL) {
			free(tableP);
			return NULL;
		}
		tableP = newTableP;

		count = GetIfTable(tableP, &size, TRUE);
	}

	if (count != NO_ERROR) {
		free(tableP);
		return NULL;
	}

	{
		ifrowP = tableP->table;
		for (i=0; i<tableP->dwNumEntries; i++) {
			/*
			 * Warning: the real index is obtained by GetFriendlyIfIndex()
			 */
			ifindex = GetFriendlyIfIndex(ifrowP->dwIndex);
			if (ifindex == index) {
				/*
				 * Create a copy of the entry so that we can free the table.
				 */
				ret = (MIB_IFROW *) malloc(sizeof(MIB_IFROW));
				if (ret == NULL) {
					free(tableP);
					return NULL;
				}
				memcpy(ret, ifrowP, sizeof(MIB_IFROW));
				break;
			}

			/* onto the next interface */
			ifrowP++;
		}
		free(tableP);
	}
	return ret;
}

/*
 * Enumerate network interfaces using IP Helper Library routine GetIfTable.
 * We use GetIfTable rather than other IP helper routines because it's
 * available on 98 & NT SP4+.
 *
 * Returns the number of interfaces found or -1 if error. If no error
 * occurs then netifPP be returned as list of netif structures or NULL
 * if no interfaces are found.
 */
static netif* enumInterfaces(void)
{
	netif *netifP;
	MIB_IFTABLE *tableP;
	MIB_IFROW *ifrowP;
	ULONG size;
	DWORD ret;
	int count;
	DWORD i;
	int lo=0, eth=0, tr=0, fddi=0, ppp=0, sl=0, wlan=0, net=0, wlen=0;

	/*
	 * Ask the IP Helper library to enumerate the adapters
	 */
	size = sizeof(MIB_IFTABLE);
	tableP = (MIB_IFTABLE *)malloc(size);
	if (tableP == NULL) {
		return NULL;
	}

	ret = GetIfTable(tableP, &size, TRUE);
	if (ret == ERROR_INSUFFICIENT_BUFFER || ret == ERROR_BUFFER_OVERFLOW) {
		MIB_IFTABLE * newTableP = (MIB_IFTABLE *)realloc(tableP, size);
		if (newTableP == NULL) {
			free(tableP);
			return NULL;
		}
		tableP = newTableP;
		ret = GetIfTable(tableP, &size, TRUE);
	}

	if (ret != NO_ERROR) {
		free(tableP);
		return NULL;
	}

	/*
	 * Iterate through the list of adapters
	 */
	count = 0;
	netifP = NULL;

	ifrowP = tableP->table;
	for (i=0; i<tableP->dwNumEntries; i++) {
		char dev_name[8];
		netif *curr;

		/*
		 * Generate a name for the device as Windows doesn't have any
		 * real concept of a device name.
		 */
		switch (ifrowP->dwType) {
		case MIB_IF_TYPE_ETHERNET:
			_snprintf_s(dev_name, 8, _TRUNCATE, "eth%d", eth++);
			break;

		case MIB_IF_TYPE_TOKENRING:
			_snprintf_s(dev_name, 8, _TRUNCATE, "tr%d", tr++);
			break;

		case MIB_IF_TYPE_FDDI:
			_snprintf_s(dev_name, 8, _TRUNCATE, "fddi%d", fddi++);
			break;

		case MIB_IF_TYPE_LOOPBACK:
			/* There should only be only IPv4 loopback address */
			if (lo > 0) {
				continue;
			}
			strncpy_s(dev_name, 8, "lo", _TRUNCATE);
			lo++;
			break;

		case MIB_IF_TYPE_PPP:
			_snprintf_s(dev_name, 8, _TRUNCATE, "ppp%d", ppp++);
			break;

		case MIB_IF_TYPE_SLIP:
			_snprintf_s(dev_name, 8, _TRUNCATE, "sl%d", sl++);
			break;

		case IF_TYPE_IEEE80211:
			_snprintf_s(dev_name, 8, _TRUNCATE, "wlan%d", wlan++);
			break;

		default:
			_snprintf_s(dev_name, 8, _TRUNCATE, "net%d", net++);
		}

		/*
		 * Allocate a netif structure and space for the name and
		 * display name (description in this case).
		 */
		curr = (netif *)calloc(1, sizeof(netif));
		if (curr != NULL) {
			wlen = MultiByteToWideChar(CP_OEMCP, 0, (LPCSTR)ifrowP->bDescr,
					   ifrowP->dwDescrLen, NULL, 0);
			if(wlen == 0) {
				// MultiByteToWideChar should not fail
				// But in rare case it fails, we allow 'char' to be displayed
				curr->displayName = (char *)malloc(ifrowP->dwDescrLen + 1);
			} else {
				curr->displayName = (char *)malloc(wlen*(sizeof(wchar_t))+1);
			}

			curr->name = (char *)malloc(strlen(dev_name) + 1);

			if (curr->name == NULL || curr->displayName == NULL) {
				if (curr->name) free(curr->name);
				if (curr->displayName) free(curr->displayName);
				curr = NULL;
			}
		}
		if (curr == NULL) {
			freeif(netifP);
			free(tableP);
			return NULL;
		}

		/*
		 * Populate the interface. Note that we need to convert the
		 * index into its "friendly" value as otherwise we will expose
		 * 32-bit numbers as index values.
		 */
		strcpy(curr->name, dev_name);
		if (wlen == 0) {
			// display char type in case of MultiByteToWideChar failure
			strncpy(curr->displayName, (char*)ifrowP->bDescr, ifrowP->dwDescrLen);
			curr->displayName[ifrowP->dwDescrLen] = '\0';
		} else {
			// call MultiByteToWideChar again to fill curr->displayName
			// it should not fail, because we have called it once before
			if (MultiByteToWideChar(CP_OEMCP, 0, (LPCSTR)ifrowP->bDescr,
				   ifrowP->dwDescrLen, (LPWSTR)curr->displayName, wlen) == 0) {
				freeif(netifP);
				free(tableP);
				free(curr->name);
				free(curr->displayName);
				free(curr);
				return NULL;
			} else {
				curr->displayName[wlen*(sizeof(wchar_t))] = '\0';
				curr->dNameIsUnicode = TRUE;
			}
		}

		curr->dwIndex = ifrowP->dwIndex;
		curr->ifType = ifrowP->dwType;
		curr->index = GetFriendlyIfIndex(ifrowP->dwIndex);

		/*
		 * Put the interface at tail of list as GetIfTable(,,TRUE) is
		 * returning the interfaces in index order.
		 */
		count++;
		if (netifP == NULL) {
			netifP = curr;
		} else {
			netif *tail = netifP;
			while (tail->next != NULL) {
				tail = tail->next;
			}
			tail->next = curr;
		}

		/* onto the next interface */
		ifrowP++;
	}

	/*
	 * Free the interface table and return the interface list
	 */
	if (tableP) {
		free(tableP);
	}
	return netifP;
}

/*
 * Enumerate the IP addresses on an interface using the IP helper library
 * routine GetIfAddrTable and matching based on the index name. There are
 * more efficient routines but we use GetIfAddrTable because it's avaliable
 * on 98 and NT.
 *
 * Returns the count of addresses, or -1 if error. If no error occurs then
 * netaddrPP will return a list of netaddr structures with the IP addresses.
 */
static int enumAddresses_win(netif *netifP, netaddr **netaddrPP)
{
	MIB_IPADDRTABLE *tableP;
	ULONG size;
	DWORD ret;
	DWORD i;
	netaddr *netaddrP;
	int count = 0;
	unsigned long mask;

	/*
	 * Use GetIpAddrTable to enumerate the IP Addresses
	 */
	size = sizeof(MIB_IPADDRTABLE);
	tableP = (MIB_IPADDRTABLE *)malloc(size);

	ret = GetIpAddrTable(tableP, &size, FALSE);
	if (ret == ERROR_INSUFFICIENT_BUFFER || ret == ERROR_BUFFER_OVERFLOW) {
		MIB_IPADDRTABLE * newTableP = (MIB_IPADDRTABLE *)realloc(tableP, size);
		if (newTableP == NULL) {
			free(tableP);
			return -1;
		}
		tableP = newTableP;

		ret = GetIpAddrTable(tableP, &size, FALSE);
	}
	if (ret != NO_ERROR) {
		if (tableP) {
			free(tableP);
		}
		return -1;
	}

	/*
	 * Iterate through the table to find the addresses with the
	 * matching dwIndex. Ignore 0.0.0.0 addresses.
	 */
	count = 0;
	netaddrP = NULL;

	i = 0;
	while (i<tableP->dwNumEntries) {
		if (tableP->table[i].dwIndex == netifP->dwIndex &&
		            tableP->table[i].dwAddr != 0) {
			netaddr *curr = (netaddr *)malloc(sizeof(netaddr));
			if (curr == NULL) {
				free_netaddr(netaddrP);
				free(tableP);
				return -1;
			}

			curr->addr.him4.sin_family = AF_INET;
			curr->addr.him4.sin_addr.s_addr = tableP->table[i].dwAddr;
			/*
			 * Get netmask / broadcast address
			 */
			switch (netifP->ifType) {
				case MIB_IF_TYPE_ETHERNET:
				case MIB_IF_TYPE_TOKENRING:
				case MIB_IF_TYPE_FDDI:
				case MIB_IF_TYPE_LOOPBACK:
				case IF_TYPE_IEEE80211:
				/**
				 * Contrary to what it seems to indicate, dwBCastAddr doesn't
				 * contain the broadcast address but 0 or 1 depending on whether
				 * the broadcast address should set the bits of the host part
				 * to 0 or 1.
				 * Yes, I know it's stupid, but what can I say, it's MSFTs API.
				 */
				curr->brdcast.him4.sin_family = AF_INET;
				if (tableP->table[i].dwBCastAddr == 1)
					curr->brdcast.him4.sin_addr.s_addr = (tableP->table[i].dwAddr & tableP->table[i].dwMask) | (0xffffffff ^ tableP->table[i].dwMask);
				else
					curr->brdcast.him4.sin_addr.s_addr = (tableP->table[i].dwAddr & tableP->table[i].dwMask);
				mask = ntohl(tableP->table[i].dwMask);
				curr->mask = 0;
				while (mask) {
					mask <<= 1;
					curr->mask++;
				}
				break;
				case MIB_IF_TYPE_PPP:
				case MIB_IF_TYPE_SLIP:
				default:
				/**
				 * these don't have broadcast/subnet
				 */
				curr->mask = -1;
				break;
			}

			curr->next = netaddrP;
			netaddrP = curr;
			count++;
		}
		i++;
	}

	*netaddrPP = netaddrP;
	free(tableP);
	return count;
}

static llong getMacAddr0(int inAddr, const char* ifname, int index) {
	llong ret = 0;
	int len;
	MIB_IFROW *ifRowP;

	ifRowP = getIF(index);
	if (ifRowP != NULL) {
		switch(ifRowP->dwType) {
			case MIB_IF_TYPE_ETHERNET:
			case MIB_IF_TYPE_TOKENRING:
			case MIB_IF_TYPE_FDDI:
			case IF_TYPE_IEEE80211:
			len = ifRowP->dwPhysAddrLen;
			memcpy(&ret, (byte*)ifRowP->bPhysAddr, len);
			break;
		}
		free(ifRowP);
	}
	return ret;
}

#else //!

typedef struct _netaddr  {
    struct sockaddr *addr;
    struct sockaddr *brdcast;
    short mask;
    int family; /* to make searches simple */
    struct _netaddr *next;
} netaddr;

typedef struct _netif {
    char *name;
    int index;
    char isvirtual;
    netaddr *addr;
    struct _netif *childs;
    struct _netif *next;
} netif;

#define CHECKED_MALLOC3(_pointer,_type,_size) \
       do{ \
        _pointer = (_type)malloc( _size ); \
       } while(0)


/* Declare functions */

static netif* addif(int sock, const char* if_name, netif* ifs,
		struct sockaddr* ifr_addrP, int family, short prefix, boolean* error);

static void freeif(netif* ifs);

/* Define functions */

#ifdef __bsd__

/**
 * Get the Hardware address (usually MAC address) for the named interface.
 * return puts the data in buf, and returns the length, in byte, of the
 * MAC address. Returns -1 if there is no hardware address on that interface.
 */
static int getMacAddress(int sock, const char* ifname, const struct in_addr* addr, unsigned char *buf) {
    struct ifaddrs *ifa0, *ifa;
    struct sockaddr *saddr;
    int i;

    /* Grab the interface list */
    if (!getifaddrs(&ifa0)) {
        /* Cycle through the interfaces */
        for (i = 0, ifa = ifa0; ifa != NULL; ifa = ifa->ifa_next, i++) {
            saddr = ifa->ifa_addr;
            /* Link layer contains the MAC address */
            if (saddr->sa_family == AF_LINK && !strcmp(ifname, ifa->ifa_name)) {
                struct sockaddr_dl *sadl = (struct sockaddr_dl *) saddr;
                /* Check the address is the correct length */
                if (sadl->sdl_alen == ETHER_ADDR_LEN) {
                    memcpy(buf, (sadl->sdl_data + sadl->sdl_nlen), ETHER_ADDR_LEN);
                    freeifaddrs(ifa0);
                    return ETHER_ADDR_LEN;
                }
            }
        }
        freeifaddrs(ifa0);
    }

    return -1;
}

static netif* enumIPv4Interfaces(int sock, netif* ifs, boolean* error) {
	struct ifaddrs *ifa, *origifa;

	if (getifaddrs(&origifa) != 0) {
		*error = true;
		return ifs;
	}

	for (ifa = origifa; ifa != NULL; ifa = ifa->ifa_next) {

		/*
		 * Skip non-AF_INET entries.
		 */
		if (ifa->ifa_addr == NULL || ifa->ifa_addr->sa_family != AF_INET)
			continue;

		/*
		 * Add to the list.
		 */
		ifs = addif(sock, ifa->ifa_name, ifs, ifa->ifa_addr, AF_INET, 0, error);

		/*
		 * If an exception occurred then free the list.
		 */
		if (*error) {
			freeifaddrs(origifa);
			freeif(ifs);
			return NULL;
		}
	}

	/*
	 * Free socket and buffer
	 */
	freeifaddrs(origifa);
	return ifs;
}

static int getIndex(int sock, const char *name){
#ifdef __FreeBSD__
     /*
      * Try to get the interface index
      * (Not supported on Solaris 2.6 or 7)
      */
    struct ifreq if2;
    strcpy(if2.ifr_name, name);

    if (ioctl(sock, SIOCGIFINDEX, (char *)&if2) < 0) {
        return -1;
    }

    return if2.ifr_index;
#else
    /*
     * Try to get the interface index using BSD specific if_nametoindex
     */
    int index = if_nametoindex(name);
    return (index == 0) ? -1 : index;
#endif
}

#elif defined(__linux__) || defined(_AIX)

/* Open socket for further ioct calls, try v4 socket first and
 * if it falls return v6 socket
 */

/**
 * Get the Hardware address (usually MAC address) for the named interface.
 * return puts the data in buf, and returns the length, in byte, of the
 * MAC address. Returns -1 if there is no hardware address on that interface.
 */
static int getMacAddress(int sock, const char* ifname, const struct in_addr* addr, unsigned char *buf) {
#if defined (_AIX)
    int size;
    struct kinfo_ndd *nddp;
    void *end;

    size = getkerninfo(KINFO_NDD, 0, 0, 0);
    if (size == 0) {
        return -1;
    }

    if (size < 0) {
        perror("getkerninfo 1");
        return -1;
    }

    nddp = (struct kinfo_ndd *)malloc(size);
    if (!nddp) {
        return -1;
    }

    if (getkerninfo(KINFO_NDD, nddp, &size, 0) < 0) {
        perror("getkerninfo 2");
        return -1;
    }

    end = (void *)nddp + size;
    while ((void *)nddp < end) {
        if (!strcmp(nddp->ndd_alias, ifname) ||
                !strcmp(nddp->ndd_name, ifname)) {
            bcopy(nddp->ndd_addr, buf, 6);
            return 6;
        } else {
            nddp++;
        }
    }

    return -1;

#elif defined(__linux__)
    static struct ifreq ifr;
    int i;

    strcpy(ifr.ifr_name, ifname);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        return -1;
    }

    memcpy(buf, &ifr.ifr_hwaddr.sa_data, IFHWADDRLEN);

   /*
    * All bytes to 0 means no hardware address.
    */

    for (i = 0; i < IFHWADDRLEN; i++) {
        if (buf[i] != 0)
            return IFHWADDRLEN;
    }

    return -1;
#endif
}

static netif *enumIPv4Interfaces(int sock, netif *ifs, boolean* error) {
    struct ifconf ifc;
    struct ifreq *ifreqP;
    char *buf = NULL;
    int numifs;
    unsigned i;
    int siocgifconfRequest = SIOCGIFCONF;


#if defined(__linux__)
    /* need to do a dummy SIOCGIFCONF to determine the buffer size.
     * SIOCGIFCOUNT doesn't work
     */
    ifc.ifc_buf = NULL;
    if (ioctl(sock, SIOCGIFCONF, (char *)&ifc) < 0) {
        *error = true;
        return ifs;
    }
#elif defined(_AIX)
    ifc.ifc_buf = NULL;
    if (ioctl(sock, SIOCGSIZIFCONF, &(ifc.ifc_len)) < 0) {
    	*error = true;
        return ifs;
    }
#endif /* __linux__ */

    CHECKED_MALLOC3(buf,char *, ifc.ifc_len);

    ifc.ifc_buf = buf;
#if defined(_AIX)
    siocgifconfRequest = CSIOCGIFCONF;
#endif
    if (ioctl(sock, siocgifconfRequest, (char *)&ifc) < 0) {
    	*error = true;
        (void) free(buf);
        return ifs;
    }

    /*
     * Iterate through each interface
     */
    ifreqP = ifc.ifc_req;
    for (i=0; i<ifc.ifc_len/sizeof (struct ifreq); i++, ifreqP++) {
#if defined(_AIX)
        if (ifreqP->ifr_addr.sa_family != AF_INET) continue;
#endif
        /*
         * Add to the list
         */
        ifs = addif(sock, ifreqP->ifr_name, ifs, (struct sockaddr *) & (ifreqP->ifr_addr), AF_INET, 0, error);

        /*
         * If an exception occurred then free the list
         */
        if (*error) {
            free(buf);
            freeif(ifs);
            return NULL;
        }
    }

    /*
     * Free socket and buffer
     */
    free(buf);
    return ifs;
}

static int getIndex(int sock, const char *name){
     /*
      * Try to get the interface index
      */
#if defined(_AIX)
    return if_nametoindex(name);
#else
    struct ifreq if2;
    strcpy(if2.ifr_name, name);

    if (ioctl(sock, SIOCGIFINDEX, (char *)&if2) < 0) {
        return -1;
    }

    return if2.ifr_ifindex;
#endif
}

#else

#error "not support"

#endif //!__bsd__

/**
 * Returns the IPv4 broadcast address of a named interface, if it exists.
 * Returns 0 if it doesn't have one.
 */
static struct sockaddr* getBroadcast(int sock, const char* ifname,
		struct sockaddr* brdcast_store, boolean* error) {
	struct sockaddr *ret = NULL;
	struct ifreq if2;

	memset((char*) (&if2), 0, sizeof(if2));
	strcpy(if2.ifr_name, ifname);

	/* Let's make sure the interface does have a broadcast address */
	if (ioctl(sock, SIOCGIFFLAGS, (char *) &if2) < 0) {
		*error = true;
		return NULL;
	}

	if (if2.ifr_flags & IFF_BROADCAST) {
		/* It does, let's retrieve it*/
		if (ioctl(sock, SIOCGIFBRDADDR, (char *) &if2) < 0) {
			*error = true;
			return NULL;
		}

		ret = brdcast_store;
		memcpy(ret, &if2.ifr_broadaddr, sizeof(struct sockaddr));
	}

	return ret;
}

/**
 * Returns the IPv4 subnet prefix length (aka subnet mask) for the named
 * interface, if it has one, otherwise return -1.
 */
static short getSubnet(int sock, const char* ifname, boolean* error) {
	unsigned int mask;
	short ret;
	struct ifreq if2;

	memset((char*) (&if2), 0, sizeof(if2));
	strcpy(if2.ifr_name, ifname);

	if (ioctl(sock, SIOCGIFNETMASK, (char*) (&if2)) < 0) {
		*error = true;
		return -1;
	}

	mask = ntohl(((struct sockaddr_in*) &(if2.ifr_addr))->sin_addr.s_addr);
	ret = 0;
	while (mask) {
		mask <<= 1;
		ret++;
	}

	return ret;
}

static int getMTU(int sock,  const char *ifname) {
    struct ifreq if2;

    memset((char *) &if2, 0, sizeof(if2));
    strcpy(if2.ifr_name, ifname);

    if (ioctl(sock, SIOCGIFMTU, (char *)&if2) < 0) {
        return -1;
    }

    return  if2.ifr_mtu;
}

static int getFlags(int sock, const char *ifname, int *flags) {
  struct ifreq if2;
  int ret = -1;

  memset((char *) &if2, 0, sizeof(if2));
  strcpy(if2.ifr_name, ifname);

  if (ioctl(sock, SIOCGIFFLAGS, (char *)&if2) < 0){
      return -1;
  }

  if (sizeof(if2.ifr_flags) == sizeof(short)) {
    *flags = (if2.ifr_flags & 0xffff);
  } else {
    *flags = if2.ifr_flags;
  }
  return 0;
}

/*
 * Free an interface list (including any attached addresses)
 */
static void freeif(netif* ifs) {
	netif* currif = ifs;
	netif *child = NULL;
	while (currif != NULL) {
		netaddr *addrP = currif->addr;
		while (addrP != NULL) {
			netaddr *next = addrP->next;
			free(addrP);
			addrP = next;
		}

		/*
		 * Don't forget to free the sub-interfaces.
		 */
		if (currif->childs != NULL) {
			freeif(currif->childs);
		}

		ifs = currif->next;
		free(currif);
		currif = ifs;
	}
}

static netif* addif(int sock, const char* if_name, netif* ifs,
		struct sockaddr* ifr_addrP, int family, short prefix, boolean* error) {
	netif *currif = ifs, *parent;
	netaddr* addrP;
	int ifnam_size = IFNAMSIZ;
	char name[IFNAMSIZ], vname[IFNAMSIZ];
	char* name_colonP;
	int mask;
	int isVirtual = 0;
	int addr_size;
	int flags = 0;

	/*
	 * If the interface name is a logical interface then we
	 * remove the unit number so that we have the physical
	 * interface (eg: hme0:1 -> hme0). NetworkInterface
	 * currently doesn't have any concept of physical vs.
	 * logical interfaces.
	 */
	strncpy(name, if_name, ifnam_size);
	name[ifnam_size - 1] = '\0';
	*vname = 0;

	/*
	 * Create and populate the netaddr node. If allocation fails
	 * return an un-updated list.
	 */
	/*Allocate for addr and brdcast at once*/
	addr_size = sizeof(struct sockaddr_in);

	CHECKED_MALLOC3(addrP, netaddr *, sizeof(netaddr)+2*addr_size);
	addrP->addr = (struct sockaddr*) (((char*) (addrP) + sizeof(netaddr)));
	memcpy(addrP->addr, ifr_addrP, addr_size);

	addrP->family = family;
	addrP->brdcast = NULL;
	addrP->mask = prefix;
	addrP->next = 0;
	if (family == AF_INET) {
		// Deal with broadcast addr & subnet mask
		struct sockaddr * brdcast_to = (struct sockaddr *) ((char *) addrP
				+ sizeof(netaddr) + addr_size);
		addrP->brdcast = getBroadcast(sock, name, brdcast_to, error);
		if (*error) {
			free(addrP); //?
			return ifs;
		}
		if ((mask = getSubnet(sock, name, error)) != -1) {
			addrP->mask = mask;
		} else if (*error) {
			free(addrP); //?
			return ifs;
		}
	}

	/**
	 * Deal with virtual interface with colon notation e.g. eth0:1
	 */
	name_colonP = strchr(name, ':');
	if (name_colonP != NULL) {
		/**
		 * This is a virtual interface. If we are able to access the parent
		 * we need to create a new entry if it doesn't exist yet *and* update
		 * the 'parent' interface with the new records.
		 */
		*name_colonP = 0;
		if (getFlags(sock, name, &flags) < 0 || flags < 0) {
			// failed to access parent interface do not create parent.
			// We are a virtual interface with no parent.
			isVirtual = 1;
			*name_colonP = ':';
		} else {
			// Got access to parent, so create it if necessary.
			// Save original name to vname and truncate name by ':'
			memcpy(vname, name, sizeof(vname));
			vname[name_colonP - name] = ':';
		}
	}

	/*
	 * Check if this is a "new" interface. Use the interface
	 * name for matching because index isn't supported on
	 * Solaris 2.6 & 7.
	 */
	while (currif != NULL) {
		if (strcmp(name, currif->name) == 0) {
			break;
		}
		currif = currif->next;
	}

	/*
	 * If "new" then create an netif structure and
	 * insert it onto the list.
	 */
	if (currif == NULL) {
		CHECKED_MALLOC3(currif, netif *, sizeof(netif) + ifnam_size);
		currif->name = (char *) currif + sizeof(netif);
		strncpy(currif->name, name, ifnam_size);
		currif->name[ifnam_size - 1] = '\0';
		currif->index = getIndex(sock, name);
		currif->addr = NULL;
		currif->childs = NULL;
		currif->isvirtual = isVirtual;
		currif->next = ifs;
		ifs = currif;
	}

	/*
	 * Finally insert the address on the interface
	 */
	addrP->next = currif->addr;
	currif->addr = addrP;

	parent = currif;

	/**
	 * Let's deal with the virtual interface now.
	 */
	if (vname[0]) {
		netaddr *tmpaddr;

		currif = parent->childs;

		while (currif != NULL) {
			if (strcmp(vname, currif->name) == 0) {
				break;
			}
			currif = currif->next;
		}

		if (currif == NULL) {
			CHECKED_MALLOC3(currif, netif *, sizeof(netif) + ifnam_size);
			currif->name = (char *) currif + sizeof(netif);
			strncpy(currif->name, vname, ifnam_size);
			currif->name[ifnam_size - 1] = '\0';
			currif->index = getIndex(sock, vname);
			currif->addr = NULL;
			/* Need to duplicate the addr entry? */
			currif->isvirtual = 1;
			currif->childs = NULL;
			currif->next = parent->childs;
			parent->childs = currif;
		}

		CHECKED_MALLOC3(tmpaddr, netaddr *, sizeof(netaddr)+2*addr_size);
		memcpy(tmpaddr, addrP, sizeof(netaddr));
		if (addrP->addr != NULL) {
			tmpaddr->addr = (struct sockaddr *) ((char*) tmpaddr
					+ sizeof(netaddr));
			memcpy(tmpaddr->addr, addrP->addr, addr_size);
		}

		if (addrP->brdcast != NULL) {
			tmpaddr->brdcast = (struct sockaddr *) ((char *) tmpaddr
					+ sizeof(netaddr) + addr_size);
			memcpy(tmpaddr->brdcast, addrP->brdcast, addr_size);
		}

		tmpaddr->next = currif->addr;
		currif->addr = tmpaddr;
	}

	return ifs;
}

/*
 * Enumerates all interfaces
 */
static netif* enumInterfaces(void) {
	netif* ifs;
	int sock;
	boolean error = false;

	/*
	 * Enumerate IPv4 addresses
	 */
	sock = ENetWrapper::socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return NULL;
	}

	ifs = enumIPv4Interfaces(sock, NULL, &error);
	close(sock);

	if (ifs == NULL && error) {
		return NULL;
	}

	/* return partial list if an exception occurs in the middle of process ???*/

	return ifs;
}

static llong getMacAddr0(int inAddr, const char* ifname, int ignore) {
	struct in_addr iaddr;
	llong ret = 0;
	unsigned char mac[16];
	int len;
	int sock;

	sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return 0;
	}

	if (inAddr) {
		iaddr.s_addr = inAddr;
		len = getMacAddress(sock, ifname, &iaddr, mac);
	} else {
		len = getMacAddress(sock, ifname, NULL, mac);
	}
	if (len > 0) {
		memcpy(&ret, mac, sizeof(llong));
	}

	::close(sock);
	return ret;
}

static int getFlags0(const char* ifname) {
	int ret, sock;
	int flags = 0;

	if ((sock = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		return -1;
	}

	ret = getFlags(sock, ifname, &flags);

	::close(sock);

	if (ret < 0) {
		return -1;
	}

	return flags;
}

#endif //!WIN32

//=============================================================================
ENetworkInterface::~ENetworkInterface() {
	delete addrs;
	delete bindings;
	delete childs;
}

ENetworkInterface::ENetworkInterface() :
		index(0), addrs(null), bindings(null), childs(null), parent(null), isvirtual(
				false) {
}

ENetworkInterface::ENetworkInterface(const char* name, int index,
		EA<EInetAddress*>* addrs) :
		index(0), addrs(null), bindings(null), childs(null), parent(null), isvirtual(
				false) {
	this->name = name;
	this->index = index;
	if (addrs) {
		this->addrs = new EA<EInetAddress*>(*addrs);
	}
}

const char* ENetworkInterface::getName() {
	return name.c_str();
}

sp<EEnumeration<EInetAddress*> > ENetworkInterface::getInetAddresses() {
	class checkedAddresses : public EEnumeration<EInetAddress*> {
	private:
		ENetworkInterface* ni;
		int i;
	public:
		checkedAddresses(ENetworkInterface* n) : ni(n), i(0) {
		}
		virtual EInetAddress* nextElement() {
			if (i < ni->addrs->length()) {
				return (*ni->addrs)[i++];
			} else {
				throw ENoSuchElementException(__FILE__, __LINE__);
			}
		}

		virtual boolean hasMoreElements() {
			return (i < ni->addrs->length());
		}
	};
	return new checkedAddresses(this);
}

sp<EList<EInterfaceAddress*> > ENetworkInterface::getInterfaceAddresses() {
	EList<EInterfaceAddress*>* lst = new EArrayList<EInterfaceAddress*>(false, 1);
	for (int j=0; j<bindings->length(); j++) {
		lst->add((*bindings)[j]);
	}
	return lst;
}

sp<EEnumeration<ENetworkInterface*> > ENetworkInterface::getSubInterfaces() {
	class subIFs : public EEnumeration<ENetworkInterface*> {
	private:
		ENetworkInterface* ni;
		int i;
	public:
		subIFs(ENetworkInterface* n) : ni(n), i(0) {
		}
		virtual ENetworkInterface* nextElement() {
			if (i < ni->childs->length()) {
				return (*ni->childs)[i++];
			} else {
				throw ENoSuchElementException(__FILE__, __LINE__);
			}
		}

		virtual boolean hasMoreElements() {
			return (i < ni->childs->length());
		}
	};
	return new subIFs(this);
}

ENetworkInterface* ENetworkInterface::getParent() {
	return parent;
}

int ENetworkInterface::getIndex() {
	return index;
}

const char* ENetworkInterface::getDisplayName() {
	/* strict TCK conformance */
	return displayName.isEmpty() ? null : displayName.c_str();
}

boolean ENetworkInterface::isUp() {
#ifdef WIN32
	boolean ret = false;
	MIB_IFROW *ifRowP;
	ifRowP = getIF(index);
	if (ifRowP != NULL) {
		ret = ifRowP->dwAdminStatus == MIB_IF_ADMIN_STATUS_UP &&
		(ifRowP->dwOperStatus == MIB_IF_OPER_STATUS_OPERATIONAL ||
				ifRowP->dwOperStatus == MIB_IF_OPER_STATUS_CONNECTED);
		free(ifRowP);
	}
	return ret;
#else
	int ret = getFlags0(name.c_str());
	return ((ret & IFF_UP) && (ret & IFF_RUNNING)) ? true :  false;
#endif
}

boolean ENetworkInterface::isLoopback() {
#ifdef WIN32
	MIB_IFROW *ifRowP;
	boolean ret = false;

	// Retained for now to support IPv4 only stack, java.net.preferIPv4Stack
	ifRowP = getIF(index);
	if (ifRowP != NULL) {
		if (ifRowP->dwType == MIB_IF_TYPE_LOOPBACK)
		ret = true;
		free(ifRowP);
	}
	return ret;
#else
	int ret = getFlags0(name.c_str());
	return (ret & IFF_LOOPBACK) ? true :  false;
#endif
}

boolean ENetworkInterface::isPointToPoint() {
#ifdef WIN32
	MIB_IFROW *ifRowP;
	boolean ret = false;

	// Retained for now to support IPv4 only stack, java.net.preferIPv4Stack
	ifRowP = getIF(index);
	if (ifRowP != NULL) {
		switch(ifRowP->dwType) {
			case MIB_IF_TYPE_PPP:
			case MIB_IF_TYPE_SLIP:
			ret = true;
			break;
		}
		free(ifRowP);
	}
	return ret;
#else
	int ret = getFlags0(name.c_str());
	return (ret & IFF_POINTOPOINT) ? true :  false;
#endif
}

boolean ENetworkInterface::supportsMulticast() {
#ifdef WIN32
	//@see: http://fossies.org/linux/xbmc/lib/libUPnP/Neptune/Source/System/Win32/NptWin32Network.cpp

	// create a socket to talk to the TCP/IP stack
	SOCKET net;
	if((net = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0, 0)) == INVALID_SOCKET) {
		return false;
	}

	// get a list of interfaces
	INTERFACE_INFO query[32];  // get up to 32 interfaces
	DWORD bytes_returned;
	int io_result = WSAIoctl(net,
							 SIO_GET_INTERFACE_LIST,
							 NULL, 0,
							 &query, sizeof(query),
							 &bytes_returned,
							 NULL, NULL);
	if (io_result == SOCKET_ERROR) {
		closesocket(net);
		return false;
	}

	// we don't need the socket anymore
	closesocket(net);

	// Display interface information
	int interface_count = (bytes_returned/sizeof(INTERFACE_INFO));

	for (int i=0; i<addrs->length(); i++) {
		EInetAddress* addr = addrs->getAt(i);

		for (int j=0; j<interface_count; j++) {
			SOCKADDR_IN* address;

			// primary address
			address = (SOCKADDR_IN*)&query[j].iiAddress;
			if (address->sin_addr.s_addr == addr->getAddress()) {
				if (query[j].iiFlags & IFF_MULTICAST) {
					return true;
				} else {
					return false;
			}
		}
	}
	}

	return false;

#else
	int ret = getFlags0(name.c_str());
	return (ret & IFF_MULTICAST) ? true :  false;
#endif
}

llong ENetworkInterface::getHardwareAddress() {
	/* @see:
	SecurityManager sec = System.getSecurityManager();
	if (sec != null) {
		try {
			sec.checkPermission(new NetPermission("getNetworkInformation"));
		} catch (SecurityException e) {
			if (!getInetAddresses().hasMoreElements()) {
				// don't have connect permission to any local address
				return null;
			}
		}
	}
	*/
	llong ret = 0;
	for (int i=0; i<addrs->length(); i++) {
		EInetAddress* addr = addrs->getAt(i); //only the first one.
		return getMacAddr0(addr->getAddress(), name.c_str(), index);
	}

	return getMacAddr0(0, name.c_str(), index);
}

int ENetworkInterface::getMTU() {
#ifdef WIN32
	int ret = -1;
	MIB_IFROW *ifRowP;

	ifRowP = getIF(index);
	if (ifRowP != NULL) {
		ret = ifRowP->dwMtu;
		free(ifRowP);
	}
	return ret;
#else
	int ret = -1;
	int sock;

	sock = ENetWrapper::socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
	   throw ESocketException(__FILE__, __LINE__);
	}

	//@see: ret = getMTU(env, sock, name_utf);
	{
		struct ifreq if2;

		memset((char *) &if2, 0, sizeof(if2));
		strcpy(if2.ifr_name, name.c_str());

		if (ioctl(sock, SIOCGIFMTU, (char *)&if2) < 0) {
			throw ESocketException(__FILE__, __LINE__, "IOCTL SIOCGIFMTU failed");
			return -1;
		}

		ret = if2.ifr_mtu;
	}

	ENetWrapper::close(sock);

	return ret;
#endif
}

boolean ENetworkInterface::isVirtual() {
	return isvirtual;
}

boolean ENetworkInterface::equals(EObject* obj) {
	ENetworkInterface* that = dynamic_cast<ENetworkInterface*>(obj);
	if (!that) {
		return false;
	}
	if (!this->name.isEmpty()) {
		if (!this->name.equals(that->name)) {
			return false;
		}
	} else {
		if (!that->name.isEmpty()) {
			return false;
		}
	}

	if (this->addrs == null) {
		return that->addrs == null;
	} else if (that->addrs == null) {
		return false;
	}

	/* Both addrs not null. Compare number of addresses */

	if (this->addrs->length() != that->addrs->length()) {
		return false;
	}

	EA<EInetAddress*>* thatAddrs = that->addrs;
	int count = thatAddrs->length();

	for (int i=0; i<count; i++) {
		boolean found = false;
		for (int j=0; j<count; j++) {
			if ((*addrs)[i]->equals((*thatAddrs)[j])) {
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

int ENetworkInterface::hashCode() {
	return name.isEmpty() ? 0: name.hashCode();
}

EString ENetworkInterface::toString() {
	EString result = "name:";
	result += name.isEmpty()? "null": name;
	if (!displayName.isEmpty()) {
		result += " (" + displayName + ")";
	}
	return result;
}

sp<ENetworkInterface> ENetworkInterface::getByName(const char* name) {
	if (name == null)
		throw ENullPointerException(__FILE__, __LINE__);

	//@see: return getByName0(name);
	{
		netif *ifs, *curr;
		ENetworkInterface* obj;

		ifs = enumInterfaces();
		if (ifs == NULL) {
			return null;
		}

		/*
		 * Search the list of interface based on name
		 */
		curr = ifs;
		while (curr != NULL) {
			if (eso_strcmp(name, curr->name) == 0) {
				break;
			}
			curr = curr->next;
		}

		/* if found create a NetworkInterface */
		if (curr != NULL) {
			obj = createNetworkInterface(curr, -1, NULL);
		}

		/* release the interface list */
		freeif(ifs);

		return obj;
	}
}

sp<ENetworkInterface> ENetworkInterface::getByIndex(int index) {
	if (index < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Interface index can't be negative");
	//@see: return getByIndex0(index);
	{
		netif *ifs, *curr;
		ENetworkInterface* obj = NULL;

		if (index <= 0) {
			return null;
		}

		ifs = enumInterfaces();
		if (ifs == NULL) {
			return null;
		}

		/*
		 * Search the list of interface based on index
		 */
		curr = ifs;
		while (curr != NULL) {
			if (index == curr->index) {
				break;
			}
			curr = curr->next;
		}

		/* if found create a NetworkInterface */
		if (curr != NULL) {;
			obj = createNetworkInterface(curr, -1, NULL);
		}

		freeif(ifs);
		return obj;
	}
}

sp<ENetworkInterface> ENetworkInterface::getByInetAddress(EInetAddress* addr) {
	if (addr == null) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	//@see: return getByInetAddress0(addr);
#ifdef WIN32
	netif *ifList, *curr;
	ENetworkInterface* netifObj = null;

	// Retained for now to support IPv4 only stack, java.net.preferIPv4Stack
	//...

	/* get the list of interfaces */
	ifList = enumInterfaces();
	if (ifList == NULL) {
		return null;
	}

	/*
	 * Enumerate the addresses on each interface until we find a
	 * matching address.
	 */
	curr = ifList;
	while (curr != NULL) {
		int count;
		netaddr *addrList;
		netaddr *addrP;

		/* enumerate the addresses on this interface */
		count = enumAddresses_win(curr, &addrList);
		if (count < 0) {
			freeif(ifList);
			return null;
		}

		/* iterate through each address */
		addrP = addrList;

		while (addrP != NULL) {
			int address1 = addrP->addr.him4.sin_addr.s_addr;
			int address2 = addr->getAddress();

			if (address1 == address2) {
				break;
			}
			addrP = addrP->next;
		}

		/*
		 * Address matched so create NetworkInterface for this interface
		 * and address list.
		 */
		if (addrP != NULL) {
			/* createNetworkInterface will free addrList */
			netifObj = createNetworkInterface(curr, count, addrList);
			break;
		}

		/* on next interface */
		curr = curr->next;
	}

	/* release the interface list */
	freeif(ifList);

	return netifObj;
#else
	{
		ENetworkInterface* obj = null;

		netif *ifs, *curr;
		int family =  AF_INET;
		boolean match = false;

		ifs = enumInterfaces();
		if (ifs == NULL) {
			return null;
		}

		curr = ifs;
		while (curr != NULL) {
			netaddr *addrP = curr->addr;

			/*
			 * Iterate through each address on the interface
			 */
			while (addrP != NULL) {
				if (family == addrP->family) {
					if (family == AF_INET) {
						int address1 = ((struct sockaddr_in*)addrP->addr)->sin_addr.s_addr;
						int address2 = addr->getAddress();

						if (address1 == address2) {
							match = true;
							break;
						}
					}
				}

				if (match) {
					break;
				}
				addrP = addrP->next;
			}

			if (match) {
				break;
			}
			curr = curr->next;
		}

		/* if found create a NetworkInterface */
		if (match) {;
			obj = createNetworkInterface(curr, -1, NULL);
		}

		freeif(ifs);
		return obj;
	}
#endif
}

sp<EEnumeration<ENetworkInterface*> > ENetworkInterface::getNetworkInterfaces() {
	netif *ifs, *curr;
	int arr_index, ifCount;

	ifs = enumInterfaces();
	if (ifs == NULL) {
		return null;
	}

	/* count the interface */
	ifCount = 0;
	curr = ifs;
	while (curr != NULL) {
		ifCount++;
		curr = curr->next;
	}

	/* allocate a NetworkInterface array */
	sp<EA<ENetworkInterface*> > netIFArr = new EA<ENetworkInterface*>(ifCount);

	/*
	 * Iterate through the interfaces, create a NetworkInterface instance
	 * for each array element and populate the object.
	 */
	curr = ifs;
	arr_index = 0;
	while (curr != NULL) {
		ENetworkInterface* netifObj;

		netifObj = createNetworkInterface(curr, -1, NULL);
		if (netifObj == NULL) {
			freeif(ifs);
			return null;
		}

		/* put the NetworkInterface into the array */
		netIFArr->setAt(arr_index++, netifObj);

		curr = curr->next;
	}

	freeif(ifs);

	class Enumeration : public EEnumeration<ENetworkInterface*> {
	private:
		sp<EA<ENetworkInterface*> > ifarr;
		int i;
	public:
		Enumeration(sp<EA<ENetworkInterface*> >& ifarr) : i(0) {
			this->ifarr = ifarr;
		}
		virtual ENetworkInterface* nextElement() {
			if (ifarr != null && i < ifarr->length()) {
				ENetworkInterface* netif = ifarr->getAt(i++);
				return netif;
			} else {
				throw ENoSuchElementException(__FILE__, __LINE__);
			}
		}

		virtual boolean hasMoreElements() {
			return (ifarr != null && i < ifarr->length());
		}
	};

	return new Enumeration(netIFArr);
}

ENetworkInterface* ENetworkInterface::getDefault() {
#ifdef __APPLE__
	//@see: openjdk-8/src/macosx/classes/java/net/DefaultInterface.java
	//TODO...
	return null;
#else
	return null;
#endif
}

#ifdef WIN32
ENetworkInterface* ENetworkInterface::createNetworkInterface(void *netifP, int netaddrCount, void *netaddrP) {
	netif* ifs = (netif*) (netifP);
	netaddr *addrP = (netaddr*) (netaddrP);
	int addr_index;
	int bind_index;

	ENetworkInterface* netifObj = new ENetworkInterface();

	/*
	 * Create a NetworkInterface object and populate it
	 */
	netifObj->name = ifs->name;
	if (ifs->dNameIsUnicode) {
		es_string_t* utf8_str = NULL;
		eso_ucs2_to_utf8_string(&utf8_str, (char *)ifs->displayName);
		netifObj->displayName = utf8_str;
		eso_mfree(utf8_str);
	} else {
		netifObj->displayName = ifs->displayName;
	}
	netifObj->index = ifs->index;

	/*
	 * Get the IP addresses for this interface if necessary
	 * Note that 0 is a valid number of addresses.
	 */
	if (netaddrCount < 0) {
		netaddrCount = enumAddresses_win(ifs, &addrP);
		if (netaddrCount == -1) {
			delete netifObj;
			return NULL;
		}
	}
	netifObj->addrs = new EA<EInetAddress*>(netaddrCount);
	netifObj->bindings = new EA<EInterfaceAddress*>(netaddrCount);

	addr_index = 0;
	bind_index = 0;
	while (addrP != NULL) {
		if (addrP->addr.him.sa_family == AF_INET) {
			int addr = addrP->addr.him4.sin_addr.s_addr;
			netifObj->addrs->setAt(addr_index++, new EInetAddress(NULL, addr));
			if (addrP->mask != -1) {
				netifObj->bindings->setAt(bind_index++, new EInterfaceAddress(new EInetAddress(NULL, addr),
						new EInetAddress(NULL, addrP->brdcast.him4.sin_addr.s_addr),
						addrP->mask));
			}
		}

		addrP = addrP->next;
	}

	free_netaddr(addrP);

	/*
	 * Windows doesn't have virtual interfaces, so child array
	 * is always empty.
	 */
	netifObj->childs = new EA<ENetworkInterface*>(0);

	/* return the NetworkInterface */
	return netifObj;
}
#else //!
ENetworkInterface* ENetworkInterface::createNetworkInterface(void *netifP, int ignore1, void* ignore2) {
	netif* ifs = (netif*) (netifP);

	ENetworkInterface* netifObj = new ENetworkInterface();

	/*
	 * Create a NetworkInterface object and populate it
	 */
	netifObj->name = ifs->name;
	netifObj->displayName = ifs->name;
	netifObj->index = ifs->index;
	netifObj->isvirtual = ifs->isvirtual ? true : false;

	/*
	 * Count the number of address on this interface
	 */
    netaddr *addrs;
	int addr_index, addr_count, bind_index;
	int child_count, child_index;
	netaddr *addrP;
	netif *childP;

	addr_count = 0;
	addrP = ifs->addr;
	while (addrP != NULL) {
		addr_count++;
		addrP = addrP->next;
	}

    /*
     * Create the array of InetAddresses
     */
	netifObj->addrs = new EA<EInetAddress*>(addr_count);
	netifObj->bindings = new EA<EInterfaceAddress*>(addr_count);
    addrP = ifs->addr;
    addr_index = 0;
    bind_index = 0;
    while (addrP != NULL) {
        if (addrP->family == AF_INET) {
        	int addr = ((struct sockaddr_in*)addrP->addr)->sin_addr.s_addr;
        	netifObj->addrs->setAt(addr_index++, new EInetAddress(NULL, addr));
        	netifObj->bindings->setAt(bind_index++, new EInterfaceAddress(new EInetAddress(NULL, addr),
        			addrP->brdcast ? new EInetAddress(NULL, ((struct sockaddr_in*)addrP->brdcast)->sin_addr.s_addr) : null,
        			addrP->mask));
        }

        addrP = addrP->next;
    }

    /*
     * See if there is any virtual interface attached to this one.
     */
    child_count = 0;
    childP = ifs->childs;
    while (childP) {
        child_count++;
        childP = childP->next;
    }

    netifObj->childs = new EA<ENetworkInterface*>(child_count);

    /*
     * Create the NetworkInterface instances for the sub-interfaces as
     * well.
     */
    child_index = 0;
    childP = ifs->childs;
	while (childP) {
		ENetworkInterface* tmp = createNetworkInterface(childP, -1, NULL);
		tmp->parent = netifObj;
		netifObj->childs->setAt(child_index++, tmp);
		childP = childP->next;
	}

    /* return the NetworkInterface */
    return netifObj;
}
#endif

} /* namespace efc */
