/*
 * ESSLSocket.cpp
 *
 *  Created on: 2017-1-11
 *      Author: cxxjava@163.com
 */

#include "ESSLSocket.hh"
#include "ESSLCommon.hh"
#include "EFileInputStream.hh"
#include "ERandomAccessFile.hh"
#include "EToDoException.hh"

#ifdef HAVE_OPENSSL

#ifndef WIN32
#include <sys/ioctl.h>
#endif

namespace efc {

/**
 * A <code>ESocketOutputStream</code> obtains output bytes
 * to a socket.
 *
 * <code>ESocketOutputStream</code> is not the same of
 * Java's <code>SocketOutputStream</code>.
 */
class SSLSocketOutputStream : public EOutputStream
{
public:
	~SSLSocketOutputStream() {
	}

	SSLSocketOutputStream(ESSLSocket *s) {
    	socket = s;
    }

	void write(const void *b, int len) THROWS(EIOException) {
		socket->write(b, len);
	}

	void close() THROWS(EIOException) {
		socket->shutdownOutput();
	}

private:
	ESSLSocket *socket;
};

/**
 * A <code>ESocketInputStream</code> obtains input bytes
 * from a socket.
 *
 * <code>ESocketInputStream</code> is not the same of
 * Java's <code>SocketInputStream</code>.
 */
class SSLSocketInputStream : public EInputStream
{
public:
	~SSLSocketInputStream() {
	}

	SSLSocketInputStream(ESSLSocket *s): eof(false) {
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
	ESSLSocket *socket;
	boolean eof;
};

//=============================================================================

/* The passwd_cb() function must write the password into the provided buffer buf which is of length 'size' */
static int passwd_cb(char* buf, int size, int rwflag, void* password)
{
   es_uint32_t written = strlen((const char*)password);
   if (written > size) { // error
	   buf[0] = 0;
	   return -1;
   }
   memset(buf, 0, size);
   memcpy(buf, (const char*)password, written);
   return written;
}

//=============================================================================

void ESSLSocket::doneConnect() {
	if (!ctx) {
		ctx = SSL_CTX_new(SSLv23_method());

		SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

		//@see: nginx-1.3.2/src/event/ngx_event_openssl.c
		SSL_CTX_set_options(ctx, SSL_OP_MICROSOFT_SESS_ID_BUG);
		SSL_CTX_set_options(ctx, SSL_OP_NETSCAPE_CHALLENGE_BUG);
	}

	if (isConnected()) {
		ssl = SSL_new(ctx);
		if (!ssl) {
			throw EIOException(__FILE__, __LINE__, ESSLCommon::getErrors().c_str());
		}

		if (!SSL_set_fd(ssl, socket)) {
			throw EIOException(__FILE__, __LINE__, ESSLCommon::getErrors().c_str());
		}

		RETRY:

		ERR_clear_error();

		int n, e;
		if ((n = SSL_connect(ssl)) != 1) {
			e = SSL_get_error(ssl, n);
			switch (e) {
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
				goto RETRY;
			default:
				throw EIOException(__FILE__, __LINE__, ESSLCommon::getErrors(e).c_str());
			}
		}
	}
}

ESSLSocket::~ESSLSocket() {
	if (ssl) SSL_free(ssl);
	if (ctx) SSL_CTX_free(ctx);
}

ESSLSocket::ESSLSocket() :
		ESocket(), ssl(null), ctx(null) {
	doneConnect();
}

ESSLSocket::ESSLSocket(const char* host, int port) :
		ESocket(host, port), ssl(null), ctx(null) {
	doneConnect();
}

ESSLSocket::ESSLSocket(EInetAddress *address, int port) :
		ESocket(address, port), ssl(null), ctx(null) {
	doneConnect();
}

ESSLSocket::ESSLSocket(const char* host, int port, EInetAddress *localAddr,
		int localPort) :
		ESocket(host, port, localAddr, localPort), ssl(null), ctx(null) {
	doneConnect();
}

ESSLSocket::ESSLSocket(EInetAddress *address, int port, EInetAddress *localAddr,
		int localPort) :
		ESocket(address, port, localAddr, localPort), ssl(null), ctx(null) {
	doneConnect();
}

ESSLSocket::ESSLSocket(SSL* ssl, const int fd, boolean connected, boolean bound, int raddr, int rport) :
		ESocket(fd, connected, bound, raddr, rport), ssl(ssl), ctx(null) {
	// only for server mode.
}

//@see: ULib/src/ulib/ssl/net/sslsocket.cpp::setContext()
boolean ESSLSocket::setSSLParameters(const char* cert_file,
		const char* private_key_file, const char* passwd) {

	int result = 0;

	// Load CERT PEM file

	if (cert_file && *cert_file) {
		result = SSL_CTX_use_certificate_chain_file(ctx, cert_file);

		if (result == 0)
			return false;
	}

	// Load private key PEM file and give passwd callback if any

	if (private_key_file && *private_key_file) {
		if (passwd && *passwd) {
			SSL_CTX_set_default_passwd_cb(ctx, passwd_cb);
			SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*) passwd);
		}

		for (int i = 0; i < 3; ++i) {
			result = SSL_CTX_use_PrivateKey_file(ctx, private_key_file,
					SSL_FILETYPE_PEM);

			if (result)
				break;

			unsigned long error = ERR_peek_error();

			if (ERR_GET_REASON(error) == EVP_R_BAD_DECRYPT) {
				if (i < 2) { // Give the user two tries
					(void) ERR_get_error(); // remove from stack
					continue;
				}
			}

			return false;
		}

