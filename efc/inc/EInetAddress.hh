/*
 * EInetAddress.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EInetAddress_HH_
#define EInetAddress_HH_

#include "EString.hh"
#include "EArray.hh"
#include "EIOException.hh"
#include "EUnknownHostException.hh"

namespace efc {

/**
 * This class represents an Internet Protocol version 4 (IPv4) address.
 * Defined by <a href="http://www.ietf.org/rfc/rfc790.txt">
 * <i>RFC&nbsp;790: Assigned Numbers</i></a>,
 * <a href="http://www.ietf.org/rfc/rfc1918.txt">
 * <i>RFC&nbsp;1918: Address Allocation for Private Internets</i></a>,
 * and <a href="http://www.ietf.org/rfc/rfc2365.txt"><i>RFC&nbsp;2365:
 * Administratively Scoped IP Multicast</i></a>
 *
 * <h4> <A NAME="format">Textual representation of IP addresses</a> </h4>
 *
 * Textual representation of IPv4 address used as input to methods
 * takes one of the following forms:
 *
 * <blockquote><table cellpadding=0 cellspacing=0 summary="layout">
 * <tr><td><tt>d.d.d.d</tt></td></tr>
 * <tr><td><tt>d.d.d</tt></td></tr>
 * <tr><td><tt>d.d</tt></td></tr>
 * <tr><td><tt>d</tt></td></tr>
 * </table></blockquote>
 *
 * <p> When four parts are specified, each is interpreted as a byte of
 * data and assigned, from left to right, to the four bytes of an IPv4
 * address.

 * <p> When a three part address is specified, the last part is
 * interpreted as a 16-bit quantity and placed in the right most two
 * bytes of the network address. This makes the three part address
 * format convenient for specifying Class B net- work addresses as
 * 128.net.host.
 *
 * <p> When a two part address is supplied, the last part is
 * interpreted as a 24-bit quantity and placed in the right most three
 * bytes of the network address. This makes the two part address
 * format convenient for specifying Class A network addresses as
 * net.host.
 *
 * <p> When only one part is given, the value is stored directly in
 * the network address without any byte rearrangement.
 *
 * <p> For methods that return a textual representation as output
 * value, the first form, i.e. a dotted-quad string, is used.
 *
 * <h4> The Scope of a Multicast Address </h4>
 * 
 * Historically the IPv4 TTL field in the IP header has doubled as a
 * multicast scope field: a TTL of 0 means node-local, 1 means
 * link-local, up through 32 means site-local, up through 64 means
 * region-local, up through 128 means continent-local, and up through
 * 255 are global. However, the administrative scoping is preferred.
 * Please refer to <a href="http://www.ietf.org/rfc/rfc2365.txt">
 * <i>RFC&nbsp;2365: Administratively Scoped IP Multicast</i></a>
 * @since 1.4
 */

class ENetworkInterface;

class EInetAddress : virtual public EObject {
public:
	virtual ~EInetAddress();

    EInetAddress();
    EInetAddress(const char* hostName, byte addr[4]);
    EInetAddress(const char* hostName, int address);
    
    /**
     * Utility routine to check if the InetAddress is an
     * IP multicast address. IP multicast address is a Class D
     * address i.e first four bits of the address are 1110.
     * @return a <code>boolean</code> indicating if the InetAddress is 
     * an IP multicast address
     * @since   JDK1.1
     */
    boolean isMulticastAddress();

    /**
     * Utility routine to check if the InetAddress in a wildcard address.
     * @return a <code>boolean</code> indicating if the Inetaddress is
     *         a wildcard address.
     * @since 1.4
     */    
    boolean isAnyLocalAddress();

    /**
     * Utility routine to check if the InetAddress is a loopback address. 
     *
     * @return a <code>boolean</code> indicating if the InetAddress is 
     * a loopback address; or false otherwise.
     * @since 1.4
     */
    boolean isLoopbackAddress();

    /**
     * Utility routine to check if the InetAddress is an link local address. 
     *
     * @return a <code>boolean</code> indicating if the InetAddress is 
     * a link local address; or false if address is not a link local unicast address.
     * @since 1.4
     */
    boolean isLinkLocalAddress();

    /**
     * Utility routine to check if the InetAddress is a site local address. 
     *
     * @return a <code>boolean</code> indicating if the InetAddress is 
     * a site local address; or false if address is not a site local unicast address.
     * @since 1.4
     */
    boolean isSiteLocalAddress();

