/*
 * EServerSocketAdaptor.cpp
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#include "./EServerSocketAdaptor.hh"
#include "./ESocketAdaptor.hh"
#include "../../inc/EThreadLocal.hh"
#include "../../inc/ESystem.hh"
#include "../../inc/ENetWrapper.hh"
#include "../../inc/EIllegalBlockingModeException.hh"

namespace efc {
namespace nio {

EServerSocketAdaptor::~EServerSocketAdaptor() {
	//
}

EServerSocketAdaptor* EServerSocketAdaptor::create(EServerSocketChannel* ssc) {
	return new EServerSocketAdaptor(ssc);
}

void EServerSocketAdaptor::bind(int port, int backlog) {
	EInetSocketAddress local(port);
	_ssc->bind(&local, backlog);
}

EInetSocketAddress* EServerSocketAdaptor::getLocalSocketAddress() {
	return _ssc->localAddress();
}

void EServerSocketAdaptor::setOption(int optID, const void* optval,
		int optlen) {
	ENetWrapper::setOption(_ssc->getFDVal(), optID, (char*)optval, optlen);
}

void EServerSocketAdaptor::getOption(int optID, void* optval, int* optlen) {
	ENetWrapper::getOption(_ssc->getFDVal(), optID, optval, optlen);
}

int EServerSocketAdaptor::getFD() {
	return _ssc->getFDVal();
}

EServerSocketAdaptor::EServerSocketAdaptor(EServerSocketChannel* ssc) : EServerSocket((void*)0),
		_ssc(ssc), _timeout(0) {
}

void EServerSocketAdaptor::bind(EInetSocketAddress* local, int backlog) {
	if (local == null) {
		local = new EInetSocketAddress(0);
		_ssc->bind(local, backlog);
		delete local;
	}
	else {
		_ssc->bind(local, backlog);
	}
}

EInetAddress* EServerSocketAdaptor::getInetAddress() {
	if (!_ssc->isBound())
		return null;
	return _ssc->localAddress()->getAddress();
}

int EServerSocketAdaptor::getLocalPort() {
	if (!_ssc->isBound())
		return -1;
	return _ssc->localAddress()->getPort();
}

ESocket* EServerSocketAdaptor::accept() {
	SYNCBLOCK(_ssc->blockingLock()) {
		if (!_ssc->isBound())
			throw EILLEGALBLOCKINGMODEEXCEPTION;

		try {
			if (_timeout == 0) {
				ESocketChannel* sc = _ssc->accept();
				if (sc == null && !_ssc->isBlocking())
					throw EILLEGALBLOCKINGMODEEXCEPTION;
                return sc ? sc->socket() : null; //fixed on 20140326
			}

			// Implement timeout with a selector
			sp<ESelectionKey> sk = null;
			ESelector* sel = null;
			ESocketChannel* sc = null;
			_ssc->configureBlocking(false);
			try {
				if ((sc = _ssc->accept()) != null)
					goto FINALLY;//return sc->socket();
				sel = ESelector::open();
				sk = _ssc->register_(sel, ESelectionKey::OP_ACCEPT);
				llong to = _timeout;
				for (;;) {
					if (!_ssc->isOpen())
						throw ECLOSEDCHANNELEXCEPTION;
					llong st = ESystem::currentTimeMillis();
					int ns = sel->select(to);
					if (ns > 0 &&
						sk->isAcceptable() && ((sc = _ssc->accept()) != null))
						goto FINALLY;//return sc->socket();
					sel->selectedKeys()->remove(sk.get());
					to -= ESystem::currentTimeMillis() - st;
					if (to <= 0)
						throw ESOCKETTIMEOUTEXCEPTION;
				}
			} catch (...) {
				finally {
					if (sk != null)
						sk->cancel();
					if (_ssc->isOpen())
						_ssc->configureBlocking(true);
					delete sel;
				}
				throw;
			}
			FINALLY: {
				if (sk != null)
					sk->cancel();
				if (_ssc->isOpen())
					_ssc->configureBlocking(true);
				delete sel;

				return sc ? sc->socket() : null; //success!
			}
		} catch (...) {
			throw; //Net.translateException(x);
			ES_ASSERT(false);
			return null;            // Never happens
		}
    }}
}

void EServerSocketAdaptor::close() {
	_ssc->close();
}

sp<EServerSocketChannel> EServerSocketAdaptor::getChannel() {
	return dynamic_pointer_cast<EServerSocketChannel>(_ssc->shared_from_this());
}

boolean EServerSocketAdaptor::isBound() {
	return _ssc->isBound();
}

boolean EServerSocketAdaptor::isClosed() {
	return !_ssc->isOpen();
}

void EServerSocketAdaptor::setSoTimeout(int timeout) {
	_timeout = timeout;
}

int EServerSocketAdaptor::getSoTimeout() {
	return _timeout;
}

void EServerSocketAdaptor::setReuseAddress(boolean on) {
	int opt = on ? 1 : 0;
	_ssc->setOption(_SO_REUSEADDR, &opt, sizeof(int));
}

boolean EServerSocketAdaptor::getReuseAddress() {
	int opt, len;
    len = sizeof(int);
	_ssc->getOption(_SO_REUSEADDR, &opt, &len);
	return opt;
}

void EServerSocketAdaptor::setReceiveBufferSize(int size) {
	int opt = size;
	_ssc->setOption(_SO_RCVBUF, &opt, sizeof(int));
}

int EServerSocketAdaptor::getReceiveBufferSize() {
	int result, len;
    len = sizeof(int);
	_ssc->getOption(_SO_RCVBUF, &result, &len);
	return result;
}

EString EServerSocketAdaptor::toString() {
	if (!isBound())
		return "ServerSocket[unbound]";
	return EString::formatOf("ServerSocket[addr=%s,localport=%d]",
			getInetAddress()->toString().c_str(), getLocalPort());
}

} /* namespace nio */
} /* namespace efc */
