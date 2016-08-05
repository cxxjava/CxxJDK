/*
 * ESocketChannel.hh
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#ifndef ESOCKETCHANNEL_HH_
#define ESOCKETCHANNEL_HH_

#include "EA.hh"
#include "ESocket.hh"
#include "EByteChannel.hh"
#include "EIOByteBuffer.hh"
#include "ESelectionKey.hh"
#include "ESelector.hh"
#include "EInetSocketAddress.hh"
#include "EReentrantLock.hh"
#include "ESocketChannel.hh"
#include "ESynchronizeable.hh"
#include "EIOException.hh"
#include "ENullPointerException.hh"
#include "EClosedChannelException.hh"
#include "EAlreadyConnectedException.hh"
#include "EConnectionPendingException.hh"
#include "ENotYetConnectedException.hh"
#include "EAsynchronousCloseException.hh"
#include "EAlreadyBoundException.hh"

namespace efc {
namespace nio {

class ESocketAdaptor;
class ESocketChannel: public ESelectableChannel, virtual public EByteChannel {
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
	static ESocketChannel* open(EInetSocketAddress* remote)
			THROWS(EIOException);

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
	int validOps();

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
	boolean connect(EInetSocketAddress* remote) THROWS(EIOException);

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
	int read(EIOByteBuffer* dst) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	long read(EA<EIOByteBuffer*>* dsts, int offset, int length) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	long read(EA<EIOByteBuffer*>* dsts) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	int write(EIOByteBuffer* src) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	long write(EA<EIOByteBuffer*>* srcs, int offset, int length) THROWS(EIOException);

	/**
	 * @throws  NotYetConnectedException
	 *          If this channel is not yet connected
	 */
	long write(EA<EIOByteBuffer*>* srcs) THROWS(EIOException);

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

	int getFDVal();

	EStringBase toString();

protected:
	void implConfigureBlocking(boolean block) THROWS(EIOException);
	void implCloseSelectableChannel() THROWS(EIOException);

	boolean translateAndUpdateReadyOps(int ops, ESelectionKey* sk);
	boolean translateAndSetReadyOps(int ops, ESelectionKey* sk);

	void translateAndSetInterestOps(int ops, ESelectionKey* sk);

	void kill() THROWS(EIOException);

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
