/*
 * EBigDecimal.cpp
 *
 *  Created on: 2017-6-28
 *      Author: cxxjava@163.com
 */

#include "EBigDecimal.hh"
#include "ENumberFormatException.hh"
#include "EArithmeticException.hh"
#include "EIllegalArgumentException.hh"

#include <math.h>

namespace efc {

EBigDecimal::~EBigDecimal() {
}

EBigDecimal::EBigDecimal(llong val): decimalDigits(0) {
	*this = val;
}

EBigDecimal::EBigDecimal(const char* val): decimalDigits(0) {
	*this = val;
}

EBigDecimal::EBigDecimal(const EBigInteger& val) {
	*this = val;
}

EBigDecimal::EBigDecimal(const EBigDecimal& that) {
	*this = that;
}

EBigDecimal EBigDecimal::operator -() {
	EBigDecimal r(*this);
	if (r.intVal != 0)
		r.intVal.setSign(!r.intVal.getSign());

	return r;
}

EBigDecimal& EBigDecimal::operator =(const llong val) {
	intVal = val;
	decimalDigits = 0;
	return *this;
}

EBigDecimal& EBigDecimal::operator =(const char* val) {
	EString s(val);
	int dotPos = s.lastIndexOf('.');
	if (dotPos > 0) {
		s.rtrim('0'); //The decimal part removes the right '0'
		if (s.endsWith(".")) {
			decimalDigits = 0;
		} else {
			decimalDigits = s.length() - dotPos - 1;
		}
		s.replace(".", "");
		intVal = s.c_str();
	} else {
		intVal = val;
		decimalDigits = 0;
	}
	return *this;
}

EBigDecimal& EBigDecimal::operator =(const EBigInteger& b) {
	intVal = b;
	decimalDigits = 0;
	return *this;
}

EBigDecimal& EBigDecimal::operator =(const EBigDecimal& d) {
	if (this == &d)
		return *this;

	EBigDecimal* bd = (EBigDecimal*) ((&d));
	intVal = bd->intVal;
	decimalDigits = bd->decimalDigits;
	return *this;
}
boolean EBigDecimal::operator ==(llong val) {
	EBigDecimal d(val);
	return this->operator ==(d);
}

boolean EBigDecimal::operator ==(EBigInteger& b) {
	EBigDecimal d(b);
	return this->operator ==(d);
}

boolean EBigDecimal::operator ==(EBigDecimal& d) {
	if (this->decimalDigits != d.decimalDigits || this->intVal != d.intVal) {
		return false;
	}
	return true;
}

boolean EBigDecimal::operator !=(llong val) {
	EBigDecimal d(val);
	return this->operator !=(d);
}

boolean EBigDecimal::operator !=(EBigInteger& b) {
	EBigDecimal d(b);
	return this->operator !=(d);
}

boolean EBigDecimal::operator !=(EBigDecimal& d) {
	return !(*this == d);
}

boolean EBigDecimal::operator >(llong val) {
	EBigDecimal d(val);
	return this->operator >(d);
}

boolean EBigDecimal::operator >(EBigInteger& b) {
	EBigDecimal d(b);
	return this->operator >(d);
}

boolean EBigDecimal::operator >(EBigDecimal& d) {
	if (decimalDigits == d.decimalDigits) {
		return (intVal > d.intVal);
	} else {
		if (decimalDigits > d.decimalDigits) {
			EBigInteger val(d.intVal);
			val = val.pow10(decimalDigits - d.decimalDigits);
			return (intVal > val);
		} else {
			EBigInteger val(intVal);
			val = val.pow10(d.decimalDigits - decimalDigits);
			return (val > d.intVal);
		}
	}
}

boolean EBigDecimal::operator <(llong val) {
	EBigDecimal d(val);
	return this->operator <(d);
}

boolean EBigDecimal::operator <(EBigInteger& b) {
	EBigDecimal d(b);
	return this->operator <(d);
}

boolean EBigDecimal::operator <(EBigDecimal& d) {
	return (*this > d) && !(*this == d);
}

boolean EBigDecimal::operator >=(llong val) {
	EBigDecimal d(val);
	return this->operator >=(d);
}

boolean EBigDecimal::operator >=(EBigInteger& b) {
	EBigDecimal d(b);
	return this->operator >=(d);
}

boolean EBigDecimal::operator >=(EBigDecimal& d) {
	return (*this > d) || (*this == d);
}

boolean EBigDecimal::operator <=(llong val) {
	EBigDecimal d(val);
	return this->operator <=(d);
}

boolean EBigDecimal::operator <=(EBigInteger& b) {
	EBigDecimal d(b);
	return this->operator <=(d);
}

boolean EBigDecimal::operator <=(EBigDecimal& d) {
	return !(*this > d);
}

EBigDecimal EBigDecimal::operator +(llong val) {
	return add(val);
}

EBigDecimal EBigDecimal::operator +(EBigInteger& b) {
	return add(b);
}

EBigDecimal EBigDecimal::operator +(EBigDecimal& d) {
	return add(d);
}

EBigDecimal EBigDecimal::operator -(llong val) {
	return subtract(val);
}

EBigDecimal EBigDecimal::operator -(EBigInteger& b) {
	return subtract(b);
}

EBigDecimal EBigDecimal::operator -(EBigDecimal& d) {
	return subtract(d);
}

EBigDecimal EBigDecimal::operator *(llong val) {
	return multiply(val);
}

EBigDecimal EBigDecimal::operator *(EBigInteger& b) {
	return multiply(b);
}

EBigDecimal EBigDecimal::operator *(EBigDecimal& d) {
	return multiply(d);
}

EBigDecimal EBigDecimal::operator /(llong val) {
	return divide(val);
}

EBigDecimal EBigDecimal::operator /(EBigInteger& b) {
	return divide(b);
}

EBigDecimal EBigDecimal::operator /(EBigDecimal& d) {
	return divide(d);
}

EBigDecimal EBigDecimal::operator %(llong val) {
	return remainder(val);
}

EBigDecimal EBigDecimal::operator %(EBigInteger& b) {
	return remainder(b);
}

EBigDecimal EBigDecimal::operator %(EBigDecimal& d) {
	return remainder(d);
}

EBigDecimal& EBigDecimal::operator +=(llong val) {
	EBigInteger b(val);
	b = b.pow10(decimalDigits);
	intVal += b;
	return *this;
}

EBigDecimal& EBigDecimal::operator +=(EBigInteger& b) {
	EBigInteger val = b.pow10(decimalDigits);
	intVal += val;
	return *this;
}

EBigDecimal& EBigDecimal::operator +=(EBigDecimal& d) {
	if (decimalDigits == d.decimalDigits) {
		intVal += d.intVal;
		return *this;
	} else {
		if (decimalDigits > d.decimalDigits) {
			EBigInteger val(d.intVal);
			val = val.pow10(decimalDigits - d.decimalDigits);
			intVal += val;
			return *this;
		} else {
			intVal = intVal.pow10(d.decimalDigits - decimalDigits);
			decimalDigits = d.decimalDigits;
			intVal += d.intVal;
			return *this;
		}
	}
}

EBigDecimal& EBigDecimal::operator -=(llong val) {
	EBigInteger b(val);
	b = b.pow10(decimalDigits);
	intVal -= b;
	return *this;
}

EBigDecimal& EBigDecimal::operator -=(EBigInteger& b) {
	EBigInteger val = b.pow10(decimalDigits);
	intVal -= val;
	return *this;
}

EBigDecimal& EBigDecimal::operator -=(EBigDecimal& d) {
	if (decimalDigits == d.decimalDigits) {
		intVal -= d.intVal;
		return *this;
	} else {
		if (decimalDigits > d.decimalDigits) {
			EBigInteger val(d.intVal);
			val = val.pow10(decimalDigits - d.decimalDigits);
			intVal -= val;
			return *this;
		} else {
			intVal = intVal.pow10(d.decimalDigits - decimalDigits);
			decimalDigits = d.decimalDigits;
			intVal -= d.intVal;
			return *this;
		}
	}
}

EBigDecimal& EBigDecimal::operator *=(llong val) {
	EBigInteger b(val);
	intVal *= b;
	return *this;
}

EBigDecimal& EBigDecimal::operator *=(EBigInteger& b) {
	intVal *= b;
	return *this;
}

EBigDecimal& EBigDecimal::operator *=(EBigDecimal& d) {
	intVal *= d.intVal;
	decimalDigits += d.decimalDigits;
	return *this;
}

EBigDecimal& EBigDecimal::operator /=(llong val) {
	EBigInteger b(val);
	b = b.pow10(decimalDigits);
	decimalDigits = 0;//
	intVal /= val;
	return *this;
}

EBigDecimal& EBigDecimal::operator /=(EBigInteger& b) {
	EBigInteger val = b.pow10(decimalDigits);
	decimalDigits = 0;//
	intVal /= val;
	return *this;
}

EBigDecimal& EBigDecimal::operator /=(EBigDecimal& d) {
	if (decimalDigits == d.decimalDigits) {
		intVal /= d.intVal;
	} else {
		if (decimalDigits > d.decimalDigits) {
			EBigInteger val(d.intVal.pow10(decimalDigits - d.decimalDigits));
			intVal /= val;
		} else {
			intVal = intVal.pow10(d.decimalDigits - decimalDigits) / d.intVal;
		}
	}
	decimalDigits = 0;//
	return *this;
}

EBigDecimal& EBigDecimal::operator %=(llong val) {
	// we use the identity  x = i * y + r to determine r
	EBigDecimal d = *this / val;
	EBigDecimal m = (d * val);
	*this = *this - m;
	return *this;
}

EBigDecimal& EBigDecimal::operator %=(EBigInteger& b) {
	// we use the identity  x = i * y + r to determine r
	EBigDecimal d = *this / b;
	EBigDecimal m = (d * b);
	*this = *this - m;
	return *this;
}

EBigDecimal& EBigDecimal::operator %=(EBigDecimal& d) {
	// we use the identity  x = i * y + r to determine r
	EBigDecimal v = *this / d;
	EBigDecimal m = (d * v);
	*this = *this - m;
	return *this;
}

EBigDecimal EBigDecimal::add(llong val) {
	EBigDecimal d(val);
	return add(d);
}

EBigDecimal EBigDecimal::add(EBigInteger& b) {
	EBigDecimal d(b);
	return add(d);
}

EBigDecimal EBigDecimal::add(EBigDecimal& d) {
	EBigDecimal r(*this);
	r += d;
	return r;
}

EBigDecimal EBigDecimal::subtract(llong val) {
	EBigDecimal d(val);
	return subtract(d);
}

EBigDecimal EBigDecimal::subtract(EBigInteger& b) {
	EBigDecimal d(b);
	return subtract(d);
}

EBigDecimal EBigDecimal::subtract(EBigDecimal& d) {
	EBigDecimal r(*this);
	r -= d;
	return r;
}

EBigDecimal EBigDecimal::multiply(llong val) {
	EBigDecimal d(val);
	return multiply(d);
}

EBigDecimal EBigDecimal::multiply(EBigInteger& b) {
	EBigDecimal d(b);
	return multiply(d);
}

EBigDecimal EBigDecimal::multiply(EBigDecimal& d) {
	EBigDecimal r(*this);
	r *= d;
	return r;
}

EBigDecimal EBigDecimal::divide(llong val) {
	EBigDecimal d(val);
	return divide(d);
}

EBigDecimal EBigDecimal::divide(EBigInteger& b) {
	EBigDecimal d(b);
	return divide(d);
}

EBigDecimal EBigDecimal::divide(EBigDecimal& d) {
	EBigDecimal r(*this);
	r /= d;
	return r;
}

EBigDecimal EBigDecimal::divide(llong val, int scale, int roundingMode) {
	EBigDecimal d(val);
	return divide(d, scale, roundingMode);
}

EBigDecimal EBigDecimal::divide(EBigInteger& b, int scale, int roundingMode) {
	EBigDecimal d(b);
	return divide(d, scale, roundingMode);
}

EBigDecimal EBigDecimal::divide(EBigDecimal& d, int scale, int roundingMode) {
	if (scale < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "scale < 0");
	}

