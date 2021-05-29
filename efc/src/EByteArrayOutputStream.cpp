/*
 * EByteArrayOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EByteArrayOutputStream.hh"
#include "EArrays.hh"
#include "EInteger.hh"
#include "EOutOfMemoryError.hh"

namespace efc {

#undef SYNCHRONIZED
#define SYNCHRONIZED(obj) { \
	ESentry __synchronizer__(lock ? (obj)->getLock() : null);

EByteArrayOutputStream::~EByteArrayOutputStream()
{
	if (bufData) eso_free(bufData);
}

EByteArrayOutputStream::EByteArrayOutputStream(int size, boolean lock) :
		bufLen(size), count(0), lock(lock), defaultCapacity(size) {
	bufData = (byte*)eso_malloc(bufLen);
}

synchronized void EByteArrayOutputStream::write(const void *b, int len)
{
	SYNCHRONIZED(this) {
		if (b == null) {
			throw ENullPointerException(__FILE__, __LINE__);
		}
		if (len == 0) {
		    return;
		}
		
		ensureCapacity(count + len);
		ES_ASSERT(bufData);
		eso_memmove(bufData + count, (byte*)b, len);
		count += len;
    }}
}

synchronized void EByteArrayOutputStream::write(const char *s)
{
	EOutputStream::write(s);
}

synchronized void EByteArrayOutputStream::write(int b)
{
	EOutputStream::write(b);
}


synchronized void EByteArrayOutputStream::writeTo(EOutputStream *out) THROWS(EIOException)
{
	if (out) {
		out->write(bufData, count);
	}
}

synchronized void EByteArrayOutputStream::reset()
{
	SYNCHRONIZED(this) {
		count = 0;
    }}
}

synchronized sp<EA<byte> > EByteArrayOutputStream::reset(boolean copy)
{
	SYNCHRONIZED(this) {
		if (copy) {
			EA<byte>* out = new EA<byte>(count);
			eso_memcpy(out->address(), bufData, count);
			count = 0;
			return out;
		} else {
			EA<byte>* old = new EA<byte>(bufData, count, true, MEM_MALLOC);
			bufLen = defaultCapacity;
			bufData = (byte*)eso_malloc(bufLen);
			count = 0;
			return old;
		}
    }}
}

synchronized sp<EA<byte> > EByteArrayOutputStream::toByteArray()
{
	SYNCHRONIZED(this) {
		EA<byte>* out = new EA<byte>(count);
		eso_memcpy(out->address(), bufData, count);
		return out;
    }}
}

synchronized void EByteArrayOutputStream::toByteArray(EByteBuffer *obj)
{
	SYNCHRONIZED(this) {
		if (obj) {
			obj->append(bufData, count);
		}
    }}
}

synchronized int EByteArrayOutputStream::size()
{
	SYNCHRONIZED(this) {
		return count;
    }}
}

synchronized byte* EByteArrayOutputStream::data() {
	SYNCHRONIZED(this) {
		return bufData;
	}}
}

synchronized EString EByteArrayOutputStream::toString()
{
	SYNCHRONIZED(this) {
		return EString((char*)bufData, 0, count);
    }}
}

void EByteArrayOutputStream::ensureCapacity(int minCapacity) {
	// overflow-conscious code
	if (minCapacity - bufLen > 0)
		grow(minCapacity);
}

void EByteArrayOutputStream::grow(int minCapacity) {
	// overflow-conscious code
	int oldCapacity = bufLen;
	int newCapacity = oldCapacity << 1;
	if (newCapacity - minCapacity < 0)
		newCapacity = minCapacity;
	if (newCapacity < 0) {
		if (minCapacity < 0) // overflow
			throw EOutOfMemoryError(__FILE__, __LINE__);
		newCapacity = EInteger::MAX_VALUE;
	}
	bufLen = newCapacity;
	bufData = (byte*)eso_realloc(bufData, bufLen);
}

} /* namespace efc */
