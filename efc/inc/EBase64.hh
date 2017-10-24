/*
 * EBase64.hh
 *
 *  Created on: 2013-7-17
 *      Author: cxxjava@163.com
 */

#ifndef EBASE64_HH_
#define EBASE64_HH_

#include "EA.hh"
#include "EByteBuffer.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

class EBase64 {
public:
	/**
	 * Translates the specified byte array into a Base64 string as per
	 * Preferences.put(byte[]).
	 */
	static EString byteArrayToBase64(byte* a, int len);
	static EString encodeBase64(byte* a, int len);

	/**
	 * Translates the specified Base64 string (as per Preferences.get(byte[]))
	 * into a byte array.
	 *
	 * @throw IllegalArgumentException if <tt>s</tt> is not a valid Base64
	 *        string.
	 */
	static void base64ToByteArray(EByteBuffer *out, const char* s) THROWS(EIllegalArgumentException);
	static EA<byte> decodeBase64(const char* s) THROWS(EIllegalArgumentException);
};

} /* namespace efc */
#endif /* EBASE64_HH_ */
