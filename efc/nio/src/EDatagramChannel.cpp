/*
 * EDatagramChannel.cpp
 *
 *  Created on: 2016-8-24
 *      Author: cxxjava@163.com
 */

#include "../inc/EDatagramChannel.hh"
#include "../../inc/ENetWrapper.hh"
#include "../inc/ESelectionKey.hh"
#include "./ENativeThread.hh"
#include "./EDatagramDispatcher.hh"
#include "./EDatagramSocketAdaptor.hh"
#include "./EMembershipRegistry.hh"
#include "../../inc/ENullPointerException.hh"
#include "../../inc/EAlreadyBoundException.hh"
#include "../inc/EClosedChannelException.hh"
#include "../../inc/EPortUnreachableException.hh"
#include "../../inc/ENotYetConnectedException.hh"
#include "../../inc/EIndexOutOfBoundsException.hh"
#include "../../inc/EDatagramSocket.hh"

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#else //!
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#endif //!WIN32

namespace efc {
namespace nio {

//@see: openjdk-8/src/share/classes/java/nio/channels/DatagramChannel.java
//@see: openjdk-8/src/share/classes/sun/nio/ch/DatagramChannelImpl.java
//@see: openjdk-8/src/windows/native/sun/nio/ch/DatagramChannelImpl.c
//@see: openjdk-8/src/solaris/native/sun/nio/ch/DatagramChannelImpl.c

// State (does not necessarily increase monotonically)
#define ST_UNINITIALIZED	-1
#define ST_UNCONNECTED		0
#define ST_CONNECTED		1
#define ST_KILLED			2

/**
 * IP_MULTICAST_ALL supported since 2.6.31 but may not be available at
 * build time.
 */
#ifdef __linux__
  #ifndef IP_MULTICAST_ALL
  #define IP_MULTICAST_ALL    49
  #endif
#endif

#ifdef WIN32

typedef struct my_ip_mreq_source {
    IN_ADDR imr_multiaddr;
    IN_ADDR imr_sourceaddr;
    IN_ADDR imr_interface;
};

typedef struct my_group_source_req {
    ULONG gsr_interface;
    SOCKADDR_STORAGE gsr_group;
    SOCKADDR_STORAGE gsr_source;
};

#else //!

#if defined(__bsd__) || defined(_AIX)

#ifndef IP_BLOCK_SOURCE

#if defined(_AIX)

#define IP_BLOCK_SOURCE                 58   /* Block data from a given source to a given group */
#define IP_UNBLOCK_SOURCE               59   /* Unblock data from a given source to a given group */
#define IP_ADD_SOURCE_MEMBERSHIP        60   /* Join a source-specific group */
#define IP_DROP_SOURCE_MEMBERSHIP       61   /* Leave a source-specific group */

#else

#define IP_ADD_SOURCE_MEMBERSHIP        70   /* join a source-specific group */
#define IP_DROP_SOURCE_MEMBERSHIP       71   /* drop a single source */
#define IP_BLOCK_SOURCE                 72   /* block a source */
#define IP_UNBLOCK_SOURCE               73   /* unblock a source */

#endif /* _AIX */

#endif  /* IP_BLOCK_SOURCE */

#ifndef MCAST_BLOCK_SOURCE

#if defined(_AIX)

#define MCAST_BLOCK_SOURCE              64
#define MCAST_UNBLOCK_SOURCE            65
#define MCAST_JOIN_SOURCE_GROUP         66
#define MCAST_LEAVE_SOURCE_GROUP        67

#else

#define MCAST_JOIN_SOURCE_GROUP         82   /* join a source-specific group */
#define MCAST_LEAVE_SOURCE_GROUP        83   /* leave a single source */
#define MCAST_BLOCK_SOURCE              84   /* block a source */
#define MCAST_UNBLOCK_SOURCE            85   /* unblock a source */

#endif /* _AIX */

#endif /* MCAST_BLOCK_SOURCE */

#if defined(_AIX)

struct my_ip_mreq_source {
        struct in_addr  imr_multiaddr;
        struct in_addr  imr_sourceaddr;
        struct in_addr  imr_interface;
};

#else

struct my_ip_mreq_source {
        struct in_addr  imr_multiaddr;
        struct in_addr  imr_interface;
        struct in_addr  imr_sourceaddr;
};

#endif /* _AIX */

struct my_group_source_req {
        uint32_t                gsr_interface;  /* interface index */
        struct sockaddr_storage gsr_group;      /* group address */
        struct sockaddr_storage gsr_source;     /* source address */
};

#else   /* __bsd__ */

#define my_ip_mreq_source         ip_mreq_source
#define my_group_source_req       group_source_req

#endif //!__bsd__

#endif //!WIN32

static int joinOrDrop4(boolean join, int fd, int group, int interf, int source) THROWS(EIOException) {
#ifdef WIN32
	struct ip_mreq mreq;
	struct my_ip_mreq_source mreq_source;
	int opt, n, optlen;
	void* optval;

	if (source == 0) {
		mreq.imr_multiaddr.s_addr = htonl(group);
		mreq.imr_interface.s_addr = htonl(interf);
		opt = (join) ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
		optval = (void*)&mreq;
		optlen = sizeof(mreq);
	} else {
		mreq_source.imr_multiaddr.s_addr = htonl(group);
		mreq_source.imr_sourceaddr.s_addr = htonl(source);
		mreq_source.imr_interface.s_addr = htonl(interf);
		opt = (join) ? IP_ADD_SOURCE_MEMBERSHIP : IP_DROP_SOURCE_MEMBERSHIP;
		optval = (void*)&mreq_source;
		optlen = sizeof(mreq_source);
	}

	n = ::setsockopt(fd, IPPROTO_IP, opt, (const char*)optval, optlen);
	if (n < 0) {
		if (join && (WSAGetLastError() == WSAENOPROTOOPT))
			return EIOStatus::UNAVAILABLE;
		ENetWrapper::handleSocketError(WSAGetLastError());
	}
	return 0;
#else //!
	struct ip_mreq mreq;
	struct my_ip_mreq_source mreq_source;
	int opt, n, optlen;
	void* optval;

	if (source == 0) {
		mreq.imr_multiaddr.s_addr = htonl(group);
		mreq.imr_interface.s_addr = htonl(interf);
		opt = (join) ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP;
		optval = (void*)&mreq;
		optlen = sizeof(mreq);
	} else {
#ifdef MACOSX
		/* no IPv4 include-mode filtering for now */
		return EIOStatus::UNAVAILABLE;
#else

#ifdef _AIX
		/* check AIX for support of source filtering */
		/* TODO:
		if (isSourceFilterSupported() != JNI_TRUE){
			return EIOStatus::UNAVAILABLE;
		}
		*/
#endif

		mreq_source.imr_multiaddr.s_addr = htonl(group);
		mreq_source.imr_sourceaddr.s_addr = htonl(source);
		mreq_source.imr_interface.s_addr = htonl(interf);
		opt = (join) ? IP_ADD_SOURCE_MEMBERSHIP : IP_DROP_SOURCE_MEMBERSHIP;
		optval = (void*)&mreq_source;
		optlen = sizeof(mreq_source);
#endif
	}

	n = setsockopt(fd, IPPROTO_IP, opt, optval, optlen);
	if (n < 0) {
		if (join && (errno == ENOPROTOOPT || errno == EOPNOTSUPP))
			return EIOStatus::UNAVAILABLE;
		ENetWrapper::handleSocketError(errno);
	}
	return 0;
#endif //!WIN32
}

static int blockOrUnblock4(boolean block, int fd, int group, int interf, int source) THROWS(EIOException) {
#ifdef WIN32
	struct my_ip_mreq_source mreq_source;
	int n;
	int opt = (block) ? IP_BLOCK_SOURCE : IP_UNBLOCK_SOURCE;

	mreq_source.imr_multiaddr.s_addr = htonl(group);
	mreq_source.imr_sourceaddr.s_addr = htonl(source);
	mreq_source.imr_interface.s_addr = htonl(interf);

	n = ::setsockopt(fd, IPPROTO_IP, opt,
				(const char*)&mreq_source, sizeof(mreq_source));
	if (n < 0) {
		if (block && (WSAGetLastError() == WSAENOPROTOOPT))
			return EIOStatus::UNAVAILABLE;
		ENetWrapper::handleSocketError(WSAGetLastError());
	}
	return 0;
#else //!
#ifdef MACOSX
    /* no IPv4 exclude-mode filtering for now */
    return EIOStatus::UNAVAILABLE;
#else
    struct my_ip_mreq_source mreq_source;
    int n;
    int opt = (block) ? IP_BLOCK_SOURCE : IP_UNBLOCK_SOURCE;

#ifdef _AIX
    /* check AIX for support of source filtering */
    /* TODO:
    if (isSourceFilterSupported() != JNI_TRUE){
        return IOS_UNAVAILABLE;
    }
    */
#endif

    mreq_source.imr_multiaddr.s_addr = htonl(group);
    mreq_source.imr_sourceaddr.s_addr = htonl(source);
    mreq_source.imr_interface.s_addr = htonl(interf);

    n = setsockopt(fd, IPPROTO_IP, opt,
                   (void*)&mreq_source, sizeof(mreq_source));
    if (n < 0) {
        if (block && (errno == ENOPROTOOPT || errno == EOPNOTSUPP))
            return EIOStatus::UNAVAILABLE;
        ENetWrapper::handleSocketError(errno);
    }
    return 0;
#endif
#endif //!WIN32
}

EDatagramChannel::~EDatagramChannel() {
	delete _registry;
	delete _socket;
}

EDatagramChannel::EDatagramChannel():
		_readerThread(0),
		_writerThread(0),
		_state(ST_UNINITIALIZED),
		_socket(null),
		_registry(null),
		_sender() {

	//@see: openjdk-8/src/solaris/native/sun/nio/ch/Net.c#L233

	_fd = ENetWrapper::socket(PF_INET, SOCK_DGRAM, 0);

#if defined(__linux__)
    {
        int arg = 0;
        int level = IPPROTO_IP;
        if ((::setsockopt(_fd, level, IP_MULTICAST_ALL, (char*)&arg, sizeof(arg)) < 0) &&
            (errno != ENOPROTOOPT)) {
        	::close(_fd);
            throw ESocketException(__FILE__, __LINE__, "Unable to set IP_MULTICAST_ALL");
        }
    }
#endif

    this->_state = ST_UNCONNECTED;
}

EDatagramChannel* EDatagramChannel::open() {
	return new EDatagramChannel();
}

int EDatagramChannel::validOps() {
	return (ESelectionKey::OP_READ
			| ESelectionKey::OP_WRITE);
}

EDatagramChannel* EDatagramChannel::bind(EInetSocketAddress* local) {
	SYNCBLOCK(&_readLock) {
		SYNCBLOCK(&_writeLock) {
			SYNCBLOCK(&_stateLock) {
				ensureOpen();
				if (_localAddress != null)
					throw EAlreadyBoundException(__FILE__, __LINE__);
				/* @see:
				InetSocketAddress isa;
				if (local == null) {
					// only Inet4Address allowed with IPv4 socket
					if (family == StandardProtocolFamily.INET) {
						isa = new InetSocketAddress(InetAddress.getByName("0.0.0.0"), 0);
					} else {
						isa = new InetSocketAddress(0);
					}
				} else {
					isa = Net.checkAddress(local);

					// only Inet4Address allowed with IPv4 socket
					if (family == StandardProtocolFamily.INET) {
						InetAddress addr = isa.getAddress();
						if (!(addr instanceof Inet4Address))
							throw new UnsupportedAddressTypeException();
					}
				}
				SecurityManager sm = System.getSecurityManager();
				if (sm != null) {
					sm.checkListen(isa.getPort());
				}
				Net.bind(family, fd, isa.getAddress(), isa.getPort());
				localAddress = Net.localAddress(fd);
				*/
				ENetWrapper::checkAddress(local);
				ENetWrapper::bind(_fd, local?local->getAddress():null, local?local->getPort():0);
				_localAddress = ENetWrapper::SocketToInetSocketAddress(_fd);
            }}
        }}
    }}
	return this;
}

