/*
 * EAdler32.cpp
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#include "EAdler32.hh"

#include "../libc/src/zlib/zlib.h"

namespace efc {

EAdler32::~EAdler32() {
}

EAdler32::EAdler32() {
	adler = 1;
}

void EAdler32::update(byte b) {
	adler = adler32(adler, (Bytef*)&b, 1);
}

void EAdler32::update(byte* b, int len) {
	adler = adler32(adler, (Bytef*)b, len);
}

llong EAdler32::getValue() {
	return (llong)adler & 0xffffffffL;
}

void EAdler32::reset() {
	adler = 1;
}

} /* namespace efc */
