/*
 * ESSLServerSocket.hh
 *
 *  Created on: 2017-1-11
 *      Author: cxxjava@163.com
 */

#ifndef ESSLSERVERSOCKET_HH_
#define ESSLSERVERSOCKET_HH_

#include "es_config.h"

#ifdef HAVE_OPENSSL

#include "EServerSocket.hh"
#include "ESSLSocket.hh"

namespace efc {

/**
 * This class extends <code>ServerSocket</code>s and
 * provides secure server sockets using protocols such as the Secure
 * Sockets Layer (SSL) or Transport Layer Security (TLS) protocols.
 * <P>
 * Instances of this class are generally created using a
 * <code>SSLServerSocketFactory</code>.  The primary function
 * of <code>SSLServerSocket</code>s
 * is to create <code>SSLSocket</code>s by <code>accept</code>ing
 * connections.
 * <P>
 * <code>SSLServerSocket</code>s contain several pieces of state data
 * which are inherited by the <code>SSLSocket</code> at
 * socket creation.  These include the enabled cipher
 * suites and protocols, whether client
 * authentication is necessary, and whether created sockets should
 * begin handshaking in client or server mode.  The state
 * inherited by the created <code>SSLSocket</code> can be
 * overriden by calling the appropriate methods.
 *
 * @see java.net.ServerSocket
 * @see SSLSocket
 *
 * @since 1.4
 */

class ESSLServerSocket: public EServerSocket {
public:
	virtual ~ESSLServerSocket();

	/**
	 * Create an unbound TCP server socket using the default authentication
     * context.
     *
     * @throws IOException if an I/O error occurs when creating the socket
	 */
	ESSLServerSocket();

	/**
	 * Create a TCP server socket on a port, using the default
	 * authentication context and a specified backlog of connections
	 * as well as a particular specified network interface.  This
	 * constructor is used on multihomed hosts, such as those used
	 * for firewalls or as routers, to control through which interface
	 * a network service is provided.
	 * <P>
	 * If there is a security manager, its <code>checkListen</code>
	 * method is called with the <code>port</code> argument as its
	 * argument to ensure the operation is allowed. This could result
	 * in a SecurityException.
	 * <P>
	 * A port number of <code>0</code> creates a socket on any free port.
	 * <P>
	 * The <code>backlog</code> argument is the requested maximum number of
	 * pending connections on the socket. Its exact semantics are implementation
	 * specific. In particular, an implementation may impose a maximum length
	 * or may choose to ignore the parameter altogther. The value provided
	 * should be greater than <code>0</code>. If it is less than or equal to
	 * <code>0</code>, then an implementation specific default will be used.
	 * <P>
	 * If <i>address</i> is null, it will default accepting connections
	 * on any/all local addresses.
	 *
	 * @param port the port on which to listen
	 * @param backlog  requested maximum length of the queue of incoming
	 *                  connections.
	 * @param address the address of the network interface through
	 *          which connections will be accepted
	 * @throws IOException if an I/O error occurs when creating the socket
	 * @throws SecurityException if a security manager exists and its
	 *         <code>checkListen</code> method doesn't allow the operation.
	 * @throws IllegalArgumentException if the port parameter is outside the
	 *         specified range of valid port values, which is between 0 and
	 *         65535, inclusive.
	 * @see    SecurityManager#checkListen
	 */
	ESSLServerSocket(int port, int backlog=50, EInetAddress *bindAddr=null) THROWS(EIOException);

	//TODO:
	ESSLServerSocket(const ESSLServerSocket& that);
	ESSLServerSocket& operator= (const ESSLServerSocket& that);

	/**
	 *
	 */
	boolean setSSLParameters(const char* dh_file, const char* cert_file,
			const char* private_key_file, const char* passwd,
			const char* CAfile);

	/**
	 *
	 */
	virtual ESSLSocket* accept() THROWS(EIOException);

	/**
	 *
	 */
	virtual void close() THROWS(EIOException);

	/**
	 * Returns the names of the cipher suites which could be enabled for use
	 * on an SSL connection.
	 * <P>
	 * Normally, only a subset of these will actually
	 * be enabled by default, since this list may include cipher suites which
	 * do not meet quality of service requirements for those defaults.  Such
	 * cipher suites are useful in specialized applications.
	 *
	 * @return an array of cipher suite names
	 * @see #getEnabledCipherSuites()
	 * @see #setEnabledCipherSuites(String [])
	 */
	EA<EString*> getSupportedCipherSuites();

	/**
	 * Returns the list of cipher suites which are currently enabled
	 * for use by newly accepted connections.
	 * <P>
	 * If this list has not been explicitly modified, a system-provided
	 * default guarantees a minimum quality of service in all enabled
	 * cipher suites.
	 * <P>
	 * There are several reasons why an enabled cipher suite might
	 * not actually be used.  For example:  the server socket might
	 * not have appropriate private keys available to it or the cipher
	 * suite might be anonymous, precluding the use of client authentication,
	 * while the server socket has been told to require that sort of
	 * authentication.
	 *
	 * @return an array of cipher suites enabled
	 * @see #getSupportedCipherSuites()
	 * @see #setEnabledCipherSuites(String [])
	 */
	EA<EString*> getEnabledCipherSuites();

protected:
	SSL_CTX* ctx;
	X509_STORE* store;
	int renegotiations, ciphersuite_model;

private:
	void init();
	boolean useDHFile(const char* dh_file);

	static int nextProto(SSL* ssl, const unsigned char** data, unsigned int* len, void* arg) {
		*data = (unsigned char*) arg;
		*len = (unsigned int)(sizeof("\x2h2\x5h2-16\x5h2-14")-1);
		return SSL_TLSEXT_ERR_OK;
	}

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
	static int selectProto(SSL* ssl, const unsigned char** out,
			unsigned char* outlen, const unsigned char* in, unsigned int inlen,
			void* arg) {
		const unsigned char* p;
		const unsigned char* end;

		for (p = in, end = in + inlen; p <= end; p += *p + 1) {
			if (memcmp(p, "\x2h2", 3) == 0
					|| memcmp(p, "\x5h2-16", 6) == 0
					|| memcmp(p, "\x5h2-14", 6) == 0) {
				*out = p + 1;
				*outlen = *p;

				return SSL_TLSEXT_ERR_OK;
			}
		}

		return SSL_TLSEXT_ERR_NOACK;
	}
#endif
};

} /* namespace efc */
#endif //!HAVE_OPENSSL
#endif /* ESSLSERVERSOCKET_HH_ */