EDatagramChannel* EDatagramChannel::bind(const char* hostname, int port) {
	EInetSocketAddress isa(hostname, port);
	return this->bind(&isa);
}

EDatagramSocket* EDatagramChannel::socket() {
	SYNCBLOCK(&_stateLock) {
		if (_socket == null)
			_socket = EDatagramSocketAdaptor::create(this);
		return _socket;
    }}
}

boolean EDatagramChannel::isConnected() {
	SYNCBLOCK(&_stateLock) {
		return (_state == ST_CONNECTED);
    }}
}

EDatagramChannel* EDatagramChannel::connect(EInetSocketAddress* remote) {
	SYNCBLOCK(&_readLock) {
		SYNCBLOCK(&_writeLock) {
			SYNCBLOCK(&_stateLock) {
				ensureOpenAndUnconnected();
				/*
				EInetSocketAddress* isa = Net.checkAddress(sa);
				SecurityManager sm = System.getSecurityManager();
				if (sm != null)
					sm.checkConnect(isa.getAddress().getHostAddress(),
									isa.getPort());
				*/
				ENetWrapper::checkAddress(remote);
				int n = ENetWrapper::connect(_fd,
									remote->getAddress(),
									remote->getPort(),
									0);
				if (n <= 0)
					throw ERuntimeException(__FILE__, __LINE__);      // Can't happen

				// Connection succeeded; disallow further invocation
				_state = ST_CONNECTED;
				_remoteAddress = new EInetSocketAddress(*remote);
				_sender.addr = remote->getAddress()->getAddress();
				_sender.port = remote->getPort();

				// set or refresh local address
				_localAddress = ENetWrapper::SocketToInetSocketAddress(_fd);
            }}
        }}
    }}
	return this;
}

