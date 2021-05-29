/*
 * EByte.cpp
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#include "EByte.hh"
#include "EInteger.hh"

namespace efc {

EByte::EByte(byte value) {
	this->value = value;
}

boolean EByte::equals(byte that)
{
	return (value == that);
}

EString EByte::toString()
{
	return EString::valueOf(value);
}

byte EByte::parseByte(const char *s, int radix) THROWS(ENumberFormatException)
{
	int i = EInteger::parseInt(s, radix);
	if (i < MIN_VALUE || i > MAX_VALUE)
		throw ENumberFormatException(__FILE__, __LINE__);
	return (byte) i;
}

int EByte::compare(byte x, byte y) {
	return x - y;
}

boolean EByte::equals(EByte* obj)
{
	if (!obj) return false;
	return value == obj->byteValue();
}

boolean EByte::equals(EObject* obj) {
	if (!obj) return false;
	EByte* that = dynamic_cast<EByte*>(obj);
	if (!that) return false;
	return value == that->byteValue();
}

int  EByte::hashCode() {
	return (int)value;
}

int EByte::compareTo(EByte* anotherByte) {
	return compare(this->value, anotherByte->value);
}


byte EByte::byteValue()
{
	return (byte)value;
}

short EByte::shortValue()
{
	return (short)value;
}

int EByte::intValue()
{
	return (int)value;
}

llong EByte::llongValue()
{
	return (llong)value;
}

float EByte::floatValue() {
	return (float)value;
}

double EByte::doubleValue() {
	return (double)value;
}

} /* namespace efc */
