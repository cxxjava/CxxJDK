/*
 * EServerSocketChannel.hh
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#ifndef ESERVERSOCKETCHANNEL_HH_
#define ESERVERSOCKETCHANNEL_HH_

#include "../../inc/EA.hh"
#include "../../inc/EThread.hh"
#include "../../inc/EServerSocket.hh"
#include "../../inc/EInetSocketAddress.hh"
#include "../../inc/concurrent/EReentrantLock.hh"
#include "../../inc/ESynchronizeable.hh"
#include "../../inc/EIOException.hh"
#include "../../inc/EUnresolvedAddressException.hh"
#include "../../inc/EAlreadyBoundException.hh"
#include "../../inc/ERuntimeException.hh"
#include "./ESocketChannel.hh"
#include "./ESelectionKey.hh"
#include "./ESelector.hh"
#include "./EClosedChannelException.hh"

namespace efc {
namespace nio {

/**
 * A selectable channel for stream-oriented listening sockets.
 *
 * <p> A server-socket channel is created by invoking the {@link #open() open}
 * method of this class.  It is not possible to create a channel for an arbitrary,
 * pre-existing {@link ServerSocket}. A newly-created server-socket channel is
 * open but not yet bound.  An attempt to invoke the {@link #accept() accept}
 * method of an unbound server-socket channel will cause a {@link NotYetBoundException}
 * to be thrown. A server-socket channel can be bound by invoking one of the
 * {@link #bind(java.net.SocketAddress,int) bind} methods defined by this class.
 *
 * <p> Socket options are configured using the {@link #setOption(SocketOption,Object)
 * setOption} method. Server-socket channels support the following options:
 * <blockquote>
 * <table border summary="Socket options">
 *   <tr>
 *     <th>Option Name</th>
 *     <th>Description</th>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_RCVBUF SO_RCVBUF} </td>
 *     <td> The size of the socket receive buffer </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_REUSEADDR SO_REUSEADDR} </td>
 *     <td> Re-use address </td>
 *   </tr>
 * </table>
 * </blockquote>
 * Additional (implementation specific) options may also be supported.
 *
 * <p> Server-socket channels are safe for use by multiple concurrent threads.
 * </p>
 *
 * @author Mark Reinhold
 * @author JSR-51 Expert Group
 * @since 1.4
 */

class EServerSocketAdaptor;
class EServerSocketChannel: public ESelectableChannel, virtual public ESocketOptions {
public:
	virtual ~EServerSocketChannel();

	/**
	 * Opens a server-socket channel.
	 *
	 * <p> The new channel is created by invoking the {@link
	 * java.nio.channels.spi.SelectorProvider#openServerSocketChannel
	 * openServerSocketChannel} method of the system-wide default {@link
	 * java.nio.channels.spi.SelectorProvider} object.
	 *
	 * <p> The new channel's socket is initially unbound; it must be bound to a
	 * specific address via one of its socket's {@link
	 * java.net.ServerSocket#bind(SocketAddress) bind} methods before
	 * connections can be accepted.  </p>
	 *
	 * @return  A new socket channel
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 */
	static EServerSocketChannel* open() THROWS(EIOException);

	/**
	 * Returns an operation set identifying this channel's supported
	 * operations.
	 *
	 * <p> Server-socket channels only support the accepting of new
	 * connections, so this method returns {@link SelectionKey#OP_ACCEPT}.
	 * </p>
	 *
	 * @return  The valid-operation set
	 */
	virtual int validOps();

	// -- ServerSocket-specific operations --

	/**
	 * Retrieves a server socket associated with this channel.
	 *
	 * <p> The returned object will not declare any public methods that are not
	 * declared in the {@link java.net.ServerSocket} class.  </p>
	 *
	 * @return  A server socket associated with this channel
	 */
	EServerSocket* socket();

	/**
	 *
	 */
	virtual void bind(EInetSocketAddress* local, int backlog=50) THROWS(EIOException);
	virtual void bind(const char* hostname, int port, int backlog=50) THROWS(EIOException);

	/**
	 * Accepts a connection made to this channel's socket.
	 *
	 * <p> If this channel is in non-blocking mode then this method will
	 * immediately return <tt>null</tt> if there are no pending connections.
	 * Otherwise it will block indefinitely until a new connection is available
	 * or an I/O error occurs.
	 *
	 * <p> The socket channel returned by this method, if any, will be in
	 * blocking mode regardless of the blocking mode of this channel.
	 *
	 * <p> This method performs exactly the same security checks as the {@link
	 * java.net.ServerSocket#accept accept} method of the {@link
	 * java.net.ServerSocket} class.  That is, if a security manager has been
	 * installed then for each new connection this method verifies that the
	 * address and port number of the connection's remote endpoint are
	 * permitted by the security manager's {@link
	 * java.lang.SecurityManager#checkAccept checkAccept} method.  </p>
	 *
	 * @return  The socket channel for the new connection,
	 *          or <tt>null</tt> if this channel is in non-blocking mode
	 *          and no connection is available to be accepted
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the accept operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the accept operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  NotYetBoundException
	 *          If this channel's socket has not yet been bound
	 *
	 * @throws  SecurityException
	 *          If a security manager has been installed
	 *          and it does not permit access to the remote endpoint
	 *          of the new connection
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	ESocketChannel* accept() THROWS(EIOException);

	/**
	 *
	 */
	virtual EInetSocketAddress* localAddress();

	/**
	 *
	 */
	virtual boolean isBound();
	
	/**
	 *
	 */
	virtual int getFDVal();

	/**
	 *
	 */
	virtual EStringBase toString();

	/*
	 * {@inheritDoc}
	 */
	virtual void setOption(int optID, const void* optval, int optlen) THROWS(ESocketException);
	virtual void getOption(int optID, void* optval, int* optlen) THROWS(ESocketException);

protected:
	/**
	 *
	 */
	boolean translateAndUpdateReadyOps(int ops, ESelectionKey* sk);
	boolean translateAndSetReadyOps(int ops, ESelectionKey* sk);

	/**
	 * Translates an interest operation set into a native poll event set
	 */
	void translateAndSetInterestOps(int ops, ESelectionKey* sk);

	void implCloseSelectableChannel() THROWS(EIOException);
	void implConfigureBlocking(boolean block) THROWS(EIOException);

	virtual void kill() THROWS(EIOException);

private:
	friend class EServerSocketAdaptor;

	// Our file descriptor
	int _fd;

	// ID of native thread currently blocked in this channel, for signalling
	volatile es_os_thread_t _thread;// = 0;

	// Lock held by thread currently blocked in this channel
	EReentrantLock _lock;

	// Lock held by any thread that modifies the state fields declared below
	// DO NOT invoke a blocking I/O operation while holding this lock!
	EReentrantLock _stateLock;

	// Our socket adaptor, if any
	EServerSocketAdaptor* _socket;

	// Binding
	EInetSocketAddress* _localAddress;// = null; // null => unbound

	// Channel state, increases monotonically
	static const int ST_UNINITIALIZED = -1;
	static const int ST_INUSE = 0;
	static const int ST_KILLED = 1;
	byte _state;	// = ST_UNINITIALIZED;

	// Safe free when _socket != null;
	boolean _freeing;// = false;

	/**
	 * Initializes a new instance of this class.
	 */
	EServerSocketChannel();

	/**
	 * Translates native poll revent set into a ready operation set
	 */
	boolean translateReadyOps(int ops, int initialOps, ESelectionKey* sk);
};

} /* namespace nio */
} /* namespace efc */

#endif /* ESERVERSOCKETCHANNEL_HH_ */
