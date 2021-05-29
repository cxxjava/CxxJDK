/*
 * EBufferedInputStream.cpp
 *
 *  Created on: 2015-6-9
 *      Author: cxxjava@163.com
 */

#include "EBufferedInputStream.hh"
#include "EInteger.hh"
#include "ESystem.hh"
#include "EIllegalArgumentException.hh"
#include "EIndexOutOfBoundsException.hh"
#include "../inc/concurrent/EUnsafe.hh"
#include <limits.h>

namespace efc {

EBufferedInputStream::~EBufferedInputStream() {
	/* 20180207: removed for destroy error when owned object freed before!
	try {
		close();
	} catch (...) {
	}
	*/
    delete buf;
}

EBufferedInputStream::EBufferedInputStream(EInputStream* in, int size, boolean owned) :
		EFilterInputStream(in, owned),
		buf(null), count(0), pos(0), markpos(-1), marklimit(0) {
	if (size <= 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Buffer size <= 0");
	}
	buf = new EA<byte>(size);
}

int EBufferedInputStream::read() {
	SYNCHRONIZED(this) {
		if (pos >= count) {
			fill();
			if (pos >= count)
				return -1;
		}
		return (*getBufIfOpen())[pos++] & 0xff;
    }}
}

int EBufferedInputStream::read(void* b, int len) {
	SYNCHRONIZED(this) {
		getBufIfOpen(); // Check for closed stream
		//if ((off | len | (off + len) | (b.length - (off + len))) < 0) {
		if (len < 0) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		} else if (len == 0) {
			return 0;
		}

		int n = 0;
		for (;;) {
			int nread = read1(b, n, len - n);
			if (nread <= 0)
				return (n == 0) ? nread : n;
			n += nread;
			if (n >= len)
				return n;
			// if not closed but no bytes available, return
			EInputStream* input = _in;
			if (input != null && input->available() <= 0)
				return n;
		}
    }}
}

long EBufferedInputStream::skip(long n) {
	SYNCHRONIZED(this) {
		getBufIfOpen(); // Check for closed stream
		if (n <= 0) {
			return 0;
		}
		long avail = count - pos;

		if (avail <= 0) {
			// If no mark position set then don't keep in buffer
			if (markpos <0)
				return getInIfOpen()->skip(n);

			// Fill in buffer to save bytes for reset
			fill();
			avail = count - pos;
			if (avail <= 0)
				return 0;
		}

		long skipped = (avail < n) ? avail : n;
		pos += skipped;
		return skipped;
    }}
}

long EBufferedInputStream::available() {
	SYNCHRONIZED(this) {
		long n = count - pos;
		long avail = getInIfOpen()->available();
		return n > (LONG_MAX - avail)
					? LONG_MAX
					: n + avail;
    }}
}

void EBufferedInputStream::mark(int readlimit) {
	SYNCHRONIZED(this) {
		marklimit = readlimit;
		markpos = pos;
    }}
}

void EBufferedInputStream::reset() {
	SYNCHRONIZED(this) {
		getBufIfOpen(); // Cause exception if closed
		if (markpos < 0)
			throw EIOException(__FILE__, __LINE__, "Resetting to invalid mark");
		pos = markpos;
    }}
}

boolean EBufferedInputStream::markSupported() {
	return true;
}

void EBufferedInputStream::close() {
	EA<byte>* buffer;
	while ( (buffer = buf) != null) {
		//if (bufUpdater.compareAndSet(this, buffer, null)) {
		if (EUnsafe::compareAndSwapObject(&buf, buffer, null)) {
			try {
				EInputStream* input = _in;
				_in = null;
				if (input != null)
					input->close();
				if (_owned) {
					delete input;
				}
			} catch (...) {
				delete buffer; //!
				throw; //!
			} finally {
				delete buffer; //!
			}
			return;
		}
		// Else retry in case a new buf was CASed in fill()
	}
}

EInputStream* EBufferedInputStream::getInIfOpen() {
	EInputStream* input = _in;
	if (input == null)
		throw EIOException(__FILE__, __LINE__, "Stream closed");
	return input;
}

EA<byte>* EBufferedInputStream::getBufIfOpen() {
	EA<byte>* buffer = buf;
	if (buffer == null)
		throw EIOException(__FILE__, __LINE__, "Stream closed");
	return buffer;
}

void EBufferedInputStream::fill() {
	EA<byte>* buffer = getBufIfOpen();
	if (markpos < 0)
		pos = 0;            /* no mark: throw away the buffer */
	else if (pos >= buffer->length())  /* no room left in buffer */
		if (markpos > 0) {  /* can throw away early part of the buffer */
			int sz = pos - markpos;
			ESystem::arraycopy(buffer, markpos, buffer, 0, sz);
			pos = sz;
			markpos = 0;
		} else if (buffer->length() >= marklimit) {
			markpos = -1;   /* buffer got too big, invalidate mark */
			pos = 0;        /* drop buffer contents */
		} else {            /* grow buffer */
			int nsz = pos * 2;
			if (nsz > marklimit)
				nsz = marklimit;
			EA<byte>* nbuf = new EA<byte>(nsz);
			ESystem::arraycopy(buffer, 0, nbuf, 0, pos);
			//if (!bufUpdater.compareAndSet(this, buffer, nbuf)) {
			if (!EUnsafe::compareAndSwapObject(&buf, buffer, nbuf)) {
				// Can't replace buf if there was an async close.
				// Note: This would need to be changed if fill()
				// is ever made accessible to multiple threads.
				// But for now, the only way CAS can fail is via close.
				// assert buf == null;
				delete nbuf;
				throw EIOException(__FILE__, __LINE__, "Stream closed");
			}
			delete buffer;
			buffer = nbuf;
		}
	count = pos;
	int n = getInIfOpen()->read(buffer->address() + pos, buffer->length() - pos);
	if (n > 0)
		count = n + pos;
}

int EBufferedInputStream::read1(void* b, int off, int len) {
	int avail = count - pos;
	if (avail <= 0) {
		/* If the requested length is at least as large as the buffer, and
		   if there is no mark/reset activity, do not bother to copy the
		   bytes into the local buffer.  In this way buffered streams will
		   cascade harmlessly. */
		if (len >= getBufIfOpen()->length() && markpos < 0) {
			return getInIfOpen()->read((byte*)b + off, len);
		}
		fill();
		avail = count - pos;
		if (avail <= 0) return -1;
	}
	int cnt = (avail < len) ? avail : len;
	ESystem::arraycopy(getBufIfOpen()->address(), pos, b, off, cnt);
	pos += cnt;
	return cnt;
}

} /* namespace efc */
