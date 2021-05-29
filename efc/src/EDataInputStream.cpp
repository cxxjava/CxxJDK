/*
 * EDataInputStream.cpp
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#include "EDataInputStream.hh"
#include "EEOFException.hh"
#include "EStream.hh"

namespace efc {

EDataInputStream::~EDataInputStream() {
	eso_buffer_free(&_lineBuffer);
}

EDataInputStream::EDataInputStream(EInputStream* in, boolean owned) :
		EFilterInputStream(in, owned) {
	_lineBuffer = NULL;
}

int EDataInputStream::read(void* b, int len) {
	return _in->read(b, len);
}

void EDataInputStream::readFully(byte* b, int len) {
	ES_ASSERT(len >= 0);

	int n = 0;
	while (n < len) {
		int count = _in->read(b + n, len - n);
		if (count < 0)
			throw EEOFEXCEPTION;
		n += count;
	}
}

int EDataInputStream::skipBytes(int n) {
	int total = 0;
	int cur = 0;

	while ((total < n) && ((cur = (int) _in->skip(n - total)) > 0)) {
		total += cur;
	}

	return total;
}

boolean EDataInputStream::readBoolean() {
	int ch = _in->read();
	if (ch < 0)
		throw EEOFEXCEPTION;
	return (ch != 0);
}

byte EDataInputStream::readByte() {
	int ch = _in->read();
	if (ch < 0)
		throw EEOFEXCEPTION;
	return (byte) (ch);
}

ubyte EDataInputStream::readUnsignedByte() {
	int ch = _in->read();
	if (ch < 0)
		throw EEOFEXCEPTION;
	return (ubyte)ch;
}

short EDataInputStream::readShort() {
	es_byte_t v[2];
	int ret = read(v, 2);
	if (ret != 2) {
		throw EEOFEXCEPTION;
	}
	return EStream::readShort(v);
}

ushort EDataInputStream::readUnsignedShort() {
	return (ushort)readShort();
}

int EDataInputStream::readInt() {
	es_byte_t v[4];
	int ret = read(v, 4);
	if (ret != 4) {
		throw EEOFEXCEPTION;
	}
	return EStream::readInt(v);
}

llong EDataInputStream::readLLong() {
	es_byte_t v[8];
	int ret = read(v, 8);
	if (ret != 8) {
		throw EEOFEXCEPTION;
	}
	return EStream::readLLong(v);
}

float EDataInputStream::readFloat() {
	es_byte_t v[4];
	int ret = read(v, 4);
	if (ret != 4) {
		throw EEOFEXCEPTION;
	}
	int i = EStream::readInt(v);
	return eso_intBits2float(i);
}

double EDataInputStream::readDouble() {
	es_byte_t v[8];
	int ret = read(v, 8);
	if (ret != 8) {
		throw EEOFEXCEPTION;
	}
	llong l = EStream::readLLong(v);
	return eso_llongBits2double(l);
}

sp<EString> EDataInputStream::readLine() {
	return readLine(false);
}

sp<EString> EDataInputStream::readLine(boolean ignoreLF) {
	int lineLength = 0;
	int c2;
	char _c;

	if (_lineBuffer == NULL) {
		_lineBuffer = eso_buffer_make(128, 0);
	}
	else {
		eso_buffer_clear(_lineBuffer);
	}

	int c;
	while ((c = _in->read()) >= 0) {
		switch (c) {
		case '\n':
			goto LINE;
		case '\r':
			c2 = _in->read();
			if (c2 == '\n') {
				goto LINE;
			} else {
				if (ignoreLF) {
					_c = c;
					eso_buffer_append(_lineBuffer, &_c, 1);
					lineLength++;

					if (c2 == -1) {
						goto LINE;
					} else {
						_c = c2;
						eso_buffer_append(_lineBuffer, &_c, 1);
						lineLength++;
					}
				} else {
					if (c2 == -1) {
						goto LINE;
					} else {
						_c = c2;
						eso_buffer_append(_lineBuffer, &_c, 1);
						goto LINE;
					}
				}
			}
			break;
		default:
			_c = c;
			eso_buffer_append(_lineBuffer, &_c, 1);
			lineLength++;
			break;
		}
	}
	if ((c == -1) && (lineLength == 0)) {
		return null;
	}
LINE:
	return new EString((char*)_lineBuffer->data, 0, lineLength);
}

} /* namespace efc */
