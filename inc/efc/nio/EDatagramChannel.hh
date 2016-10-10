/*
 * EDatagramChannel.hh
 *
 *  Created on: 2016-8-24
 *      Author: cxxjava@163.com
 */

#ifndef EDATAGRAMCHANNEL_HH_
#define EDATAGRAMCHANNEL_HH_

#include "ESelectableChannel.hh"
#include "EByteChannel.hh"
#include "EMulticastChannel.hh"
#include "EDatagramSocket.hh"
#include "EMembershipKey.hh"
#include "EInetSocketAddress.hh"

namespace efc {
namespace nio {

/**
 * A selectable channel for datagram-oriented sockets.
 *
 * <p> A datagram channel is created by invoking one of the {@link #open open} methods
 * of this class. It is not possible to create a channel for an arbitrary,
 * pre-existing datagram socket. A newly-created datagram channel is open but not
 * connected. A datagram channel need not be connected in order for the {@link #send
 * send} and {@link #receive receive} methods to be used.  A datagram channel may be
 * connected, by invoking its {@link #connect connect} method, in order to
 * avoid the overhead of the security checks are otherwise performed as part of
 * every send and receive operation.  A datagram channel must be connected in
 * order to use the {@link #read(java.nio.ByteBuffer) read} and {@link
 * #write(java.nio.ByteBuffer) write} methods, since those methods do not
 * accept or return socket addresses.
 *
 * <p> Once connected, a datagram channel remains connected until it is
 * disconnected or closed.  Whether or not a datagram channel is connected may
 * be determined by invoking its {@link #isConnected isConnected} method.
 *
 * <p> Socket options are configured using the {@link #setOption(SocketOption,Object)
 * setOption} method. A datagram channel to an Internet Protocol socket supports
 * the following options:
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
 *     <td> {@link java.net.StandardSocketOptions#SO_REUSEADDR SO_REUSEADDR} </td>
 *     <td> Re-use address </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#SO_BROADCAST SO_BROADCAST} </td>
 *     <td> Allow transmission of broadcast datagrams </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#IP_TOS IP_TOS} </td>
 *     <td> The Type of Service (ToS) octet in the Internet Protocol (IP) header </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#IP_MULTICAST_IF IP_MULTICAST_IF} </td>
 *     <td> The network interface for Internet Protocol (IP) multicast datagrams </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#IP_MULTICAST_TTL
 *       IP_MULTICAST_TTL} </td>
 *     <td> The <em>time-to-live</em> for Internet Protocol (IP) multicast
 *       datagrams </td>
 *   </tr>
 *   <tr>
 *     <td> {@link java.net.StandardSocketOptions#IP_MULTICAST_LOOP
 *       IP_MULTICAST_LOOP} </td>
 *     <td> Loopback for Internet Protocol (IP) multicast datagrams </td>
 *   </tr>
 * </table>
 * </blockquote>
 * Additional (implementation specific) options may also be supported.
 *
 * <p> Datagram channels are safe for use by multiple concurrent threads.  They
 * support concurrent reading and writing, though at most one thread may be
 * reading and at most one thread may be writing at any given time.  </p>
 *
 * @author Mark Reinhold
 * @author JSR-51 Expert Group
 * @since 1.4
 */

class EDatagramSocketAdaptor;
class EMembershipRegistry;
class EDatagramChannel: public ESelectableChannel, virtual public EMulticastChannel {
public:
	virtual ~EDatagramChannel();

	/**
	 * Opens a datagram channel.
	 *
	 * <p> The new channel is created by invoking the {@link
	 * java.nio.channels.spi.SelectorProvider#openDatagramChannel()
	 * openDatagramChannel} method of the system-wide default {@link
	 * java.nio.channels.spi.SelectorProvider} object.  The channel will not be
	 * connected.
	 *
	 * <p> The {@link ProtocolFamily ProtocolFamily} of the channel's socket
	 * is platform (and possibly configuration) dependent and therefore unspecified.
	 * The {@link #open(ProtocolFamily) open} allows the protocol family to be
	 * selected when opening a datagram channel, and should be used to open
	 * datagram channels that are intended for Internet Protocol multicasting.
	 *
	 * @return  A new datagram channel
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 */
	static EDatagramChannel* open() THROWS(EIOException);