	if (intVal == 0LL || d.intVal == 0LL) {
		return 0LL; //The divisor or is divided into 0 and the result is 0
	}

	EBigDecimal r(*this);
	EBigDecimal v(d);
//	printf("r=%s\n", r.intVal.toString().c_str());
//	printf("v=%s\n", v.intVal.toString().c_str());

	if (scale > 0 || roundingMode != ROUND_DOWN) {
		r.intVal = r.intVal.pow10(scale + ((roundingMode == ROUND_DOWN) ? 0 : 1)); //After +1, for rounding processing
	}
//	printf("r1=%s\n", r.intVal.toString().c_str());

	if (r.decimalDigits > v.decimalDigits) {
		v.intVal = v.intVal.pow10(r.decimalDigits - v.decimalDigits);
	} else if (r.decimalDigits < v.decimalDigits) {
		r.intVal = r.intVal.pow10(v.decimalDigits - r.decimalDigits);
	}
//	printf("r2=%s\n", r.intVal.toString().c_str());

	r.decimalDigits = scale; //

	//@see: https://my.oschina.net/sunchp/blog/670909
	if (roundingMode != ROUND_DOWN) {
		r.intVal /= v.intVal;
//		printf("xx1=%s\n", r.intVal.toString().c_str());
		switch (roundingMode) {
		case ROUND_UP:
			r.intVal += (r.intVal.getSign() ? -9 : 9);
			break;
		case ROUND_CEILING:
			if (!r.intVal.getSign()) {
				r.intVal += 9;
			}
			break;
		case ROUND_FLOOR:
			if (r.intVal.getSign()) {
				r.intVal -= 9;
			}
			break;
		case ROUND_HALF_UP:
			r.intVal += (r.intVal.getSign() ? -5 : 5);
			break;
		case ROUND_HALF_DOWN:
			r.intVal += (r.intVal.getSign() ? -4 : 4);
			break;
		case ROUND_HALF_EVEN:
		{
			EVector<int>& number = *r.intVal.getNumber();
			if ((number[0] > 4) && (number[1] % 2 == 1)) {
				r.intVal += (r.intVal.getSign() ? -5 : 5);
			}
			break;
		}
		case ROUND_UNNECESSARY:
		{
			EVector<int>& number = *r.intVal.getNumber();
			if (number[0] != 0) {
				throw EArithmeticException(__FILE__, __LINE__, EString::valueOf(number[0]).c_str());
			}
			break;
		}
		default:
			break;
		}
//		printf("xx2=%s\n", r.intVal.toString().c_str());
		r.intVal /= 10;
	} else {
		r.intVal /= v.intVal;
	}

