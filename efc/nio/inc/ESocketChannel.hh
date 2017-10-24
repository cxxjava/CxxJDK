/*
 * ESocketChannel.hh
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#ifndef ESOCKETCHANNEL_HH_
#define ESOCKETCHANNEL_HH_

#include "../../inc/EA.hh"
#include "../../inc/ESocket.hh"
#include "../../inc/EInetSocketAddress.hh"
#include "../../inc/concurrent/EReentrantLock.hh"
#include "../../inc/ESynchronizeable.hh"
#include "../../inc/EIOException.hh"
#include "../../inc/ENullPointerException.hh"
#include "../../inc/EAlreadyConnectedException.hh"
#include "../../inc/ENotYetConnectedException.hh"
#include "../../inc/EAlreadyBoundException.hh"
#include "./EByteChannel.hh"
#include "./EIOByteBuffer.hh"
#include "./ESelectionKey.hh"
#include "./ESelector.hh"
#include "./ESocketChannel.hh"
#include "./EClosedChannelException.hh"
#include "./EConnectionPendingException.hh"
#include "./EAsynchronousCloseException.hh"

namespace efc {
namespace nio {

/**
 * A selectable channel for stream-oriented connecting sockets.
 *
 * <p> A socket channel is created by invoking one of the {@link #open open}
 * methods of this class.  It is not possible to create a channel for an arbitrary,
 * pre-existing socket. A newly-created socket channel is open but not yet
 * connected.  An attempt to invoke an I/O operation upon an unconnected
 * channel will cause a {@link NotYetConnectedException} to be thrown.  A
 * socket channel can be connected by invoking its {@link #connect connect}
 * method; once connected, a socket channel remains connected until it is
 * closed.  Whether or not a socket channel is connected may be determined by
 * invoking its {@link #isConnected isConnected} method.
 *
 * <p> Socket channels support <i>non-blocking connection:</i>&nbsp;A socket
 * channel may be created and the process of establishing the link to the
 * remote socket may be initiated via the {@link #connect connect} method for
 * later completion by the {@link #finishConnect finishConnect} method.
 * Whether or not a connection operation is in progress may be determined by
 * invoking the {@link #isConnectionPending isConnectionPending} method.
 *
 * <p> Socket channels support <i>asynchronous shutdown,</i> which is similar
 * to the asynchronous close operation specified in the {@link Channel} class.
 * If the input side of a socket is shut down by one thread while another
 * thread is blocked in a read operation on the socket's channel, then the read
 * operation in the blocked thread will complete without reading any bytes and
 * will return <tt>-1</tt>.  If the output side of a socket is shut down by one
 * thread while another thread is blocked in a write operation on the socket's
 * channel, then the blocked thread will receive an {@link
 * AsynchronousCloseException}.
 *
 * <p> Socket options are configured using the {@link #setOption(SocketOption,Object)
 * setOption} method. Socket channels support the following options:
 * <blockquote>
 * <table border summary="Socket options">
 *   <tr>
 *     <th>Option Name</th>
 *     <th>Description</th>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_SNDBUF SO_SNDBUF} </td>
 *     <td> The size of the socket send buffer </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_RCVBUF SO_RCVBUF} </td>
 *     <td> The size of the socket receive buffer </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_KEEPALIVE SO_KEEPALIVE} </td>
 *     <td> Keep connection alive </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_REUSEADDR SO_REUSEADDR} </td>
 *     <td> Re-use address </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_LINGER SO_LINGER} </td>
 *     <td> Linger on close if data is present (when configured in blocking mode
 *          only) </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#TCP_NODELAY TCP_NODELAY} </td>
 *     <td> Disable the Nagle algorithm </td>
 *   </tr>
 * </table>
 * </blockquote>
 * Additional (implementation specific) options may also be supported.
 *
 * <p> Socket channels are safe for use by multiple concurrent threads.  They
 * support concurrent reading and writing, though at most one thread may be
 * reading and at most one thread may be writing at any given time.  The {@link
 * #connect connect} and {@link #finishConnect finishConnect} methods are
 * mutually synchronized against each other, and an attempt to initiate a read
 * or write operation while an invocation of one of these methods is in
 * progress will block until that invocation is complete.  </p>
 *
 * @author Mark Reinhold
 * @author JSR-51 Expert Group
 * @since 1.4
 */

class ESocketAdaptor;
class ESocketChannel: public ESelectableChannel, virtual public ESocketOptions, virtual public EByteChannel {
public:
	static const int SHUT_RD_ = 0;
	static const int SHUT_WR_ = 1;
	static const int SHUT_RDWR_ = 2;

public:
	virtual ~ESocketChannel();

	/**
	 * Opens a socket channel.
	 *
	 * <p> The new channel is created by invoking the {@link
	 * java.nio.channels.spi.SelectorProvider#openSocketChannel
	 * openSocketChannel} method of the system-wide default {@link
	 * java.nio.channels.spi.SelectorProvider} object.  </p>
	 *
	 * @return  A new socket channel
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 */
	static ESocketChannel* open() THROWS(EIOException);