    /**
     * Utility routine to check if the multicast address has global scope.
     *
     * @return a <code>boolean</code> indicating if the address has 
     *         is a multicast address of global scope, false if it is not 
     *         of global scope or it is not a multicast address
     * @since 1.4
     */
    boolean isMCGlobal();

    /**
     * Utility routine to check if the multicast address has node scope.
     *
     * @return a <code>boolean</code> indicating if the address has 
     *         is a multicast address of node-local scope, false if it is not 
     *         of node-local scope or it is not a multicast address
     * @since 1.4
     */
    boolean isMCNodeLocal();

    /**
     * Utility routine to check if the multicast address has link scope.
     *
     * @return a <code>boolean</code> indicating if the address has 
     *         is a multicast address of link-local scope, false if it is not 
     *         of link-local scope or it is not a multicast address
     * @since 1.4
     */
    boolean isMCLinkLocal();

    /**
     * Utility routine to check if the multicast address has site scope.
     *
     * @return a <code>boolean</code> indicating if the address has 
     *         is a multicast address of site-local scope, false if it is not 
     *         of site-local scope or it is not a multicast address
     * @since 1.4
     */
    boolean isMCSiteLocal();

    /**
     * Utility routine to check if the multicast address has organization scope.
     *
     * @return a <code>boolean</code> indicating if the address has 
     *         is a multicast address of organization-local scope, 
     *         false if it is not of organization-local scope 
     *         or it is not a multicast address
     * @since 1.4
     */
    boolean isMCOrgLocal();

	/**
     * Gets the host name for this IP address.
     *
     * <p>If this InetAddress was created with a host name,
     * this host name will be remembered and returned; 
     * otherwise, a reverse name lookup will be performed
     * and the result will be returned based on the system 
     * configured name lookup service. If a lookup of the name service
     * is required, call 
     * {@link #getCanonicalHostName() getCanonicalHostName}.
     *
     * <p>If there is a security manager, its
     * <code>checkConnect</code> method is first called
     * with the hostname and <code>-1</code> 
     * as its arguments to see if the operation is allowed.
     * If the operation is not allowed, it will return
     * the textual representation of the IP address.
     *
     * @return  the host name for this IP address, or if the operation
     *    is not allowed by the security check, the textual 
     *    representation of the IP address.
     * 
     * @see InetAddress#getCanonicalHostName
     * @see SecurityManager#checkConnect
     */
    const char* getHostName() THROWS(EUnknownHostException);
    
    /**
     * Gets the fully qualified domain name for this IP address.
     * Best effort method, meaning we may not be able to return 
     * the FQDN depending on the underlying system configuration.
     *
     * <p>If there is a security manager, this method first
     * calls its <code>checkConnect</code> method
     * with the hostname and <code>-1</code> 
     * as its arguments to see if the calling code is allowed to know
     * the hostname for this IP address, i.e., to connect to the host.
     * If the operation is not allowed, it will return
     * the textual representation of the IP address.
     * 
     * @return  the fully qualified domain name for this IP address, 
     *    or if the operation is not allowed by the security check,
     *    the textual representation of the IP address.
     *
     * @see SecurityManager#checkConnect
     *
     * @since 1.4
     */
    const char* getCanonicalHostName() THROWS(EUnknownHostException);
    
    /**
     * Returns the raw IP address of this <code>InetAddress</code>
     * object. The result is in network byte order: the highest order
     * byte of the address is in <code>getAddress()[0]</code>.
     *
     * @return  the raw IP address of this object.
     */
    int getAddress();

    /**
     * Returns the IP address string in textual presentation form.
     *
     * @return  the raw IP address in a string format.
     * @since   JDK1.0.2
     */
    EString getHostAddress();
	
	/**
     * Test whether that address is reachable. Best effort is made by the
     * implementation to try to reach the host, but firewalls and server
     * configuration may block requests resulting in a unreachable status
     * while some specific ports may be accessible.
     * A typical implementation will use ICMP ECHO REQUESTs if the
     * privilege can be obtained, otherwise it will try to establish
     * a TCP connection on port 7 (Echo) of the destination host.
     * <p>
     * The timeout value, in milliseconds, indicates the maximum amount of time
     * the try should take. If the operation times out before getting an
     * answer, the host is deemed unreachable. A negative value will result
     * in an IllegalArgumentException being thrown.
     *
     * @param	timeout	the time, in milliseconds, before the call aborts
     * @return a <code>boolean</code> indicating if the address is reachable.
     * @throws IOException if a network error occurs
     * @throws  IllegalArgumentException if <code>timeout</code> is negative.
     * @since 1.5
     */
    boolean isReachable(int timeout) THROWS(EIOException);