	/**
	 * Returns an operation set identifying this channel's supported
	 * operations.
	 *
	 * <p> Datagram channels support reading and writing, so this method
	 * returns <tt>(</tt>{@link SelectionKey#OP_READ} <tt>|</tt>&nbsp;{@link
	 * SelectionKey#OP_WRITE}<tt>)</tt>.  </p>
	 *
	 * @return  The valid-operation set
	 */
	virtual int validOps();

	// -- Socket-specific operations --

	/**
	 * @throws  AlreadyBoundException               {@inheritDoc}
	 * @throws  UnsupportedAddressTypeException     {@inheritDoc}
	 * @throws  ClosedChannelException              {@inheritDoc}
	 * @throws  IOException                         {@inheritDoc}
	 * @throws  SecurityException
	 *          If a security manager has been installed and its {@link
	 *          SecurityManager#checkListen checkListen} method denies the
	 *          operation
	 *
	 * @since 1.7
	 */
	virtual EDatagramChannel* bind(EInetSocketAddress* local) THROWS(EIOException);
	virtual EDatagramChannel* bind(const char* hostname, int port) THROWS(EIOException);

	/**
	 * Retrieves a datagram socket associated with this channel.
	 *
	 * <p> The returned object will not declare any public methods that are not
	 * declared in the {@link java.net.DatagramSocket} class.  </p>
	 *
	 * @return  A datagram socket associated with this channel
	 */
	virtual EDatagramSocket* socket();

	/**
	 * Tells whether or not this channel's socket is connected.
	 *
	 * @return  {@code true} if, and only if, this channel's socket
	 *          is {@link #isOpen open} and connected
	 */
	virtual boolean isConnected();

	/**
	 * Connects this channel's socket.
	 *
	 * <p> The channel's socket is configured so that it only receives
	 * datagrams from, and sends datagrams to, the given remote <i>peer</i>
	 * address.  Once connected, datagrams may not be received from or sent to
	 * any other address.  A datagram socket remains connected until it is
	 * explicitly disconnected or until it is closed.
	 *
	 * <p> This method performs exactly the same security checks as the {@link
	 * java.net.DatagramSocket#connect connect} method of the {@link
	 * java.net.DatagramSocket} class.  That is, if a security manager has been
	 * installed then this method verifies that its {@link
	 * java.lang.SecurityManager#checkAccept checkAccept} and {@link
	 * java.lang.SecurityManager#checkConnect checkConnect} methods permit
	 * datagrams to be received from and sent to, respectively, the given
	 * remote address.
	 *
	 * <p> This method may be invoked at any time.  It will not have any effect
	 * on read or write operations that are already in progress at the moment
	 * that it is invoked. If this channel's socket is not bound then this method
	 * will first cause the socket to be bound to an address that is assigned
	 * automatically, as if invoking the {@link #bind bind} method with a
	 * parameter of {@code null}. </p>
	 *
	 * @param  remote
	 *         The remote address to which this channel is to be connected
	 *
	 * @return  This datagram channel
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
	 * @throws  SecurityException
	 *          If a security manager has been installed
	 *          and it does not permit access to the given remote address
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual EDatagramChannel* connect(EInetSocketAddress* remote) THROWS(EIOException);
	virtual EDatagramChannel* connect(const char* hostname, int port) THROWS(EIOException);

	/**
	 * Disconnects this channel's socket.
	 *
	 * <p> The channel's socket is configured so that it can receive datagrams
	 * from, and sends datagrams to, any remote address so long as the security
	 * manager, if installed, permits it.
	 *
	 * <p> This method may be invoked at any time.  It will not have any effect
	 * on read or write operations that are already in progress at the moment
	 * that it is invoked.
	 *
	 * <p> If this channel's socket is not connected, or if the channel is
	 * closed, then invoking this method has no effect.  </p>
	 *
	 * @return  This datagram channel
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual EDatagramChannel* disconnect() THROWS(EIOException);

	/**
	 * Returns the remote address to which this channel's socket is connected.
	 *
	 * @return  The remote address; {@code null} if the channel's socket is not
	 *          connected
	 *
	 * @throws  ClosedChannelException
	 *          If the channel is closed
	 * @throws  IOException
	 *          If an I/O error occurs
	 *
	 * @since 1.7
	 */
	virtual EInetSocketAddress* getRemoteAddress() THROWS(EIOException);

