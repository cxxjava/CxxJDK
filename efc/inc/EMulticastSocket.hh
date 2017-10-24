/*
 * EMulticastSocket.hh
 *
 *  Created on: 2016-8-11
 *      Author: cxxjava@163.com
 */

#ifndef EMULTICASTSOCKET_HH_
#define EMULTICASTSOCKET_HH_

#include "EDatagramSocket.hh"
#include "EInetSocketAddress.hh"
#include "ENetworkInterface.hh"

namespace efc {

/**
 * The multicast datagram socket class is useful for sending
 * and receiving IP multicast packets.  A MulticastSocket is
 * a (UDP) DatagramSocket, with additional capabilities for
 * joining "groups" of other multicast hosts on the internet.
 * <P>
 * A multicast group is specified by a class D IP address
 * and by a standard UDP port number. Class D IP addresses
 * are in the range <CODE>224.0.0.0</CODE> to <CODE>239.255.255.255</CODE>,
 * inclusive. The address 224.0.0.0 is reserved and should not be used.
 * <P>
 * One would join a multicast group by first creating a MulticastSocket
 * with the desired port, then invoking the
 * <CODE>joinGroup(InetAddress groupAddr)</CODE>
 * method:
 * <PRE>
 * // join a Multicast group and send the group salutations
 * ...
 * String msg = "Hello";
 * InetAddress group = InetAddress.getByName("228.5.6.7");
 * MulticastSocket s = new MulticastSocket(6789);
 * s.joinGroup(group);
 * DatagramPacket hi = new DatagramPacket(msg.getBytes(), msg.length(),
 *                             group, 6789);
 * s.send(hi);
 * // get their responses!
 * byte[] buf = new byte[1000];
 * DatagramPacket recv = new DatagramPacket(buf, buf.length);
 * s.receive(recv);
 * ...
 * // OK, I'm done talking - leave the group...
 * s.leaveGroup(group);
 * </PRE>
 *
 * When one sends a message to a multicast group, <B>all</B> subscribing
 * recipients to that host and port receive the message (within the
 * time-to-live range of the packet, see below).  The socket needn't
 * be a member of the multicast group to send messages to it.
 * <P>
 * When a socket subscribes to a multicast group/port, it receives
 * datagrams sent by other hosts to the group/port, as do all other
 * members of the group and port.  A socket relinquishes membership
 * in a group by the leaveGroup(InetAddress addr) method.  <B>
 * Multiple MulticastSocket's</B> may subscribe to a multicast group
 * and port concurrently, and they will all receive group datagrams.
 * <P>
 * Currently applets are not allowed to use multicast sockets.
 *
 * @author Pavani Diwanji
 * @since  JDK1.1
 */

class EMulticastSocket: public EDatagramSocket {
public:
	virtual ~EMulticastSocket();

	/**
	 * Create a multicast socket.
	 *
	 * <p>If there is a security manager,
	 * its {@code checkListen} method is first called
	 * with 0 as its argument to ensure the operation is allowed.
	 * This could result in a SecurityException.
	 * <p>
	 * When the socket is created the
	 * {@link DatagramSocket#setReuseAddress(boolean)} method is
	 * called to enable the SO_REUSEADDR socket option.
	 *
	 * @exception IOException if an I/O exception occurs
	 * while creating the MulticastSocket
	 * @exception  SecurityException  if a security manager exists and its
	 *             {@code checkListen} method doesn't allow the operation.
	 * @see SecurityManager#checkListen
	 * @see java.net.DatagramSocket#setReuseAddress(boolean)
	 */
	EMulticastSocket() THROWS(EIOException);

	/**
	 * Create a multicast socket and bind it to a specific port.
	 *
	 * <p>If there is a security manager,
	 * its {@code checkListen} method is first called
	 * with the {@code port} argument
	 * as its argument to ensure the operation is allowed.
	 * This could result in a SecurityException.
	 * <p>
	 * When the socket is created the
	 * {@link DatagramSocket#setReuseAddress(boolean)} method is
	 * called to enable the SO_REUSEADDR socket option.
	 *
	 * @param port port to use
	 * @exception IOException if an I/O exception occurs
	 * while creating the MulticastSocket
	 * @exception  SecurityException  if a security manager exists and its
	 *             {@code checkListen} method doesn't allow the operation.
	 * @see SecurityManager#checkListen
	 * @see java.net.DatagramSocket#setReuseAddress(boolean)
	 */
	EMulticastSocket(int port) THROWS(EIOException);