    /**
	 * Test whether that address is reachable. Best effort is made by the
	 * implementation to try to reach the host, but firewalls and server
	 * configuration may block requests resulting in a unreachable status
	 * while some specific ports may be accessible.
	 * A typical implementation will use ICMP ECHO REQUESTs if the
	 * privilege can be obtained, otherwise it will try to establish
	 * a TCP connection on port 7 (Echo) of the destination host.
	 * <p>
	 * The {@code network interface} and {@code ttl} parameters
	 * let the caller specify which network interface the test will go through
	 * and the maximum number of hops the packets should go through.
	 * A negative value for the {@code ttl} will result in an
	 * IllegalArgumentException being thrown.
	 * <p>
	 * The timeout value, in milliseconds, indicates the maximum amount of time
	 * the try should take. If the operation times out before getting an
	 * answer, the host is deemed unreachable. A negative value will result
	 * in an IllegalArgumentException being thrown.
	 *
	 * @param   netif   the NetworkInterface through which the
	 *                    test will be done, or null for any interface
	 * @param   ttl     the maximum numbers of hops to try or 0 for the
	 *                  default
	 * @param   timeout the time, in milliseconds, before the call aborts
	 * @throws  IllegalArgumentException if either {@code timeout}
	 *                          or {@code ttl} are negative.
	 * @return a {@code boolean}indicating if the address is reachable.
	 * @throws IOException if a network error occurs
	 * @since 1.5
	 */
	boolean isReachable(ENetworkInterface* netif, int ttl,
							   int timeout) THROWS(EIOException);

    /**
     * Returns a hashcode for this IP address.
     *
     * @return  a hash code value for this IP address.
     */
    virtual int hashCode();
    
    /**
     * Compares this object against the specified object.
     * The result is <code>true</code> if and only if the argument is
     * not <code>null</code> and it represents the same IP address as
     * this object.
     * <p>
     * Two instances of <code>InetAddress</code> represent the same IP
     * address if the length of the byte arrays returned by
     * <code>getAddress</code> is the same for both, and each of the
     * array components is the same for the byte arrays.
     *
     * @param   obj   the object to compare against.
     * @return  <code>true</code> if the objects are the same;
     *          <code>false</code> otherwise.
     * @see     java.net.InetAddress#getAddress()
     */
    boolean equals(EInetAddress *obj);
    virtual boolean equals(EObject* obj);
    
    /**
     * Converts this IP address to a <code>String</code>. The 
     * string returned is of the form: hostname / literal IP 
     * address.
     *
     * If the host name is unresolved, no reverse name service loopup
     * is performed. The hostname part will be represented by an empty string.
     *
     * @return  a string representation of this IP address.
     */
    virtual EStringBase toString();
    
	/*
     * Returns the InetAddress representing anyLocalAddress
     * (typically 0.0.0.0 or ::0)
     */
    static EInetAddress anyLocalAddress();
    
    /*
	 * Returns the InetAddress representing loopbackAddress
	 * (typically 127.0.0.1)
	 */
    static EInetAddress loopbackAddress();

    /**
     * Returns an <code>InetAddress</code> object given the raw IP address . 
     * The argument is in network byte order: the highest order
     * byte of the address is in <code>getAddress()[0]</code>.
     *
     * <p> This method doesn't block, i.e. no reverse name service lookup
     * is performed.
     *
     * <p> IPv4 address byte array must be 4 bytes long and IPv6 byte array 
     * must be 16 bytes long
     *
     * @param addr the raw IP address in network byte order
     * @return  an InetAddress object created from the raw IP address.
     * @exception  UnknownHostException  if IP address is of illegal length
     * @since 1.4
     */
    static EInetAddress getByAddress(byte addr[4]) THROWS(EUnknownHostException);
    
    /**
     * Create an InetAddress based on the provided host name and IP address
     * No name service is checked for the validity of the address. 
     *
     * <p> The host name can either be a machine name, such as
     * "<code>java.sun.com</code>", or a textual representation of its IP
     * address.
     * <p> No validity checking is done on the host name either.
     *
     * <p> If addr specifies an IPv4 address an instance of Inet4Address 
     * will be returned; otherwise, an instance of Inet6Address 
     * will be returned.
     *
     * <p> IPv4 address byte array must be 4 bytes long and IPv6 byte array 
     * must be 16 bytes long
     *
     * @param host the specified host
     * @param addr the raw IP address in network byte order
     * @return  an InetAddress object created from the raw IP address.
     * @exception  UnknownHostException  if IP address is of illegal length
     * @since 1.4
     */
    static EInetAddress getByAddress(const char* host, byte addr[4]) THROWS(EUnknownHostException);
    
