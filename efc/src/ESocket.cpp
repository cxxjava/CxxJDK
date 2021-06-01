/**
 * ESocket.cpp
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#include "ESocket.hh"
#include "ESentry.hh"
#include "ENetWrapper.hh"
#include "EFileInputStream.hh"
#include "ERandomAccessFile.hh"
#include "ENullPointerException.hh"

#ifndef WIN32
#include <sys/ioctl.h>
#ifdef __APPLE__
#include <sys/uio.h>
#else //__linux__
#include <sys/sendfile.h>
#endif
#endif

namespace efc {

#ifndef WIN32

/*
 * file descriptor used for dup2
 */
int ESocket::preCloseFD = -1;

void ESocket::init() {
	if (preCloseFD == -1) {
		int sp[2];
		if (socketpair(PF_UNIX, SOCK_STREAM, 0, sp) < 0) {
			throw EIOException(__FILE__, __LINE__, "socketpair failed");
			return;
		}
		preCloseFD = sp[0];
		::close(sp[1]);
	}
}

void ESocket::socketPreClose() {
	init();

	if (preCloseFD >= 0) {
		int rv;
		do {
			rv = dup2(preCloseFD, socket);
		} while (rv == -1 && errno == EINTR);
	}
}
#endif

//=============================================================================

/**
 * A <code>ESocketOutputStream</code> obtains output bytes
 * to a socket.
 *
 * <code>ESocketOutputStream</code> is not the same of
 * Java's <code>SocketOutputStream</code>.
 */
class SocketOutputStream : public EOutputStream
{
public:
	~SocketOutputStream() {
	}

    SocketOutputStream(ESocket *s) {
    	socket = s;
    }

	void write(const void *b, int len) THROWS(EIOException) {
		socket->write(b, len);
	}

	void close() THROWS(EIOException) {
		socket->shutdownOutput();
	}

private:
	ESocket *socket;
};

/**
 * A <code>ESocketInputStream</code> obtains input bytes
 * from a socket.
 *
 * <code>ESocketInputStream</code> is not the same of
 * Java's <code>SocketInputStream</code>.
 */
class SocketInputStream : public EInputStream
{
public:
	~SocketInputStream() {
	}

    SocketInputStream(ESocket *s): eof(false) {
		socket = s;
	}

	int read(void *b, int len) THROWS(EIOException) {
		// EOF already encountered
		if (eof) {
			return -1;
		}
		int n = socket->read(b, len);
		if (n == -1) {
			eof = true;
		}
		return n;
	}

	long skip(long n) THROWS(EIOException) {
		if (n <= 0) {
			return 0;
		}
		long numbytes = n;
		int buflen = (int) ES_MIN(1024, numbytes);
		byte *data = (byte*)eso_malloc(buflen);
		while (numbytes > 0) {
			int r = read(data, (int) ES_MIN((long) buflen, numbytes));
			if (r < 0) {
				break;
			}
			numbytes -= r;
		}
		eso_free(data);
		return numbytes - numbytes;
	}

	long available() THROWS(EIOException) {
		if (socket->isClosed() || socket->isConnectionReset()) {
			return 0;
		}

		long available;
		long ret;

		do {
	#ifdef WIN32
			ret = ::ioctlsocket(socket->getFD(), FIONREAD, (unsigned long*)&available);
	#else
			ret = ::ioctl(socket->getFD(), FIONREAD, &available);
	#endif
		} while ((ret == -1) && (errno == EINTR));

		if (ret < 0) {
			throw ESocketException(__FILE__, __LINE__, "socket available");
		}
		return available;
	}

	void close() THROWS(EIOException) {
		socket->shutdownInput();
	}

private:
	ESocket *socket;
	boolean eof;
};

//=============================================================================

ESocket::~ESocket()
{
	delete remoteSocketAddress;
	delete localSocketAddress;
	delete socketInputStream;
	delete socketOutputStream;

	if (socket >= 0) {
		try {
			close();
		} catch (...) {
		}
	}
}

ESocket::ESocket() THROWS(EIOException) :
		socketInputStream(null), socketOutputStream(null),
		remoteSocketAddress(null), localSocketAddress(null),
		status(0)
{
	socket = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		throw EIOException(__FILE__, __LINE__);
	}
}

