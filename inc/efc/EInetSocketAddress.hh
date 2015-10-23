/*
 * EInetSocketAddress.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EInetSocketAddress_HH_
#define EInetSocketAddress_HH_

#include "EIOException.hh"
#include "EInetAddress.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

/**
 *
 * This class implements an IP Socket Address (IP address + port number)
 * It can also be a pair (hostname + port number), in which case an attempt
 * will be made to resolve the hostname. If resolution fails then the address
 * is said to be <I>unresolved</I> but can still be used on some circumstances
 * like connecting through a proxy.
 * <p>
 * It provides an immutable object used by sockets for binding, connecting, or
 * as returned values.
 * <p>
 * The <i>wildcard</i> is a special local IP address. It usually means "any"
 * and can only be used for <code>bind</code> operations.
 *
 * @see	java.net.Socket
 * @see	java.net.ServerSocket
 * @since 1.4
 */

class EInetSocketAddress {
public:
	virtual ~EInetSocketAddress();
	
    /**
     * Creates a socket address where the IP address is the wildcard address
     * and the port number a specified value.
     * <p>
     * A valid port value is between 0 and 65535.
     * A port number of <code>zero</code> will let the system pick up an
     * ephemeral port in a <code>bind</code> operation.
     * <p>
     * @param	port	The port number
     * @throws IllegalArgumentException if the port parameter is outside the specified
     * range of valid port values. 
     */
    EInetSocketAddress(int port) THROWS(EIllegalArgumentException);

    /**
     *
     * Creates a socket address from an IP address and a port number.
     * <p>
     * A valid port value is between 0 and 65535.
     * A port number of <code>zero</code> will let the system pick up an
     * ephemeral port in a <code>bind</code> operation.
     * <P>
     * A <code>null</code> address will assign the <i>wildcard</i> address.
     * <p>
     * @param	addr	The IP address
     * @param	port	The port number
     * @throws IllegalArgumentException if the port parameter is outside the specified
     * range of valid port values.
     */
    EInetSocketAddress(EInetAddress *addr, int port) THROWS(EIllegalArgumentException);
    EInetSocketAddress(int address, int port) THROWS(EIllegalArgumentException);

    /**
     *
     * Creates a socket address from a hostname and a port number.
     * <p>
     * An attempt will be made to resolve the hostname into an InetAddress.
     * If that attempt fails, the address will be flagged as <I>unresolved</I>.
     * <p>
     * If there is a security manager, its <code>checkConnect</code> method
     * is called with the host name as its argument to check the permissiom
     * to resolve it. This could result in a SecurityException.
     * <P>
     * A valid port value is between 0 and 65535.
     * A port number of <code>zero</code> will let the system pick up an
     * ephemeral port in a <code>bind</code> operation.
     * <P>
     * @param	hostname the Host name
     * @param	port	The port number
     * @throws IllegalArgumentException if the port parameter is outside the range
     * of valid port values, or if the hostname parameter is <TT>null</TT>.
     * @throws SecurityException if a security manager is present and
     *				 permission to resolve the host name is
     *				 denied.
     * @see	#isUnresolved()
     */
    EInetSocketAddress(const char* hostname, int port) THROWS2(EIllegalArgumentException,EUnknownHostException);

    /**
     *
     * Creates an unresolved socket address from a hostname and a port number.
     * <p>
     * No attempt will be made to resolve the hostname into an InetAddress.
     * The address will be flagged as <I>unresolved</I>.
     * <p>
     * A valid port value is between 0 and 65535.
     * A port number of <code>zero</code> will let the system pick up an
     * ephemeral port in a <code>bind</code> operation.
     * <P>
     * @param	host    the Host name
     * @param	port	The port number
     * @throws IllegalArgumentException if the port parameter is outside
     *                  the range of valid port values, or if the hostname
     *                  parameter is <TT>null</TT>.
     * @see	#isUnresolved()
     * @return  a <code>EInetSocketAddress</code> representing the unresolved
     *          socket address
     * @since 1.5
     */
    static EInetSocketAddress createUnresolved(const char* host, int port);

    /**
     * Gets the port number.
     *
     * @return the port number.
     */
    int getPort();

    /**
     * 
     * Gets the <code>InetAddress</code>.
     *
     * @return the InetAdress or <code>null</code> if it is unresolved.
     */
    EInetAddress* getAddress();

    /**
     * Gets the <code>hostname</code>.
     *
     * @return	the hostname part of the address.
     */
    const char* getHostName();

    /**
     * Returns the hostname, or the String form of the address if it
     * doesn't have a hostname (it was created using a litteral).
     * This has the benefit of <b>not</b> attemptimg a reverse lookup.
     *
     * @return the hostname, or String representation of the address.
     * @since 1.6
     */
    EString getHostString() THROWS(ENullPointerException);

    /**
     * Checks whether the address has been resolved or not.
     *
     * @return <code>true</code> if the hostname couldn't be resolved into
     *		an <code>InetAddress</code>.
     */
    boolean isUnresolved();

    /**
     * Constructs a string representation of this EInetSocketAddress.
     * This String is constructed by calling toString() on the InetAddress
     * and concatenating the port number (with a colon). If the address
     * is unresolved then the part before the colon will only contain the hostname.
     *
     * @return  a string representation of this object.
     */
    EString toString();

    /**
     * Compares this object against the specified object.
     * The result is <code>true</code> if and only if the argument is
     * not <code>null</code> and it represents the same address as
     * this object.
     * <p>
     * Two instances of <code>EInetSocketAddress</code> represent the same
     * address if both the InetAddresses (or hostnames if it is unresolved) and port
     * numbers are equal.
     * If both addresses are unresolved, then the hostname & the port number
     * are compared.
     *
     * @param   obj   the object to compare against.
     * @return  <code>true</code> if the objects are the same;
     *          <code>false</code> otherwise.
     * @see java.net.InetAddress#equals(java.lang.Object)
     */
    boolean equals(EInetSocketAddress* obj);

    /**
     * Returns a hashcode for this socket address.
     *
     * @return  a hash code value for this socket address.
     */
    int hashCode();

private:
	
	/* The hostname of the Socket Address
     * @serial
     */     
    EString hostname;
    /* The IP address of the Socket Address
     * @serial
     */
    EInetAddress* addr;
    /* The port number of the Socket Address
     * @serial
     */   
    int port;

    EInetSocketAddress() {
    }
};

} /* namespace efc */
#endif //!EInetSocketAddress_HH_
