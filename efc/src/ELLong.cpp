/*
 * ELLong.cpp
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#include "ELLong.hh"
#include "EInteger.hh"
#include "ECharacter.hh"

namespace efc {

//@see: openjdk-8/src/share/classes/java/lang/Long.java
//@see: activemq-cpp-library-3.9.3/src/main/decaf/lang/Long.cpp

/**
 * All possible chars for representing a number as a String
 */
static const char digits[] = {
	'0' , '1' , '2' , '3' , '4' , '5' ,
	'6' , '7' , '8' , '9' , 'a' , 'b' ,
	'c' , 'd' , 'e' , 'f' , 'g' , 'h' ,
	'i' , 'j' , 'k' , 'l' , 'm' , 'n' ,
	'o' , 'p' , 'q' , 'r' , 's' , 't' ,
	'u' , 'v' , 'w' , 'x' , 'y' , 'z'
};

ELLong::ELLong(llong value) {
	this->value = value;
}

int ELLong::intValue() {
	return (int)value;
}

llong ELLong::llongValue()
{
	return value;
}

float ELLong::floatValue() {
	return (float)value;
}

double ELLong::doubleValue()
{
	return (double)value;
}

EString ELLong::toString()
{
	return ELLong::toString(value, 10).c_str();
}

EString ELLong::toString(llong i, int radix)
{
	if (radix < ECharacter::MIN_RADIX || radix > ECharacter::MAX_RADIX)
		radix = 10;

	char buf[65] = {0};
	int charPos = 64;
	boolean negative = (i < 0);

	if (!negative) {
		i = -i;
	}

	while (i <= -radix) {
		buf[charPos--] = digits[(int) (-(i % radix))];
		i = i / radix;
	}
	buf[charPos] = digits[(int) (-i)];

	if (negative) {
		buf[--charPos] = '-';
	}

	return EString(buf, charPos, (65 - charPos));
}

EString ELLong::toHexString(llong i)
{
	return toUnsignedString0(i, 4);
}

EString ELLong::toOctalString(llong i)
{
	return toUnsignedString0(i, 3);
}

EString ELLong::toBinaryString(llong i)
{
	return toUnsignedString0(i, 1);
}

EString ELLong::toUnsignedString0(llong i, int shift)
{
	char buf[64];
	int charPos = 64;
	int radix = 1 << shift;
	llong mask = radix - 1;
	do {
	    buf[--charPos] = digits[(int)(i & mask)];
	    i = ((ullong)i) >> shift;
	} while (i != 0);

	return EString(buf, charPos, (64 - charPos));
}

EString ELLong::toUnsignedString(llong i, int radix)
{
	if (i >= 0)
		return toString(i, radix);
	else {
		if (radix < ECharacter::MIN_RADIX || radix > ECharacter::MAX_RADIX)
			radix = 10;

		ullong ui = (ullong)i;

		char buf[65] = {0};
		int charPos = 64;

		while (ui >= radix) {
			buf[charPos--] = digits[(int) (ui % radix)];
			ui = ui / radix;
		}
		buf[charPos] = digits[(int) ui];

		return EString(buf, charPos, (65 - charPos));
	}
}

llong ELLong::parseLLong(const char* s, int radix) THROWS(ENumberFormatException)
{
	if (!s || !*s) {
		return 0;
	}

	if (radix < ECharacter::MIN_RADIX) {
		throw ENumberFormatException(__FILE__, __LINE__);
	}
	if (radix > ECharacter::MAX_RADIX) {
		throw ENumberFormatException(__FILE__, __LINE__);
	}

	llong result = 0;
	boolean negative = false;
	int i = 0, max = eso_strlen(s);
	llong limit;
	llong multmin;
	int digit;

	if (max > 0) {
		if (s[0] == '-') {
			negative = true;
			limit = ELLong::MIN_VALUE;
			i++;
		} else {
			limit = -ELLong::MAX_VALUE;
		}
		multmin = limit / radix;
		if (i < max) {
			digit = EInteger::toDigit(s[i++], radix);
			if (digit < 0) {
				throw ENumberFormatException(__FILE__, __LINE__);
			} else {
				result = -digit;
			}
		}
		while (i < max) {
			// Accumulating negatively avoids surprises near MAX_VALUE
			digit = EInteger::toDigit(s[i++], radix);
			if (digit < 0) {
				throw ENumberFormatException(__FILE__, __LINE__);
			}
			if (result < multmin) {
				throw ENumberFormatException(__FILE__, __LINE__);
			}
			result *= radix;
			if (result < limit + digit) {
				throw ENumberFormatException(__FILE__, __LINE__);
			}
			result -= digit;
		}
	} else {
		throw ENumberFormatException(__FILE__, __LINE__);
	}
	if (negative) {
		if (i > 1) {
			return result;
		} else { /* Only got "-" */
			throw ENumberFormatException(__FILE__, __LINE__);
		}
	} else {
		return -result;
	}
}

ELLong ELLong::valueOf(llong ll)
{
	return ELLong(ll);
}

ELLong ELLong::valueOf(const char* s, int radix)
{
	return ELLong(parseLLong(s, radix));
}

