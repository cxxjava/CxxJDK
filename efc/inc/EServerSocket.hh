/**
 * EServerSocket.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EServerSocket_HH_
#define EServerSocket_HH_

#include "ECloseable.hh"
#include "ESocket.hh"
#include "ESynchronizeable.hh"
#include "ESocketOptions.hh"
#include "EInetAddress.hh"
#include "EInetSocketAddress.hh"
#include "EIOException.hh"
#include "ESocketException.hh"
#include "EBindException.hh"
#include "../nio/inc/EServerSocketChannel.hh"

namespace efc {

/**
 * This class implements server sockets. A server socket waits for 
 * requests to come in over the network. It performs some operation 
 * based on that request, and then possibly returns a result to the requester.
 * <p>
 * The actual work of the server socket is performed by an instance 
 * of the <code>SocketImpl</code> class. An application can 
 * change the socket factory that creates the socket 
 * implementation to configure itself to create sockets 
 * appropriate to the local firewall. 
 *
 * @version 1.91, 03/08/07
 * @see     java.net.SocketImpl
 * @see     java.net.ServerSocket#setSocketFactory(java.net.SocketImplFactory)
 * @see     java.nio.channels.ServerSocketChannel
 * @since   JDK1.0
 */

class EServerSocket : public ESynchronizeable, virtual public ESocketOptions, virtual public ECloseable {
public:
	virtual ~EServerSocket();
	
	/**
     * Creates an unbound server socket.
     *
     * @exception IOException IO error when opening the socket.
     * @revised 1.4
     */
    EServerSocket() THROWS(EIOException);

	/** 
     * Create a server with the specified port, listen backlog, and 
     * local IP address to bind to.  The <i>bindAddr</i> argument
     * can be used on a multi-homed host for a ServerSocket that
     * will only accept connect requests to one of its addresses.
     * If <i>bindAddr</i> is null, it will default accepting
     * connections on any/all local addresses.
     * The port must be between 0 and 65535, inclusive.
     * 
     * <P>If there is a security manager, this method 
     * calls its <code>checkListen</code> method
     * with the <code>port</code> argument
     * as its argument to ensure the operation is allowed. 
     * This could result in a SecurityException.
     *
     * <P>The <code>backlog</code> argument must be a positive
     * value greater than 0. If the value passed if equal or less
     * than 0, then the default value will be assumed.
     * <P>
     * @param port the local TCP port
     * @param backlog the listen backlog
     * @param bindAddr the local InetAddress the server will bind to
     * 
     * @throws  SecurityException if a security manager exists and 
     * its <code>checkListen</code> method doesn't allow the operation.
     * 
     * @throws  IOException if an I/O error occurs when opening the socket.
     *
     * @see SocketOptions
     * @see SocketImpl
     * @see SecurityManager#checkListen
     * @since   JDK1.1
     */
    EServerSocket(int port, int backlog=50, EInetAddress *bindAddr=null) THROWS(EIOException);
    
    //TODO:
    EServerSocket(const EServerSocket& that);
    EServerSocket& operator= (const EServerSocket& that);

    /**
     *
     * Binds the <code>ServerSocket</code> to a specific address
     * (IP address and port number).
     * <p>
     * If the address is <code>null</code>, then the system will pick up
     * an ephemeral port and a valid local address to bind the socket.
     * <P>
     * The <code>backlog</code> argument must be a positive
     * value greater than 0. If the value passed if equal or less
     * than 0, then the default value will be assumed.
     * @param	endpoint	The IP address & port number to bind to.
     * @param	backlog		The listen backlog length.
     * @throws	IOException if the bind operation fails, or if the socket
     *			   is already bound.
     * @throws	SecurityException	if a <code>SecurityManager</code> is present and
     * its <code>checkListen</code> method doesn't allow the operation.
     * @throws  IllegalArgumentException if endpoint is a
     *          SocketAddress subclass not supported by this socket
     * @since 1.4
     */
    virtual void bind(EInetSocketAddress *endpoint, int backlog=50) THROWS(EIOException);
    virtual void bind(const char* hostname, int port, int backlog=50) THROWS(EIOException);
    virtual void bind(int port, int backlog=50) THROWS(EIOException);
    
    /**
     * Returns the local address of this server socket.
     *
     * @return  the address to which this socket is bound,
     *          or <code>null</code> if the socket is unbound.
     */
    virtual EInetAddress* getInetAddress();
    
    /**
     * Returns the port on which this socket is listening.
     *
     * @return  the port number to which this socket is listening or
     *	        -1 if the socket is not bound yet.
     */
    virtual int getLocalPort();
    
    /**
     * Returns the address of the endpoint this socket is bound to, or
     * <code>null</code> if it is not bound yet.
     *
     * @return a <code>SocketAddress</code> representing the local endpoint of this
     *	       socket, or <code>null</code> if it is not bound yet.
     * @see #getInetAddress()
     * @see #getLocalPort()
     * @see #bind(SocketAddress)
     * @since 1.4
     */
    virtual EInetSocketAddress* getLocalSocketAddress();
    
