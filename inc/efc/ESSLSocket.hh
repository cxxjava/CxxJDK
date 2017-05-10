/*
 * ESSLSocket.hh
 *
 *  Created on: 2017-1-11
 *      Author: cxxjava@163.com
 */

#ifndef ESSLSOCKET_HH_
#define ESSLSOCKET_HH_

#include "es_config.h"

#ifdef HAVE_OPENSSL

#include "ESocket.hh"

namespace efc {

/**
 * This class extends <code>Socket</code>s and provides secure
 * socket using protocols such as the "Secure
 * Sockets Layer" (SSL) or IETF "Transport Layer Security" (TLS) protocols.
 * <P>
 * Such sockets are normal stream sockets, but they
 * add a layer of security protections over the underlying network transport
 * protocol, such as TCP.  Those protections include: <UL>
 *
 *      <LI> <em>Integrity Protection</em>.  SSL protects against
 *      modification of messages by an active wiretapper.
 *
 *      <LI> <em>Authentication</em>.  In most modes, SSL provides
 *      peer authentication.  Servers are usually authenticated,
 *      and clients may be authenticated as requested by servers.
 *
 *      <LI> <em>Confidentiality (Privacy Protection)</em>.  In most
 *      modes, SSL encrypts data being sent between client and server.
 *      This protects the confidentiality of data, so that passive
 *      wiretappers won't see sensitive data such as financial
 *      information or personal information of many kinds.
 *
 *      </UL>
 *
 * <P>These kinds of protection are specified by a "cipher suite", which
 * is a combination of cryptographic algorithms used by a given SSL connection.
 * During the negotiation process, the two endpoints must agree on
 * a ciphersuite that is available in both environments.
 * If there is no such suite in common, no SSL connection can
 * be established, and no data can be exchanged.
 *
 * <P> The cipher suite used is established by a negotiation process
 * called "handshaking".  The goal of this
 * process is to create or rejoin a "session", which may protect many
 * connections over time.  After handshaking has completed, you can access
 * session attributes by using the <em>getSession</em> method.
 * The initial handshake on this connection can be initiated in
 * one of three ways: <UL>
 *
 *      <LI> calling <code>startHandshake</code> which explicitly
 *              begins handshakes, or
 *      <LI> any attempt to read or write application data on
 *              this socket causes an implicit handshake, or
 *      <LI> a call to <code>getSession</code> tries to set up a session
 *              if there is no currently valid session, and
 *              an implicit handshake is done.
 * </UL>
 *
 * <P>If handshaking fails for any reason, the <code>SSLSocket</code>
 * is closed, and no further communications can be done.
 *
 * <P>There are two groups of cipher suites which you will need to know
 * about when managing cipher suites: <UL>
 *
 *      <LI> <em>Supported</em> cipher suites:  all the suites which are
 *      supported by the SSL implementation.  This list is reported
 *      using <em>getSupportedCipherSuites</em>.
 *
 *      <LI> <em>Enabled</em> cipher suites, which may be fewer
 *      than the full set of supported suites.  This group is
 *      set using the <em>setEnabledCipherSuites</em> method, and
 *      queried using the <em>getEnabledCipherSuites</em> method.
 *      Initially, a default set of cipher suites will be enabled on
 *      a new socket that represents the minimum suggested configuration.
 *
 *      </UL>
 *
 * <P> Implementation defaults require that only cipher
 * suites which authenticate servers and provide confidentiality
 * be enabled by default.
 * Only if both sides explicitly agree to unauthenticated and/or
 * non-private (unencrypted) communications will such a ciphersuite be
 * selected.
 *
 * <P>When <code>SSLSocket</code>s are first created, no handshaking
 * is done so that applications may first set their communication
 * preferences:  what cipher suites to use, whether the socket should be
 * in client or server mode, etc.
 * However, security is always provided by the time that application data
 * is sent over the connection.
 *
 * <P> You may register to receive event notification of handshake
 * completion.  This involves
 * the use of two additional classes.  <em>HandshakeCompletedEvent</em>
 * objects are passed to <em>HandshakeCompletedListener</em> instances,
 * which are registered by users of this API.
 *
 * <code>SSLSocket</code>s are created by <code>SSLSocketFactory</code>s,
 * or by <code>accept</code>ing a connection from a
 * <code>SSLServerSocket</code>.
 *
 * <P>A SSL socket must choose to operate in the client or server mode.
 * This will determine who begins the handshaking process, as well
 * as which messages should be sent by each party.  Each
 * connection must have one client and one server, or handshaking
 * will not progress properly.  Once the initial handshaking has started, a
 * socket can not switch between client and server modes, even when
 * performing renegotiations.
 *
 * @see java.net.Socket
 * @see SSLServerSocket
 * @see SSLSocketFactory
 *
 * @since 1.4
 */

class ESSLSocket: public ESocket {
public:
	virtual ~ESSLSocket();