EDatagramChannel* EDatagramChannel::connect(const char* hostname, int port) {
	EInetSocketAddress isa(hostname, port);
	return this->connect(&isa);
}

EDatagramChannel* EDatagramChannel::disconnect() {
	SYNCBLOCK(&_readLock) {
		SYNCBLOCK(&_writeLock) {
			SYNCBLOCK(&_stateLock) {
				if (!isConnected() || !isOpen())
					return this;
				/* @see:
				InetSocketAddress isa = remoteAddress;
				SecurityManager sm = System.getSecurityManager();
				if (sm != null)
					sm.checkConnect(isa.getAddress().getHostAddress(),
									isa.getPort());
				boolean isIPv6 = (family == StandardProtocolFamily.INET6);
				disconnect0(fd, isIPv6);
				*/
				{
#ifdef WIN32
					//@see: openjdk-8/src/windows/native/sun/nio/ch/DatagramChannelImpl.c#L105

					int rv = 0;
					union sockaddr_union sa;
					int sa_len = SIZEOF_SOCKADDR(sa);

					memset(&sa, 0, sa_len);

					rv = ::connect(_fd, (struct sockaddr *)&sa, sa_len);
					if (rv == SOCKET_ERROR) {
						ENetWrapper::handleSocketError(WSAGetLastError());
					} else {
						/* Disable WSAECONNRESET errors as socket is no longer connected */
						BOOL enable = FALSE;
						DWORD bytesReturned = 0;
						WSAIoctl((SOCKET)_fd, SIO_UDP_CONNRESET, &enable, sizeof(enable),
								 NULL, 0, &bytesReturned, NULL, NULL);
					}
#else //!
					//@see: openjdk-8/src/solaris/native/sun/nio/ch/DatagramChannelImpl.c#L74

					int rv;

				#ifdef __solaris__
					rv = ::connect(_fd, 0, 0);
				#endif

				#if defined(__linux__) || defined(__bsd__) || defined(_AIX)
					{
						int len;
						union sockaddr_union sa;

						memset(&sa, 0, sizeof(sa));

						{
							struct sockaddr_in *him4 = (struct sockaddr_in*)&sa;
				#if defined(__bsd__)
							him4->sin_family = AF_INET;
				#else
							him4->sin_family = AF_UNSPEC;
				#endif
							len = sizeof(struct sockaddr_in);
						}

						rv = ::connect(_fd, (struct sockaddr *)&sa, len);

				#if defined(__bsd__)
						if (rv < 0 && errno == EADDRNOTAVAIL)
								rv = errno = 0;
				#endif
				#if defined(_AIX)
						/* See W. Richard Stevens, "UNIX Network Programming, Volume 1", p. 254:
						 * 'Setting the address family to AF_UNSPEC might return EAFNOSUPPORT
						 * but that is acceptable.
						 */
						if (rv < 0 && errno == EAFNOSUPPORT)
								rv = errno = 0;
				#endif
					}
				#endif

					if (rv < 0)
						ENetWrapper::handleSocketError(errno);
#endif //!WIN32
				}

				_remoteAddress = null;
				_state = ST_UNCONNECTED;

				// refresh local address
				_localAddress = ENetWrapper::SocketToInetSocketAddress(_fd);
            }}
        }}
    }}
	return this;
}

