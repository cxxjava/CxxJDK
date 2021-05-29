/*
 * EServerSocketChannel.cpp
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#include "../inc/EServerSocketChannel.hh"
#include "./EServerSocketAdaptor.hh"
#include "./ESocketDispatcher.hh"
#include "./ENativeThread.hh"
#include "../../inc/EIOStatus.hh"
#include "../../inc/ENetWrapper.hh"
#include "../../inc/EServerSocket.hh"

namespace efc {
namespace nio {

EServerSocketChannel::~EServerSocketChannel() {
	delete _localAddress;
	delete _socket;
}

EServerSocketChannel::EServerSocketChannel() :
		_thread(0), _socket(null), _localAddress(null), _state(ST_UNINITIALIZED) {
	_fd =  ENetWrapper::socket(PF_INET, SOCK_STREAM, 0);
	int arg = 1;
	ENetWrapper::setOption(_fd, ESocketOptions::_SO_REUSEADDR, (char*)&arg, sizeof(arg));
	_state = ST_INUSE;
}

/**
 * @see:
 * openjdk-6-src-b27-26_oct_2012.tar/jdk/src/share/classes/sun/nio/ch/SelectorProviderImpl.java=>openServerSocketChannel
 */
EServerSocketChannel* EServerSocketChannel::open() {
	return new EServerSocketChannel();
}

int EServerSocketChannel::validOps() {
	return ESelectionKey::OP_ACCEPT;
}

EServerSocket* EServerSocketChannel::socket() {
	SYNCBLOCK(&_stateLock) {
		if (_socket == null)
			_socket = EServerSocketAdaptor::create(this);
		return _socket;
    }}
}

boolean EServerSocketChannel::isBound() {
	SYNCBLOCK(&_stateLock) {
		return _localAddress != null;
    }}
}

EInetSocketAddress* EServerSocketChannel::localAddress() {
	SYNCBLOCK(&_stateLock) {
		return _localAddress;
    }}
}

void EServerSocketChannel::bind(EInetSocketAddress* local, int backlog) {
	ENetWrapper::checkAddress(local);
	SYNCBLOCK(&_lock) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		if (isBound())
			throw EALREADYBOUNDEXCEPTION;
		//SecurityManager sm = System.getSecurityManager();
		//if (sm != null)
		//    sm.checkListen(isa.getPort());
		ENetWrapper::bind(_fd, local->getAddress(), local->getPort());
		ENetWrapper::listen(_fd, backlog < 1 ? 50 : backlog);
		SYNCBLOCK (&_stateLock) {
			if (_localAddress) {
				delete _localAddress;
			}
            int laddr, lport;
            ENetWrapper::localAddress(_fd, &laddr, &lport);
			_localAddress = new EInetSocketAddress(laddr, lport);
        }}
    }}
}

void EServerSocketChannel::bind(const char* hostname, int port, int backlog) {
	EInetSocketAddress local(hostname, port);
	this->bind(&local, backlog);
}

ESocketChannel* EServerSocketChannel::accept() {
	//@see: openjdk-6/jdk/src/share/classes/sun/nio/ch/ServerSocketChannelImpl.java

	SYNCBLOCK(&_lock) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		if (!isBound())
			throw ESocketException(__FILE__, __LINE__, "Socket is not bound yet");

		EA<EInetSocketAddress*> isaa(1);
		int newfd = -1;
		int n = 0;
		ESocketChannel* rv = null;
		try {
			begin();
			if (!isOpen()) {
				n = 0; //@see: return null;
				goto FINALLY;
			}
			_thread = ENativeThread::current();
			for (;;) {
				n = ENetWrapper::accept(_fd, &newfd, &isaa);
				if ((n == EIOStatus::INTERRUPTED) && isOpen())
					continue;
				break;
			}
		} catch (...) {
			finally {
				_thread = 0;
				end(n > 0);
				ES_ASSERT(EIOStatus::check(n));
			}
			throw; //!
		}
		FINALLY:
		finally {
			_thread = 0;
			end(n > 0);
			ES_ASSERT(EIOStatus::check(n));
		}

		if (n < 1)
			return null;

		ENetWrapper::configureBlocking(newfd, true);
		EInetSocketAddress* isa = isaa[0];
		return new ESocketChannel(newfd, isa);
    }}
}

