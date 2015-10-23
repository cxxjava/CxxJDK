/*
 * EIPAddressUtil.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EIPAddressUtil_HH_
#define EIPAddressUtil_HH_

#include "ENumberFormatException.hh"

namespace efc {

class EIPAddressUtil {
public:
    /*
     * Converts IPv4 address in its textual presentation form
     * into its numeric binary form.
     *
     * @param src a String representing an IPv4 address in standard format
     * @param addr4 a byte array representing the IPv4 numeric address
     * @return 0 = success, -1 = failure
     */
    static int textToNumericFormatV4(const char* src, byte addr4[4]);

    /*
     * Convert IPv6 presentation level address to network order binary form.
     * credit:
     *  Converted from C code from Solaris 8 (inet_pton)
     *
     * Any component of the string following a per-cent % is ignored.
     *
     * @param src a String representing an IPv6 address in textual format
     * @param addr6 a byte array representing the IPv6 numeric address
     * @return 0 = success, -1 = failure
     */
    static int textToNumericFormatV6(const char* src, byte addr6[12]);

    /**
     * @param src a String representing an IPv4 address in textual format
     * @return a boolean indicating whether src is an IPv4 literal address
     */
    static boolean isIPv4LiteralAddress(const char* src);

    /**
     * @param src a String representing an IPv6 address in textual format
     * @return a boolean indicating whether src is an IPv6 literal address
     */
    static boolean isIPv6LiteralAddress(const char* src);

    /*
     * Convert IPv4-Mapped address to IPv4 address. Both input and
     * returned value are in network order binary form.
     *
     * @param addr6 a String representing an IPv4-Mapped address in textual format
     * @param addr4 a byte array representing the IPv4 numeric address
     * @return 0 = success, -1 = failure
     */
    static int convertFromIPv4MappedAddress(byte addr6[12], byte addr4[4]);

private:
    /**
     * Utility routine to check if the InetAddress is an
     * IPv4 mapped IPv6 address.
     *
     * @return a <code>boolean</code> indicating if the InetAddress is
     * an IPv4 mapped IPv6 address; or false if address is IPv4 address.
     */
     static boolean isIPv4MappedAddress(byte addr6[12]);
};

} /* namespace efc */
#endif //!EIPAddressUtil_HH_