EInetSocketAddress* EDatagramChannel::getRemoteAddress() {
	SYNCBLOCK(&_stateLock) {
		if (!isOpen())
			throw EClosedChannelException(__FILE__, __LINE__);
		return _remoteAddress.get();
    }}
}

sp<EInetSocketAddress> EDatagramChannel::receive(EIOByteBuffer* dst) {
	if (dst->isReadOnly())
		throw EIllegalArgumentException(__FILE__, __LINE__, "Read-only buffer");
	if (dst == null)
		throw ENullPointerException(__FILE__, __LINE__);
	SYNCBLOCK(&_readLock) {
		ensureOpen();
		// Socket was not bound before attempting receive
		if (_localAddress == null)
			bind(null);
		int n = 0;
		EInetSocketAddress* isa = null;
		try {
			begin();
			if (!isOpen()) {
				//@see: return null;
				isa = null;
				goto FINALLY;
			}
			_readerThread = ENativeThread::current();
			//@see: if (isConnected() || (security == null)) {
			{
				do {
					//@see: n = receive(fd, dst);
					{
						//@see: openjdk-8/src/share/classes/sun/nio/ch/DatagramChannelImpl.java#L399
						int pos = dst->position();
						int lim = dst->limit();
						ES_ASSERT (pos <= lim);
						int rem = (pos <= lim ? lim - pos : 0);
						/* @see:
						if (dst instanceof DirectBuffer && rem > 0)
							return receiveIntoNativeBuffer(fd, dst, rem, pos);

						// Substitute a native buffer. If the supplied buffer is empty
						// we must instead use a nonempty buffer, otherwise the call
						// will not block waiting for a datagram on some platforms.
						int newSize = Math.max(rem, 1);
						ByteBuffer bb = Util.getTemporaryDirectBuffer(newSize);
						try {
							int n = receiveIntoNativeBuffer(fd, bb, newSize, 0);
							bb.flip();
							if (n > 0 && rem > 0)
								dst.put(bb);
							return n;
						} finally {
							Util.releaseTemporaryDirectBuffer(bb);
						}
						*/
						int raddr;
						int rport;
						if (rem > 0) {
							n = EDatagramDispatcher::recvfrom(_fd, dst->current(), rem, isConnected(), &raddr, &rport);
							if (n > 0) {
								dst->position(pos + n);
							}
						} else {
							byte buf[1];
							n = EDatagramDispatcher::recvfrom(_fd, buf, 1, isConnected(), &raddr, &rport);
						}
						if (n >= 0) {
							//@see: openjdk-8/src/solaris/native/sun/nio/ch/DatagramChannelImpl.c#L176
							/*
							 * If the source address and port match the cached address
							 * and port in DatagramChannelImpl then we don't need to
							 * create InetAddress and InetSocketAddress objects.
							 */
							_sender.addr = raddr;
							_sender.port = rport;
						}
					}
				} while ((n == EIOStatus::INTERRUPTED) && isOpen());
				if (n == EIOStatus::UNAVAILABLE) {
					//@see: return null;
					isa = null;
					goto FINALLY;
				}
			}
			//@see: return sender;
			isa = new EInetSocketAddress(_sender.addr, _sender.port);
		} catch (...) {
			finally {
				_readerThread = 0;
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			_readerThread = 0;
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			ES_ASSERT(EIOStatus::check(n));
		}
		return isa;
    }}
}