	/**
	 * Opens a socket channel and connects it to a remote address.
	 *
	 * <p> This convenience method works as if by invoking the {@link #open()}
	 * method, invoking the {@link #connect(SocketAddress) connect} method upon
	 * the resulting socket channel, passing it <tt>remote</tt>, and then
	 * returning that channel.  </p>
	 *
	 * @param  remote
	 *         The remote address to which the new channel is to be connected
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the connect operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the connect operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  UnresolvedAddressException
	 *          If the given remote address is not fully resolved
	 *
	 * @throws  UnsupportedAddressTypeException
	 *          If the type of the given remote address is not supported
	 *
	 * @throws  SecurityException
	 *          If a security manager has been installed
	 *          and it does not permit access to the given remote endpoint
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	static ESocketChannel* open(EInetSocketAddress* remote) THROWS(EIOException);
	static ESocketChannel* open(const char* hostname, int port) THROWS(EIOException);

	/**
	 * Returns an operation set identifying this channel's supported
	 * operations.
	 *
	 * <p> Socket channels support connecting, reading, and writing, so this
	 * method returns <tt>(</tt>{@link SelectionKey#OP_CONNECT}
	 * <tt>|</tt>&nbsp;{@link SelectionKey#OP_READ} <tt>|</tt>&nbsp;{@link
	 * SelectionKey#OP_WRITE}<tt>)</tt>.  </p>
	 *
	 * @return  The valid-operation set
	 */
	virtual int validOps();

	// -- Socket-specific operations --

	/**
	 * Retrieves a socket associated with this channel.
	 *
	 * <p> The returned object will not declare any public methods that are not
	 * declared in the {@link java.net.Socket} class.  </p>
	 *
	 * @return  A socket associated with this channel
	 */
	ESocket* socket();

	/**
	 *
	 */
	void bind(EInetSocketAddress* local) THROWS(EIOException);
	void bind(const char* hostname, int port) THROWS(EIOException);

	/**
	 * Tells whether or not this channel's network socket is connected.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if, this channel's network socket
	 *          is connected
	 */
	boolean isConnected();

	/**
	 * Tells whether or not a connection operation is in progress on this
	 * channel.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if, a connection operation has been
	 *          initiated on this channel but not yet completed by invoking the
	 *          {@link #finishConnect finishConnect} method
	 */
	boolean isConnectionPending();

	/**
	 * Connects this channel's socket.
	 *
	 * <p> If this channel is in non-blocking mode then an invocation of this
	 * method initiates a non-blocking connection operation.  If the connection
	 * is established immediately, as can happen with a local connection, then
	 * this method returns <tt>true</tt>.  Otherwise this method returns
	 * <tt>false</tt> and the connection operation must later be completed by
	 * invoking the {@link #finishConnect finishConnect} method.
	 *
	 * <p> If this channel is in blocking mode then an invocation of this
	 * method will block until the connection is established or an I/O error
	 * occurs.
	 *
	 * <p> This method performs exactly the same security checks as the {@link
	 * java.net.Socket} class.  That is, if a security manager has been
	 * installed then this method verifies that its {@link
	 * java.lang.SecurityManager#checkConnect checkConnect} method permits
	 * connecting to the address and port number of the given remote endpoint.
	 *
	 * <p> This method may be invoked at any time.  If a read or write
	 * operation upon this channel is invoked while an invocation of this
	 * method is in progress then that operation will first block until this
	 * invocation is complete.  If a connection attempt is initiated but fails,
	 * that is, if an invocation of this method throws a checked exception,
	 * then the channel will be closed.  </p>
	 *
	 * @param  remote
	 *         The remote address to which this channel is to be connected
	 *
	 * @return  <tt>true</tt> if a connection was established,
	 *          <tt>false</tt> if this channel is in non-blocking mode
	 *          and the connection operation is in progress
	 *
	 * @throws  AlreadyConnectedException
	 *          If this channel is already connected
	 *
	 * @throws  ConnectionPendingException
	 *          If a non-blocking connection operation is already in progress
	 *          on this channel
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the connect operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the connect operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  UnresolvedAddressException
	 *          If the given remote address is not fully resolved
	 *
	 * @throws  UnsupportedAddressTypeException
	 *          If the type of the given remote address is not supported
	 *
	 * @throws  SecurityException
	 *          If a security manager has been installed
	 *          and it does not permit access to the given remote endpoint
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual boolean connect(EInetSocketAddress* remote) THROWS(EIOException);
	virtual boolean connect(const char* hostname, int port) THROWS(EIOException);

	/**
	 * Finishes the process of connecting a socket channel.
	 *
	 * <p> A non-blocking connection operation is initiated by placing a socket
	 * channel in non-blocking mode and then invoking its {@link #connect
	 * connect} method.  Once the connection is established, or the attempt has
	 * failed, the socket channel will become connectable and this method may
	 * be invoked to complete the connection sequence.  If the connection
	 * operation failed then invoking this method will cause an appropriate
	 * {@link java.io.IOException} to be thrown.
	 *
	 * <p> If this channel is already connected then this method will not block
	 * and will immediately return <tt>true</tt>.  If this channel is in
	 * non-blocking mode then this method will return <tt>false</tt> if the
	 * connection process is not yet complete.  If this channel is in blocking
	 * mode then this method will block until the connection either completes
	 * or fails, and will always either return <tt>true</tt> or throw a checked
	 * exception describing the failure.
	 *
	 * <p> This method may be invoked at any time.  If a read or write
	 * operation upon this channel is invoked while an invocation of this
	 * method is in progress then that operation will first block until this
	 * invocation is complete.  If a connection attempt fails, that is, if an
	 * invocation of this method throws a checked exception, then the channel
	 * will be closed.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if, this channel's socket is now
	 *          connected
	 *
	 * @throws  NoConnectionPendingException
	 *          If this channel is not connected and a connection operation
	 *          has not been initiated
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the connect operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the connect operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	boolean finishConnect() THROWS(EIOException);

	// -- ByteChannel operations --

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	virtual int read(EIOByteBuffer* dst) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	virtual long read(EA<EIOByteBuffer*>* dsts, int offset, int length) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	virtual long read(EA<EIOByteBuffer*>* dsts) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	virtual int write(EIOByteBuffer* src) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	virtual long write(EA<EIOByteBuffer*>* srcs, int offset, int length) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	virtual long write(EA<EIOByteBuffer*>* srcs) THROWS(EIOException);

	/**
	 *
	 */
	EInetSocketAddress* localAddress();
	EInetSocketAddress* remoteAddress();