ESocket::ESocket(const char* host, int port) THROWS2(EUnknownHostException, EIOException) :
                 socketInputStream(null), socketOutputStream(null),
                 localSocketAddress(null),
                 status(0)
{
	remoteSocketAddress = new EInetSocketAddress(host, port); //EUnknownHostException

	socket = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		delete remoteSocketAddress;
		throw EIOException(__FILE__, __LINE__);
	}

	int ret = eso_net_connect(socket,
			remoteSocketAddress->getAddress()->getHostAddress().c_str(),
			port,
			-1);
	if (ret != 0) {
		eso_net_close(socket);
		delete remoteSocketAddress;
		throw EIOException(__FILE__, __LINE__);
	}

	status.connected = 1;
}

ESocket::ESocket(EInetAddress *address, int port) THROWS(EIOException) :
   	             socketInputStream(null), socketOutputStream(null),
	             localSocketAddress(null),
	             status(0)
{
	remoteSocketAddress = new EInetSocketAddress(address, port);

	int ret;

	socket = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		goto FAILURE;
	}

	ret = eso_net_connect(socket,
			address->getHostAddress().c_str(),
			port,
			-1);
	if (ret != 0) {
		goto FAILURE;
	}

	status.connected = 1;

	return;

FAILURE:
	if (socket >= 0) {
		eso_net_close(socket);
	}
	delete remoteSocketAddress;
	throw EIOException(__FILE__, __LINE__);
}

ESocket::ESocket(const char* host, int port, EInetAddress *localAddr,
		         int localPort) THROWS2(EUnknownHostException,EIOException) :
		         socketInputStream(null), socketOutputStream(null),
		         status(0)
{
	remoteSocketAddress = new EInetSocketAddress(host, port); //EUnknownHostException
	localSocketAddress = new EInetSocketAddress(localAddr, localPort);

	int ret;

	socket = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		goto FAILURE;
	}

	ret = eso_net_bind(socket, localAddr->getHostAddress().c_str(), localPort);
	if (ret != 0) {
		goto FAILURE;
	}

	ret = eso_net_connect(socket,
			remoteSocketAddress->getAddress()->getHostAddress().c_str(),
			port,
			-1);
	if (ret != 0) {
		goto FAILURE;
	}

	status.connected = 1;
	status.bound = 1;

	return;

FAILURE:
	if (socket >= 0) {
		eso_net_close(socket);
	}
	delete remoteSocketAddress;
	delete localSocketAddress;
	throw EIOException(__FILE__, __LINE__);
}

ESocket::ESocket(EInetAddress *address, int port, EInetAddress *localAddr,
		         int localPort) THROWS(EIOException) :
		         socketInputStream(null), socketOutputStream(null),
		         status(0)
{
	remoteSocketAddress = new EInetSocketAddress(address, port);
	localSocketAddress = new EInetSocketAddress(localAddr, localPort);

	int ret;

	socket = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	if (socket < 0) {
		goto FAILURE;
	}

	ret = eso_net_bind(socket, localAddr->getHostAddress().c_str(), localPort);
	if (ret != 0) {
		goto FAILURE;
	}

	ret = eso_net_connect(socket,
			address->getHostAddress().c_str(),
			port,
			-1);
	if (ret != 0) {
		goto FAILURE;
	}

	status.connected = 1;
	status.bound = 1;

	return;

FAILURE:
	if (socket >= 0) {
		eso_net_close(socket);
	}
	delete remoteSocketAddress;
	delete localSocketAddress;
	throw EIOException(__FILE__, __LINE__);
}

ESocket::ESocket(const int fd, boolean connected, boolean bound, int raddr, int rport) :
				socketInputStream(null), socketOutputStream(null),
				remoteSocketAddress(null), localSocketAddress(null),
				status(0) {
	socket = fd;
	remoteSocketAddress = new EInetSocketAddress(raddr, rport); //EUnknownHostException
	status.connected = connected ? 1 : 0;
	status.bound = bound ? 1 : 0;
}

ESocket* ESocket::createFromFD(int fd, boolean connected, boolean bound, int raddr, int rport) {
	return new ESocket(fd, connected, bound, raddr, rport);
}

