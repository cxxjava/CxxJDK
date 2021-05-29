/*
 * ESocketAdaptor.cpp
 *
 *  Created on: 2013-9-5
 *      Author: cxxjava@163.com
 */

#include "./ESocketAdaptor.hh"
#include "../inc/EChannelInputStream.hh"
#include "../../inc/ESystem.hh"
#include "../../inc/ENetWrapper.hh"

namespace efc {
namespace nio {

class SocketOutputStream : public EOutputStream
{
public:
	SocketOutputStream(ESocketChannel* sc) : bb(null), bs(null), b1(null) {
		this->sc = sc;
	}

	void write(const void *b, int len) THROWS(EIOException) {
		if (len < 0) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		} else if (len == 0) {
			return;
		}

		ES_ASSERT(b);

		if (this->bs != b) {
			delete bb;
			bb = EIOByteBuffer::wrap((void*)b, len);
		}
		bb->limit(ES_MIN(len, bb->capacity()));
		bb->position(0);
		this->bs = (void*)b;

		//@see: Channels.write(ch, bb);
		SYNCBLOCK (sc->blockingLock()) {
			if (!sc->isBlocking())
				throw EILLEGALBLOCKINGMODEEXCEPTION;
			sc->write(bb);
        }}
	}

	void close() THROWS(EIOException) {
		sc->close();
	}

private:
	ESocketChannel* sc;
	EIOByteBuffer* bb;// = null;
	void* bs;// = null;           // Invoker's previous array
	void* b1;// = null;
};

class SocketInputStream : public EChannelInputStream
{
public:
	SocketInputStream(ESocketChannel* sc, int t) :
			EChannelInputStream(sc), timeout(t) {
	}

	int read(EIOByteBuffer* bb) THROWS(EIOException) {
		ESocketChannel* sc = dynamic_cast<ESocketChannel*>(ch);
        SYNCBLOCK (sc->blockingLock()) {
            if (!sc->isBlocking())
                throw EILLEGALBLOCKINGMODEEXCEPTION;
            if (timeout == 0)
                return sc->read(bb);

            // Implement timeout with a selector
            sp<ESelectionKey> sk = null;
            ESelector* sel = null;
            sc->configureBlocking(false);
            try {
                int n;
                if ((n = sc->read(bb)) != 0) {
                	if (sc->isOpen())
                		sc->configureBlocking(true);
                    return n;
                }
                sel = ESelector::open();
                sk = sc->register_(sel, ESelectionKey::OP_READ);
                llong to = timeout;
                for (;;) {
                    if (!sc->isOpen())
                        throw ECLOSEDCHANNELEXCEPTION;
                    llong st = ESystem::currentTimeMillis();
                    int ns = sel->select(to);
                    if (ns > 0 && sk->isReadable()) {
                        if ((n = sc->read(bb)) != 0) {
                        	finally {
								if (sk != null)
									sk->cancel();
								if (sc->isOpen())
									sc->configureBlocking(true);
								delete sel;
							}
                            return n;
                        }
                    }
                    sel->selectedKeys()->remove(sk.get());
                    to -= ESystem::currentTimeMillis() - st;
                    if (to <= 0)
                        throw ESOCKETTIMEOUTEXCEPTION;
                }
            } catch(...) {
            	finally {
					if (sk != null)
						sk->cancel();
					if (sc->isOpen())
						sc->configureBlocking(true);
					delete sel;
				}
            	throw; //!
            } finally {
                if (sk != null)
                    sk->cancel();
                if (sc->isOpen())
                    sc->configureBlocking(true);
                delete sel;
            }
            return 0; //!always not reach here?
        }}
    }

private:
	int timeout;
};

ESocketAdaptor::~ESocketAdaptor() {
	//
}

ESocketAdaptor::ESocketAdaptor(ESocketChannel* sc) :
		ESocket(-1, false, false),
		_sc(sc), _timeout(0), _trafficClass(0) {
}

ESocketAdaptor* ESocketAdaptor::create(ESocketChannel* sc) {
	return new ESocketAdaptor(sc);
}

void ESocketAdaptor::setTcpNoDelay(boolean on) {
	int opt = on ? 1 : 0;
	_sc->setOption(_TCP_NODELAY, &opt, sizeof(int));
}

boolean ESocketAdaptor::getTcpNoDelay() {
	int opt, len;
    len = sizeof(int);
	_sc->getOption(_TCP_NODELAY, &opt, &len);
	return opt;
}

void ESocketAdaptor::setSoLinger(boolean on, int linger) {
	struct linger l;
	if (!on) {
		l.l_onoff = 0;
		l.l_linger = 0;
	} else {
		if (linger > 65535)
			linger = 65535;
		l.l_onoff = 1;
		l.l_linger = linger;
	}
	_sc->setOption(_SO_LINGER, &l, sizeof(l));
}

int ESocketAdaptor::getSoLinger() {
	struct linger linger;
	int len = sizeof(linger);
	_sc->getOption(_SO_LINGER, &linger, &len);
	return linger.l_onoff ? (int)linger.l_linger : (int)-1;
}