		// Check private key

		result = SSL_CTX_check_private_key(ctx);

		if (result == 0)
			return false;
	}

	return true;
}

void ESSLSocket::connect(EInetSocketAddress *endpoint, int timeout) {
	ESocket::connect(endpoint, timeout);
	doneConnect();
}

void ESSLSocket::connect(const char *hostname, int port, int timeout) {
	if (hostname == null)
		throw EIllegalArgumentException(__FILE__, __LINE__, "connect: The address can't be null");

	EInetSocketAddress isa(hostname, port);
	this->connect(&isa, timeout);
}

EInputStream* ESSLSocket::getInputStream() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isConnected())
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
	if (isInputShutdown())
		throw ESocketException(__FILE__, __LINE__, "Socket input is shutdown");

	if (!socketInputStream) {
		socketInputStream = new SSLSocketInputStream(this);
	}
	return socketInputStream;
}

EOutputStream* ESSLSocket::getOutputStream() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isConnected())
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected");
	if (isOutputShutdown())
		throw ESocketException(__FILE__, __LINE__, "Socket output is shutdown");

	if (!socketOutputStream) {
		socketOutputStream = new SSLSocketOutputStream(this);
	}
	return socketOutputStream;
}

int ESSLSocket::read(void *b, int len) {
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}

	RETRY:

	errno = 0;
	ERR_clear_error();

	/*
	 * SSL_read() may return data in parts, so try to read
	 * until SSL_read() would return no data
	 */
	int n = SSL_read(ssl, b, len);
	if (n > 0) {
		return n; // (e == SSL_ERROR_NONE) ?
	} else if (n == 0) {
		return -1; //EOF
	} else {
		int e = SSL_get_error(ssl, n);
		switch (e) {
		case SSL_ERROR_ZERO_RETURN:
			/*
			 * If we get here we are at EOF, the socket has been closed,
			 * or the connection has been reset.
			 */
			return -1;
		case SSL_ERROR_WANT_READ:
		case SSL_ERROR_WANT_WRITE: //@see: http://wetest.qq.com/lab/view/257.html
			goto RETRY;
		default:
			if (errno == ETIMEDOUT) { //only under non-block mode?
				throw ESocketTimeoutException(__FILE__, __LINE__, "read timed out");
			} else {
				throw ESocketException(__FILE__, __LINE__, ESSLCommon::getErrors(e).c_str());
			}
		}
	}
}

void ESSLSocket::write(const void *b, int len) {
	if (isClosed()) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	}

	int n, e;
	int count = 0;
	do {
		RETRY:

		ERR_clear_error();

		n = SSL_write(ssl, (char*)b + count, len - count);
		e = SSL_get_error(ssl, n);

		switch (e) {
		case SSL_ERROR_NONE:
			count += n;
			break;
		case SSL_ERROR_ZERO_RETURN:
			status.reset = 1;
			throw ESocketException(__FILE__, __LINE__, "The connection was broken unexpectedly.");
		case SSL_ERROR_WANT_WRITE:
		case SSL_ERROR_WANT_READ: //@see: http://wetest.qq.com/lab/view/257.html
			goto RETRY;
		default:
			throw ESocketException(__FILE__, __LINE__, ESSLCommon::getErrors(e).c_str());
		}
	} while (count < len);
}

void ESSLSocket::shutdownInput() {
	if (ssl) {
		int mode = SSL_get_shutdown(ssl);
		SSL_set_shutdown(ssl, mode | SSL_RECEIVED_SHUTDOWN);
		SSL_shutdown(ssl);
	}
	ESocket::shutdownInput();
}

void ESSLSocket::shutdownOutput() {
	if (ssl) {
		int mode = SSL_get_shutdown(ssl);
		SSL_set_shutdown(ssl, mode | SSL_SENT_SHUTDOWN);
		SSL_shutdown(ssl);
	}
	ESocket::shutdownOutput();
}

void ESSLSocket::shutdown() {
	if (ssl) {
		int mode = SSL_get_shutdown(ssl);
		SSL_set_shutdown(ssl, mode | SSL_RECEIVED_SHUTDOWN | SSL_SENT_SHUTDOWN);
		SSL_shutdown(ssl);
	}
	ESocket::shutdown();
}

void ESSLSocket::close() {
	if (ssl) {
		SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
		SSL_set_quiet_shutdown(ssl, 1);
		SSL_shutdown(ssl);
	}
	ESocket::close();
}

long ESSLSocket::sendfile(const char* filename, long position, long count) {
	ERandomAccessFile raf(filename, "r");
	return this->sendfile(eso_fileno(raf.getFD()), position, (count < 0 ? raf.length() : count));
}

long ESSLSocket::sendfile(EFile* file, long position, long count) {
	ERandomAccessFile raf(file, "r");
	return this->sendfile(eso_fileno(raf.getFD()), position, (count < 0 ? raf.length() : count));
}

long ESSLSocket::sendfile(int in_fd, long position, long count) {
	if (count <= 0) {
		return 0;
	}

	position = ES_MAX(0, position);

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
	}
	return -1;
}

EA<EString*> ESSLSocket::getSupportedCipherSuites() {
	//TODO...
	throw EToDoException(__FILE__, __LINE__);
}

EA<EString*> ESSLSocket::getEnabledCipherSuites() {
	//TODO...
	throw EToDoException(__FILE__, __LINE__);
}

} /* namespace efc */
#endif //!HAVE_OPENSSL