	/**
	 * Receives a datagram via this channel.
	 *
	 * <p> If a datagram is immediately available, or if this channel is in
	 * blocking mode and one eventually becomes available, then the datagram is
	 * copied into the given byte buffer and its source address is returned.
	 * If this channel is in non-blocking mode and a datagram is not
	 * immediately available then this method immediately returns
	 * <tt>null</tt>.
	 *
	 * <p> The datagram is transferred into the given byte buffer starting at
	 * its current position, as if by a regular {@link
	 * ReadableByteChannel#read(java.nio.ByteBuffer) read} operation.  If there
	 * are fewer bytes remaining in the buffer than are required to hold the
	 * datagram then the remainder of the datagram is silently discarded.
	 *
	 * <p> This method performs exactly the same security checks as the {@link
	 * java.net.DatagramSocket#receive receive} method of the {@link
	 * java.net.DatagramSocket} class.  That is, if the socket is not connected
	 * to a specific remote address and a security manager has been installed
	 * then for each datagram received this method verifies that the source's
	 * address and port number are permitted by the security manager's {@link
	 * java.lang.SecurityManager#checkAccept checkAccept} method.  The overhead
	 * of this security check can be avoided by first connecting the socket via
	 * the {@link #connect connect} method.
	 *
	 * <p> This method may be invoked at any time.  If another thread has
	 * already initiated a read operation upon this channel, however, then an
	 * invocation of this method will block until the first operation is
	 * complete. If this channel's socket is not bound then this method will
	 * first cause the socket to be bound to an address that is assigned
	 * automatically, as if invoking the {@link #bind bind} method with a
	 * parameter of {@code null}. </p>
	 *
	 * @param  dst
	 *         The buffer into which the datagram is to be transferred
	 *
	 * @return  The datagram's source address,
	 *          or <tt>null</tt> if this channel is in non-blocking mode
	 *          and no datagram was immediately available
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the read operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the read operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  SecurityException
	 *          If a security manager has been installed
	 *          and it does not permit datagrams to be accepted
	 *          from the datagram's sender
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual sp<EInetSocketAddress> receive(EIOByteBuffer* dst) THROWS(EIOException);

	/**
	 * Sends a datagram via this channel.
	 *
	 * <p> If this channel is in non-blocking mode and there is sufficient room
	 * in the underlying output buffer, or if this channel is in blocking mode
	 * and sufficient room becomes available, then the remaining bytes in the
	 * given buffer are transmitted as a single datagram to the given target
	 * address.
	 *
	 * <p> The datagram is transferred from the byte buffer as if by a regular
	 * {@link WritableByteChannel#write(java.nio.ByteBuffer) write} operation.
	 *
	 * <p> This method performs exactly the same security checks as the {@link
	 * java.net.DatagramSocket#send send} method of the {@link
	 * java.net.DatagramSocket} class.  That is, if the socket is not connected
	 * to a specific remote address and a security manager has been installed
	 * then for each datagram sent this method verifies that the target address
	 * and port number are permitted by the security manager's {@link
	 * java.lang.SecurityManager#checkConnect checkConnect} method.  The
	 * overhead of this security check can be avoided by first connecting the
	 * socket via the {@link #connect connect} method.
	 *
	 * <p> This method may be invoked at any time.  If another thread has
	 * already initiated a write operation upon this channel, however, then an
	 * invocation of this method will block until the first operation is
	 * complete. If this channel's socket is not bound then this method will
	 * first cause the socket to be bound to an address that is assigned
	 * automatically, as if by invoking the {@link #bind bind} method with a
	 * parameter of {@code null}. </p>
	 *
	 * @param  src
	 *         The buffer containing the datagram to be sent
	 *
	 * @param  target
	 *         The address to which the datagram is to be sent
	 *
	 * @return   The number of bytes sent, which will be either the number
	 *           of bytes that were remaining in the source buffer when this
	 *           method was invoked or, if this channel is non-blocking, may be
	 *           zero if there was insufficient room for the datagram in the
	 *           underlying output buffer
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  AsynchronousCloseException
	 *          If another thread closes this channel
	 *          while the read operation is in progress
	 *
	 * @throws  ClosedByInterruptException
	 *          If another thread interrupts the current thread
	 *          while the read operation is in progress, thereby
	 *          closing the channel and setting the current thread's
	 *          interrupt status
	 *
	 * @throws  SecurityException
	 *          If a security manager has been installed
	 *          and it does not permit datagrams to be sent
	 *          to the given address
	 *
	 * @throws  IOException
	 *          If some other I/O error occurs
	 */
	virtual int send(EIOByteBuffer* src, EInetSocketAddress* target) THROWS(EIOException);
	virtual int send(EIOByteBuffer* src, const char* hostname, int port) THROWS(EIOException);