void EServerSocketChannel::implCloseSelectableChannel() {
	SYNCBLOCK (&_stateLock) {
		ESocketDispatcher::preClose(_fd);
		es_os_thread_t th = _thread;
		if (_thread != 0)
			ENativeThread::signal(th);
		if (!isRegistered())
			kill();
    }}
}

void EServerSocketChannel::implConfigureBlocking(boolean block) {
	ENetWrapper::configureBlocking(_fd, block);
}

boolean EServerSocketChannel::translateAndUpdateReadyOps(int ops, ESelectionKey* sk) {
	return translateReadyOps(ops, sk->nioReadyOps(), sk);
}

boolean EServerSocketChannel::translateAndSetReadyOps(int ops, ESelectionKey* sk) {
	return translateReadyOps(ops, 0, sk);
}

boolean EServerSocketChannel::translateReadyOps(int ops, int initialOps,
		ESelectionKey* sk) {
	int intOps = sk->nioInterestOps(); // Do this just once, it synchronizes
	int oldOps = sk->nioReadyOps();
	int newOps = initialOps;

	if ((ops & ENetWrapper::POLLNVAL_) != 0) {
		// This should only happen if this channel is pre-closed while a
		// selection operation is in progress
		// ## Throw an error if this channel has not been pre-closed
		return false ;
	}

	if ((ops & (ENetWrapper::POLLERR_ | ENetWrapper::POLLHUP_))
			!= 0) {
		newOps = intOps;
		sk->nioReadyOps(newOps);
		return (newOps & ~oldOps) != 0;
	}

	if (((ops & ENetWrapper::POLLIN_) != 0)
			&& ((intOps & ESelectionKey::OP_ACCEPT) != 0))
		newOps |= ESelectionKey::OP_ACCEPT;

	sk->nioReadyOps(newOps);
	return (newOps & ~oldOps) != 0;
}

void EServerSocketChannel::translateAndSetInterestOps(int ops, ESelectionKey* sk) {
	int newOps = 0;

	// Translate ops
	if ((ops & ESelectionKey::OP_ACCEPT) != 0)
		newOps |= ENetWrapper::POLLIN_;
	// Place ops into pollfd array
	sk->selector()->putEventOps(sk, newOps);
}

void EServerSocketChannel::kill() {
	SYNCBLOCK (&_stateLock) {
		if (_state == ST_KILLED)
			return;
		if (_state == ST_UNINITIALIZED) {
			_state = ST_KILLED;
			return;
		}
		ES_ASSERT(!isOpen() && !isRegistered());
		ESocketDispatcher::close(_fd);
		_state = ST_KILLED;
    }}
}

int EServerSocketChannel::getFDVal() {
	return _fd;
}

EString EServerSocketChannel::toString() {
	EString sb;
	sb << "EServerSocketChannel";
	sb << '[';
	if (!isOpen())
		sb << "closed";
	else {
		SYNCBLOCK (&_stateLock) {
			if (localAddress() == null) {
				sb << "unbound";
			} else {
				sb << localAddress()->toString();
			}
        }}
	}
	sb << ']';
	return sb;
}

void EServerSocketChannel::setOption(int optID, const void* optval, int optlen) {
	ENetWrapper::setOption(_fd, optID, (char*)optval, optlen);
}

void EServerSocketChannel::getOption(int optID, void* optval, int* optlen) {
	ENetWrapper::getOption(_fd, optID, optval, optlen);
}

} /* namespace nio */
} /* namespace efc */