    /**
     * Listens for a connection to be made to this socket and accepts 
     * it. The method blocks until a connection is made. 
     *
     * <p>A new Socket <code>s</code> is created and, if there 
     * is a security manager, 
     * the security manager's <code>checkAccept</code> method is called
     * with <code>s.getInetAddress().getHostAddress()</code> and
     * <code>s.getPort()</code>
     * as its arguments to ensure the operation is allowed. 
     * This could result in a SecurityException.
     * 
     * @exception  IOException  if an I/O error occurs when waiting for a
     *               connection.
     * @exception  SecurityException  if a security manager exists and its  
     *             <code>checkAccept</code> method doesn't allow the operation.
     * @exception  SocketTimeoutException if a timeout was previously set with setSoTimeout and
     *             the timeout has been reached.
     * @exception  java.nio.channels.IllegalBlockingModeException
     *             if this socket has an associated channel, the channel is in
     *             non-blocking mode, and there is no connection ready to be
     *             accepted
     *
     * @return the new Socket
     * @see SecurityManager#checkAccept
     * @revised 1.4
     * @spec JSR-51
     *
     * @remark The returned ESocket* handle need to free!
     */
    virtual ESocket* accept() THROWS(EIOException);
    
    /**
     * Closes this socket. 
     * 
     * Any thread currently blocked in {@link #accept()} will throw
     * a {@link SocketException}.
     *
     * <p> If this socket has an associated channel then the channel is closed
     * as well.
     *
     * @exception  IOException  if an I/O error occurs when closing the socket.
     * @revised 1.4
     * @spec JSR-51
     */
    virtual void close() THROWS(EIOException);
    
    /**
     * Returns the binding state of the ServerSocket.
     *
     * @return true if the ServerSocket succesfuly bound to an address
     * @since 1.4
     */
    virtual boolean isBound();

    /**
     * Returns the closed state of the ServerSocket.
     *
     * @return true if the socket has been closed
     * @since 1.4
     */
    virtual boolean isClosed();

    /**
     * Enable/disable SO_TIMEOUT with the specified timeout, in
     * milliseconds.  With this option set to a non-zero timeout,
     * a call to accept() for this ServerSocket
     * will block for only this amount of time.  If the timeout expires,
     * a <B>java.net.SocketTimeoutException</B> is raised, though the
     * ServerSocket is still valid.  The option <B>must</B> be enabled
     * prior to entering the blocking operation to have effect.  The 
     * timeout must be > 0.
     * A timeout of zero is interpreted as an infinite timeout.  
     * @param timeout the specified timeout, in milliseconds
     * @exception SocketException if there is an error in 
     * the underlying protocol, such as a TCP error. 
     * @since   JDK1.1
     * @see #getSoTimeout()
     */
    virtual void setSoTimeout(int timeout) THROWS(ESocketException);

    /** 
     * Retrieve setting for SO_TIMEOUT.  0 returns implies that the
     * option is disabled (i.e., timeout of infinity).
     * @return the SO_TIMEOUT value
     * @exception IOException if an I/O error occurs
     * @since   JDK1.1
     * @see #setSoTimeout(int)
     */
    virtual int getSoTimeout() THROWS(EIOException);

    /**
     * Enable/disable the SO_REUSEADDR socket option.
     * <p>
     * When a TCP connection is closed the connection may remain
     * in a timeout state for a period of time after the connection
     * is closed (typically known as the <tt>TIME_WAIT</tt> state 
     * or <tt>2MSL</tt> wait state).
     * For applications using a well known socket address or port 
     * it may not be possible to bind a socket to the required
     * <tt>SocketAddress</tt> if there is a connection in the
     * timeout state involving the socket address or port. 
     * <p>
     * Enabling <tt>SO_REUSEADDR</tt> prior to binding the socket
     * using {@link #bind(SocketAddress)} allows the socket to be
     * bound even though a previous connection is in a timeout
     * state.
     * <p>
     * When a <tt>ServerSocket</tt> is created the initial setting
     * of <tt>SO_REUSEADDR</tt> is not defined. Applications can
     * use {@link #getReuseAddress()} to determine the initial 
     * setting of <tt>SO_REUSEADDR</tt>. 
     * <p>
     * The behaviour when <tt>SO_REUSEADDR</tt> is enabled or
     * disabled after a socket is bound (See {@link #isBound()})
     * is not defined.
     * 
     * @param on  whether to enable or disable the socket option
     * @exception SocketException if an error occurs enabling or
     *            disabling the <tt>SO_RESUEADDR</tt> socket option,
     *		  or the socket is closed.
     * @since 1.4
     * @see #getReuseAddress()     
     * @see #bind(SocketAddress)
     * @see #isBound()
     * @see #isClosed()
     */     
    virtual void setReuseAddress(boolean on) THROWS(ESocketException);