	// -- ByteChannel operations --

	/**
	 * Reads a datagram from this channel.
	 *
	 * <p> This method may only be invoked if this channel's socket is
	 * connected, and it only accepts datagrams from the socket's peer.  If
	 * there are more bytes in the datagram than remain in the given buffer
	 * then the remainder of the datagram is silently discarded.  Otherwise
	 * this method behaves exactly as specified in the {@link
	 * ReadableByteChannel} interface.  </p>
	 *
	 * @throws  NotYetConnectedException
	 *          If this channel's socket is not connected
	 */
	virtual int read(EIOByteBuffer* dst) THROWS(EIOException);

	/**
	 * Reads a datagram from this channel.
	 *
	 * <p> This method may only be invoked if this channel's socket is
	 * connected, and it only accepts datagrams from the socket's peer.  If
	 * there are more bytes in the datagram than remain in the given buffers
	 * then the remainder of the datagram is silently discarded.  Otherwise
	 * this method behaves exactly as specified in the {@link
	 * ScatteringByteChannel} interface.  </p>
	 *
	 * @throws  NotYetConnectedException
	 *          If this channel's socket is not connected
	 */
	virtual long read(EA<EIOByteBuffer*>* dsts, int offset, int length) THROWS(EIOException);

	/**
	 * Reads a datagram from this channel.
	 *
	 * <p> This method may only be invoked if this channel's socket is
	 * connected, and it only accepts datagrams from the socket's peer.  If
	 * there are more bytes in the datagram than remain in the given buffers
	 * then the remainder of the datagram is silently discarded.  Otherwise
	 * this method behaves exactly as specified in the {@link
	 * ScatteringByteChannel} interface.  </p>
	 *
	 * @throws  NotYetConnectedException
	 *          If this channel's socket is not connected
	 */
	virtual long read(EA<EIOByteBuffer*>* dsts) THROWS(EIOException);

	/**
	 * Writes a datagram to this channel.
	 *
	 * <p> This method may only be invoked if this channel's socket is
	 * connected, in which case it sends datagrams directly to the socket's
	 * peer.  Otherwise it behaves exactly as specified in the {@link
	 * WritableByteChannel} interface.  </p>
	 *
	 * @throws  NotYetConnectedException
	 *          If this channel's socket is not connected
	 */
	virtual int write(EIOByteBuffer* src) THROWS(EIOException);

	/**
	 * Writes a datagram to this channel.
	 *
	 * <p> This method may only be invoked if this channel's socket is
	 * connected, in which case it sends datagrams directly to the socket's
	 * peer.  Otherwise it behaves exactly as specified in the {@link
	 * GatheringByteChannel} interface.  </p>
	 *
	 * @return   The number of bytes sent, which will be either the number
	 *           of bytes that were remaining in the source buffer when this
	 *           method was invoked or, if this channel is non-blocking, may be
	 *           zero if there was insufficient room for the datagram in the
	 *           underlying output buffer
	 *
	 * @throws  NotYetConnectedException
	 *          If this channel's socket is not connected
	 */
	virtual long write(EA<EIOByteBuffer*>* srcs, int offset, int length) THROWS(EIOException);

	/**
	 * Writes a datagram to this channel.
	 *
	 * <p> This method may only be invoked if this channel's socket is
	 * connected, in which case it sends datagrams directly to the socket's
	 * peer.  Otherwise it behaves exactly as specified in the {@link
	 * GatheringByteChannel} interface.  </p>
	 *
	 * @return   The number of bytes sent, which will be either the number
	 *           of bytes that were remaining in the source buffer when this
	 *           method was invoked or, if this channel is non-blocking, may be
	 *           zero if there was insufficient room for the datagram in the
	 *           underlying output buffer
	 *
	 * @throws  NotYetConnectedException
	 *          If this channel's socket is not connected
	 */
	virtual long write(EA<EIOByteBuffer*>* srcs) THROWS(EIOException);