    /**
     * Determines the IP address of a host, given the host's name.
     *
     * <p> The host name can either be a machine name, such as
     * "<code>java.sun.com</code>", or a textual representation of its
     * IP address. If a literal IP address is supplied, only the
     * validity of the address format is checked.
     *
     * <p> For <code>host</code> specified in literal IPv6 address,
     * either the form defined in RFC 2732 or the literal IPv6 address
     * format defined in RFC 2373 is accepted. IPv6 scoped addresses are also
     * supported. See <a href="Inet6Address.html#scoped">here</a> for a description of IPv6
     * scoped addresses.
     *
     * <p> If the host is <tt>null</tt> then an <tt>InetAddress</tt>
     * representing an address of the loopback interface is returned.
     * See <a href="http://www.ietf.org/rfc/rfc3330.txt">RFC&nbsp;3330</a>
     * section&nbsp;2 and <a href="http://www.ietf.org/rfc/rfc2373.txt">RFC&nbsp;2373</a>
     * section&nbsp;2.5.3. </p>
     *
     * @param      host   the specified host, or <code>null</code>.
     * @return     an IP address for the given host name.
     * @exception  UnknownHostException  if no IP address for the
     *               <code>host</code> could be found, or if a scope_id was specified
     *		     for a global IPv6 address.
     * @exception  SecurityException if a security manager exists
     *             and its checkConnect method doesn't allow the operation
     */
    static EInetAddress getByName(const char* host) THROWS(EUnknownHostException);
    
    /**
     * Given the name of a host, returns an array of its IP addresses,
     * based on the configured name service on the system.
     * 
     * <p> The host name can either be a machine name, such as
     * "<code>java.sun.com</code>", or a textual representation of its IP
     * address. If a literal IP address is supplied, only the
     * validity of the address format is checked.
     *
     * <p> For <code>host</code> specified in <i>literal IPv6 address</i>,
     * either the form defined in RFC 2732 or the literal IPv6 address
     * format defined in RFC 2373 is accepted. A literal IPv6 address may
     * also be qualified by appending a scoped zone identifier or scope_id.
     * The syntax and usage of scope_ids is described 
     * <a href="Inet6Address.html#scoped">here</a>.
     * <p> If the host is <tt>null</tt> then an <tt>InetAddress</tt>
     * representing an address of the loopback interface is returned.
     * See <a href="http://www.ietf.org/rfc/rfc3330.txt">RFC&nbsp;3330</a>
     * section&nbsp;2 and <a href="http://www.ietf.org/rfc/rfc2373.txt">RFC&nbsp;2373</a>
     * section&nbsp;2.5.3. </p>
     *
     * <p> If there is a security manager and <code>host</code> is not 
     * null and <code>host.length() </code> is not equal to zero, the
     * security manager's
     * <code>checkConnect</code> method is called
     * with the hostname and <code>-1</code> 
     * as its arguments to see if the operation is allowed.
     *
     * @param      host   the name of the host, or <code>null</code>.
     * @return     an array of all the IP addresses for a given host name.
     * 
     * @exception  UnknownHostException  if no IP address for the
     *               <code>host</code> could be found, or if a scope_id was specified
     *		     for a global IPv6 address.
     * @exception  SecurityException  if a security manager exists and its  
     *               <code>checkConnect</code> method doesn't allow the operation.
     * 
     * @see SecurityManager#checkConnect
     */
    static EArray<EInetAddress*> getAllByName(const char* host) THROWS(EUnknownHostException);
    
    /**
     * Returns the local host.
     *
     * <p>If there is a security manager, its
     * <code>checkConnect</code> method is called
     * with the local host name and <code>-1</code> 
     * as its arguments to see if the operation is allowed. 
     * If the operation is not allowed, an InetAddress representing
     * the loopback address is returned.
     *
     * @return     the IP address of the local host.
     * 
     * @exception  UnknownHostException  if no IP address for the
     *               <code>host</code> could be found.
     * 
     * @see SecurityManager#checkConnect
     */
    static EInetAddress getLocalHost() THROWS(EUnknownHostException);

private:
	/**
     * @serial
     */
    sp<EString> _hostName;

    /* Used to store the best available hostname */
    sp<EString> _canonicalHostName;

    /**
     * Holds a 32-bit IPv4 address.
     *
     * the same to network byte order.
     *
     * @serial
     */
    es_byte_t _address[4];

	void setAddress(byte addr[4]);
	void setAddress(int address);
};

} /* namespace efc */
#endif //!EInetAddress_HH_