ELLong ELLong::decode(const char* s) {
	int radix = 10;
	int index = 0;
	boolean negative = false;
	ELLong result(0);

	EString nm(s);

	if (nm.length() == 0)
		throw ENumberFormatException(__FILE__, __LINE__, "Zero length string");
	char firstChar = nm.charAt(0);
	// Handle sign, if present
	if (firstChar == '-') {
		negative = true;
		index++;
	} else if (firstChar == '+')
		index++;

	// Handle radix specifier, if present
	if (nm.startsWith("0x", index) || nm.startsWith("0X", index)) {
		index += 2;
		radix = 16;
	}
	else if (nm.startsWith("#", index)) {
		index ++;
		radix = 16;
	}
	else if (nm.startsWith("0", index) && nm.length() > 1 + index) {
		index ++;
		radix = 8;
	}

	if (nm.startsWith("-", index) || nm.startsWith("+", index))
		throw ENumberFormatException(__FILE__, __LINE__, "Sign character in wrong position");

	try {
		result = ELLong::valueOf(nm.substring(index).c_str(), radix);
		result = negative ? ELLong::valueOf(-result.llongValue()) : result;
	} catch (ENumberFormatException& e) {
		// If number is Long.MIN_VALUE, we'll end up here. The next line
		// handles this case, and causes any genuine format error to be
		// rethrown.
		EString constant = negative ? ("-" + nm.substring(index))
								   : nm.substring(index);
		result = ELLong::valueOf(constant.c_str(), radix);
	}
	return result;
}

int ELLong::compare(llong x, llong y) {
	return (x < y) ? -1 : ((x == y) ? 0 : 1);
}

int ELLong::compareUnsigned(llong x, llong y) {
	return compare(x + MIN_VALUE, y + MIN_VALUE);
}

llong ELLong::divideUnsigned(llong dividend, llong divisor) {
	return (ullong)dividend / (ullong)divisor;
}

llong ELLong::remainderUnsigned(llong dividend, llong divisor) {
	return (ullong)dividend % (ullong)divisor;
}

int ELLong::numberOfLeadingZeros(llong value) {
	if (value == 0) {
		return 64;
	}

	ullong uvalue = (ullong) value;

	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue |= uvalue >> 8;
	uvalue |= uvalue >> 16;
	uvalue |= uvalue >> 32;

	return bitCount(~uvalue);
}

int ELLong::numberOfTrailingZeros(llong value) {
	ullong uvalue = (ullong) value;
	return bitCount((uvalue & (-1 * uvalue)) - 1);
}

int ELLong::bitCount(llong value) {
	if (value == 0) {
		return 0;
	}

	ullong uvalue = (ullong) value;

	uvalue = (uvalue & 0x5555555555555555LL) + ((uvalue >> 1) & 0x5555555555555555LL);
	uvalue = (uvalue & 0x3333333333333333LL) + ((uvalue >> 2) & 0x3333333333333333LL);
	// adjust for 64-bit integer
	uint i = (uint) ((uvalue >> 32) + uvalue);
	i = (i & 0x0F0F0F0F) + ((i >> 4) & 0x0F0F0F0F);
	i = (i & 0x00FF00FF) + ((i >> 8) & 0x00FF00FF);
	i = (i & 0x0000FFFF) + ((i >> 16) & 0x0000FFFF);
	return i;
}

llong ELLong::rotateLeft(llong i, int distance) {
	return (i << distance) | (((ullong)i) >> -distance);
}

llong ELLong::rotateRight(llong i, int distance) {
	return (((ullong)i) >> distance) | (i << -distance);
}

llong ELLong::reverse(llong i) {
	// HD, Figure 7-1
	i = (i & 0x5555555555555555LL) << 1 | ((((ullong)i) >> 1) & 0x5555555555555555LL);
	i = (i & 0x3333333333333333LL) << 2 | ((((ullong)i) >> 2) & 0x3333333333333333LL);
	i = (i & 0x0f0f0f0f0f0f0f0fLL) << 4 | ((((ullong)i) >> 4) & 0x0f0f0f0f0f0f0f0fLL);
	i = (i & 0x00ff00ff00ff00ffLL) << 8 | ((((ullong)i) >> 8) & 0x00ff00ff00ff00ffLL);
	i = (i << 48) | ((i & 0xffff0000L) << 16) |
		((((ullong)i) >> 16) & 0xffff0000L) | (((ullong)i) >> 48);
	return i;
}

int ELLong::signum(llong i) {
	// HD, Section 2-7
	return (int) ((i >> 63) | (((ullong)-i) >> 63));
}

llong ELLong::reverseBytes(llong i) {
	if (i == 0) {
		return 0;
	}
	i = (i & 0x00ff00ff00ff00ffLL) << 8 | ((((ullong)i) >> 8) & 0x00ff00ff00ff00ffLL);
	return (i << 48) | ((i & 0xffff0000L) << 16) |
		((((ullong)i) >> 16) & 0xffff0000L) | (((ullong)i) >> 48);
}

boolean ELLong::equals(ELLong* obj)
{
	return obj ? (value == obj->llongValue()) : false;
}

boolean ELLong::equals(EObject* obj)
{
	ELLong* other = dynamic_cast<ELLong*>(obj);
	return other ? (value == other->llongValue()) : false;
}

int ELLong::hashCode() {
	ullong v = (ullong)value;
	return (int)(v ^ (v >> 32));
}

int ELLong::compareTo(ELLong* anotherLong) {
	llong thisVal = this->value;
	llong anotherVal = anotherLong->value;
	return (thisVal<anotherVal ? -1 : (thisVal==anotherVal ? 0 : 1));
}

llong ELLong::highestOneBit(llong value) {
	if (value == 0) {
		return 0;
	}

	ullong uvalue = (ullong) value;

	uvalue |= (uvalue >> 1);
	uvalue |= (uvalue >> 2);
	uvalue |= (uvalue >> 4);
	uvalue |= (uvalue >> 8);
	uvalue |= (uvalue >> 16);
	uvalue |= (uvalue >> 32);

	return (uvalue & ~(uvalue >> 1));
}

llong ELLong::lowestOneBit(llong i) {
	// HD, Section 2-1
	return i & -i;
}

} /* namespace efc */
