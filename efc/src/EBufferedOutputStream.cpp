/*
 * EBufferedOutputStream.cpp
 *
 *  Created on: 2015-6-9
 *      Author: cxxjava@163.com
 */

#include "EBufferedOutputStream.hh"
#include "ESystem.hh"
#include "EIllegalArgumentException.hh"
#include "EIndexOutOfBoundsException.hh"

namespace efc {

EBufferedOutputStream::~EBufferedOutputStream() {
	/* 20180207: removed for destroy error when owned object freed before!
	try {
		close();
	} catch (...) {
	}
	*/
	delete buf;
}

EBufferedOutputStream::EBufferedOutputStream(EOutputStream* out, int size, boolean owned) :
				EFilterOutputStream(out, owned),
				buf(null), count(0)
{
	if (size <= 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Buffer size <= 0");
	}
	buf = new EA<byte>(size);
}

void EBufferedOutputStream::write(int b) {
	SYNCHRONIZED(this) {
		if (count >= buf->length()) {
			flushBuffer();
		}
		(*buf)[count++] = (byte)b;
    }}
}

void EBufferedOutputStream::write(const char* s) {
	EFilterOutputStream::write(s);
}

void EBufferedOutputStream::write(const void* b, int len) {
	SYNCHRONIZED(this) {
		if (len >= buf->length()) {
			/* If the request length exceeds the size of the output buffer,
			   flush the output buffer and then write the data directly.
			   In this way buffered streams will cascade harmlessly. */
			flushBuffer();
			_out->write(b, len);
			return;
		}
		if (len > buf->length() - count) {
			flushBuffer();
		}
		ESystem::arraycopy((void*)b, 0, (void*)buf->address(), count, len);
		count += len;
    }}
}

void EBufferedOutputStream::flush() {
	SYNCHRONIZED(this) {
		flushBuffer();
		_out->flush();
    }}
}

void EBufferedOutputStream::flushBuffer() {
	if (count > 0) {
		_out->write(buf->address(), count);
		count = 0;
	}
}

} /* namespace efc */