	/**
	 * {@inheritDoc}
	 * <p>
	 * If there is a security manager set, its {@code checkConnect} method is
	 * called with the local address and {@code -1} as its arguments to see
	 * if the operation is allowed. If the operation is not allowed,
	 * a {@code SocketAddress} representing the
	 * {@link java.net.InetAddress#getLoopbackAddress loopback} address and the
	 * local port of the channel's socket is returned.
	 *
	 * @return  The {@code SocketAddress} that the socket is bound to, or the
	 *          {@code SocketAddress} representing the loopback address if
	 *          denied by the security manager, or {@code null} if the
	 *          channel's socket is not bound
	 *
	 * @throws  ClosedChannelException     {@inheritDoc}
	 * @throws  IOException                {@inheritDoc}
	 */
	virtual EInetSocketAddress* getLocalAddress() THROWS(EIOException);

	/*
	 * {@inheritDoc}
	 */
	virtual EMembershipKey* join(EInetAddress* group, ENetworkInterface* interf, EInetAddress* source=null) THROWS(EIOException);

	/*
	 * {@inheritDoc}
	 */
	virtual void setOption(int optID, const void* optval, int optlen) THROWS(ESocketException);
	virtual void getOption(int optID, void* optval, int* optlen) THROWS(ESocketException);

	virtual int getFDVal();

	virtual EStringBase toString();

protected:
	virtual void implConfigureBlocking(boolean block) THROWS(EIOException);
	virtual void implCloseSelectableChannel() THROWS(EIOException);

	virtual boolean translateAndUpdateReadyOps(int ops, ESelectionKey* sk);
	virtual boolean translateAndSetReadyOps(int ops, ESelectionKey* sk);

	virtual void translateAndSetInterestOps(int ops, ESelectionKey* sk);

	virtual void kill() THROWS(EIOException);

private:
	friend class EMembershipKey;
	friend class EDatagramSocketAdaptor;

	// fd value needed for dev/poll. This value will remain valid
	// even after the value in the file descriptor object has been set to -1
	int _fd;// = -1;

	// IDs of native threads doing reads and writes, for signalling
	volatile es_os_thread_t _readerThread;	// = 0;
	volatile es_os_thread_t _writerThread;	// = 0;

	// Lock held by current reading or connecting thread
	EReentrantLock _readLock;

	// Lock held by current writing or connecting thread
	EReentrantLock _writeLock;

	// Lock held by any thread that modifies the state fields declared below
	// DO NOT invoke a blocking I/O operation while holding this lock!
	EReentrantLock _stateLock;

	// State, increases monotonically
	byte _state;	// = ST_UNINITIALIZED;

	// -- The following fields are protected by stateLock

	// Binding
	sp<EInetSocketAddress> _localAddress;
	sp<EInetSocketAddress> _remoteAddress;

	// Our socket adaptor, if any
	EDatagramSocketAdaptor* _socket;

	// Multicast support
	EMembershipRegistry* _registry;

	struct Sender {
		int addr;
		int port;

		Sender(): addr(0), port(0) {
		}
	};
	Sender _sender;// = {0, 0};

	// -- End of fields protected by stateLock

	// Safe free when _socket != null;
	boolean _freeing;// = false;

	/**
	 * Initializes a new instance of this class.
	 */
	EDatagramChannel();

	// package-private
	void drop(EMembershipKey* key);
	int poll(int events, llong timeout) THROWS(EIOException);

	/**
     * Block datagrams from given source if a memory to receive all
     * datagrams.
     */
    void block(EMembershipKey* key, EInetAddress* source) THROWS(EIOException);

    /**
	 * Unblock given source.
	 */
	void unblock(EMembershipKey* key, EInetAddress* source);

	void ensureOpen() THROWS(EClosedChannelException);
	void ensureOpenAndUnconnected() THROWS(EIOException);

	/**
	 * Translates native poll revent set into a ready operation set
	 */
	boolean translateReadyOps(int ops, int initialOps, ESelectionKey* sk);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EDATAGRAMCHANNEL_HH_ */
