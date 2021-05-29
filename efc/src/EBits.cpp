/*
 * EBits.cpp
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#include "EBits.hh"
#include "EStream.hh"

namespace efc {

long EBits::pageSize_ = -1;

void EBits::checkRange(EByteBuffer& b, int off, int size) THROWS(EIndexOutOfBoundsException)
{
	if (off + size > b.size()) {
		throw EIndexOutOfBoundsException(__FILE__, __LINE__);
	}
}

boolean EBits::getBoolean(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(boolean));

	return ((es_byte_t*)b.data())[off] != 0;
}

char EBits::getChar(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(char));

	return (char) (((es_byte_t*)b.data())[off]);
}

short EBits::getShort(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(short));

	return EStream::readShort((es_byte_t*)b.data()+off);
}

int EBits::getInt(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(int));

	return EStream::readInt((es_byte_t*)b.data()+off);
}

llong EBits::getLLong(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(llong));

	return EStream::readLLong((es_byte_t*) b.data() + off);
}

float EBits::getFloat(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(float));

	int i = EStream::readInt((es_byte_t*)b.data() + off);
	return eso_intBits2float(i);
}

double EBits::getDouble(EByteBuffer& b, int off) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(double));

	llong l = EStream::readLLong((es_byte_t*)b.data() + off);
	return eso_llongBits2double(l);
}

/*
 * Methods for packing primitive values into byte arrays starting at given
 * offsets.
 */

void EBits::putBoolean(EByteBuffer& b, int off, boolean val) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(boolean));

	((es_byte_t*)b.data())[off] = (byte) (val ? 1 : 0);
}

void EBits::putChar(EByteBuffer& b, int off, char val) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(char));

	((es_byte_t*)b.data())[off] = val;
}

void EBits::putShort(EByteBuffer& b, int off, short val) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(short));

	EStream::writeShort((es_byte_t*)b.data()+off, val);
}

void EBits::putInt(EByteBuffer& b, int off, int val) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(int));

	EStream::writeInt((es_byte_t*)b.data()+off, val);
}

void EBits::putLLong(EByteBuffer& b, int off, long val) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(long));

	EStream::writeLLong((es_byte_t*)b.data()+off, val);
}

void EBits::putFloat(EByteBuffer& b, int off, float val) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(float));

	int i = eso_float2intBits(val);
	EStream::writeInt((es_byte_t*)b.data()+off, i);
}

void EBits::putDouble(EByteBuffer& b, int off, double val) THROWS(EIndexOutOfBoundsException)
{
	checkRange(b, off, sizeof(double));

	llong l = eso_double2llongBits(val);
	EStream::writeLLong((es_byte_t*)b.data()+off, l);
}

long EBits::pageSize() {
	if (pageSize_ == -1) {
#ifdef WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	pageSize_ = si.dwPageSize;
#else
	pageSize_ = sysconf(_SC_PAGESIZE);
#endif
	}
	return pageSize_;
}

} /* namespace efc */