int EDatagramChannel::send(EIOByteBuffer* src, EInetSocketAddress* target) {
	if (src == null)
		throw ENullPointerException(__FILE__, __LINE__);

	SYNCBLOCK(&_writeLock) {
		ensureOpen();
		ENetWrapper::checkAddress(target);
		EInetAddress* ia = target->getAddress();
		if (ia == null)
			throw EIOException(__FILE__, __LINE__, "Target address not resolved");
		SYNCBLOCK(&_stateLock) {
			if (!isConnected()) {
				if (target == null)
					throw ENullPointerException(__FILE__, __LINE__);
				/*
				SecurityManager sm = System.getSecurityManager();
				if (sm != null) {
					if (ia.isMulticastAddress()) {
						sm.checkMulticast(ia);
					} else {
						sm.checkConnect(ia.getHostAddress(),
										isa.getPort());
					}
				}
				*/
			} else { // Connected case; Check address then write
				if (!target->equals(_remoteAddress.get())) {
					throw EIllegalArgumentException(__FILE__, __LINE__,
						"Connected address not equal to target address");
				}
				return this->write(src);
			}
        }}

		int r, n = 0;
		try {
			begin();
			if (!isOpen()) {
				r = 0; //return 0;
				goto FINALLY;
			}
			_writerThread = ENativeThread::current();
			do {
				//@see: n = send(fd, src, isa);
				{
					//@see: sendFromNativeBuffer
					int pos = src->position();
					int lim = src->limit();
					ES_ASSERT (pos <= lim);
					int rem = (pos <= lim ? lim - pos : 0);

					int written;
					try {
						written = EDatagramDispatcher::sendto(_fd, (byte*)src->address() + pos,
										rem, target->getAddress()->getAddress(), target->getPort());
					} catch (EPortUnreachableException& pue) {
						if (isConnected())
							throw pue;
						written = rem;
					}
					if (written > 0)
						src->position(pos + written);
					n = written;
				}
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());

			SYNCBLOCK(&_stateLock) {
				if (isOpen() && (_localAddress == null)) {
					_localAddress = ENetWrapper::SocketToInetSocketAddress(_fd);
				}
            }}
			r = EIOStatus::normalize(n);
		} catch (...) {
			finally {
				_writerThread = 0;
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			_writerThread = 0;
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			ES_ASSERT(EIOStatus::check(n));
		}
		return r;
    }}
}

int EDatagramChannel::send(EIOByteBuffer* src, const char* hostname, int port) {
	EInetSocketAddress isa(hostname, port);
	return this->send(src, &isa);
}