void ESocketAdaptor::sendUrgentData(int data) {
	SYNCBLOCK(_sc->blockingLock()) {
		if (!_sc->isBlocking())
			throw EIllegalBlockingModeException(__FILE__, __LINE__);
		int n = _sc->sendOutOfBandData((byte)data);
		ES_ASSERT(n == 1);
    }}
}

void ESocketAdaptor::setOOBInline(boolean on) {
	int opt = on ? 1 : 0;
	_sc->setOption(_SO_OOBINLINE, &opt, sizeof(int));
}

boolean ESocketAdaptor::getOOBInline() {
	int opt, len;
    len = sizeof(int);
	_sc->getOption(_SO_OOBINLINE, &opt, &len);
	return opt;
}

void ESocketAdaptor::setSoTimeout(int timeout) {
	this->_timeout = timeout;
}

int ESocketAdaptor::getSoTimeout() {
	return _timeout;
}

void ESocketAdaptor::setSendBufferSize(int size) {
	_sc->setOption(_SO_SNDBUF, &size, sizeof(int));
}

int ESocketAdaptor::getSendBufferSize() {
	int result, len;
    len = sizeof(int);
	_sc->getOption(_SO_SNDBUF, &result, &len);
	return result;
}

void ESocketAdaptor::setReceiveBufferSize(int size) {
	_sc->setOption(_SO_RCVBUF, &size, sizeof(int));
}

int ESocketAdaptor::getReceiveBufferSize() {
	int result, len;
    len = sizeof(int);
	_sc->getOption(_SO_RCVBUF, &result, &len);
	return result;
}

void ESocketAdaptor::setKeepAlive(boolean on) {
	int opt = on ? 1 : 0;
	_sc->setOption(_SO_KEEPALIVE, &opt, sizeof(int));
}

boolean ESocketAdaptor::getKeepAlive() {
	int opt, len;
    len = sizeof(int);
	_sc->getOption(_SO_KEEPALIVE, &opt, &len);
	return opt;
}

void ESocketAdaptor::setTrafficClass(int tc) {
	if (tc < 0 || tc > 255)
		throw EIllegalArgumentException(__FILE__, __LINE__, "tc is not in range 0 -- 255");
	_sc->setOption(_IP_TOS, &tc, sizeof(int));
}

int ESocketAdaptor::getTrafficClass() {
	int opt, len;
    len = sizeof(int);
	_sc->getOption(_IP_TOS, &opt, &len);
	return opt;
}

void ESocketAdaptor::setReuseAddress(boolean on) {
	int opt = on ? 1 : 0;
	_sc->setOption(_SO_REUSEADDR, &opt, sizeof(int));
}

boolean ESocketAdaptor::getReuseAddress() {
	int opt, len;
    len = sizeof(int);
	_sc->getOption(_SO_REUSEADDR, &opt, &len);
	return opt;
}

boolean ESocketAdaptor::isConnected() {
	return _sc->isConnected();
}

boolean ESocketAdaptor::isBound() {
	return _sc->isBound();
}

boolean ESocketAdaptor::isClosed() {
	return !_sc->isOpen();
}

boolean ESocketAdaptor::isInputShutdown() {
	return !_sc->isInputOpen();
}

boolean ESocketAdaptor::isOutputShutdown() {
	return !_sc->isOutputOpen();
}

EString ESocketAdaptor::toString() {
	if (_sc->isConnected())
		return EString::formatOf("Socket[addr=%s,port=%d,localport=%d]",
			getInetAddress()->toString().c_str(), getPort(), getLocalPort());
	return EString("Socket[unconnected]");
}

void ESocketAdaptor::shutdownInput() {
	_sc->shutdownInput();
}

void ESocketAdaptor::shutdownOutput() {
	_sc->shutdownOutput();
}

void ESocketAdaptor::close() {
	_sc->close();
}

EInetSocketAddress* ESocketAdaptor::getRemoteSocketAddress() {
	return _sc->remoteAddress();
}

EInetSocketAddress* ESocketAdaptor::getLocalSocketAddress() {
	return _sc->localAddress();
}

void ESocketAdaptor::setOption(int optID, const void* optval, int optlen) {
	ENetWrapper::setOption(_sc->getFDVal(), optID, (char*)optval, optlen);
}

void ESocketAdaptor::getOption(int optID, void* optval, int* optlen) {
	ENetWrapper::getOption(_sc->getFDVal(), optID, optval, optlen);
}

int ESocketAdaptor::getFD() {
	return _sc->getFDVal();
}

sp<ESocketChannel> ESocketAdaptor::getChannel() {
	return dynamic_pointer_cast<ESocketChannel>(_sc->shared_from_this());
}