	/**
	 * Constructs an uninitialized, unconnected TCP socket.
	 */
	ESSLSocket();

	/**
	 * Constructs a TCP connection to a named host at a specified port.
	 * This acts as the SSL client.
	 * <p>
	 * If there is a security manager, its <code>checkConnect</code>
	 * method is called with the host address and <code>port</code>
	 * as its arguments. This could result in a SecurityException.
	 *
	 * @param host name of the host with which to connect, or
	 *        <code>null</code> for the loopback address.
	 * @param port number of the server's port
	 * @throws IOException if an I/O error occurs when creating the socket
	 * @throws SecurityException if a security manager exists and its
	 *         <code>checkConnect</code> method doesn't allow the operation.
	 * @throws UnknownHostException if the host is not known
	 * @throws IllegalArgumentException if the port parameter is outside the
	 *         specified range of valid port values, which is between 0 and
	 *         65535, inclusive.
	 * @see SecurityManager#checkConnect
	 */
	ESSLSocket(const char* host, int port) THROWS2(EUnknownHostException, EIOException);

	/**
	 * Constructs a TCP connection to a server at a specified address
	 * and port.  This acts as the SSL client.
	 * <p>
	 * If there is a security manager, its <code>checkConnect</code>
	 * method is called with the host address and <code>port</code>
	 * as its arguments. This could result in a SecurityException.
	 *
	 * @param address the server's host
	 * @param port its port
	 * @throws IOException if an I/O error occurs when creating the socket
	 * @throws SecurityException if a security manager exists and its
	 *         <code>checkConnect</code> method doesn't allow the operation.
	 * @throws IllegalArgumentException if the port parameter is outside the
	 *         specified range of valid port values, which is between 0 and
	 *         65535, inclusive.
	 * @throws NullPointerException if <code>address</code> is null.
	 * @see SecurityManager#checkConnect
	 */
	ESSLSocket(EInetAddress *address, int port) THROWS(EIOException);

	/**
	 * Constructs an SSL connection to a named host at a specified port,
	 * binding the client side of the connection a given address and port.
	 * This acts as the SSL client.
	 * <p>
	 * If there is a security manager, its <code>checkConnect</code>
	 * method is called with the host address and <code>port</code>
	 * as its arguments. This could result in a SecurityException.
	 *
	 * @param host name of the host with which to connect, or
	 *        <code>null</code> for the loopback address.
	 * @param port number of the server's port
	 * @param clientAddress the client's address the socket is bound to, or
	 *        <code>null</code> for the <code>anyLocal</code> address.
	 * @param clientPort the client's port the socket is bound to, or
	 *        <code>zero</code> for a system selected free port.
	 * @throws IOException if an I/O error occurs when creating the socket
	 * @throws SecurityException if a security manager exists and its
	 *         <code>checkConnect</code> method doesn't allow the operation.
	 * @throws UnknownHostException if the host is not known
	 * @throws IllegalArgumentException if the port parameter or clientPort
	 *         parameter is outside the specified range of valid port values,
	 *         which is between 0 and 65535, inclusive.
	 * @see SecurityManager#checkConnect
	 */
	ESSLSocket(const char* host, int port, EInetAddress *localAddr,
			    int localPort) THROWS2(EUnknownHostException,EIOException);

	/**
	 * Constructs an SSL connection to a server at a specified address
	 * and TCP port, binding the client side of the connection a given
	 * address and port.  This acts as the SSL client.
	 * <p>
	 * If there is a security manager, its <code>checkConnect</code>
	 * method is called with the host address and <code>port</code>
	 * as its arguments. This could result in a SecurityException.
	 *
	 * @param address the server's host
	 * @param port its port
	 * @param clientAddress the client's address the socket is bound to, or
	 *        <code>null</code> for the <code>anyLocal</code> address.
	 * @param clientPort the client's port the socket is bound to, or
	 *        <code>zero</code> for a system selected free port.
	 * @throws IOException if an I/O error occurs when creating the socket
	 * @throws SecurityException if a security manager exists and its
	 *         <code>checkConnect</code> method doesn't allow the operation.
	 * @throws IllegalArgumentException if the port parameter or clientPort
	 *         parameter is outside the specified range of valid port values,
	 *         which is between 0 and 65535, inclusive.
	 * @throws NullPointerException if <code>address</code> is null.
	 * @see SecurityManager#checkConnect
	 */
	ESSLSocket(EInetAddress *address, int port, EInetAddress *localAddr,
			    int localPort) THROWS(EIOException);

