/*
 * EIPAddressUtil.cpp
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EIPAddressUtil.hh"
#include "EString.hh"
#include "EPattern.hh"
#include "EInteger.hh"
#include "ELLong.hh"

namespace efc {

int EIPAddressUtil::textToNumericFormatV4(const char* src, byte addr4[4]) {
	if (!src || !*src) {
		return -1;
	}

	EArray<EString*> s = EPattern::split("\\.", src, 0);
	llong val;
	int i;
	try {
		switch (s.length()) {
		case 1:
			/*
			 * When only one part is given, the value is stored directly in
			 * the network address without any byte rearrangement.
			 */

			val = ELLong::parseLLong(s[0]->c_str());
			if (val < 0 || val > 0xffffffffL)
				return -1;
			val = htonl(val);
			addr4[0] = (byte) ((val >> 24) & 0xff);
			addr4[1] = (byte) (((val & 0xffffff) >> 16) & 0xff);
			addr4[2] = (byte) (((val & 0xffff) >> 8) & 0xff);
			addr4[3] = (byte) (val & 0xff);
			break;
		case 2:
			/*
			 * When a two part address is supplied, the last part is
			 * interpreted as a 24-bit quantity and placed in the right
			 * most three bytes of the network address. This makes the
			 * two part address format convenient for specifying Class A
			 * network addresses as net.host.
			 */

			val = EInteger::parseInt(s[0]->c_str());
			if (val < 0 || val > 0xff)
				return -1;
			addr4[0] = (byte) (val & 0xff);
			val = EInteger::parseInt(s[1]->c_str());
			if (val < 0 || val > 0xffffff)
				return -1;
			val = htonl(val);
			addr4[1] = (byte) ((val >> 16) & 0xff);
			addr4[2] = (byte) (((val & 0xffff) >> 8) & 0xff);
			addr4[3] = (byte) (val & 0xff);
			break;
		case 3:
			/*
			 * When a three part address is specified, the last part is
			 * interpreted as a 16-bit quantity and placed in the right
			 * most two bytes of the network address. This makes the
			 * three part address format convenient for specifying
			 * Class B net- work addresses as 128.net.host.
			 */
			for (i = 0; i < 2; i++) {
				val = EInteger::parseInt(s[i]->c_str());
				if (val < 0 || val > 0xff)
					return -1;
				addr4[i] = (byte) (val & 0xff);
			}
			val = EInteger::parseInt(s[2]->c_str());
			if (val < 0 || val > 0xffff)
				return -1;
			val = htonl(val);
			addr4[2] = (byte) ((val >> 8) & 0xff);
			addr4[3] = (byte) (val & 0xff);
			break;
		case 4:
			/*
			 * When four parts are specified, each is interpreted as a
			 * byte of data and assigned, from left to right, to the
			 * four bytes of an IPv4 address.
			 */
			for (i = 0; i < 4; i++) {
				val = EInteger::parseInt(s[i]->c_str());
				if (val < 0 || val > 0xff)
					return -1;
				addr4[i] = (byte) (val & 0xff);
			}
			break;
		default:
			return -1;
		}
	} catch (ENumberFormatException &e) {
		return -1;
	}
	return 0;
}

int EIPAddressUtil::textToNumericFormatV6(const char* src, byte addr6[12]) {
    //TODO...
	return -1;
}

boolean EIPAddressUtil::isIPv4LiteralAddress(const char* src) {
	byte addr4[4];
	return (textToNumericFormatV4(src, addr4) != -1);
}

boolean EIPAddressUtil::isIPv6LiteralAddress(const char* src) {
	//TODO...
	return false;
}

int EIPAddressUtil::convertFromIPv4MappedAddress(byte addr6[12],
		byte addr4[4]) {
	//TODO...
	return -1;
}

boolean EIPAddressUtil::isIPv4MappedAddress(byte addr6[12]) {
	//TODO...
	return false;
}

} /* namespace efc */
