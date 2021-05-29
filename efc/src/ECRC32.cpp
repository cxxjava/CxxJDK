/*
 * ECRC32.cpp
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#include "ECRC32.hh"

namespace efc {

ECRC32::~ECRC32() {
}

ECRC32::ECRC32() {
	eso_crc32_init(_crc);
}

void ECRC32::update(byte b) {
	eso_crc32_update(&_crc, (es_byte_t*)&b, 1);
}

void ECRC32::update(byte* b, int len) {
	eso_crc32_update(&_crc, (es_byte_t*)b, len);
}

llong ECRC32::getValue() {
	return eso_crc32_final(_crc);
}

void ECRC32::reset() {
	eso_crc32_init(_crc);
}

} /* namespace efc */