	return r;
}

EBigDecimal EBigDecimal::remainder(llong val) {
	EBigDecimal d(val);
	return remainder(d);
}

EBigDecimal EBigDecimal::remainder(EBigInteger& b) {
	EBigDecimal d(b);
	return remainder(d);
}

EBigDecimal EBigDecimal::remainder(EBigDecimal& d) {
	EBigDecimal r(*this);
	r %= d;
	return r;
}

EA<EBigDecimal*> EBigDecimal::divideAndRemainder(EBigDecimal& d) {
	// we use the identity  x = i * y + r to determine r
	EA<EBigDecimal*> r(2);
	r[0] = new EBigDecimal(*this / d);
	EBigDecimal m = (d * (*r[0]));
	r[1] = new EBigDecimal(*this - m);
	return r;
}

EBigDecimal EBigDecimal::abs() {
	EBigDecimal r(*this);
	if (r.intVal.getSign()) {
		r.intVal.setSign(false);
	}
	return r;
}

int EBigDecimal::scale() {
	return decimalDigits;
}

EBigInteger EBigDecimal::toBigInteger() {
	EBigInteger r(this->intVal);

	EVector<int>& number = *r.getNumber();
	for (int i = 0; i < decimalDigits; i++) {
		number.removeAt(0);
	}

	return r;
}

