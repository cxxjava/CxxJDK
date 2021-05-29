/*
 * EDeflater.cpp
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#include "EDeflater.hh"
#include "ENullPointerException.hh"

#include "../libc/src/zlib/zlib.h"

namespace efc {

#define DEF_MEM_LEVEL 8

EDeflater::~EDeflater() {
	this->end();
}

EDeflater::EDeflater(int level, boolean nowrap) {
	init(level, EDeflater::DEFAULT_STRATEGY, nowrap);
}

void EDeflater::setInput(byte* b, int len) {
	_buf = b;
	_off = 0;
	_len = len;
}

void EDeflater::setDictionary(byte* b, int len) {
	ES_ASSERT(b);
	ES_ASSERT(len >= 0);

	int res = deflateSetDictionary((z_stream*)_strm, (Bytef*) b, len);
	switch (res) {
	case Z_OK:
		break;
	case Z_STREAM_ERROR:
		throw EDataFormatException(__FILE__, __LINE__);
		break;
	default:
		throw ERuntimeException(__FILE__, __LINE__);
		break;
	}
}

void EDeflater::setStrategy(int strategy) {
	switch (strategy) {
		  case EDeflater::DEFAULT_STRATEGY:
		  case EDeflater::FILTERED:
		  case EDeflater::HUFFMAN_ONLY:
		    break;
		  default:
		    throw EIllegalArgumentException(__FILE__, __LINE__);
		}
		if (_strategy != strategy) {
		    _strategy = strategy;
		    _setParams = true;
		}
}

void EDeflater::setLevel(int level) {
	if ((level < 0 || level > 9) && level != EDeflater::DEFAULT_COMPRESSION) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "invalid compression level");
	}
	if (_level != level) {
		_level = level;
		_setParams = true;
	}
}

boolean EDeflater::needsInput() {
	return _len <= 0;
}

void EDeflater::finish() {
	_finish = true;
}

boolean EDeflater::finished() {
	return _finished;
}

int EDeflater::deflate(byte* b, int len, int flush) {
	if (b == null) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	if (len < 0 ) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	ensureOpen();
	if (flush == NO_FLUSH || flush == SYNC_FLUSH ||
					flush == FULL_FLUSH) {
		int res;
		z_stream* strm = (z_stream*)_strm;
		if (_setParams) {
			strm->next_in = (Bytef*) _buf + _off;
			strm->avail_in = _len;
			strm->next_out = (Bytef*) b;
			strm->avail_out = len;

			res = deflateParams(strm, _level, _strategy);
			switch (res) {
			case Z_OK:
				_setParams = false;
				_off += _len - strm->avail_in;
				_len = strm->avail_in;
				return len - strm->avail_out;
			case Z_BUF_ERROR:
				_setParams = false;
				return 0;
			default:
				throw ERuntimeException(__FILE__, __LINE__);
			}
		} else {
			strm->next_in = (Bytef*) _buf + _off;
			strm->avail_in = _len;
			strm->next_out = (Bytef*) b;
			strm->avail_out = len;

			res = ::deflate(strm, _finish ? Z_FINISH : flush);
			switch (res) {
			case Z_STREAM_END:
				_finished = true;
				/* fall through */
			case Z_OK:
				_off += _len - strm->avail_in;
				_len = strm->avail_in;
				return len - strm->avail_out;
			case Z_BUF_ERROR:
				return 0;
			default:
				throw ERuntimeException(__FILE__, __LINE__);
			}
		}
	}
	throw EIllegalArgumentException(__FILE__, __LINE__);
}

int EDeflater::getAdler() {
	ensureOpen();
	return ((z_stream*)_strm)->adler;
}

int EDeflater::getTotalIn() {
	return (int) getBytesRead();
}

long EDeflater::getBytesRead() {
	ensureOpen();
	return ((z_stream*)_strm)->total_in;
}

int EDeflater::getTotalOut() {
	return (int) getBytesWritten();
}

long EDeflater::getBytesWritten() {
	ensureOpen();
	return ((z_stream*)_strm)->total_out;
}

void EDeflater::reset() {
	ensureOpen();
	deflateReset((z_stream*)_strm);
	_finish = false;
	_finished = false;
	_off = _len = 0;
}

void EDeflater::end() {
	z_stream* strm = (z_stream*)_strm;
	if (strm) {
		deflateEnd(strm);
		eso_free(strm);
		_strm = NULL;
	}
}

void EDeflater::ensureOpen() {
	if (_strm == 0)
		throw ENULLPOINTEREXCEPTION;
}

void EDeflater::init(int level, int strategy, boolean nowrap) {
	_strm = (z_stream*) eso_calloc(sizeof(z_stream));
	_buf = null;
	_off = 0;
	_len = 0;
	_level = level;
	_strategy = strategy;
	_setParams = false;
	_finish = false;
	_finished = false;

	if (::deflateInit2((z_stream*)_strm, _level, Z_DEFLATED,
		     nowrap ? -MAX_WBITS : MAX_WBITS,
		    		 DEF_MEM_LEVEL, strategy) != Z_OK) {
		throw ERuntimeException(__FILE__, __LINE__, "Inflater init fail.");
	}
}

} /* namespace efc */
