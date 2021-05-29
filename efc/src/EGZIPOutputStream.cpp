/*
 * EGZIPOutputStream.cpp
 *
 *  Created on: 2013-7-29
 *      Author: cxxjava@163.com
 */

#include "EGZIPOutputStream.hh"

namespace efc {

/**
 * GZIP header magic number.
 */
#define GZIP_MAGIC  0x8b1f

/*
 * Trailer size in bytes.
 *
 */
#define TRAILER_SIZE 8

static byte header[] = {
        (byte) GZIP_MAGIC,                // Magic number (short)
        (byte)(GZIP_MAGIC >> 8),          // Magic number (short)
        8,                // Compression method (CM) -->EDeflater::DEFLATED
        0,                                // Flags (FLG)
        0,                                // Modification time MTIME (int)
        0,                                // Modification time MTIME (int)
        0,                                // Modification time MTIME (int)
        0,                                // Modification time MTIME (int)
        0,                                // Extra flags (XFLG)
        0                                 // Operating system (OS)
    };

EGZIPOutputStream::~EGZIPOutputStream() {
	delete _def;
}

EGZIPOutputStream::EGZIPOutputStream(EOutputStream* out, int size, boolean syncFlush) :
		EDeflaterOutputStream(out, size, syncFlush, false) {
	_def = new EDeflater(EDeflater::DEFAULT_COMPRESSION, true);
	EDeflaterOutputStream::setDeflater(_def);
	_usesDefaultDeflater = true;
	writeHeader();
	_crc.reset();
}

void EGZIPOutputStream::write(const void* b, int len) {
	EDeflaterOutputStream::write(b, len);
	_crc.update((es_int8_t*)b, len);
}

void EGZIPOutputStream::write(const char *s)
{
	EOutputStream::write(s);
}

void EGZIPOutputStream::write(int b)
{
	EOutputStream::write(b);
}

void EGZIPOutputStream::finish() {
	if (!_def->finished()) {
		_def->finish();
		while (!_def->finished()) {
			int len = _def->deflate((byte*)_buf->address(), _buf->length(), EDeflater::NO_FLUSH);
			if (_def->finished() && len <= _buf->length() - TRAILER_SIZE) {
				// last deflater buffer. Fit trailer at the end
				writeTrailer((byte*)_buf->address() + len);
				len = len + TRAILER_SIZE;
				_out->write(_buf->address(), len);
				return;
			}
			if (len > 0)
				_out->write(_buf->address(), len);
		}
		// if we can't fit the trailer at the end of the last
		// deflater buffer, we write it separately
		byte trailer[TRAILER_SIZE] = {0};
		writeTrailer(trailer);
		_out->write(trailer, sizeof(trailer));
	}
}

void EGZIPOutputStream::writeHeader() {
	_out->write(header, sizeof(header));
}

void EGZIPOutputStream::writeTrailer(byte buf[8]) {
	int crc = (int)_crc.getValue();
	int totalIn = _def->getTotalIn();

	eso_llong2array(crc, (es_byte_t*)buf, 4); // CRC-32 of uncompr. data
	eso_llong2array(totalIn, (es_byte_t*)buf+4, 4); // Number of uncompr. bytes
}

} /* namespace efc */
