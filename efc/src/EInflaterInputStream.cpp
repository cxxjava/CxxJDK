/*
 * EInflaterInputStream.cpp
 *
 *  Created on: 2013-7-19
 *      Author: cxxjava@163.com
 */

#include "EInflaterInputStream.hh"
#include "EInteger.hh"
#include "EEOFException.hh"
#include <limits.h>

namespace efc {

#define SKIP_BUFFER_SIZE 512

EInflaterInputStream::~EInflaterInputStream() {
	/* 20180207: removed for destroy error when owned object freed before!
	try {
		close();
	} catch (...) {
	}
	*/
	eso_buffer_free(&_buf);
	if (_needFreeInflater && _inf) {
		delete _inf;
	}
}

EInflaterInputStream::EInflaterInputStream(EInputStream* in, int size, boolean useDefault, boolean owned) :
		EFilterInputStream(in, owned), _inf(null), _closed(false), _reachEOF(false) {
	_usesDefaultInflater = false;
	_needFreeInflater = false;
	if (useDefault) {
		_inf = new EInflater();
		_usesDefaultInflater = true;
		_needFreeInflater = true;
	}
	_buf = eso_buffer_make(size, 0);
	_len = 0;
}

void EInflaterInputStream::setInflater(EInflater *inf) THROWS(EIOException) {
	if (_inf) {
		throw EIOEXCEPTION;
	}
	_inf = inf;
}

int EInflaterInputStream::read(void* b, int len) {
	ES_ASSERT(b);
	ES_ASSERT(len >= 0);

	ensureOpen();
	if (len == 0) {
		return 0;
	}

	try {
		int n;
		while ((n = _inf->inflate((byte*) b, len)) == 0) {
			if (_inf->finished() || _inf->needsDictionary()) {
				_reachEOF = true;
				return -1;
			}
			if (_inf->needsInput()) {
				fill();
			}
		}
		return n;
	} catch (EDataFormatException& e) {
		throw EIOException(__FILE__, __LINE__, "Invalid ZLIB data format");
	}

	// not reach here.
	return 0;
}

long EInflaterInputStream::skip(long n) {
	ensureOpen();
	long max = (int) ES_MIN(n, LONG_MAX);
	long total = 0;
	byte *b = (byte*)eso_malloc(SKIP_BUFFER_SIZE);
	while (total < max) {
		long len = max - total;
		if (len > SKIP_BUFFER_SIZE) {
			len = SKIP_BUFFER_SIZE;
		}
		len = read(b, len);
		if (len == -1) {
			_reachEOF = true;
			break;
		}
		total += len;
	}
	eso_free(b);
	return total;
}

long EInflaterInputStream::available() {
	ensureOpen();
	if (_reachEOF) {
		return 0;
	} else {
		return 1;
	}
}

void EInflaterInputStream::close() {
	if (!_closed) {
		if (_usesDefaultInflater) {
			_inf->end();
		}
		_in->close();
		_closed = true;
	}
}

void EInflaterInputStream::fill() {
	ensureOpen();
	_len = _in->read(_buf->data, _buf->capacity);
	if (_len == -1) {
		throw EEOFException(__FILE__, __LINE__, "Unexpected end of ZLIB input stream");
	}
	_inf->setInput((byte*)_buf->data, _len);
}

void EInflaterInputStream::ensureOpen() {
	if (_closed) {
		throw EIOException(__FILE__, __LINE__, "Stream closed");
	}
}

} /* namespace efc */