    /**
     * Tests if SO_REUSEADDR is enabled.
     *
     * @return a <code>boolean</code> indicating whether or not SO_REUSEADDR is enabled.
     * @exception SocketException if there is an error
     * in the underlying protocol, such as a TCP error. 
     * @since   1.4
     * @see #setReuseAddress(boolean)
     */
    virtual boolean getReuseAddress() THROWS(ESocketException);

    /**
     * Returns the implementation address and implementation port of 
     * this socket as a <code>String</code>.
     *
     * @return  a string representation of this socket.
     */
    virtual EString toString();
    
    /**
     * Sets a default proposed value for the SO_RCVBUF option for sockets 
     * accepted from this <tt>ServerSocket</tt>. The value actually set 
     * in the accepted socket must be determined by calling 
     * {@link Socket#getReceiveBufferSize()} after the socket 
     * is returned by {@link #accept()}. 
     * <p>
     * The value of SO_RCVBUF is used both to set the size of the internal
     * socket receive buffer, and to set the size of the TCP receive window
     * that is advertized to the remote peer.
     * <p>
     * It is possible to change the value subsequently, by calling 
     * {@link Socket#setReceiveBufferSize(int)}. However, if the application 
     * wishes to allow a receive window larger than 64K bytes, as defined by RFC1323
     * then the proposed value must be set in the ServerSocket <B>before</B> 
     * it is bound to a local address. This implies, that the ServerSocket must be 
     * created with the no-argument constructor, then setReceiveBufferSize() must 
     * be called and lastly the ServerSocket is bound to an address by calling bind(). 
     * <p>
     * Failure to do this will not cause an error, and the buffer size may be set to the
     * requested value but the TCP receive window in sockets accepted from 
     * this ServerSocket will be no larger than 64K bytes.
     *
     * @exception SocketException if there is an error
     * in the underlying protocol, such as a TCP error. 
     *
     * @param size the size to which to set the receive buffer
     * size. This value must be greater than 0.
     *
     * @exception IllegalArgumentException if the 
     * value is 0 or is negative.
     *
     * @since 1.4
     * @see #getReceiveBufferSize
     */
    virtual void setReceiveBufferSize (int size) THROWS(ESocketException);

    /**
     * Gets the value of the SO_RCVBUF option for this <tt>ServerSocket</tt>, 
     * that is the proposed buffer size that will be used for Sockets accepted
     * from this <tt>ServerSocket</tt>.
     * 
     * <p>Note, the value actually set in the accepted socket is determined by
     * calling {@link Socket#getReceiveBufferSize()}.
     * @return the value of the SO_RCVBUF option for this <tt>Socket</tt>.
     * @exception SocketException if there is an error
     * in the underlying protocol, such as a TCP error. 
     * @see #setReceiveBufferSize(int)
     * @since 1.4
     */
    virtual int getReceiveBufferSize() THROWS(ESocketException);
    
    /**
	 * @see ESocketOptions#setOption
	 */
    virtual void setOption(int optID, const void* optval, int optlen)
			THROWS(ESocketException);

	/**
	 * @see ESocketOptions#getOption
	 */
    virtual void getOption(int optID, void* optval, int* optlen)
			THROWS(ESocketException);

	/**
	 * Returns the native OS socket handle.
	 * @exception  IOException  if an I/O error occurs.
	 */
    virtual int getFD() THROWS(EIOException);

    /**
 	 * Returns the unique {@link java.nio.channels.ServerSocketChannel} object
 	 * associated with this socket, if any.
 	 *
 	 * <p> A server socket will have a channel if, and only if, the channel
 	 * itself was created via the {@link
 	 * java.nio.channels.ServerSocketChannel#open ServerSocketChannel.open}
 	 * method.
 	 *
 	 * @return  the server-socket channel associated with this socket,
 	 *          or <tt>null</tt> if this socket was not created
 	 *          for a channel
 	 *
 	 * @since 1.4
 	 * @spec JSR-51
 	 */
     virtual sp<nio::EServerSocketChannel> getChannel() {return null;};

protected:
     /**
	  * Only for sub class.
	  */
     EServerSocket(void* dummy);

protected:
	/* FileDescriptor */
    int socket;
    
    /* Socket Address */
	EInetSocketAddress *localSocketAddress;
	
	/**
     * Various states of this socket.
     */
    struct socketStatus {
		socketStatus(int z) :
				bound(0), closed(0) {
		}
    	es_uint8_t bound:1;// = 0;
    	es_uint8_t closed:1;// = 0;
    } status;

    EReentrantLock closeLock;// = new Object();
};

} /* namespace efc */
#endif //!EServerSocket_HH_
