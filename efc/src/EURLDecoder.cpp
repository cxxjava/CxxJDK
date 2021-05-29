/*
 * EURLDecoder.cpp
 *
 *  Created on: 2013-3-16
 *      Author: cxxjava@163.com
 */

#include "EURLDecoder.hh"

namespace efc {

EString EURLDecoder::decode(const char* s)
{
	EString decodeStr;

	if (!s || !*s) {
		return decodeStr;
	}

	es_string_t* decode_str = NULL;
	eso_murldecode(&decode_str, s);
	decodeStr.concat(decode_str);
	eso_mfree(decode_str);
	
	return decodeStr;
}

} /* namespace efc */
