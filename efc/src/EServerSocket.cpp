/**
 * EServerSocket.cpp
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#include "EServerSocket.hh"
#include "ENetWrapper.hh"

namespace efc {

EServerSocket::~EServerSocket()
{
	delete localSocketAddress;

	if (socket >= 0) {
		try {
			close();
		} catch (...) {
		}
	}
}

EServerSocket::EServerSocket(void* dummy) : socket(-1),
 		localSocketAddress(null), status(0) {
}

EServerSocket::EServerSocket() :
		localSocketAddress(null), status(0)
{
	socket = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		throw EIOException(__FILE__, __LINE__);
	}
}

EServerSocket::EServerSocket(int port, int backlog, EInetAddress *bindAddr) :
				status(0)
{
	socket = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		throw EIOException(__FILE__, __LINE__);
	}
	int ret = eso_net_bind(socket, bindAddr ? bindAddr->getHostAddress().c_str() : NULL, port);
	if (ret != 0) {
		eso_net_close(socket);
		throw EBindException(__FILE__, __LINE__, "Bind failed");
	}
	ret = eso_net_listen(socket, backlog);
	if (ret != 0) {
		eso_net_close(socket);
		throw EIOException(__FILE__, __LINE__);
	}

	status.bound = 1;

	localSocketAddress = new EInetSocketAddress(bindAddr, port);
}

void EServerSocket::bind(EInetSocketAddress *endpoint, int backlog)
{
    if (isClosed())
        throw ESocketException(__FILE__, __LINE__, "Socket is closed");
    if (isBound())
        throw ESocketException(__FILE__, __LINE__, "Already bound");
    
    EInetSocketAddress isa(0);
    if (endpoint == null)
        endpoint = &isa;
    
	if (endpoint->isUnresolved())
		throw ESocketException(__FILE__, __LINE__, "Unresolved address");

	int ret = eso_net_bind(socket,
                           endpoint->getAddress()->getHostAddress().c_str(),
                           endpoint->getPort());
	if (ret != 0) {
		throw EBindException(__FILE__, __LINE__, "Bind failed");
	}

	status.bound = 1;

	ret = eso_net_listen(socket, backlog);
	if (ret != 0) {
		throw EIOException(__FILE__, __LINE__, "Listen failed");
	}

	localSocketAddress = new EInetSocketAddress(endpoint->getAddress(), endpoint->getPort());
}

void EServerSocket::bind(const char* hostname, int port, int backlog)
{
	EInetSocketAddress endpoint(hostname, port);
	this->bind(&endpoint, backlog);
}

void EServerSocket::bind(int port, int backlog) {
	EInetSocketAddress endpoint(0, port);
	this->bind(&endpoint, backlog);
}

EInetAddress* EServerSocket::getInetAddress()
{
	getLocalSocketAddress();
	return localSocketAddress ? localSocketAddress->getAddress() : null;
}

int EServerSocket::getLocalPort()
{
	getLocalSocketAddress();
	return localSocketAddress ? localSocketAddress->getPort() : -1;
}

EInetSocketAddress* EServerSocket::getLocalSocketAddress()
{
	if (!isBound())
		return null;
	if (!localSocketAddress) {
		struct ip_addr laddr;
		int lport;
		int ret = eso_net_localaddr(socket, &laddr, &lport);
		if (ret != 0) {
			return null;
		}
		localSocketAddress = new EInetSocketAddress(laddr.ip.s_addr, lport);
	}
	return localSocketAddress;
}

ESocket* EServerSocket::accept()
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isBound())
		throw ESocketException(__FILE__, __LINE__, "Socket is not bound yet");

	struct ip_addr raddr;
	int rport;
	int newsock = eso_net_accept(socket, &raddr, &rport);
	if (newsock < 0) {
		if (newsock == -2) {
			throw ESocketTimeoutException(__FILE__, __LINE__, "Accept timed out");
		} else {
			throw ESocketException(__FILE__, __LINE__, "Accept failed");
		}
	}

	ESocket *newSocket = null;
	try {
		newSocket = new ESocket(newsock, true, true, raddr.ip.s_addr, rport);
	} catch (EIOException& e) {
		eso_net_close(newsock);
		delete newSocket;
		throw ESocketException(__FILE__, __LINE__, "Implement accept socket failed");
	}
	return newSocket;
}

void EServerSocket::close()
{
	SYNCBLOCK(&closeLock) {
		if (isClosed())
			return;

		//@see: impl.close();
		eso_net_close(socket);
		socket = -1;

		status.closed = 1;
	}}
}

boolean EServerSocket::isBound()
{
	return (status.bound == 1);
}

boolean EServerSocket::isClosed()
{
	SYNCBLOCK(&closeLock) {
		return (status.closed == 1);
	}}
}

void EServerSocket::setSoTimeout(int timeout) THROWS(ESocketException)
{
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
#ifdef WIN32
	setOption(_SO_TIMEOUT, &timeout, sizeof(int));
#else
	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = (timeout % 1000) * 1000;
	setOption(_SO_TIMEOUT, &tv, sizeof(tv));
#endif
}

int EServerSocket::getSoTimeout()
{
	int result, len;

	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
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
}

void EServerSocket::setReuseAddress(boolean on) THROWS(ESocketException)
{
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
	int opt = on ? 1 : 0;
	setOption(_SO_REUSEADDR, &opt, sizeof(int));
}

boolean EServerSocket::getReuseAddress() THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt, len;
    len = sizeof(int);
	getOption(_SO_REUSEADDR, &opt, &len);
	return opt;
}

EString EServerSocket::toString()
 {
	if (!isBound())
		return EString("ServerSocket[unbound]");
	return EString::formatOf("ServerSocket[addr=%s,port=%d]",
			getInetAddress()->getHostAddress().c_str(), getLocalPort());
}

void EServerSocket::setReceiveBufferSize (int size) THROWS(ESocketException)
{
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
	setOption(_SO_RCVBUF, &size, sizeof(int));
}

int EServerSocket::getReceiveBufferSize() THROWS(ESocketException)
{
	int result, len;

	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
    len = sizeof(int);
	getOption(_SO_RCVBUF, &result, &len);

	return result;
}

void EServerSocket::setOption(int optID, const void* optval, int optlen)
		THROWS(ESocketException) {
	ENetWrapper::setOption(socket, optID, (char*)optval, optlen);
}

void EServerSocket::getOption(int optID, void* optval, int* optlen)
		THROWS(ESocketException) {
	ENetWrapper::getOption(socket, optID, optval, optlen);
}

int EServerSocket::getFD() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	return socket;
}

} /* namespace efc */
