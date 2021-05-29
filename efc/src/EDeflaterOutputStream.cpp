/*
 * EDeflaterOutputStream.cpp
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#include "EDeflaterOutputStream.hh"

namespace efc {

EDeflaterOutputStream::~EDeflaterOutputStream() {
	/* 20180207: removed for destroy error when owned object freed before!
	try {
		close();
	} catch (...) {
	}
	*/
	delete _buf;
	if (_needFreeDeflater && _def) {
		delete _def;
	}
}

EDeflaterOutputStream::EDeflaterOutputStream(EOutputStream* out, int size,
		boolean syncFlush, boolean useDefault, boolean owned) :
		EFilterOutputStream(out, owned), _def(null), _syncFlush(syncFlush) {
	_usesDefaultDeflater = false;
	_needFreeDeflater = false;
	if (useDefault) {
		_def = new EDeflater();
		_usesDefaultDeflater = true;
		_needFreeDeflater = true;
	}
	_buf = new EA<byte>(size);
}

void EDeflaterOutputStream::setDeflater(EDeflater* def) {
	if (_def) {
		throw EIOEXCEPTION;
	}
	_def = def;
}

void EDeflaterOutputStream::write(const void* b, int len) {
	ES_ASSERT(b); ES_ASSERT(len >= 0);

	if (_def->finished()) {
		throw EIOException(__FILE__, __LINE__, "write beyond end of stream");
	}
	if (len == 0) {
		return;
	}
	if (!_def->finished()) {
#if 0 //jdk6
		// Deflate no more than stride bytes at a time.  This avoids
		// excess copying in deflateBytes (see Deflater.c)
		int stride = _buf->length();
		for (int i = 0; i < len; i += stride) {
			_def->setInput((byte*) b + i, ES_MIN(stride, len - i));
			while (!_def->needsInput()) {
				deflate();
			}
		}
#else //jdk7
		_def->setInput((byte*)b, len);
		while (!_def->needsInput()) {
			deflate();
		}
#endif
	}
}

void EDeflaterOutputStream::write(const char *s)
{
	EOutputStream::write(s);
}

void EDeflaterOutputStream::write(int b)
{
	EOutputStream::write(b);
}

void EDeflaterOutputStream::finish() {
	if (!_def->finished()) {
		_def->finish();
		while (!_def->finished()) {
			deflate();
		}
	}
}

void EDeflaterOutputStream::close() {
	if (!closed) {
		finish();
		if (_usesDefaultDeflater)
			_def->end();
		_out->close();
		closed = true;
	}
}

void EDeflaterOutputStream::flush() {
	if (_syncFlush && !_def->finished()) {
		int len = 0;
		while ((len = _def->deflate((byte*)_buf->address(), _buf->length(), EDeflater::SYNC_FLUSH)) > 0)
		{
			_out->write(_buf->address(), len);
			if (len < _buf->length())
				break;
		}
	}
	_out->flush();
}

void EDeflaterOutputStream::deflate() {
	int len = _def->deflate((byte*)_buf->address(), _buf->length(), EDeflater::NO_FLUSH);
	if (len > 0) {
		_out->write(_buf->address(), len);
	}
}

} /* namespace efc */