	/**
	 * Create a MulticastSocket bound to the specified socket address.
	 * <p>
	 * Or, if the address is {@code null}, create an unbound socket.
	 *
	 * <p>If there is a security manager,
	 * its {@code checkListen} method is first called
	 * with the SocketAddress port as its argument to ensure the operation is allowed.
	 * This could result in a SecurityException.
	 * <p>
	 * When the socket is created the
	 * {@link DatagramSocket#setReuseAddress(boolean)} method is
	 * called to enable the SO_REUSEADDR socket option.
	 *
	 * @param bindaddr Socket address to bind to, or {@code null} for
	 *                 an unbound socket.
	 * @exception IOException if an I/O exception occurs
	 * while creating the MulticastSocket
	 * @exception  SecurityException  if a security manager exists and its
	 *             {@code checkListen} method doesn't allow the operation.
	 * @see SecurityManager#checkListen
	 * @see java.net.DatagramSocket#setReuseAddress(boolean)
	 *
	 * @since 1.4
	 */
	EMulticastSocket(EInetSocketAddress* bindaddr) THROWS(EIOException);
	EMulticastSocket(const char* hostname, int port) THROWS(EIOException);

	/**
	 * Set the default time-to-live for multicast packets sent out
	 * on this {@code MulticastSocket} in order to control the
	 * scope of the multicasts.
	 *
	 * <P> The ttl <B>must</B> be in the range {@code  0 <= ttl <=
	 * 255} or an {@code IllegalArgumentException} will be thrown.
	 * Multicast packets sent with a TTL of {@code 0} are not transmitted
	 * on the network but may be delivered locally.
	 *
	 * @param  ttl
	 *         the time-to-live
	 *
	 * @throws  IOException
	 *          if an I/O exception occurs while setting the
	 *          default time-to-live value
	 *
	 * @see #getTimeToLive()
	 */
	virtual void setTimeToLive(int ttl) THROWS(EIOException);

	/**
	 * Get the default time-to-live for multicast packets sent out on
	 * the socket.
	 * @exception IOException if an I/O exception occurs while
	 * getting the default time-to-live value
	 * @return the default time-to-live value
	 * @see #setTimeToLive(int)
	 */
	virtual int getTimeToLive() THROWS(EIOException);

	/**
	 * Joins a multicast group. Its behavior may be affected by
	 * {@code setInterface} or {@code setNetworkInterface}.
	 *
	 * <p>If there is a security manager, this method first
	 * calls its {@code checkMulticast} method
	 * with the {@code mcastaddr} argument
	 * as its argument.
	 *
	 * @param mcastaddr is the multicast address to join
	 *
	 * @exception IOException if there is an error joining
	 * or when the address is not a multicast address.
	 * @exception  SecurityException  if a security manager exists and its
	 * {@code checkMulticast} method doesn't allow the join.
	 *
	 * @see SecurityManager#checkMulticast(InetAddress)
	 */
	virtual void joinGroup(EInetAddress* mcastaddr) THROWS(EIOException);

	/**
	 * Leave a multicast group. Its behavior may be affected by
	 * {@code setInterface} or {@code setNetworkInterface}.
	 *
	 * <p>If there is a security manager, this method first
	 * calls its {@code checkMulticast} method
	 * with the {@code mcastaddr} argument
	 * as its argument.
	 *
	 * @param mcastaddr is the multicast address to leave
	 * @exception IOException if there is an error leaving
	 * or when the address is not a multicast address.
	 * @exception  SecurityException  if a security manager exists and its
	 * {@code checkMulticast} method doesn't allow the operation.
	 *
	 * @see SecurityManager#checkMulticast(InetAddress)
	 */
	virtual void leaveGroup(EInetAddress* mcastaddr) THROWS(EIOException);

	/**
	 * Joins the specified multicast group at the specified interface.
	 *
	 * <p>If there is a security manager, this method first
	 * calls its {@code checkMulticast} method
	 * with the {@code mcastaddr} argument
	 * as its argument.
	 *
	 * @param mcastaddr is the multicast address to join
	 * @param netIf specifies the local interface to receive multicast
	 *        datagram packets, or <i>null</i> to defer to the interface set by
	 *       {@link MulticastSocket#setInterface(InetAddress)} or
	 *       {@link MulticastSocket#setNetworkInterface(NetworkInterface)}
	 *
	 * @exception IOException if there is an error joining
	 * or when the address is not a multicast address.
	 * @exception  SecurityException  if a security manager exists and its
	 * {@code checkMulticast} method doesn't allow the join.
	 * @throws  IllegalArgumentException if mcastaddr is null or is a
	 *          SocketAddress subclass not supported by this socket
	 *
	 * @see SecurityManager#checkMulticast(InetAddress)
	 * @since 1.4
	 */
	virtual void joinGroup(EInetSocketAddress* mcastaddr, ENetworkInterface* netIf) THROWS(EIOException);
	virtual void joinGroup(const char* hostname, int port, ENetworkInterface* netIf) THROWS(EIOException);
	virtual void joinGroup(EInetAddress* mcastaddr, ENetworkInterface* netIf) THROWS(EIOException);
	virtual void joinGroup(const char* hostname, ENetworkInterface* netIf) THROWS(EIOException);

