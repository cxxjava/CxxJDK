/*
 * EDatagramSocketAdaptor.cpp
 *
 *  Created on: 2016-8-23
 *      Author: cxxjava@163.com
 */

#include "./EDatagramSocketAdaptor.hh"
#include "./EIONetWrapper.hh"
#include "../../inc/ESystem.hh"
#include "../inc/EDatagramChannel.hh"
#include "../../inc/ESocketTimeoutException.hh"
#include "../../inc/EIllegalArgumentException.hh"

namespace efc {
namespace nio {

EDatagramSocketAdaptor::~EDatagramSocketAdaptor() {
	//
}

EDatagramSocketAdaptor::EDatagramSocketAdaptor(EDatagramChannel* dc) :
		EDatagramSocket((void*) (NULL), (void*) (NULL)), _dc(dc), _timeout(0) {
	// Invoke the DatagramSocketAdaptor(SocketAddress) constructor,
	// passing a dummy DatagramSocketImpl object to aovid any native
	// resource allocation in super class and invoking our bind method
	// before the dc field is initialized.
}

EDatagramSocketAdaptor* EDatagramSocketAdaptor::create(EDatagramChannel* dc) {
	return new EDatagramSocketAdaptor(dc);
}

sp<EDatagramChannel> EDatagramSocketAdaptor::getChannel() {
	return dynamic_pointer_cast<EDatagramChannel>(_dc->shared_from_this());
}

void EDatagramSocketAdaptor::bind(EInetSocketAddress* local) {
	if (local == null) {
		EInetSocketAddress l(0);
		_dc->bind(&l);
	} else {
		_dc->bind(local);
	}
}

void EDatagramSocketAdaptor::connect(EInetSocketAddress* remote) {
	if (remote == null)
		throw EIllegalArgumentException(__FILE__, __LINE__,
				"Address can't be null");

	//@see: connectInternal(remote);
	{
		if (isClosed())
			return;

		try {
			_dc->connect(remote);
		} catch (EException& x) {
			EIONetWrapper::translateToSocketException(x);
		}
	}
}

void EDatagramSocketAdaptor::connect(EInetAddress* address, int port) {
	try {
		EInetSocketAddress isa(address, port);
		this->connect(&isa);
	} catch (ESocketException& x) {
		// Yes, j.n.DatagramSocket really does this
	}
}

void EDatagramSocketAdaptor::disconnect() {
	try {
		_dc->disconnect();
	} catch (EIOException& x) {
		throw ERuntimeException(__FILE__, __LINE__, &x);
	}
}

boolean EDatagramSocketAdaptor::isBound() {
	return _dc->getLocalAddress() != null;
}

boolean EDatagramSocketAdaptor::isConnected() {
	return _dc->getRemoteAddress() != null;
}

EInetAddress* EDatagramSocketAdaptor::getInetAddress() {
	return (isConnected() ? _dc->getRemoteAddress()->getAddress() : null);
}

int EDatagramSocketAdaptor::getPort() {
	return (isConnected() ? _dc->getRemoteAddress()->getPort() : -1);
}

void EDatagramSocketAdaptor::send(EDatagramPacket* p) {
	SYNCBLOCK(_dc->blockingLock()) {
		if (!_dc->isBlocking())
			throw EIllegalBlockingModeException(__FILE__, __LINE__);
		try {
			SYNCHRONIZED(p) {
				EIOByteBuffer* bb = EIOByteBuffer::wrap(p->getData(),
														p->getOffset(),
														p->getLength());
				if (_dc->isConnected()) {
					if (p->getAddress() == null) {
						// Legacy DatagramSocket will send in this case
						// and set address and port of the packet
						EInetSocketAddress* isa = _dc->getRemoteAddress();
						p->setPort(isa->getPort());
						p->setAddress(isa->getAddress());
						_dc->write(bb);
					} else {
						// Target address may not match connected address
						_dc->send(bb, p->getSocketAddress().get());
					}
				} else {
					// Not connected so address must be valid or throw
					_dc->send(bb, p->getSocketAddress().get());
				}
            }}
		} catch (EIOException& x) {
			EIONetWrapper::translateToSocketException(x);
		}
    }}
}