	//TODO:
	ESSLSocket(const ESSLSocket& that);
	ESSLSocket& operator= (const ESSLSocket& that);

	/**
	 * Connects this socket to the server with a specified timeout
	 * value.
	 *
	 * This method is either called on an unconnected SSLSocketImpl by the
	 * application, or it is called in the constructor of a regular
	 * SSLSocketImpl. If we are layering on top on another socket, then
	 * this method should not be called, because we assume that the
	 * underlying socket is already connected by the time it is passed to
	 * us.
	 *
	 * @param   endpoint the <code>SocketAddress</code>
	 * @param   timeout  the timeout value to be used, 0 is no timeout
	 * @throws  IOException if an error occurs during the connection
	 * @throws  SocketTimeoutException if timeout expires before connecting
	 */
	virtual void connect(EInetSocketAddress *endpoint, int timeout=0) THROWS(EIOException);
	virtual void connect(const char *hostname, int port, int timeout=0) THROWS(EIOException);

	/**
	 * Gets an input stream to read from the peer on the other side.
	 * Data read from this stream was always integrity protected in
	 * transit, and will usually have been confidentiality protected.
	 */
	virtual EInputStream* getInputStream() THROWS(EIOException);

	/**
	 * Gets an output stream to write to the peer on the other side.
	 * Data written on this stream is always integrity protected, and
	 * will usually be confidentiality protected.
	 */
	virtual EOutputStream* getOutputStream() THROWS(EIOException);

	/**
	 * The semantics of shutdownInput is not supported in TLS 1.0
	 * spec. Thus when the method is called on an SSL socket, an
	 * UnsupportedOperationException will be thrown.
	 *
	 * @throws UnsupportedOperationException
	 */
	virtual void shutdownInput() THROWS(EIOException);

	/**
	 * The semantics of shutdownOutput is not supported in TLS 1.0
	 * spec. Thus when the method is called on an SSL socket, an
	 * UnsupportedOperationException will be thrown.
	 *
	 * @throws UnsupportedOperationException
	 */
	virtual void shutdownOutput() THROWS(EIOException);

	/**
	 * shutdownInput() && shutdownOutput()
	 */
	virtual void shutdown() THROWS(EIOException);

	/**
	 * Closes the SSL connection.  SSL includes an application level
	 * shutdown handshake; you should close SSL sockets explicitly
	 * rather than leaving it for finalization, so that your remote
	 * peer does not experience a protocol error.
	 */
	virtual void close() THROWS(EIOException);

	/**
	 * Returns the names of the cipher suites which could be enabled for use
	 * on this connection.  Normally, only a subset of these will actually
	 * be enabled by default, since this list may include cipher suites which
	 * do not meet quality of service requirements for those defaults.  Such
	 * cipher suites might be useful in specialized applications.
	 *
	 * @return an array of cipher suite names
	 * @see #getEnabledCipherSuites()
	 * @see #setEnabledCipherSuites(String [])
	 */
	EA<EString*> getSupportedCipherSuites();

	/**
	 * Returns the names of the SSL cipher suites which are currently
	 * enabled for use on this connection.  When an SSLSocket is first
	 * created, all enabled cipher suites support a minimum quality of
	 * service.  Thus, in some environments this value might be empty.
	 * <P>
	 * Even if a suite has been enabled, it might never be used.  (For
	 * example, the peer does not support it, the requisite certificates
	 * (and private keys) for the suite are not available, or an
	 * anonymous suite is enabled but authentication is required.
	 *
	 * @return an array of cipher suite names
	 * @see #getSupportedCipherSuites()
	 * @see #setEnabledCipherSuites(String [])
	 */
	EA<EString*> getEnabledCipherSuites();

protected:
	friend class SSLSocketInputStream;
	friend class SSLSocketOutputStream;
	friend class ESSLServerSocket;

	SSL* ssl;
	SSL_CTX* ctx;

	virtual int read(void *b, int len) THROWS(EIOException);
	virtual void write(const void *b, int len) THROWS(EIOException);

	/**
	 * Initializes a new instance of this class from fd
	 */
	ESSLSocket(SSL* ssl, const int fd, struct ip_addr *raddr, int rport) THROWS(EIOException);

private:
	void doneConnect() THROWS(EIOException);
};

} /* namespace efc */
#endif //!HAVE_OPENSSL
#endif /* ESSLSOCKET_HH_ */
