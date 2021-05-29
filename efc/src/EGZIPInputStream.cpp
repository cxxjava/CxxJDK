/*
 * EGZIPInputStream.cpp
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#include "EGZIPInputStream.hh"
#include "ECheckedInputStream.hh"
#include "EStream.hh"

namespace efc {

/**
 * GZIP header magic number.
 */
#define GZIP_MAGIC  0x8b1f

#define SKIP_BUFFER_SIZE 512

/*
     * File header flags.
     */
#define FTEXT	  1 	// Extra text
#define FHCRC	  2 	// Header CRC
#define FEXTRA	  4 	// Extra field
#define FNAME	  8 	// File name
#define FCOMMENT 16 	// File comment

EGZIPInputStream::~EGZIPInputStream() {
	delete _inf;
}

EGZIPInputStream::EGZIPInputStream(EInputStream* in, int size) :
	EInflaterInputStream(in, size, false) {
	_eos = false;
	_closed = false;
	_inf = new EInflater(true);
	EInflaterInputStream::setInflater(_inf);
	_usesDefaultInflater = true;
	readHeader();
	_crc.reset();
}

int EGZIPInputStream::read(void* b, int len) {
	ensureOpen();
	if (_eos) {
		return -1;
	}
	len = EInflaterInputStream::read(b, len);
	if (len == -1) {
		readTrailer();
		_eos = true;
	} else {
		_crc.update((byte*)b, len);
	}
	return len;
}

void EGZIPInputStream::close() {
	if (!_closed) {
		EInflaterInputStream::close();
		_eos = true;
		_closed = true;
	}
}

void EGZIPInputStream::ensureOpen() {
	if (_closed) {
		throw EIOException(__FILE__, __LINE__, "Stream closed");
	}
}

void EGZIPInputStream::readHeader() {
	ECheckedInputStream in(_in, &_crc);
	_crc.reset();
	// Check header magic
	if (readUShort(in) != GZIP_MAGIC) {
		throw EIOException(__FILE__, __LINE__, "Not in GZIP format");
	}
	// Check compression method
	if (readUByte(in) != 8) {
		throw EIOException(__FILE__, __LINE__, "Unsupported compression method");
	}
	// Read flags
	int flg = readUByte(in);
	// Skip MTIME, XFL, and OS fields
	skipBytes(in, 6);
	// Skip optional extra field
	if ((flg & FEXTRA) == FEXTRA) {
		skipBytes(in, readUShort(in));
	}
	// Skip optional file name
	if ((flg & FNAME) == FNAME) {
		while (readUByte(in) != 0)
			;
	}
	// Skip optional file comment
	if ((flg & FCOMMENT) == FCOMMENT) {
		while (readUByte(in) != 0)
			;
	}
	// Check optional header CRC
	if ((flg & FHCRC) == FHCRC) {
		int v = (int) _crc.getValue() & 0xffff;
		if (readUShort(in) != v) {
			throw EIOException(__FILE__, __LINE__, "Corrupt GZIP header");
		}
	}
}

void EGZIPInputStream::readTrailer() {
	es_byte_t trailer[8];
	int n = _inf->getRemaining();
    int off = _len - n;
	if (n >= 8) {
		eso_memcpy(trailer, (char*)(_buf->data) + off, 8);
	} else {
		eso_memcpy(trailer, (char*)(_buf->data) + off, n);
		n = _in->read(trailer + n, 8 - n);
		if (n == -1) {
			throw EEOFException(__FILE__, __LINE__);
		}
	}

	// Uses left-to-right evaluation order
    unsigned crc = (unsigned)eso_array2llong(trailer, 4);
    unsigned len = (unsigned)eso_array2llong(trailer+4, 4);
	if (crc != _crc.getValue() ||
		// rfc1952; ISIZE is the input size modulo 2^32
        len != (_inf->getBytesWritten() & 0xffffffffL)) {
		throw EIOException(__FILE__, __LINE__, "Corrupt GZIP trailer");
	}
}

uint EGZIPInputStream::readUInt(EInputStream& in) {
	long s = readUShort(in);
	return ((long)readUShort(in) << 16) | s;
}

ushort EGZIPInputStream::readUShort(EInputStream& in) {
	int b = readUByte(in);
	return ((int)readUByte(in) << 8) | b;
}

ubyte EGZIPInputStream::readUByte(EInputStream& in) {
	int b = in.read();
	if (b == -1) {
		throw EEOFException(__FILE__, __LINE__);
	}
	if (b < -1 || b > 255) {
		// Report on this.in, not argument in; see read{Header, Trailer}.
		throw EIOException(__FILE__, __LINE__,
				EString::formatOf(
						".read() returned value out of range -1..255: %d", b).c_str());
	}
	return b;
}

void EGZIPInputStream::skipBytes(EInputStream& in, int n) {
	byte *buf = (byte*)eso_malloc(SKIP_BUFFER_SIZE);
	while (n > 0) {
		int len = in.read(buf, n < SKIP_BUFFER_SIZE ? n : SKIP_BUFFER_SIZE);
		if (len == -1) {
			eso_free(buf);
			throw EEOFException(__FILE__, __LINE__);
		}
		n -= len;
	}
	eso_free(buf);
}

} /* namespace efc */
