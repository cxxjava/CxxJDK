#include "EInteger.hh"
#include "ELLong.hh"
#include "ECharacter.hh"

namespace efc {

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

EInteger::EInteger(int value)
{
	this->value = value;
}

EString EInteger::toString(int i, int radix)
{
	if (radix < ECharacter::MIN_RADIX || radix > ECharacter::MAX_RADIX)
		radix = 10;
	
	char buf[33] = {0};
	boolean negative = (i < 0);
	int charPos = 32;
	
	if (!negative) {
		i = -i;
	}

	while (i <= -radix) {
		buf[charPos--] = digits[-(i % radix)];
		i = i / radix;
	}
	buf[charPos] = digits[-i];

	if (negative) {
		buf[--charPos] = '-';
	}

	return EString(buf, charPos, (33 - charPos));
}

EString EInteger::toUnsignedString0(int i, int shift) {
	char buf[32];
	int charPos = 32;
	int radix = 1 << shift;
	int mask = radix - 1;
	do {
		buf[--charPos] = digits[i & mask];
		i = ((unsigned int)i) >> shift;
	} while (i != 0);

	return EString(buf, charPos, (32 - charPos));
}

EString EInteger::toUnsignedString(int i, int radix)
{
	if (i >= 0)
		return toString(i, radix);
	else {
		if (radix < ECharacter::MIN_RADIX || radix > ECharacter::MAX_RADIX)
			radix = 10;

		uint ui = (uint)i;

		char buf[33] = {0};
		int charPos = 32;

		while (ui >= radix) {
			buf[charPos--] = digits[(ui % radix)];
			ui = ui / radix;
		}
		buf[charPos] = digits[ui];

		return EString(buf, charPos, (33 - charPos));
	}
}

EString EInteger::toHexString(int i)
{
	return toUnsignedString0(i, 4);
}

EString EInteger::toOctalString(int i)
{
	return toUnsignedString0(i, 3);
}

EString EInteger::toBinaryString(int i)
{
	return toUnsignedString0(i, 1);
}

int EInteger::parseInt(const char* s, int radix) THROWS(ENumberFormatException)
{
	if (!s || !*s) {
		throw ENumberFormatException(__FILE__, __LINE__);
	}
	
	if (radix < ECharacter::MIN_RADIX) {
		throw ENumberFormatException(__FILE__, __LINE__); //error
	}

	if (radix > ECharacter::MAX_RADIX) {
		throw ENumberFormatException(__FILE__, __LINE__); //error
	}
	
	int result = 0;
	boolean negative = false;
	int i = 0, max = eso_strlen(s);
	int limit;
	int multmin;
	int digit;

	if (max > 0) {
		if (s[0] == '-') {
			negative = true;
			limit = EInteger::MIN_VALUE;
			i++;
		} else {
			limit = -EInteger::MAX_VALUE;
		}
		multmin = limit / radix;
		if (i < max) {
			digit = toDigit(s[i++],radix);
			if (digit < 0) {
				throw ENumberFormatException(__FILE__, __LINE__); //error
			} else {
				result = -digit;
			}
		}
		while (i < max) {
			// Accumulating negatively avoids surprises near MAX_VALUE
			digit = toDigit(s[i++],radix);
			if (digit < 0) {
				throw ENumberFormatException(__FILE__, __LINE__); //error
			}
			if (result < multmin) {
				throw ENumberFormatException(__FILE__, __LINE__); //error
			}
			result *= radix;
			if (result < limit + digit) {
				throw ENumberFormatException(__FILE__, __LINE__); //error
			}
			result -= digit;
		}
	} else {
		return 0;
	}
	if (negative) {
		if (i > 1) {
			return result;
		} else {	/* Only got "-" */
			throw ENumberFormatException(__FILE__, __LINE__); //error
		}
	} else {
		return -result;
	}
}

int EInteger::toDigit(char ch, int radix)
{
	int value = -1;
	if (radix >= ECharacter::MIN_RADIX && radix <= ECharacter::MAX_RADIX) {
		if (eso_isdigit(ch)) {
			value = ch - '0';
		} else if (eso_isupper(ch) || eso_islower(ch)) {
			// Java supradecimal digit
			value = (ch & 0x1F) + 9;
		}
	}
	return (value < radix) ? value : -1;
}

EInteger EInteger::valueOf(int i)
{
	return EInteger(i);
}

EInteger EInteger::valueOf(const char* s, int radix)
{
	return EInteger(parseInt(s, radix));
}

EInteger EInteger::decode(const char* s) {
	int radix = 10;
	int index = 0;
	boolean negative = false;
	EInteger result(0);

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
		result = EInteger::valueOf(nm.substring(index).c_str(), radix);
		result = negative ? EInteger::valueOf(-result.intValue()) : result;
	} catch (ENumberFormatException& e) {
		// If number is Integer.MIN_VALUE, we'll end up here. The next line
		// handles this case, and causes any genuine format error to be
		// rethrown.
		EString constant = negative ? ("-" + nm.substring(index))
								   : nm.substring(index);
		result = EInteger::valueOf(constant.c_str(), radix);
	}
	return result;
}

