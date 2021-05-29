/*
 * EIOByteBuffer.cpp
 *
 *  Created on: 2013-9-3
 *      Author: cxxjava@163.com
 */

#include "../inc/EIOByteBuffer.hh"
#include "../../inc/EStream.hh"
#include "../../inc/EMath.hh"
#include "../inc/EInvalidMarkException.hh"
#include "../inc/EBufferOverflowException.hh"

namespace efc {
namespace nio {

//@see: sunjdk_1.6_src/java/nio/Buffer.java
//@see: sunjdk_1.6_src/java/nio/ByteBuffer.java

EIOByteBuffer::~EIOByteBuffer() {
	if (_defaultAllocated && _address) {
		eso_free(_address);
	}
}

EIOByteBuffer::EIOByteBuffer():
		_mark(-1),
		_position(0),
		_limit(0),
		_capacity(0),
		_address(null),
		_defaultAllocated(false) {
	//
}

EIOByteBuffer* EIOByteBuffer::allocate(int capacity) {
	return new EIOByteBuffer(capacity);
}

EIOByteBuffer* EIOByteBuffer::wrap(const void* address, int capacity, int offset) {
	return new EIOByteBuffer(address, capacity, offset);
}

EIOByteBuffer::EIOByteBuffer(int capacity) :
		_mark(-1), _position(0), _limit(capacity), _capacity(capacity), _defaultAllocated(true) {
	if (capacity < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	_address = eso_malloc(capacity);
}

EIOByteBuffer::EIOByteBuffer(const void* address, int capacity, int offset) :
		_mark(-1), _position(0), _limit(capacity), _capacity(capacity), _defaultAllocated(false) {
	if (!address || capacity < 0 || offset < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	_address = (char*) address + offset;
}

EIOByteBuffer* EIOByteBuffer::slice() {
	int capacity = this->remaining();
	EIOByteBuffer* buf = new EIOByteBuffer(capacity);
	eso_memcpy(buf->_address, this->current(), capacity);
	return buf;
}

EIOByteBuffer* EIOByteBuffer::duplicate() {
	EIOByteBuffer* buf = new EIOByteBuffer(_capacity);
	eso_memcpy(buf->_address, _address, _capacity);
	buf->_mark = _mark;
	buf->_position = _position;
	buf->_limit = _limit;
	buf->_capacity = _capacity;
	return buf;
}

EIOByteBuffer* EIOByteBuffer::asReadOnlyBuffer() {
	return duplicate();
}

int EIOByteBuffer::capacity() {
	return _capacity;
}

int EIOByteBuffer::position() {
	return _position;
}

EIOByteBuffer* EIOByteBuffer::position(int newPosition) {
	if ((newPosition > _limit) || (newPosition < 0))
		throw EIllegalArgumentException(__FILE__, __LINE__);
	_position = newPosition;
	if (_mark > _position) _mark = -1;
	return this;
}

int EIOByteBuffer::limit() {
	return _limit;
}

EIOByteBuffer* EIOByteBuffer::limit(int newLimit) {
	if ((newLimit > _capacity) || (newLimit < 0))
		throw EIllegalArgumentException(__FILE__, __LINE__);
	_limit = newLimit;
	if (_position > _limit) _position = _limit;
	if (_mark > _limit) _mark = -1;
	return this;
}

EIOByteBuffer* EIOByteBuffer::mark() {
	_mark = _position;
	return this;
}

int EIOByteBuffer::markValue() {
	return _mark;
}

EIOByteBuffer* EIOByteBuffer::reset() {
	int m = _mark;
	if (m < 0)
		throw EInvalidMarkException(__FILE__, __LINE__);
	_position = m;
	return this;
}

EIOByteBuffer* EIOByteBuffer::clear() {
	_position = 0;
	_limit = _capacity;
	_mark = -1;
	return this;
}

EIOByteBuffer* EIOByteBuffer::flip() {
	_limit = _position;
	_position = 0;
	_mark = -1;
	return this;
}

EIOByteBuffer* EIOByteBuffer::rewind() {
	_position = 0;
	_mark = -1;
	return this;
}

EIOByteBuffer* EIOByteBuffer::compact() {
	/* @see:
	int pos = position();
	int lim = limit();
	assert (pos <= lim);
	int rem = (pos <= lim ? lim - pos : 0);

	unsafe.copyMemory(ix(pos), ix(0), rem << $LG_BYTES_PER_VALUE$);
	position(rem);
	limit(capacity());
	discardMark();
	return this;
	*/
	ES_ASSERT (_position <= _limit);
	int rem = (_position < _limit ? _limit - _position : 0);
	if (rem > 0) {
		eso_memmove(_address, (char*)_address + _position, rem);
		_position = rem;
		_limit = _capacity;
		_mark = -1;
	}
	return this;
}

EIOByteBuffer* EIOByteBuffer::skip(int size) {
	position(position() + size);
	return this;
}

int EIOByteBuffer::remaining() {
	return _limit - _position;
}

boolean EIOByteBuffer::hasRemaining() {
	return _position < _limit;
}

boolean EIOByteBuffer::isReadOnly() {
	return false;
}

byte EIOByteBuffer::get() {
	return *((byte*)_address + nextGetIndex());
}

byte EIOByteBuffer::get(int index) {
	if (index >= _limit)
		throw EBUFFERUNDERFLOWEXCEPTION;
	return *((byte*)_address + index);
}

EIOByteBuffer* EIOByteBuffer::get(void* addr, int size, int length) {
	checkBounds(0, length, size);
	char *p = (char*)current();
	nextGetIndex(length);
	eso_memcpy(addr, p, length);
	return this;
}

EIOByteBuffer* EIOByteBuffer::put(byte b) {
	*((byte*)_address + nextGetIndex()) = b;
	return this;
}

EIOByteBuffer* EIOByteBuffer::put(int index, byte b) {
	if (index >= _limit)
		throw EBUFFERUNDERFLOWEXCEPTION;
	*((byte*)_address + index) = b;
	return this;
}

EIOByteBuffer* EIOByteBuffer::put(const void* addr, int length) {
	if (_position + length > _limit)
		throw EBUFFERUNDERFLOWEXCEPTION;
	eso_memcpy((char*)current(), addr, length);
	_position += length;
	return this;
}

EIOByteBuffer* EIOByteBuffer::put(EIOByteBuffer* src) {
	if (!src || src == this)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	int n = src->remaining();
	if (n > remaining())
		throw EBufferOverflowException(__FILE__, __LINE__);
	eso_memcpy((char*)current(), src->current(), n);
	src->position(src->position() + n);
	position(position() + n);
	return this;
}

char EIOByteBuffer::getChar() {
	return this->get();
}

char EIOByteBuffer::getChar(int index) {
	return this->get(index);
}

short EIOByteBuffer::getShort() {
	byte v[2];
	this->get(v, 2, 2);
	return EStream::readShort(v);
}

short EIOByteBuffer::getShort(int index) {
	byte v[2];
	v[0] = this->get(index + 0);
	v[1] = this->get(index + 1);
	return EStream::readShort(v);
}

int EIOByteBuffer::getInt() {
	byte v[4];
	this->get(v, 4, 4);
	return EStream::readInt(v);
}

int EIOByteBuffer::getInt(int index) {
	byte v[4];
	v[0] = this->get(index + 0);
	v[1] = this->get(index + 1);
	v[2] = this->get(index + 2);
	v[3] = this->get(index + 3);
	return EStream::readInt(v);
}

llong EIOByteBuffer::getLLong() {
	byte v[8];
	this->get(v, 8, 8);
	return EStream::readLLong(v);
}

llong EIOByteBuffer::getLLong(int index) {
	byte v[8];
	v[0] = this->get(index + 0);
	v[1] = this->get(index + 1);
	v[2] = this->get(index + 2);
	v[3] = this->get(index + 3);
	v[4] = this->get(index + 4);
	v[5] = this->get(index + 5);
	v[6] = this->get(index + 6);
	v[7] = this->get(index + 7);
	return EStream::readLLong(v);
}

float EIOByteBuffer::getFloat() {
	int i = this->getInt();
	return eso_intBits2float(i);
}

float EIOByteBuffer::getFloat(int index) {
	int i = this->getInt(index);
	return eso_intBits2float(i);
}

double EIOByteBuffer::getDouble() {
	llong l = this->getLLong();
	return eso_llongBits2double(l);
}

double EIOByteBuffer::getDouble(int index) {
	llong l = this->getLLong(index);
	return eso_llongBits2double(l);
}

EIOByteBuffer* EIOByteBuffer::putChar(char value) {
	return this->put((byte)value);
}

EIOByteBuffer* EIOByteBuffer::putChar(int index, char value) {
	return this->put(index, (byte)value);
}

EIOByteBuffer* EIOByteBuffer::putShort(short value) {
	byte v[2];
	EStream::writeShort(v, value);
	return this->put(v, 2);
}

EIOByteBuffer* EIOByteBuffer::putShort(int index, short value) {
	byte v[2];
	EStream::writeShort(v, value);
	this->put(index, v[0]);
	this->put(index + 1, v[1]);
	return this;
}

EIOByteBuffer* EIOByteBuffer::putInt(int value) {
	byte v[4];
	EStream::writeInt(v, value);
	return this->put(v, 4);
}

EIOByteBuffer* EIOByteBuffer::putInt(int index, int value) {
	byte v[4];
	EStream::writeInt(v, value);
	this->put(index, v[0]);
	this->put(index + 1, v[1]);
	this->put(index + 2, v[2]);
	this->put(index + 3, v[3]);
	return this;
}

EIOByteBuffer* EIOByteBuffer::putLLong(llong value) {
	byte v[8];
	EStream::writeLLong(v, value);
	return this->put(v, 8);
}

EIOByteBuffer* EIOByteBuffer::putLLong(int index, llong value) {
	byte v[8];
	EStream::writeLLong(v, value);
	this->put(index + 0, v[0]);
	this->put(index + 1, v[1]);
	this->put(index + 2, v[2]);
	this->put(index + 3, v[3]);
	this->put(index + 4, v[4]);
	this->put(index + 5, v[5]);
	this->put(index + 6, v[6]);
	this->put(index + 7, v[7]);
	return this;
}

EIOByteBuffer* EIOByteBuffer::putFloat(float value) {
	int i = eso_float2intBits(value);
	return this->putInt(i);
}

EIOByteBuffer* EIOByteBuffer::putFloat(int index, float value) {
	int i = eso_float2intBits(value);
	return this->putInt(index, i);
}

EIOByteBuffer* EIOByteBuffer::putDouble(double value) {
	llong l = eso_double2llongBits(value);
	return this->putLLong(l);
}

EIOByteBuffer* EIOByteBuffer::putDouble(int index, double value) {
	llong l = eso_double2llongBits(value);
	return this->putLLong(index, l);
}

void* EIOByteBuffer::address() {
	return (char*)_address;
}

void* EIOByteBuffer::current() {
	return _address ? ((char*)_address + _position) : null;
}

int EIOByteBuffer::hashCode() {
	int h = 1;
	int p = position();
	for (int i = limit() - 1; i >= p; i--)
		h = 31 * h + (int) get(i);
	return h;
}

boolean EIOByteBuffer::equals(EObject* ob) {
	if (this == ob)
		return true;

	EIOByteBuffer* that = dynamic_cast<EIOByteBuffer*>(ob);
	if (!that)
		return false;

	if (this->remaining() != that->remaining())
		return false ;
	int p = this->position();
	for (int i = this->limit() - 1, j = that->limit() - 1; i >= p; i--, j--) {
		byte v1 = this->get(i);
		byte v2 = that->get(j);
		if (v1 != v2) {
			if ((v1 != v1) && (v2 != v2))	// For float and double
				continue;
			return false ;
		}
	}
	return true ;
}

int EIOByteBuffer::compareTo(EIOByteBuffer* that) {
	int n = this->position() + EMath::min(this->remaining(), that->remaining());
	for (int i = this->position(), j = that->position(); i < n; i++, j++) {
		byte v1 = this->get(i);
		byte v2 = that->get(j);
		if (v1 == v2)
			continue;
		if ((v1 != v1) && (v2 != v2)) 	// For float and double
			continue;
		if (v1 < v2)
			return -1;
		return +1;
	}
	return this->remaining() - that->remaining();
}

EString EIOByteBuffer::toString() {
	EString sb;
	sb.append("EIOByteBuffer[pos=");
	sb.append(position());
	sb.append(" lim=");
	sb.append(limit());
	sb.append(" cap=");
	sb.append(capacity());
	sb.append("]");
	return sb;
}

int EIOByteBuffer::nextGetIndex(int nb) {
	if (_limit - _position < nb)
		throw EBUFFERUNDERFLOWEXCEPTION;
	int p = _position;
	_position += nb;
	return p;
}

void EIOByteBuffer::checkBounds(int off, int len, int size) {
	if ((off | len | (off + len) | (size - (off + len))) < 0)
		throw EINDEXOUTOFBOUNDSEXCEPTION;
}

} /* namespace nio */
} /* namespace efc */