int EBigDecimal::hashCode() {
	return 31*intVal.hashCode() + decimalDigits;
}

EString EBigDecimal::toString() {
	EString s = intVal.toString();
	if (decimalDigits > 0) {
		int i = intVal.getSign() ? 1 : 0;
		int n = decimalDigits - s.length() + i;
		if (n >= 0) { //Insufficient number of decimal places, Pre complement '0'
			s.insert(i, n+1, '0');
			s.insert(1 + i, ".");
		} else {
			s.insert(s.length() - decimalDigits, ".");
		}
	}

	return s;
}

int EBigDecimal::compareTo(EBigDecimal* val) {
	if (!val) {
		return 1;
	}
	if (*this == *val) {
		return 0;
	}
	return (*this < *val) ? -1 : 1;
}

boolean EBigDecimal::equals(EObject* obj) {
	EBigDecimal *d  = dynamic_cast<EBigDecimal*>(obj);
	if (!d) {
		return false;
	}
	return this->operator ==(*d);
}

int EBigDecimal::intValue() {
	return toBigInteger().intValue();
}

llong EBigDecimal::llongValue() {
	return toBigInteger().llongValue();
}

double EBigDecimal::doubleValue() {
	return this->intVal.doubleValue() / ::pow((double)10, decimalDigits);
}

float EBigDecimal::floatValue() {
	return (float)doubleValue();
}

} /* namespace efc */