void EDatagramSocketAdaptor::receive(EDatagramPacket* p) {
	SYNCBLOCK(_dc->blockingLock()) {
		if (!_dc->isBlocking())
			throw EIllegalBlockingModeException(__FILE__, __LINE__);
		try {
			SYNCHRONIZED(p) {
				EIOByteBuffer* bb = EIOByteBuffer::wrap(p->getData(),
														p->getOffset(),
														p->getLength());
				//@see: SocketAddress sender = receive(bb);
				sp<EInetSocketAddress> sender;
				{
					//@see: openjdk-8/src/share/classes/sun/nio/ch/ServerSocketAdaptor.java#L174
					if (timeout == 0) {
						sender = _dc->receive(bb);
						goto FINALLY;
					}

					_dc->configureBlocking(false);
					try {
						int n;
						if ((sender = _dc->receive(bb)) != null)
							goto FINALLY;
						llong to = timeout;
						for (;;) {
							if (!_dc->isOpen())
								throw EClosedChannelException(__FILE__, __LINE__);
							llong st = ESystem::currentTimeMillis();
							int result = _dc->poll(ENetWrapper::POLLIN_, to);
							if (result > 0 && ((result & ENetWrapper::POLLIN_) != 0)) {
								if ((sender = _dc->receive(bb)) != null)
									goto FINALLY;
							}
							to -= ESystem::currentTimeMillis() - st;
							if (to <= 0)
								throw ESocketTimeoutException(__FILE__, __LINE__);

						}
					} catch (...) {
						finally {
							if (_dc->isOpen())
							_dc->configureBlocking(true);
						}

						throw; //!
					}
					FINALLY:
					finally {
						if (_dc->isOpen())
						_dc->configureBlocking(true);
					}
				}
				p->setSocketAddress(sender.get());
				p->setLength(bb->position() - p->getOffset());
            }}
		} catch (EIOException& x) {
			EIONetWrapper::translateToSocketException(x);
		}
    }}
}

sp<EInetAddress> EDatagramSocketAdaptor::getLocalAddress() {
	if (isClosed())
		return null;

	EInetSocketAddress* local = _dc->getLocalAddress();
	if (local == null)
		return new EInetAddress();

	EInetAddress* result = local->getAddress();
	/*
	 SecurityManager sm = System.getSecurityManager();
	 if (sm != null) {
	 try {
	 sm.checkConnect(result.getHostAddress(), -1);
	 } catch (SecurityException x) {
	 return new InetSocketAddress(0).getAddress();
	 }
	 }
	 */
	return new EInetAddress(*result);
}

int EDatagramSocketAdaptor::getLocalPort() {
	if (isClosed())
		return -1;

	try {
		EInetSocketAddress* local = _dc->getLocalAddress();
		if (local != null) {
			return local->getPort();
		}
	} catch (EException& x) {
	}
	return 0;
}
void EDatagramSocketAdaptor::setSoTimeout(int timeout) {
	this->timeout = timeout;
}

int EDatagramSocketAdaptor::getSoTimeout() {
	return timeout;
}

void EDatagramSocketAdaptor::close() {
	try {
		_dc->close();
	} catch (EIOException& x) {
		throw ERuntimeException(__FILE__, __LINE__, &x);
	}
}

boolean EDatagramSocketAdaptor::isClosed() {
	return !_dc->isOpen();
}

int EDatagramSocketAdaptor::getFD() {
	return _dc->getFDVal();
}

void EDatagramSocketAdaptor::setSendBufferSize(int size) {
	int opt = size;
	_dc->setOption(_SO_SNDBUF, &opt, sizeof(int));
}

int EDatagramSocketAdaptor::getSendBufferSize() {
	int result, len;
    len = sizeof(int);
	_dc->getOption(_SO_SNDBUF, &result, &len);
	return result;
}

void EDatagramSocketAdaptor::setReceiveBufferSize(int size) {
	int opt = size;
	_dc->setOption(_SO_RCVBUF, &opt, sizeof(int));
}

int EDatagramSocketAdaptor::getReceiveBufferSize() {
	int result, len;
    len = sizeof(int);
	_dc->getOption(_SO_RCVBUF, &result, &len);
	return result;
}

void EDatagramSocketAdaptor::setReuseAddress(boolean on) {
	int opt = on ? 1 : 0;
	_dc->setOption(_SO_REUSEADDR, &opt, sizeof(int));
}

boolean EDatagramSocketAdaptor::getReuseAddress() {
	int opt, len;
    len = sizeof(int);
	_dc->getOption(_SO_REUSEADDR, &opt, &len);
	return opt;
}

void EDatagramSocketAdaptor::setBroadcast(boolean on) {
	int opt = on ? 1 : 0;
	_dc->setOption(_SO_BROADCAST, &opt, sizeof(int));
}

boolean EDatagramSocketAdaptor::getBroadcast() {
	int opt, len;
    len = sizeof(int);
	_dc->getOption(_SO_BROADCAST, &opt, &len);
	return opt;
}

void EDatagramSocketAdaptor::setTrafficClass(int tc) {
	int opt = tc;
	_dc->setOption(_IP_TOS, &opt, sizeof(int));
}

int EDatagramSocketAdaptor::getTrafficClass() {
	int opt, len;
    len = sizeof(int);
	_dc->getOption(_IP_TOS, &opt, &len);
	return opt;
}

} /* namespace nio */
} /* namespace efc */
