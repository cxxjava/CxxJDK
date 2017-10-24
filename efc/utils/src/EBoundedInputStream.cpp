/*
 * EBoundedInputStream.cpp
 *
 *  Created on: 2017-9-7
 *      Author: cxxjava@163.com
 */

#include "../inc/EBoundedInputStream.hh"

namespace efc {
namespace utils {

EBoundedInputStream::~EBoundedInputStream() {
	try {
		close();
	} catch (...) {
	}
	if (in && owned) {
		delete in;
	}
}

EBoundedInputStream::EBoundedInputStream(EInputStream* in, long size,
		boolean propagateClose, boolean owned) :
		pos(0),
		mark_(EOF),
		propagateClose(propagateClose) {
	if (!in) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	if (size < EOF) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Max size < -1");
	}

	// Some badly designed methods - eg the servlet API - overload length
	// such that "-1" means stream finished
	this->in = in;
	this->max = size;
	this->owned = owned;
}

boolean EBoundedInputStream::isPropagateClose() {
	return propagateClose;
}

void EBoundedInputStream::setPropagateClose(boolean propagateClose) {
	this->propagateClose = propagateClose;
}

int EBoundedInputStream::read() {
	if (max >= 0 && pos >= max) {
		return EOF;
	}
	int result = in->read();
	pos++;
	return result;
}

int EBoundedInputStream::read(void* b, int len) {
	if (max>=0 && pos>=max) {
		return EOF;
	}
	long maxRead = max>=0 ? ES_MIN(len, max-pos) : len;
	int bytesRead = in->read(b, (int)maxRead);

	if (bytesRead == EOF) {
		return EOF;
	}

	pos += bytesRead;
	return bytesRead;
}

long EBoundedInputStream::skip(long n) {
	long toSkip = max>=0 ? ES_MIN(n, max-pos) : n;
	long skippedBytes = in->skip(toSkip);
	pos += skippedBytes;
	return skippedBytes;
}

long EBoundedInputStream::available() {
	if (max>=0 && pos>=max) {
		return 0;
	}
	return in->available();
}

void EBoundedInputStream::close() {
	if (propagateClose) {
		in->close();
	}
}

void EBoundedInputStream::reset() {
	SYNCHRONIZED(this) {
		in->reset();
		pos = mark_;
	}}
}

void EBoundedInputStream::mark(int readlimit) {
	SYNCHRONIZED(this) {
		in->mark(readlimit);
		mark_ = pos;
	}}
}

boolean EBoundedInputStream::markSupported() {
	return in->markSupported();
}

EStringBase EBoundedInputStream::toString() {
	return in->toString();
}

} /* namespace utils */
} /* namespace efc */