void ESocket::connect(EInetSocketAddress *endpoint, int timeout) THROWS(EIOException)
{
	if (endpoint == null)
		throw EIllegalArgumentException(__FILE__, __LINE__, "connect: The address can't be null");

	if (isClosed())
	    throw ESocketException(__FILE__, __LINE__, "Socket is closed");

	if (isConnected())
	    throw ESocketException(__FILE__, __LINE__, "already connected");

	remoteSocketAddress = new EInetSocketAddress(endpoint->getAddress(), endpoint->getPort());

	int ret = eso_net_connect(socket,
			endpoint->getAddress()->getHostAddress().c_str(),
			endpoint->getPort(),
			timeout);
	if (ret != 0) {
		if (ret == -2)
			throw ESocketTimeoutException(__FILE__, __LINE__, "connect timeout");
		else
			throw EIOException(__FILE__, __LINE__);
	}

	status.connected = 1;

	/*
	 * If the socket was not bound before the connect, it is now because
	 * the kernel will have picked an ephemeral port & a local address
	 */
	status.bound = 1;
}

void ESocket::connect(const char *hostname, int port, int timeout) {
	if (hostname == null)
		throw EIllegalArgumentException(__FILE__, __LINE__, "connect: The address can't be null");

	EInetSocketAddress isa(hostname, port);
	this->connect(&isa, timeout);
}

void ESocket::bind(EInetSocketAddress *bindpoint) THROWS(EIOException)
{
	if (bindpoint) {
		int rv = eso_net_bind(socket, bindpoint->getHostString().c_str(), bindpoint->getPort());
		if (rv != 0) {
			throw EIOException(__FILE__, __LINE__);
		}
	}
}

void ESocket::bind(const char *hostname, int port) {
	EInetSocketAddress isa(hostname, port);
	this->bind(&isa);
}

EInetAddress* ESocket::getInetAddress()
{
	return remoteSocketAddress ? remoteSocketAddress->getAddress() : null;
}

EInetAddress* ESocket::getLocalAddress()
{
	getLocalSocketAddress();
	return localSocketAddress ? localSocketAddress->getAddress() : null;
}

int ESocket::getPort()
{
	return remoteSocketAddress ? remoteSocketAddress->getPort() : 0;
}

int ESocket::getLocalPort()
{
	getLocalSocketAddress();
	return localSocketAddress ? localSocketAddress->getPort() : -1;
}

EInetSocketAddress* ESocket::getRemoteSocketAddress()
{
	return remoteSocketAddress;
}

EInetSocketAddress* ESocket::getLocalSocketAddress()
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

EInputStream* ESocket::getInputStream() THROWS(EIOException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isConnected())
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
	if (isInputShutdown())
		throw ESocketException(__FILE__, __LINE__, "Socket input is shutdown");

	if (!socketInputStream) {
		socketInputStream = new SocketInputStream(this);
	}
	return socketInputStream;
}

EOutputStream* ESocket::getOutputStream() THROWS(EIOException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isConnected())
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
	if (isOutputShutdown())
		throw ESocketException(__FILE__, __LINE__, "Socket output is shutdown");

	if (!socketOutputStream) {
		socketOutputStream = new SocketOutputStream(this);
	}
	return socketOutputStream;
}

void ESocket::setTcpNoDelay(boolean on) THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt = on ? 1 : 0;
	setOption(_TCP_NODELAY, &opt, sizeof(int));
}

boolean ESocket::getTcpNoDelay() THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt, len;
    len = sizeof(int);
	getOption(_TCP_NODELAY, &opt, &len);
	return opt;
}

void ESocket::setSoLinger(boolean on, uint linger) THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");

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
	setOption(_SO_LINGER, &l, sizeof(l));
}

int ESocket::getSoLinger() THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");

	struct linger linger;
	int len = sizeof(linger);
	getOption(_SO_LINGER, &linger, &len);
	return linger.l_onoff ? (int)linger.l_linger : (int)-1;
}