int EInteger::compare(int x, int y) {
	return (x < y) ? -1 : ((x == y) ? 0 : 1);
}

int EInteger::compareUnsigned(int x, int y) {
	return compare(x + MIN_VALUE, y + MIN_VALUE);
}

int EInteger::divideUnsigned(int dividend, int divisor) {
	return (uint)dividend / (uint)divisor;
}

int EInteger::remainderUnsigned(int dividend, int divisor) {
	return (uint)dividend % (uint)divisor;
}

int EInteger::numberOfLeadingZeros(int value) {
	if (value == 0) {
		return 32;
	}

	unsigned int uvalue = (unsigned int) value;

	uvalue |= uvalue >> 1;
	uvalue |= uvalue >> 2;
	uvalue |= uvalue >> 4;
	uvalue |= uvalue >> 8;
	uvalue |= uvalue >> 16;

	return bitCount(~uvalue);
}

int EInteger::numberOfTrailingZeros(int value) {
	if (value == 0) {
		return 32;
	}
	unsigned int uvalue = (unsigned int) value;
	return bitCount((uvalue & -uvalue) - 1);
}

int EInteger::bitCount(int value) {
	if (value == 0) {
		return 0;
	}

	unsigned int uvalue = (unsigned int) value;

	// 32-bit recursive reduction using SWAR...
	// but first step is mapping 2-bit values
	// into sum of 2 1-bit values in sneaky way
	uvalue -= ((uvalue >> 1) & 0x55555555);
	uvalue = (((uvalue >> 2) & 0x33333333) + (uvalue & 0x33333333));
	uvalue = (((uvalue >> 4) + uvalue) & 0x0F0F0F0F);
	uvalue += (uvalue >> 8);
	uvalue += (uvalue >> 16);

	return (uvalue & 0x0000003F);
}

int EInteger::rotateLeft(int i, int distance) {
	return (i << distance) | (((uint)i) >> -distance);
}

int EInteger::rotateRight(int i, int distance) {
	return (((uint)i) >> distance) | (i << -distance);
}

int EInteger::reverse(int i) {
	// HD, Figure 7-1
	i = (i & 0x55555555) << 1 | ((((uint)i) >> 1) & 0x55555555);
	i = (i & 0x33333333) << 2 | ((((uint)i) >> 2) & 0x33333333);
	i = (i & 0x0f0f0f0f) << 4 | ((((uint)i) >> 4) & 0x0f0f0f0f);
	i = (i << 24) | ((i & 0xff00) << 8) |
		((((uint)i) >> 8) & 0xff00) | (((uint)i) >> 24);
	return i;
}

int EInteger::signum(int i) {
	// HD, Section 2-7
	return (i >> 31) | (((uint)-i) >> 31);
}

int EInteger::reverseBytes(int i) {
	return ((((uint)i) >> 24)           ) |
		   ((i >>   8) &   0xFF00) |
		   ((i <<   8) & 0xFF0000) |
		   ((i << 24));
}

byte EInteger::byteValue()
{
	return (byte)value;
}

short EInteger::shortValue()
{
	return (short)value;
}

int EInteger::intValue()
{
	return (int)value;
}

llong EInteger::llongValue()
{
	return (llong)value;
}

float EInteger::floatValue() {
	return (float)value;
}

double EInteger::doubleValue() {
	return (double)value;
}

boolean EInteger::equals(EInteger* obj)
{
	return obj ? (value == obj->intValue()) : false;
}

boolean EInteger::equals(EObject* obj) {
	EInteger* that = dynamic_cast<EInteger*>(obj);
	return that ? (value == that->intValue()) : false;
}

EString EInteger::toString() {
	return EString(value);
}

int EInteger::hashCode() {
	return value;
}

int EInteger::compareTo(EInteger* anotherInteger) {
	int thisVal = this->value;
	int anotherVal = anotherInteger->value;
	return (thisVal<anotherVal ? -1 : (thisVal==anotherVal ? 0 : 1));
}

int EInteger::highestOneBit(int i) {
	// HD, Figure 3-1
	i |= (i >>  1);
	i |= (i >>  2);
	i |= (i >>  4);
	i |= (i >>  8);
	i |= (i >> 16);
	return i - (((unsigned)i) >> 1);
}

int EInteger::lowestOneBit(int i) {
	// HD, Section 2-1
	return i & -i;
}

} /* namespace efc */