	/**
	 * Leave a multicast group on a specified local interface.
	 *
	 * <p>If there is a security manager, this method first
	 * calls its {@code checkMulticast} method
	 * with the {@code mcastaddr} argument
	 * as its argument.
	 *
	 * @param mcastaddr is the multicast address to leave
	 * @param netIf specifies the local interface or <i>null</i> to defer
	 *             to the interface set by
	 *             {@link MulticastSocket#setInterface(InetAddress)} or
	 *             {@link MulticastSocket#setNetworkInterface(NetworkInterface)}
	 * @exception IOException if there is an error leaving
	 * or when the address is not a multicast address.
	 * @exception  SecurityException  if a security manager exists and its
	 * {@code checkMulticast} method doesn't allow the operation.
	 * @throws  IllegalArgumentException if mcastaddr is null or is a
	 *          SocketAddress subclass not supported by this socket
	 *
	 * @see SecurityManager#checkMulticast(InetAddress)
	 * @since 1.4
	 */
	virtual void leaveGroup(EInetSocketAddress* mcastaddr, ENetworkInterface* netIf) THROWS(EIOException);
	virtual void leaveGroup(const char* hostname, int port, ENetworkInterface* netIf) THROWS(EIOException);
	virtual void leaveGroup(EInetAddress* mcastaddr, ENetworkInterface* netIf) THROWS(EIOException);
	virtual void leaveGroup(const char* hostname, ENetworkInterface* netIf) THROWS(EIOException);

	/**
	 * Set the multicast network interface used by methods
	 * whose behavior would be affected by the value of the
	 * network interface. Useful for multihomed hosts.
	 * @param inf the InetAddress
	 * @exception SocketException if there is an error in
	 * the underlying protocol, such as a TCP error.
	 * @see #getInterface()
	 */
	virtual void setInterface(EInetAddress* inf) THROWS(ESocketException);

	/**
	 * Retrieve the address of the network interface used for
	 * multicast packets.
	 *
	 * @return An {@code InetAddress} representing
	 *  the address of the network interface used for
	 *  multicast packets.
	 *
	 * @exception SocketException if there is an error in
	 * the underlying protocol, such as a TCP error.
	 *
	 * @see #setInterface(java.net.InetAddress)
	 */
	virtual sp<EInetAddress> getInterface() THROWS(ESocketException);

	/**
	 * Specify the network interface for outgoing multicast datagrams
	 * sent on this socket.
	 *
	 * @param netIf the interface
	 * @exception SocketException if there is an error in
	 * the underlying protocol, such as a TCP error.
	 * @see #getNetworkInterface()
	 * @since 1.4
	 */
	virtual void setNetworkInterface(ENetworkInterface* netIf) THROWS(ESocketException);

	/**
	 * Get the multicast network interface set.
	 *
	 * @exception SocketException if there is an error in
	 * the underlying protocol, such as a TCP error.
	 * @return the multicast {@code NetworkInterface} currently set
	 * @see #setNetworkInterface(NetworkInterface)
	 * @since 1.4
	 */
	virtual sp<ENetworkInterface> getNetworkInterface() THROWS(ESocketException);

	/**
	 * Disable/Enable local loopback of multicast datagrams
	 * The option is used by the platform's networking code as a hint
	 * for setting whether multicast data will be looped back to
	 * the local socket.
	 *
	 * <p>Because this option is a hint, applications that want to
	 * verify what loopback mode is set to should call
	 * {@link #getLoopbackMode()}
	 * @param disable {@code true} to disable the LoopbackMode
	 * @throws SocketException if an error occurs while setting the value
	 * @since 1.4
	 * @see #getLoopbackMode
	 */
	virtual void setLoopbackMode(boolean disable) THROWS(ESocketException);

	/**
	 * Get the setting for local loopback of multicast datagrams.
	 *
	 * @throws SocketException  if an error occurs while getting the value
	 * @return true if the LoopbackMode has been disabled
	 * @since 1.4
	 * @see #setLoopbackMode
	 */
	virtual boolean getLoopbackMode() THROWS(ESocketException);

private:
	/**
	 * Used on some platforms to record if an outgoing interface
	 * has been set for this socket.
	 */
	boolean interfaceSet;

	/**
	 * The lock on the socket's interface - used by setInterface
	 * and getInterface
	 */
	EReentrantLock infLock;// = new Object();

	/**
	 * The "last" interface set by setInterface on this MulticastSocket
	 */
	sp<EInetAddress> infAddress;// = null;

	void init(EInetSocketAddress* bindaddr) THROWS(EIOException);

	void mcast_join_leave(EInetAddress* inetaddr, ENetworkInterface* netIf, boolean join);
};

} /* namespace efc */
#endif /* EMULTICASTSOCKET_HH_ */