int EDatagramChannel::read(EIOByteBuffer* buf) {
	if (buf == null)
		throw ENullPointerException(__FILE__, __LINE__);
	SYNCBLOCK(&_readLock) {
		SYNCBLOCK(&_stateLock) {
			ensureOpen();
			if (!isConnected())
				throw ENotYetConnectedException(__FILE__, __LINE__);
        }}
		int n = 0;
		int rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			_readerThread = ENativeThread::current();
			do {
				//@see: n = IOUtil.read(fd, buf, -1, nd);
				n = EDatagramDispatcher::read(_fd, buf);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch (...) {
			finally {
				_readerThread = 0;
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				ES_ASSERT (EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			_readerThread = 0;
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			ES_ASSERT (EIOStatus::check(n));
		}
		return rv; //!
    }}
}

long EDatagramChannel::read(EA<EIOByteBuffer*>* dsts) {
	if (dsts == null)
		throw ENullPointerException(__FILE__, __LINE__);
	return read(dsts, 0, dsts->length());
}

long EDatagramChannel::read(EA<EIOByteBuffer*>* dsts, int offset, int length) {
	if ((offset < 0) || (length < 0) || (offset > dsts->length() - length))
		throw EIndexOutOfBoundsException(__FILE__, __LINE__);
	SYNCBLOCK(&_readLock) {
		SYNCBLOCK(&_stateLock) {
			ensureOpen();
			if (!isConnected())
				throw ENotYetConnectedException(__FILE__, __LINE__);
        }}
		long n = 0;
		long rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			_readerThread = ENativeThread::current();
			do {
				//@see: n = IOUtil.read(fd, dsts, offset, length, nd);
				n = EDatagramDispatcher::read(_fd, dsts, offset, length);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch (...) {
			finally {
				_readerThread = 0;
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				ES_ASSERT (EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			_readerThread = 0;
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			ES_ASSERT (EIOStatus::check(n));
		}
		return rv; //!
    }}
}

int EDatagramChannel::write(EIOByteBuffer* buf) {
	if (buf == null)
		throw ENullPointerException(__FILE__, __LINE__);
	SYNCBLOCK(&_writeLock) {
		SYNCBLOCK(&_stateLock) {
			ensureOpen();
			if (!isConnected())
				throw ENotYetConnectedException(__FILE__, __LINE__);
        }}
		int n = 0;
		int rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			_writerThread = ENativeThread::current();
			do {
				//@see: n = IOUtil.write(fd, buf, -1, nd);
				n = EDatagramDispatcher::write(_fd, buf);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch (...) {
			finally {
				_writerThread = 0;
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				ES_ASSERT (EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			_writerThread = 0;
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			ES_ASSERT (EIOStatus::check(n));
		}
		return rv; //!
    }}
}

long EDatagramChannel::write(EA<EIOByteBuffer*>* srcs) {
	if (srcs == null)
		throw ENullPointerException(__FILE__, __LINE__);
	return write(srcs, 0, srcs->length());
}

long EDatagramChannel::write(EA<EIOByteBuffer*>* srcs, int offset, int length) {
	if ((offset < 0) || (length < 0) || (offset > srcs->length() - length))
		throw EIndexOutOfBoundsException(__FILE__, __LINE__);
	SYNCBLOCK(&_writeLock) {
		SYNCBLOCK(&_stateLock) {
			ensureOpen();
			if (!isConnected())
				throw ENotYetConnectedException(__FILE__, __LINE__);
        }}
		long n = 0;
		long rv = 0;
		try {
			begin();
			if (!isOpen()) {
				rv = 0;
				goto FINALLY;
			}
			_writerThread = ENativeThread::current();
			do {
				//@see: n = IOUtil.write(fd, srcs, offset, length, nd);
				n = EDatagramDispatcher::write(_fd, srcs, offset, length);
			} while ((n == EIOStatus::INTERRUPTED) && isOpen());
			rv = EIOStatus::normalize(n);
			goto FINALLY;
		} catch (...) {
			finally {
				_writerThread = 0;
				end((n > 0) || (n == EIOStatus::UNAVAILABLE));
				ES_ASSERT (EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			_writerThread = 0;
			end((n > 0) || (n == EIOStatus::UNAVAILABLE));
			ES_ASSERT (EIOStatus::check(n));
		}
		return rv; //!
    }}
}

EInetSocketAddress* EDatagramChannel::getLocalAddress() {
	SYNCBLOCK(&_stateLock) {
		if (!isOpen())
			throw EClosedChannelException(__FILE__, __LINE__);
		// Perform security check before returning address
		//@see: return Net.getRevealedLocalAddress(localAddress);
		return _localAddress.get();
    }}
}

EMembershipKey* EDatagramChannel::join(EInetAddress* group,
		ENetworkInterface* interf, EInetAddress* source) {
	if (!group->isMulticastAddress())
		throw EIllegalArgumentException(__FILE__, __LINE__, "Group not a multicast address");

	// check multicast address is compatible with this socket
	/* @see:
	if (group instanceof Inet4Address) {
		if (family == StandardProtocolFamily.INET6 && !Net.canIPv6SocketJoinIPv4Group())
			throw new IllegalArgumentException("IPv6 socket cannot join IPv4 multicast group");
	} else if (group instanceof Inet6Address) {
		if (family != StandardProtocolFamily.INET6)
			throw new IllegalArgumentException("Only IPv6 sockets can join IPv6 multicast group");
	} else {
		throw new IllegalArgumentException("Address type not supported");
	}
	*/

	// check source address
	if (source != null) {
		if (source->isAnyLocalAddress())
			throw EIllegalArgumentException(__FILE__, __LINE__, "Source address is a wildcard address");
		if (source->isMulticastAddress())
			throw EIllegalArgumentException(__FILE__, __LINE__, "Source address is multicast address");
		/* @see:
		if (source.getClass() != group.getClass())
			throw new IllegalArgumentException("Source address is different type to group");
		*/
	}

	/* @see:
	SecurityManager sm = System.getSecurityManager();
	if (sm != null)
		sm.checkMulticast(group);
	*/

	SYNCBLOCK(&_stateLock) {
		if (!isOpen())
			throw EClosedChannelException(__FILE__, __LINE__);

		// check the registry to see if we are already a member of the group
		if (_registry == null) {
			_registry = new EMembershipRegistry();
		} else {
			// return existing membership key
			EMembershipKey* key = _registry->checkMembership(group, interf, source);
			if (key != null)
				return key;
		}

		EMembershipKey* key;
		{
			// need IPv4 address to identify interface
			/* @see:
			Inet4Address target = Net.anyInet4Address(interf);
			if (target == null)
				throw new IOException("Network interface not configured for IPv4");
			*/
			sp<EEnumeration<EInetAddress*> > e = interf->getInetAddresses();
			if (e == null || !e->hasMoreElements()) {
				throw EIOException(__FILE__, __LINE__, "Network interface not configured for IPv4");
			}

			int groupAddress = group->getAddress();
			int targetAddress = e->nextElement()->getAddress();
			int sourceAddress = (source == null) ? 0 : source->getAddress();

			// join the group
			int n = joinOrDrop4(true, _fd, groupAddress, targetAddress, sourceAddress);
			if (n == EIOStatus::UNAVAILABLE)
				throw EUnsupportedOperationException(__FILE__, __LINE__);

			key = new EMembershipKey(dynamic_pointer_cast<EMulticastChannel>(shared_from_this()), group, interf, source,
									groupAddress, targetAddress, sourceAddress);
		}

		_registry->add(key);
		return key;
    }}
}

void EDatagramChannel::setOption(int optID, const void* optval, int optlen) {
	SYNCBLOCK(&_stateLock) {
		ensureOpen();
		ENetWrapper::setOption(_fd, optID, (char*)optval, optlen);
    }}
}

