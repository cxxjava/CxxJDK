/*
 * EURLEncoder.cpp
 *
 *  Created on: 2013-3-16
 *      Author: cxxjava@163.com
 */

#include "EURLEncoder.hh"

namespace efc {

EString EURLEncoder::encode(const char* s, int len)
{
	EString encodeStr;
	
	if (!s || !*s) {
		return encodeStr;
	}
	
	if (len <= 0) {
		len = eso_strlen(s);
	}
	
	es_string_t* encode_str = NULL;
	eso_murlencode(&encode_str, s, len);
	encodeStr.concat(encode_str);
	eso_mfree(encode_str);
	
	return encodeStr;
}

} /* namespace efc */
