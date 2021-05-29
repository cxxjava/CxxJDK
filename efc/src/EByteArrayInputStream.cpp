/*
 * EByteArrayInputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EByteArrayInputStream.hh"

namespace efc {

#undef SYNCHRONIZED
#define SYNCHRONIZED(obj) { \
	ESentry __synchronizer__(lock ? (obj)->getLock() : null);

EByteArrayInputStream::~EByteArrayInputStream()
{
}

EByteArrayInputStream::EByteArrayInputStream(void *buf, int len, boolean lock)
{
	this->buf = (byte*)buf;
	this->count = len;
	this->pos = 0;
	this->lock = lock;
}

synchronized int EByteArrayInputStream::read(void *b, int len)
{
	SYNCHRONIZED(this) {
		if (b == null) {
	    	throw ENullPointerException(__FILE__, __LINE__);
		}
		if (pos >= count) {
	    	return -1;
		}
		if (pos + len > count) {
	    	len = count - pos;
		}
		if (len <= 0) {
		    return 0;
		}
		eso_memcpy(b, buf + pos, len);
		pos += len;
		return len;
    }}
}

synchronized long EByteArrayInputStream::skip(long n)
{
	SYNCHRONIZED(this) {
		if (pos + n > count) {
	    	n = count - pos;
		}
		if (n < 0) {
	    	return 0;
		}
		pos += n;
		return n;
    }}
}

synchronized long EByteArrayInputStream::available()
{
	SYNCHRONIZED(this) {
		return count - pos;
    }}
}

} /* namespace efc */