void EDatagramChannel::getOption(int optID, void* optval, int* optlen) {
	SYNCBLOCK(&_stateLock) {
		ensureOpen();
		ENetWrapper::getOption(_fd, optID, optval, optlen);
    }}
}

void EDatagramChannel::implConfigureBlocking(boolean block) {
	ENetWrapper::configureBlocking(_fd, block);
}

void EDatagramChannel::implCloseSelectableChannel() {
	SYNCBLOCK(&_stateLock) {
		if (_state != ST_KILLED)
			EDatagramDispatcher::preClose(_fd);
		//@see: ResourceManager.afterUdpClose();

		// if member of mulitcast group then invalidate all keys
		if (_registry != null)
			_registry->invalidateAll();

		es_os_thread_t th;
		if ((th = _readerThread) != 0)
			ENativeThread::signal(th);
		if ((th = _writerThread) != 0)
			ENativeThread::signal(th);
		if (!isRegistered())
			kill();
    }}
}

boolean EDatagramChannel::translateAndUpdateReadyOps(int ops, ESelectionKey* sk) {
	return translateReadyOps(ops, sk->nioReadyOps(), sk);
}

boolean EDatagramChannel::translateAndSetReadyOps(int ops, ESelectionKey* sk) {
	return translateReadyOps(ops, 0, sk);
}

boolean EDatagramChannel::translateReadyOps(int ops, int initialOps, ESelectionKey* sk) {
	int intOps = sk->nioInterestOps(); // Do this just once, it synchronizes
	int oldOps = sk->nioReadyOps();
	int newOps = initialOps;

	if ((ops & ENetWrapper::POLLNVAL_) != 0) {
		// This should only happen if this channel is pre-closed while a
		// selection operation is in progress
		// ## Throw an error if this channel has not been pre-closed
		return false;
	}

	if ((ops & (ENetWrapper::POLLERR_ | ENetWrapper::POLLHUP_)) != 0) {
		newOps = intOps;
		sk->nioReadyOps(newOps);
		return (newOps & ~oldOps) != 0;
	}

	if (((ops & ENetWrapper::POLLIN_) != 0) &&
		((intOps & ESelectionKey::OP_READ) != 0))
		newOps |= ESelectionKey::OP_READ;

	if (((ops & ENetWrapper::POLLOUT_) != 0) &&
		((intOps & ESelectionKey::OP_WRITE) != 0))
		newOps |= ESelectionKey::OP_WRITE;

	sk->nioReadyOps(newOps);
	return (newOps & ~oldOps) != 0;
}

void EDatagramChannel::translateAndSetInterestOps(int ops, ESelectionKey* sk) {
	int newOps = 0;

	if ((ops & ESelectionKey::OP_READ) != 0)
		newOps |= ENetWrapper::POLLIN_;
	if ((ops & ESelectionKey::OP_WRITE) != 0)
		newOps |= ENetWrapper::POLLOUT_;
	if ((ops & ESelectionKey::OP_CONNECT) != 0)
		newOps |= ENetWrapper::POLLIN_;
	sk->selector()->putEventOps(sk, newOps);
}

