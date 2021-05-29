/*
 * EDataOutputStream.cpp
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#include "EDataOutputStream.hh"
#include "EInteger.hh"
#include "EFloat.hh"
#include "EDouble.hh"

namespace efc {

EDataOutputStream::~EDataOutputStream() {
	//
}

EDataOutputStream::EDataOutputStream(EOutputStream* out, boolean owned) :
		EFilterOutputStream(out, owned), written(0) {
}

void EDataOutputStream::write(int b) {
	_out->write(b);
	incCount(1);
}

void EDataOutputStream::write(const char* s) {
	write(s, eso_strlen(s));
}

void EDataOutputStream::write(const void* b, int len) {
	_out->write(b, len);
	incCount(len);
}

void EDataOutputStream::flush() {
	_out->flush();
}

void EDataOutputStream::writeBoolean(boolean v) {
	_out->write(v ? 1 : 0);
	incCount(1);
}

void EDataOutputStream::writeByte(int v) {
	_out->write(v);
	incCount(1);
}

void EDataOutputStream::writeShort(int v) {
	short s = v;
	if (!ES_BIG_ENDIAN) {
		s = ES_BSWAP_16(s);
	}
	write(&s, 2);
	incCount(2);
}

void EDataOutputStream::writeInt(int v) {
	if (!ES_BIG_ENDIAN) {
		v = ES_BSWAP_32(v);
	}
	write(&v, 4);
	incCount(4);
}

void EDataOutputStream::writeLLong(llong v) {
	if (!ES_BIG_ENDIAN) {
		v = ES_BSWAP_64(v);
	}
	write(&v, 8);
	incCount(8);
}

void EDataOutputStream::writeFloat(float v) {
	writeInt(EFloat::floatToIntBits(v));
}

void EDataOutputStream::writeDouble(double v) {
	writeLLong(EDouble::doubleToLLongBits(v));
}

void EDataOutputStream::writeBytes(const char* s) {
	int len = eso_strlen(s);
	write(s, len);
	incCount(len);
}

int EDataOutputStream::size() {
	return written;
}

void EDataOutputStream::incCount(int value) {
	int temp = written + value;
	if (temp < 0) {
		temp = EInteger::MAX_VALUE;
	}
	written = temp;
}

} /* namespace efc */
