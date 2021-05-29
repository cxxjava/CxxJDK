/*
 * EPushbackInputStream.cpp
 *
 *  Created on: 2017-11-17
 *      Author: cxxjava@163.com
 */

#include "EPushbackInputStream.hh"
#include "ESystem.hh"
#include "ENullPointerException.hh"

namespace efc {

EPushbackInputStream::~EPushbackInputStream() {
	/* 20180207: removed for destroy error when owned object freed before!
	try {
		close();
	} catch (...) {
	}
	*/
}

EPushbackInputStream::EPushbackInputStream(EInputStream* in, int size, boolean owned):
		EFilterInputStream(in, owned), buf(null), pos(0) {
	if (size <= 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Buffer size <= 0");
	}
	buf = new EA<byte>(size);
	pos = size;
}

int EPushbackInputStream::read() {
	ensureOpen();
	if (pos < buf->length()) {
		return (*buf)[pos++] & 0xff;
	}
	return EInputStream::read();
}

int EPushbackInputStream::read(void *b, int len) {
	ensureOpen();
	if (b == null) {
		throw ENullPointerException(__FILE__, __LINE__);
	} else if (len < 0) {
		throw EIndexOutOfBoundsException(__FILE__, __LINE__);
	} else if (len == 0) {
		return 0;
	}

	int avail = buf->length() - pos;
	int off = 0;
	if (avail > 0) {
		if (len < avail) {
			avail = len;
		}
		ESystem::arraycopy(buf->address(), pos, b, off, avail);
		pos += avail;
		off += avail;
		len -= avail;
	}
	if (len > 0) {
		len = EFilterInputStream::read((char*)b + off, len);
		if (len == -1) {
			return avail == 0 ? -1 : avail;
		}
		return avail + len;
	}
	return avail;
}

void EPushbackInputStream::unread(int b) {
	ensureOpen();
	if (pos == 0) {
		throw EIOException(__FILE__, __LINE__, "Push back buffer is full");
	}
	(*buf)[--pos] = (byte)b;
}

void EPushbackInputStream::unread(void *b, int len) {
	ensureOpen();
	if (len > pos) {
		throw EIOException(__FILE__, __LINE__, "Push back buffer is full");
	}
	pos -= len;
	ESystem::arraycopy(b, 0, buf->address(), pos, len);
}

long EPushbackInputStream::available() {
	ensureOpen();
	int n = buf->length() - pos;
	int avail = EFilterInputStream::available();
	return n > (EInteger::MAX_VALUE - avail)
				? EInteger::MAX_VALUE
				: n + avail;
}

long EPushbackInputStream::skip(long n) {
	ensureOpen();
	if (n <= 0) {
		return 0;
	}

	long pskip = buf->length() - pos;
	if (pskip > 0) {
		if (n < pskip) {
			pskip = n;
		}
		pos += pskip;
		n -= pskip;
	}
	if (n > 0) {
		pskip += EFilterInputStream::skip(n);
	}
	return pskip;
}

boolean EPushbackInputStream::markSupported() {
	return false;
}

synchronized
void EPushbackInputStream::mark(int readlimit) {
	//
}

synchronized
void EPushbackInputStream::reset() {
	throw EIOException(__FILE__, __LINE__, "mark/reset not supported");
}

synchronized
void EPushbackInputStream::close() {
	SYNCHRONIZED(this) {
		if (_in == null)
			return;
		_in->close();
		if (_owned) {
			delete _in; //!
		}
		_in = null;
		delete buf; //!
		buf = null;
	}}
}

void EPushbackInputStream::ensureOpen() {
	if (_in == null)
		throw EIOException(__FILE__, __LINE__, "Stream closed");
}

} /* namespace efc */
