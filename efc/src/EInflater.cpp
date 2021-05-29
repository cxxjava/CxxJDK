/*
 * EInflater.cpp
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#include "EInflater.hh"
#include "ERuntimeException.hh"

#include "../libc/src/zlib/zlib.h"

namespace efc {

EInflater::~EInflater() {
	this->end();
}

EInflater::EInflater(boolean nowrap) {
	init(nowrap);
}

void EInflater::setInput(byte* b, int len) {
	_buf = b;
	_off = 0;
	_len = len;
}

void EInflater::setDictionary(byte* b, int len) {
	ES_ASSERT(b);
	ES_ASSERT(len >= 0);

	int res = inflateSetDictionary((z_stream*)_strm, (Bytef*)b, len);
	switch (res) {
	case Z_OK:
		break;
	case Z_STREAM_ERROR:
	case Z_DATA_ERROR:
		throw EDataFormatException(__FILE__, __LINE__);
		break;
	default:
		throw ERuntimeException(__FILE__, __LINE__);
		break;
	}

	_needDict = false;
}

int EInflater::getRemaining() {
	return _len;
}

boolean EInflater::needsInput() {
	return _len <= 0;
}

boolean EInflater::needsDictionary() {
	return _needDict;
}

boolean EInflater::finished() {
	return _finished;
}

int EInflater::inflate(byte* b, int len) {
	ensureOpen();

	z_stream* strm = (z_stream*)_strm;
	strm->next_in = (Bytef*)_buf + _off;
	strm->avail_in = _len;
	strm->next_out = (Bytef*)b;
	strm->avail_out = len;

	int ret = ::inflate(strm, Z_PARTIAL_FLUSH);
	switch (ret) {
	case Z_STREAM_END:
		_finished = true;
		/* fall through */
	case Z_OK:
		_off += _len - strm->avail_in;
		_len = strm->avail_in;
		return len - strm->avail_out;
	case Z_NEED_DICT:
		_needDict = true;
		/* Might have consumed some input here! */
		_off += _len - strm->avail_in;
		_len = strm->avail_in;
		return 0;
	case Z_BUF_ERROR:
		return 0;
	case Z_DATA_ERROR:
		throw EDataFormatException(__FILE__, __LINE__);
	case Z_MEM_ERROR:
	default:
		throw ERuntimeException(__FILE__, __LINE__);
	}
}

int EInflater::getAdler() {
	ensureOpen();
	return ((z_stream*)_strm)->adler;
}

int EInflater::getTotalIn() {
	return (int) getBytesRead();
}

long EInflater::getBytesRead() {
	ensureOpen();
	return ((z_stream*)_strm)->total_in;
}

int EInflater::getTotalOut() {
	return (int) getBytesWritten();
}

long EInflater::getBytesWritten() {
	ensureOpen();
	return ((z_stream*)_strm)->total_out;
}

void EInflater::reset() {
	ensureOpen();
	inflateReset((z_stream*)_strm);
	_finished = false;
	_needDict = false;
	_off = _len = 0;
	_buf = NULL;
}

void EInflater::end() {
	z_stream* strm = (z_stream*)_strm;
	if (strm) {
		inflateEnd(strm);
		eso_free(strm);
		_strm = NULL;
	}
}

void EInflater::ensureOpen() {
	if (_strm == 0)
		throw ENULLPOINTEREXCEPTION;
}

void EInflater::init(boolean nowrap) {
	_strm = (z_stream*) eso_calloc(sizeof(z_stream));
	_buf = emptyBuf;
	_off = 0;
	_len = 0;
	_finished = false;
	_needDict = false;

	if (inflateInit2((z_stream*)_strm, nowrap ? -MAX_WBITS : MAX_WBITS) != Z_OK) {
		throw ERuntimeException(__FILE__, __LINE__, "Inflater init fail.");
	}
}

} /* namespace efc */