void EDatagramChannel::kill() {
	SYNCBLOCK(&_stateLock) {
		if (_state == ST_KILLED)
			return;
		if (_state == ST_UNINITIALIZED) {
			_state = ST_KILLED;
			return;
		}
		ES_ASSERT(!isOpen() && !isRegistered());
		EDatagramDispatcher::close(_fd);
		_state = ST_KILLED;
    }}
}

void EDatagramChannel::drop(EMembershipKey* key) {
	ES_ASSERT(key->channel() == this);

	SYNCBLOCK(&_stateLock) {
		if (!key->isValid())
			return;

		try {
			//@see: Net.drop4(fd, key4.groupAddress(), key4.interfaceAddress(),	key4.source());
			joinOrDrop4(false, _fd, key->groupAddress(), key->interfaceAddress(), key->source());
		} catch (EIOException& ioe) {
			// should not happen
			throw ERuntimeException(__FILE__, __LINE__, &ioe);
		}

		key->invalidate();
		_registry->remove(key);
    }}
}

int EDatagramChannel::poll(int events, llong timeout) {
	//@see: assert Thread.holdsLock(blockingLock()) && !isBlocking();
	ES_ASSERT(!isBlocking());

	SYNCBLOCK(&_readLock) {
		int n = 0;
		try {
			begin();
			SYNCBLOCK(&_stateLock) {
				if (!isOpen()) {
					finally {
						_readerThread = 0;
						end(n > 0);
					}
					return 0;
				}
				_readerThread = ENativeThread::current();
            }}
			//@see: n = Net.poll(fd, events, timeout);
			n = eso_net_wait(_fd, events, timeout);
		} catch (...) {
			finally {
				_readerThread = 0;
				end(n > 0);
			}
			throw; //!
		} finally {
			_readerThread = 0;
			end(n > 0);
		}
		return n;
    }}
}

void EDatagramChannel::block(EMembershipKey* key, EInetAddress* source) {
	ES_ASSERT(key->channel() == this);
	ES_ASSERT(key->sourceAddress() == null);

	SYNCBLOCK(&_stateLock) {
		if (!key->isValid())
			throw EIllegalStateException(__FILE__, __LINE__, "key is no longer valid");
		if (source->isAnyLocalAddress())
			throw EIllegalArgumentException(__FILE__, __LINE__, "Source address is a wildcard address");
		if (source->isMulticastAddress())
			throw EIllegalArgumentException(__FILE__, __LINE__, "Source address is multicast address");
		/* @see:
		if (source.getClass() != key.group().getClass())
			throw new IllegalArgumentException("Source address is different type to group");
		*/

		/* @see:
		int n = Net.block4(fd, key4.groupAddress(), key4.interfaceAddress(),
						Net.inet4AsInt(source));
		*/
		int n = blockOrUnblock4(true, _fd, key->groupAddress(), key->interfaceAddress(),
						source->getAddress());
		if (n == EIOStatus::UNAVAILABLE) {
			// ancient kernel
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
    }}
}

void EDatagramChannel::unblock(EMembershipKey* key, EInetAddress* source) {
	ES_ASSERT(key->channel() == this);
	ES_ASSERT(key->sourceAddress() == null);

	SYNCBLOCK(&_stateLock) {
		if (!key->isValid())
			throw EIllegalStateException(__FILE__, __LINE__, "key is no longer valid");

		try {
			/* @see
			Net.unblock4(fd, key4.groupAddress(), key4.interfaceAddress(),
						 Net.inet4AsInt(source));
			*/
			blockOrUnblock4(false, _fd, key->groupAddress(), key->interfaceAddress(),
						source->getAddress());
		} catch (EIOException& ioe) {
			// should not happen
			throw ERuntimeException(__FILE__, __LINE__, &ioe);
		}
    }}
}

void EDatagramChannel::ensureOpen() {
	if (!isOpen())
		throw EClosedChannelException(__FILE__, __LINE__);
}

void EDatagramChannel::ensureOpenAndUnconnected() {
	SYNCBLOCK(&_stateLock) {
		if (!isOpen())
			throw EClosedChannelException(__FILE__, __LINE__);
		if (_state != ST_UNCONNECTED)
			throw EIllegalStateException(__FILE__, __LINE__, "Connect already invoked");
    }}
}

int EDatagramChannel::getFDVal() {
	return _fd;
}

EString EDatagramChannel::toString() {
	EString sb;
	sb << "EDatagramChannel";
	sb << '[';
	if (!isOpen())
		sb << "closed";
	else {
		SYNCBLOCK (&_stateLock) {
			if (getLocalAddress() == null) {
				sb << "unbound";
			} else {
				sb << getLocalAddress()->toString();
			}
        }}
	}
	sb << ']';
	return sb;
}

} /* namespace nio */
} /* namespace efc */
