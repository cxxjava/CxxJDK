/*
 * EFloat.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EFloat.hh"

namespace efc {

const float EFloat::MAX_VALUE = FLOAT_MAX_VALUE;
const float EFloat::MIN_NORMAL = FLOAT_MIN_NORMAL;
const float EFloat::MIN_VALUE = FLOAT_MIN_VALUE;
const int EFloat::MAX_EXPONENT = FLOAT_MAX_EXPONENT;
const int EFloat::MIN_EXPONENT = FLOAT_MIN_EXPONENT;
const int EFloat::SIZE = FLOAT_SIZE;

EFloat::EFloat(float value) {
	this->value = value;
}

EFloat::EFloat(double value)
{
	this->value = (float)value;
}

EFloat::EFloat(const char* s) THROWS(ENumberFormatException)
{
	if (!s || !*s) {
		throw ENumberFormatException(__FILE__, __LINE__, "empty String");
	}

	this->value = (float)eso_atof(s);
}

EString EFloat::toString(float f)
{
	return EString::formatOf("%f", f);
}

EFloat EFloat::valueOf(const char* s) THROWS(ENumberFormatException)
{
	return EFloat(s);
}

EFloat EFloat::valueOf(float f)
{
	return EFloat(f);
}

float EFloat::parseFloat(const char* s) THROWS(ENumberFormatException)
{
	EFloat f(s);
	return f.floatValue();
}

int EFloat::floatToIntBits(float value)
{
	return eso_float2intBits(value);
}

float EFloat::intBitsToFloat(int bits)
{
	return eso_intBits2float(bits);
}

int EFloat::compare(float f1, float f2)
{
	if (f1 < f2)
        return -1;		 // Neither val is NaN, thisVal is smaller
    if (f1 > f2)
        return 1;		 // Neither val is NaN, thisVal is larger

	int thisBits = EFloat::floatToIntBits(f1);
	int anotherBits = EFloat::floatToIntBits(f2);

	return (thisBits == anotherBits ?  0 : // Values are equal
           (thisBits < anotherBits ? -1 : // (-0.0, 0.0) or (!NaN, NaN)
            1));                          // (0.0, -0.0) or (NaN, !NaN)
}

EString EFloat::toString()
{
	return EString::formatOf("%f", value);
}

byte EFloat::byteValue()
{
	return (byte)value;
}

short EFloat::shortValue()
{
	return (short)value;
}

int EFloat::intValue()
{
	return (int)value;
}

llong EFloat::llongValue()
{
	return (llong)value;
}

float EFloat::floatValue()
{
	return value;
}

double EFloat::doubleValue()
{
	return (double)value;
}

int EFloat::hashCode()
{
	return floatToIntBits(value);
}

boolean EFloat::equals(EFloat *obj)
{
	return obj ? ((floatToIntBits(obj->value) == floatToIntBits(value))) : false;
}

boolean EFloat::equals(EObject* obj) {
	EFloat* that = dynamic_cast<EFloat*>(obj);
	return that ? ((floatToIntBits(that->value) == floatToIntBits(value))) : false;
}

int EFloat::compareTo(EFloat* anotherFloat)
{
	return EFloat::compare(value, anotherFloat->value);
}

} /* namespace efc */