	boolean isBound();

	void shutdownInput() THROWS(EIOException);
	void shutdownOutput() THROWS(EIOException);
	boolean isInputOpen();
	boolean isOutputOpen();

	virtual int getFDVal();

	virtual EStringBase toString();

	/*
	 * {@inheritDoc}
	 */
	virtual void setOption(int optID, const void* optval, int optlen) THROWS(ESocketException);
	virtual void getOption(int optID, void* optval, int* optlen) THROWS(ESocketException);

protected:
	virtual void implConfigureBlocking(boolean block) THROWS(EIOException);
	virtual void implCloseSelectableChannel() THROWS(EIOException);

	virtual boolean translateAndUpdateReadyOps(int ops, ESelectionKey* sk);
	virtual boolean translateAndSetReadyOps(int ops, ESelectionKey* sk);

	virtual void translateAndSetInterestOps(int ops, ESelectionKey* sk);

	virtual void kill() THROWS(EIOException);

	int sendOutOfBandData(byte b) THROWS(EIOException);

private:
	friend class EServerSocketChannel;
	friend class ESocketAdaptor;

	// fd value needed for dev/poll. This value will remain valid
	// even after the value in the file descriptor object has been set to -1
	int _fd; // = -1;

	// Lock held by current reading or connecting thread
	EReentrantLock _readLock;

	// Lock held by current writing or connecting thread
	EReentrantLock _writeLock;

	// Lock held by any thread that modifies the state fields declared below
	// DO NOT invoke a blocking I/O operation while holding this lock!
	EReentrantLock _stateLock;

	// Socket adaptor, created on demand
	ESocketAdaptor* _socket;// = null;

	// Binding
	EInetSocketAddress* _localAddress;// = null;
	EInetSocketAddress* _remoteAddress;// = null;

	// IDs of native threads doing reads and writes, for signalling
	volatile es_os_thread_t _readerThread;	// = 0;
	volatile es_os_thread_t _writerThread;	// = 0;

	// Input/Output open
	boolean _isInputOpen;// = true;
	boolean _isOutputOpen;// = true;
	boolean _readyToConnect;// = false;

	// State, increases monotonically
	byte _state;	// = ST_UNINITIALIZED;

	// Safe free when _socket != null;
	boolean _freeing;// = false;

	/**
	 * Initializes a new instance of this class.
	 */
	ESocketChannel();

	/**
	 * Initializes a new instance of this class from fd
	 */
	ESocketChannel(const int fd, EInetSocketAddress* isa) THROWS(EIOException);

	boolean ensureReadOpen() THROWS(EClosedChannelException);
	void ensureOpenAndUnconnected() THROWS(EClosedChannelException);
	void readerCleanup() THROWS(EIOException);
	void ensureWriteOpen() THROWS(EClosedChannelException);
	void writerCleanup() THROWS(EIOException);

	/**
	 * Translates native poll revent ops into a ready operation ops
	 */
	boolean translateReadyOps(int ops, int initialOps, ESelectionKey* sk);
};

} /* namespace nio */
} /* namespace efc */

#endif /* ESOCKETCHANNEL_HH_ */