void ESocketAdaptor::connect(EInetSocketAddress* remote, int timeout) {
	if (remote == null)
		throw EILLEGALARGUMENTEXCEPTIONS("connect: The address can't be null");
	if (timeout < 0)
		throw EILLEGALARGUMENTEXCEPTIONS("connect: timeout can't be negative");

	SYNCBLOCK(_sc->blockingLock()) {
		if (!_sc->isBlocking())
			throw EILLEGALARGUMENTEXCEPTION;

		try {
			if (timeout == 0) {
				_sc->connect(remote);
				return;
			}

			// Implement timeout with a selector
			sp<ESelectionKey> sk = null;
			ESelector* sel = null;
			_sc->configureBlocking(false);
			try {
				if (_sc->connect(remote)) {
					if (_sc->isOpen())
						_sc->configureBlocking(true);
					return;
				}
				sel = ESelector::open();
				sk = _sc->register_(sel, ESelectionKey::OP_CONNECT);
				llong to = timeout;
				for (;;) {
					if (!_sc->isOpen())
						throw ECLOSEDCHANNELEXCEPTION;
					llong st = ESystem::currentTimeMillis();
					int ns = sel->select(to);
					if (ns > 0 &&
						sk->isConnectable() && _sc->finishConnect())
						break;
					sel->selectedKeys()->remove(sk.get());
					to -= ESystem::currentTimeMillis() - st;
					if (to <= 0) {
						try {
							_sc->close();
						} catch (EIOException& x) { }
						throw ESOCKETTIMEOUTEXCEPTION;
					}
				}
			} catch(...) {
				finally {
					if (sk != null)
						sk->cancel();
					if (_sc->isOpen())
						_sc->configureBlocking(true);
					delete sel;
				}
				throw; //!
			} finally {
				if (sk != null)
					sk->cancel();
				if (_sc->isOpen())
					_sc->configureBlocking(true);
				delete sel;
			}

		} catch (...) {
			throw; //Net.translateException(x, true);
			ES_ASSERT(false);
		}
    }}
}

void ESocketAdaptor::bind(EInetSocketAddress* local) {
	try {
		if (local == null) {
			local = new EInetSocketAddress(0);
			_sc->bind(local);
			delete local;
		}
		else {
			_sc->bind(local);
		}
	} catch (...) {
		throw; //Net.translateException(x);
	}
}

EInetAddress* ESocketAdaptor::getInetAddress() {
	if (!_sc->isConnected())
		return null;
	return _sc->remoteAddress()->getAddress(); //Net.asInetSocketAddress(sc.remoteAddress()).getAddress();
}

EInetAddress* ESocketAdaptor::getLocalAddress() {
	if (!_sc->isBound())
		return null; //InetSocketAddress(0).getAddress();
	return _sc->localAddress()->getAddress(); //Net.asInetSocketAddress(sc.localAddress()).getAddress();
}

int ESocketAdaptor::getPort() {
	if (!_sc->isConnected())
		return 0;
	return _sc->remoteAddress()->getPort(); //Net.asInetSocketAddress(sc.remoteAddress()).getPort();
}

int ESocketAdaptor::getLocalPort() {
	if (!_sc->isBound())
		return -1;
	return _sc->localAddress()->getPort(); //Net.asInetSocketAddress(sc.localAddress()).getPort();
}

EInputStream* ESocketAdaptor::getInputStream() {
	if (!_sc->isOpen())
		throw ESOCKETEXCEPTIONS("Socket is closed");
	if (!_sc->isConnected())
		throw ESOCKETEXCEPTIONS("Socket is not connected");
	if (!_sc->isInputOpen())
		throw ESOCKETEXCEPTIONS("Socket input is shutdown");
	if (socketInputStream == null) {
//		try {
//			socketInputStream = (InputStream)AccessController.doPrivileged(
//				new PrivilegedExceptionAction() {
//					public Object run() throws IOException {
//						return new SocketInputStream();
//					}
//				});
//		} catch (java.security.PrivilegedActionException e) {
//			throw (IOException)e.getException();
//		}

		//@see openjdk-6/jdk/src/share/classes/sun/nio/ch/ChannelInputStream.java
		socketInputStream = new SocketInputStream(_sc, _timeout);
	}
	return socketInputStream;
}

EOutputStream* ESocketAdaptor::getOutputStream() {
	if (!_sc->isOpen())
		throw ESOCKETEXCEPTIONS("Socket is closed");
	if (!_sc->isConnected())
		throw ESOCKETEXCEPTIONS("Socket is not connected");
	if (!_sc->isOutputOpen())
		throw ESOCKETEXCEPTIONS("Socket output is shutdown");
//	OutputStream os = null;
//	try {
//		os = (OutputStream)
//			AccessController.doPrivileged(new PrivilegedExceptionAction() {
//				public Object run() throws IOException {
//					return Channels.newOutputStream(sc);
//				}
//			});
//	} catch (java.security.PrivilegedActionException e) {
//		throw (IOException)e.getException();
//	}

	//@see: openjdk-6/jdk/src/share/classes/java/nio/channels/Channels.java newOutputStream()
	if (!socketOutputStream) {
		socketOutputStream = new SocketOutputStream(_sc);
	}

	return socketOutputStream;
}

} /* namespace nio */
} /* namespace efc */