//@see: openjdk-8/src/solaris/native/java/net/PlainSocketImpl.c::Java_java_net_PlainSocketImpl_socketSendUrgentData
//@see: openjdk-8/src/windows/native/java/net/TwoStacksPlainSocketImpl.c::Java_java_net_TwoStacksPlainSocketImpl_socketSendUrgentData
void ESocket::sendUrgentData (int data) THROWS(ESocketException)
{
	unsigned char d = data & 0xFF;
	int n = ::send(socket, (char *)&d, 1, MSG_OOB);
	if (n == -1) {
		throw ESocketException(__FILE__, __LINE__, "send");
	}
	/*
	if (n == JVM_IO_INTR) {
		JNU_ThrowByName(env, "java/io/InterruptedIOException", 0);
		return;
	}
	*/
}

void ESocket::setOOBInline(boolean on) THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt = on ? 1 : 0;
	setOption(_SO_OOBINLINE, &opt, sizeof(int));
}

boolean ESocket::getOOBInline() THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt, len;
    len = sizeof(int);
	getOption(_SO_OOBINLINE, &opt, &len);
	return opt;
}

/**
 * SO_RECVTIMEO isn't supported by WinSock at Cygwin, unfortunately.
 * @see http://www.cygwin.com/ml/cygwin/2003-01/msg00833.html
 * @see https://www.varnish-cache.org/lists/pipermail/varnish-bugs/2011-January/003470.html
 */
void ESocket::setSoTimeout(int timeout) THROWS(ESocketException)
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

int ESocket::getSoTimeout() THROWS(ESocketException)
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

void ESocket::setSendBufferSize(uint size) THROWS(ESocketException)
{
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
	setOption(_SO_SNDBUF, &size, sizeof(int));
}

int ESocket::getSendBufferSize() THROWS(ESocketException)
{
	int result, len;

	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
    len = sizeof(int);
	getOption(_SO_SNDBUF, &result, &len);

	return result;
}

void ESocket::setReceiveBufferSize(int size) THROWS(ESocketException)
{
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
	setOption(_SO_RCVBUF, &size, sizeof(int));
}

int ESocket::getReceiveBufferSize() THROWS(ESocketException)
{
	int result, len;

	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}
    len = sizeof(int);
	getOption(_SO_RCVBUF, &result, &len);

	return result;
}

void ESocket::setKeepAlive(boolean on) THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt = on ? 1 : 0;
	setOption(_SO_KEEPALIVE, &opt, sizeof(int));
}

boolean ESocket::getKeepAlive() THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt, len;
    len = sizeof(int);
	getOption(_SO_KEEPALIVE, &opt, &len);
	return opt;
}

void ESocket::setTrafficClass(int tc) THROWS2(ESocketException, EIllegalArgumentException)
{
	if (tc < 0 || tc > 255)
		throw EIllegalArgumentException(__FILE__, __LINE__, "tc is not in range 0 -- 255");

	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");

	setOption(_IP_TOS, &tc, sizeof(int));
}

int ESocket::getTrafficClass() THROWS(ESocketException)
{
	int opt, len;
    len = sizeof(int);
	getOption(_IP_TOS, &opt, &len);
	return opt;
}

void ESocket::setReuseAddress(boolean on) THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt = on ? 1 : 0;
	setOption(_SO_REUSEADDR, &opt, sizeof(int));
}

boolean ESocket::getReuseAddress() THROWS(ESocketException)
{
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	int opt, len;
    len = sizeof(int);
	getOption(_SO_REUSEADDR, &opt, &len);
	return opt;
}

void ESocket::close() THROWS(EIOException)
{
	if (isClosed())
		return;

//	shutdownInput();
//	shutdownOutput();

#ifndef WIN32
	/*
	 * We close the FileDescriptor in two-steps - first the
	 * "pre-close" which closes the socket but doesn't
	 * release the underlying file descriptor. This operation
	 * may be lengthy due to untransmitted data and a long
	 * linger interval. Once the pre-close is done we do the
	 * actual socket to release the fd.
	 */
	socketPreClose();
#endif
	eso_net_close(socket);
	socket = -1;

	status.closed = 1;
}

void ESocket::shutdownInput() THROWS(EIOException)
{
	//@see: openjdk-6/jdk/src/share/classes/java/net/Socket.java

	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isConnected())
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
	if (isInputShutdown())
		return; //throw ESocketException(__FILE__, __LINE__, "Socket input is already shutdown");
	::shutdown(socket, 0); //0-SHUT_RD
	status.shutIn = 1;
}

