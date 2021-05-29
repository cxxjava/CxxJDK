/*
 * EShort.cpp
 *
 *  Created on: 2017-6-21
 *      Author: cxxjava@163.com
 */

#include "EShort.hh"
#include "EInteger.hh"

namespace efc {

EShort::EShort(short value) {
	this->value = value;
}

boolean EShort::equals(short that)
{
	return (value == that);
}

EString EShort::toString()
{
	return EString::valueOf(value);
}

short EShort::parseShort(const char *s, int radix) THROWS(ENumberFormatException)
{
	int i = EInteger::parseInt(s, radix);
	if (i < MIN_VALUE || i > MAX_VALUE)
		throw ENumberFormatException(__FILE__, __LINE__);
	return (short) i;
}

EShort EShort::valueOf(const char* s, int radix)
{
	return EShort(parseShort(s, radix));
}

int EShort::compare(short x, short y) {
	return x - y;
}

boolean EShort::equals(EShort* obj)
{
	if (!obj) return false;
	return value == obj->shortValue();
}

boolean EShort::equals(EObject* obj) {
	if (!obj) return false;
	EShort* that = dynamic_cast<EShort*>(obj);
	if (!that) return false;
	return value == that->shortValue();
}

int  EShort::hashCode() {
	return (int)value;
}

int EShort::compareTo(EShort* anotherShort) {
	return compare(this->value, anotherShort->value);
}

byte EShort::byteValue()
{
	return (byte)value;
}

short EShort::shortValue()
{
	return (short)value;
}

int EShort::intValue()
{
	return (int)value;
}

llong EShort::llongValue()
{
	return (llong)value;
}

float EShort::floatValue() {
	return (float)value;
}

double EShort::doubleValue() {
	return (double)value;
}

} /* namespace efc */
