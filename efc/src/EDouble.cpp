/*
 * EDouble.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EDouble.hh"

namespace efc {

const double EDouble::MAX_VALUE = DOUBLE_MAX_VALUE;
const double EDouble::MIN_NORMAL = DOUBLE_MIN_NORMAL;
const double EDouble::MIN_VALUE = DOUBLE_MIN_VALUE;
const int EDouble::MAX_EXPONENT = DOUBLE_MAX_EXPONENT;
const int EDouble::MIN_EXPONENT = DOUBLE_MIN_EXPONENT;
const int EDouble::SIZE = DOUBLE_SIZE;

EDouble::EDouble(double value) {
	this->value = value;
}

EDouble::EDouble(const char* s) THROWS(ENumberFormatException)
{
	if (!s || !*s) {
		throw ENumberFormatException(__FILE__, __LINE__, "empty String");
	}

	this->value = (double)eso_atof(s);
}

EString EDouble::toString(double f)
{
	return EString::formatOf("%f", f);
}

EDouble EDouble::valueOf(const char* s) THROWS(ENumberFormatException)
{
	return EDouble(s);
}

EDouble EDouble::valueOf(double f)
{
	return EDouble(f);
}

double EDouble::parseDouble(const char* s) THROWS(ENumberFormatException)
{
	EDouble d(s);
	return d.doubleValue();
}

llong EDouble::doubleToLLongBits(double value)
{
	return eso_double2llongBits(value);
}

double EDouble::llongBitsToDouble(llong bits)
{
	return eso_llongBits2double(bits);
}

int EDouble::compare(double d1, double d2)
{
	if (d1 < d2)
		return -1; // Neither val is NaN, thisVal is smaller
	if (d1 > d2)
		return 1; // Neither val is NaN, thisVal is larger

	long thisBits = EDouble::doubleToLLongBits(d1);
	long anotherBits = EDouble::doubleToLLongBits(d2);

	return (thisBits == anotherBits ? 0 : // Values are equal
			(thisBits < anotherBits ? -1 : // (-0.0, 0.0) or (!NaN, NaN)
					1)); // (0.0, -0.0) or (NaN, !NaN)
}

EString EDouble::toString()
{
	return EString::formatOf("%lf", value);
}

byte EDouble::byteValue()
{
	return (byte)value;
}

short EDouble::shortValue()
{
	return (short)value;
}

int EDouble::intValue()
{
	return (int)value;
}

llong EDouble::llongValue()
{
	return (llong)value;
}

float EDouble::floatValue()
{
	return (float)value;
}

double EDouble::doubleValue()
{
	return value;
}

int EDouble::hashCode()
{
	ullong bits = (ullong)eso_double2llongBits(value);
	return (int)(bits ^ (bits >> 32));
}

boolean EDouble::equals(EDouble *obj)
{
	return obj ? ((eso_double2llongBits(obj->value) == eso_double2llongBits(value))) : false;
}

boolean EDouble::equals(EObject* obj) {
	EDouble* that = dynamic_cast<EDouble*>(obj);
	return that ? ((eso_double2llongBits(that->value) == eso_double2llongBits(value))) : false;
}

int EDouble::compareTo(EDouble* anotherDouble)
{
	return EDouble::compare(value, anotherDouble->value);
}

} /* namespace efc */