void ESocket::shutdownOutput() THROWS(EIOException)
{
	//@see: openjdk-6/jdk/src/share/classes/java/net/Socket.java

	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isConnected())
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
	if (isOutputShutdown())
		return; //throw ESocketException(__FILE__, __LINE__, "Socket output is already shutdown");
	::shutdown(socket, 1); //1-SHUT_WR
	status.shutOut = 1;
}

void ESocket::shutdown() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isConnected())
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
	::shutdown(socket, 2); //2-SHUT_RDWR
	status.shutIn = 1;
	status.shutOut = 1;
}

EString ESocket::toString()
{
	return EString("");
}

boolean ESocket::isConnected()
{
	return (status.connected == 1);
}

boolean ESocket::isConnectionReset()
{
	return (status.reset == 1);
}

boolean ESocket::isBound()
{
	return (status.bound == 1);
}

boolean ESocket::isClosed()
{
	return (status.closed == 1);
}

boolean ESocket::isInputShutdown()
{
	return (status.shutIn == 1);
}

boolean ESocket::isOutputShutdown()
{
	return (status.shutOut == 1);
}

void ESocket::setOption(int optID, const void* optval, int optlen)
		THROWS(ESocketException) {
	ENetWrapper::setOption(socket, optID, (char*)optval, optlen);
}

void ESocket::getOption(int optID, void* optval, int* optlen)
		THROWS(ESocketException) {
	ENetWrapper::getOption(socket, optID, optval, optlen);
}

int ESocket::getFD() THROWS(EIOException) {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	return socket;
}

int ESocket::read(void *b, int len) THROWS(EIOException)
{
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}

	int n = eso_net_read(socket, b, len);
	if (n < 0) {
		if (n == -2) {
			throw ESocketTimeoutException(__FILE__, __LINE__, "read timed out");
		} else if (n == -3) {
			status.reset = 1;
			throw ESocketException(__FILE__, __LINE__, "connection reset");
		} else {
			throw ESocketException(__FILE__, __LINE__);
		}
	}

	/*
	 * If we get here we are at EOF, the socket has been closed,
	 * or the connection has been reset.
	 */
	if (n == 0) {
		return -1;
	}

	return n;
}

void ESocket::write(const void *b, int len) THROWS(EIOException)
{
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}

	int n;
	int count = 0;
	do {
		n = eso_net_write(socket, (char*)b + count, len - count);
		if (n < 0) {
			if (n == -2) {
				throw ESocketException(__FILE__, __LINE__, "write timed out");
			} else if (n == -3) {
				status.reset = 1;
				throw ESocketException(__FILE__, __LINE__, "connection reset");
			} else {
				throw ESocketException(__FILE__, __LINE__);
			}
		}
		count += n;
	} while (count < len);
}

long ESocket::sendfile(const char* filename, long position, long count) {
	ERandomAccessFile raf(filename, "r");
	return this->sendfile(eso_fileno(raf.getFD()), position, (count < 0 ? raf.length() : count));
}

long ESocket::sendfile(EFile* file, long position, long count) {
	ERandomAccessFile raf(file, "r");
	return this->sendfile(eso_fileno(raf.getFD()), position, (count < 0 ? raf.length() : count));
}

long ESocket::sendfile(int in_fd, long position, long count) {
	if (count <= 0) {
		return 0;
	}

	position = ES_MAX(0, position);

#ifdef WIN32
	try {
		EFileInputStream fis(in_fd);
		fis.setIOBuffered(true);
		fis.skip(position);

		long len, result = count;
		char buf[512];
		EOutputStream* os = getOutputStream();
		while (count > 0 && ((len = fis.read(buf, sizeof(buf))) > 0)) {
			os->write(buf, ES_MIN(len, count));
			count -= len;
		}
		return result;
	} catch (...) {
		return -1;
	}
#else
#ifdef __APPLE__
	off_t len = count;
	if (::sendfile(in_fd, socket, position, &len, NULL, 0) == 0) {
		return len;
	}
	return -1;
#else //__linux__
	off_t offset = position;
	return ::sendfile(socket, in_fd, &offset, count);
#endif
#endif //!WIN32
}

} /* namespace efc */
