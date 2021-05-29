/*
 * EBigInteger.cpp
 *
 *  Created on: 2017-6-26
 *      Author: cxxjava@163.com
 */

#include "EBigInteger.hh"
#include "EMath.hh"
#include "ESystem.hh"
#include "EInteger.hh"
#include "EFloat.hh"
#include "EDouble.hh"
#include "ECharacter.hh"
#include "ENumberFormatException.hh"
#include "EArithmeticException.hh"

namespace efc {

//@see: http://blueve.me/archives/215

#define RADIX 10

EBigInteger::~EBigInteger() {
}

EBigInteger::EBigInteger(const char* s) {
	num.setThreadSafe(false);
	*this = s;
}

EBigInteger::EBigInteger(llong v) {
	num.setThreadSafe(false);
	*this = v;
}

EBigInteger::EBigInteger(const EBigInteger& that) {
	*this = that;
}

boolean EBigInteger::getSign() {
	return sign;
}

void EBigInteger::setSign(boolean negative) {
	sign = negative;
}

EVector<int>* EBigInteger::getNumber() {
	return &num;
}

EBigInteger EBigInteger::add(llong val) {
	return this->operator +(val);
}

EBigInteger EBigInteger::add(EBigInteger& val) {
	return this->operator +(val);
}

EBigInteger EBigInteger::subtract(llong val) {
	return this->operator -(val);
}

EBigInteger EBigInteger::subtract(EBigInteger& b) {
	return this->operator -(b);
}

EBigInteger EBigInteger::multiply(llong val) {
	return this->operator *(val);
}

EBigInteger EBigInteger::multiply(EBigInteger& b) {
	return this->operator *(b);
}

EBigInteger EBigInteger::divide(llong val) {
	return this->operator /(val);
}

EBigInteger EBigInteger::divide(EBigInteger& b) {
	return this->operator /(b);
}

EBigInteger EBigInteger::remainder(llong val) {
	return this->operator %(val);
}

EBigInteger EBigInteger::remainder(EBigInteger& b) {
	return this->operator %(b);
}

EBigInteger EBigInteger::mod(llong val) {
	EBigInteger b = this->operator %(val);
	b.setSign(false);
	return b;
}

EBigInteger EBigInteger::mod(EBigInteger& b) {
	EBigInteger m = this->operator %(b);
	m.setSign(false);
	return m;
}

EBigInteger EBigInteger::abs() {
	EBigInteger r(*this);
	if (r.sign) {
		r.sign = false;
	}
	return r;
}

void EBigInteger::ltrimZero(EStack<int>& num) {
	//Clear leading zeros
	for (int i = num.size() - 1; num[i] == 0 && i > 0; --i) {
		num.pop();
	}
}

EBigInteger& EBigInteger::operator =(const llong v) {
	//Empty the original number
	num.clear();
	//Get digital symbols
	int tmp = v;
	if (v < 0) {
		sign = true;
		tmp = -tmp;
	} else {
		sign = false;
	}
	//In reverse order high precision integer storage
	//Storing digital bits
	while (tmp >= RADIX) {
		num.push(tmp % RADIX);
		tmp /= RADIX;
	}
	if (tmp != 0)
		num.push(tmp);
	else if (num.isEmpty())
		num.push(0);

	return *this;
}
EBigInteger& EBigInteger::operator =(const char* s) {
	//Empty the original number
	num.clear();

	if (!s || !*s) {
		return *this;
	}

	EString sn(s);
	sn.trim();

	//In reverse order high precision integer storage
	for (int i = sn.length() - 1; i > 0; --i) {
		num.push(sn[i] - '0');
	}
	//Determine numeric symbols
	if (sn[0] == '-') {
		sign = true;
	} else if (sn[0] == '+') {
		sign = false;
	} else {
		sign = false;
		num.push(sn[0] - '0');
	}
	//Clear leading zeros
	ltrimZero (num);
	return *this;
}

EBigInteger& EBigInteger::operator =(const EBigInteger& b) {
	if (this == &b)
		return *this;

	EBigInteger* bi = (EBigInteger*) (&b);
	num.clear();
	//In reverse order high precision integer storage
	for (int i = 0; i < bi->num.size(); ++i) {
		num.push(bi->num[i]);
	}
	//Symbol transfer
	sign = bi->sign;
	return *this;
}

EBigInteger EBigInteger::operator +(llong v) {
	EBigInteger b(v);
	return this->operator +(b);
}

EBigInteger EBigInteger::operator +(EBigInteger& b) {
	EBigInteger r;
	if (b.sign == this->sign) {
		//Adding the same number
		r.sign = b.sign;
	} else if (this->sign) {
		//Left negative, Right positive
		this->sign = false;
		r = b - *this;
		return r;
	} else if (b.sign) {
		//Left positive, Right negative
		b.sign = false;
		r = *this - b;
		return r;
	}

	//Gets the length of the two digits added
	int lenA = num.size();
	int lenB = b.num.size();
	int lng = ES_MAX(lenA, lenB);
	//Bit by bit addition
	for (int i = 0; i < lng; ++i) {
		int tmp = r.num.peek(); //Gets the existing number of the current bit
		//The sum of
		if (i < lenA)
			tmp += num[i];
		if (i < lenB)
			tmp += b.num[i];
		r.num[r.num.size() - 1] = tmp % RADIX; //store
		r.num.push(tmp / RADIX); //carry
	}
	//Clear leading zeros
	ltrimZero(r.num);
	return r;
}

EBigInteger EBigInteger::operator -(llong v) {
	EBigInteger b(v);
	return this->operator -(b);
}

EBigInteger EBigInteger::operator -(EBigInteger& b) {
	EBigInteger r, t1(*this), t2(b);
	EBigInteger *big = &t1, *lit = &t2;
	//Gets the length of the two digits added
	int lenA = num.size();
	int lenB = b.num.size();
	int lng, shrt;
	shrt = ES_MIN(lenA, lenB);
	lng = ES_MAX(lenA, lenB);
	//Same number subtraction
	if (b.sign == this->sign) {
		if (!b.sign) {
			//Same as positive number
			if (t1 < t2) {
				//exchange
				big = &t2;
				lit = &t1;
				r.sign = true;
			} else {
				r.sign = false;
			}
		} else {
			//Same as negative number
			if (t1 >= t2) {
				//exchange
				big = &t2;
				lit = &t1;
				r.sign = false;
			} else {
				r.sign = true;
			}
		}

	} else {
		//Left is negative & Right is positive || Left is positive & Right is negative
		t2.setSign(!t2.getSign());
		return t1 + t2;
	}
	//Bit by bit subtraction
	for (int i = 0; i < lng; ++i) {
		if (i < shrt) {
			//subtract
			if (big->num[i] < lit->num[i]) {
				//Borrow
				for (int j = i + 1;; j++) {
					if (big->num[j] > 0) {
						big->num[j] -= 1;
						break;
					} else {
						big->num[j] = 9;
					}
				}
				r.num[i] = big->num[i] + RADIX - lit->num[i]; //sub
			} else {
				r.num[i] = big->num[i] - lit->num[i]; //sub
			}
		} else {
			r.num[i] = big->num[i];
		}
		r.num.push(0);
	}
	//Clear leading zeros
	ltrimZero(r.num);
	return r;
}

EBigInteger EBigInteger::pow10(int n) {
	EBigInteger r(*this);

	if (n > 0) {
		for (int i=0; i<n; i++) {
			r.num.insertElementAt(0, 0);
		}
	} else if (n < 0) {
		n = ES_MIN(-n, r.num.size());
		for (int i=0; i<n; i++) {
			r.num.removeAt(0);
		}
	}
	return r;
}

EBigInteger EBigInteger::operator -() {
	EBigInteger r(*this);
	if (r != 0)
		r.sign = !r.sign;

	return r;
}

EBigInteger EBigInteger::operator *(llong v) {
	if (v == 0) {
		return EBigInteger(0LL);
	} else if (v == 1) {
		return *this;
	} else if (v == -1) {
		return -*this;
	} else if (v % RADIX == 0) {
		EBigInteger b(*this);
		llong x = v < 0 ? -v : v;
		for (x /= RADIX; x > 0; x /= RADIX) {
			b.num.insertElementAt(0, 0);
		}
		if ((v < 0) && (b != 0LL)) {
			b.sign = !b.sign;
		}
		return b;
	} else {
		EBigInteger b(v);
		return this->operator *(b);
	}
}

EBigInteger EBigInteger::operator *(EBigInteger& b) {
	EBigInteger r;
	//Gets the length of the two digits multiplied
	int lenA = num.size();
	int lenB = b.num.size();
	int len = 0;
	//Bit by bit multiplication
	for (int i = 0; i < lenA; ++i) {
		for (int j = 0; j < lenB; ++j) {
			if (r.num.size() - 1 < i + j) {
				++len;
				r.num.push(num[i] * b.num[j]); //Add one
			} else {
				r.num[i + j] += num[i] * b.num[j];
			}
		}

	}

	//carry
	++len;
	for (int i = 0; i < len; ++i) {
		if (i + 1 < r.num.size()) {
			r.num[i + 1] += r.num[i] / RADIX;
		} else if (r.num[i] >= RADIX) {
			r.num.push(r.num[i] / RADIX);
			++len;
		} else
			break;
		r.num[i] %= RADIX;
	}
	//positive when same sign &  negative when difference sign
	r.sign = sign ^ b.sign;
	//Clear leading zeros
	ltrimZero(r.num);
	return r;
}

EBigInteger EBigInteger::operator /(llong v) {
	EBigInteger b(v);
	return this->operator /(b);
}

EBigInteger EBigInteger::operator /(EBigInteger& b) {
	EBigInteger remainder; //Remainder
	EBigInteger ZERO(0LL), absB(b < ZERO ? -b : b);
	EString s;
	if (b == ZERO)
		return EBigInteger(0LL); //The divisor is 0 and returns 0

	//Two get the length of the digital divide
	int lenA = num.size();
	int lenB = b.num.size();
	int len = 0;
	//To order read dividend
	int n = lenA;
	while (n--) {
		if (remainder == ZERO)
			remainder.num.pop();
		remainder.num.insertElementAt(num[n], 0); //Insert a number
		//Start trial run
		EBigInteger tmp2; //Storing temporary results
		int i = RADIX;
		while (i--) //Try out operations from large to small
		{
			tmp2 = absB * i; //Here we make some judgments about the case where the divisor is negative.
			                 //We must ensure that the symbols are positive in the operation
			tmp2 = remainder - tmp2; //Subtract the test result
			if (tmp2 >= ZERO) { //The first non negative test results
				s += i; //Quotient
				remainder = tmp2;
				break;
			}
		}
	}
	EBigInteger r(s.c_str()); //result, The constructor automatically removes the leading zeros
	//positive when same sign &  negative when difference sign
	r.sign = sign ^ b.sign;
	return r;
}
EBigInteger EBigInteger::operator %(llong v) {
	EBigInteger b(v);
	return this->operator %(b);
}

EBigInteger EBigInteger::operator %(EBigInteger& b) {
	EBigInteger remainder; //Remainder
	EBigInteger ZERO(0LL), absB(b < ZERO ? -b : b);
	if (b == ZERO)
		return EBigInteger(0LL); //The divisor is 0 and returns 0

	//Two get the length of the digital divide
	int lenA = num.size();
	int lenB = b.num.size();
	int len = 0;
	//To order read dividend
	int i = lenA;
	while (i--) {
		if (remainder == ZERO)
			remainder.num.pop();
		remainder.num.insertElementAt(num[i], 0); //Insert a number
		//Start trial run
		EBigInteger tmp2; //Storing temporary results
		int i = RADIX;
		while (i--) //Try out operations from large to small
		{
			tmp2 = absB * i; //Here we make some judgments about the case where the divisor is negative.
                             //We must ensure that the symbols are positive in the operation
			tmp2 = remainder - tmp2; //Subtract the test result
			if (tmp2 >= ZERO) { //The first non negative test results
				remainder = tmp2;
				break;
			}
		}
	}
	//The remainder sign is the same as the dividend symbol
	remainder.sign = this->sign;
	return remainder;
}
EA<EBigInteger*> EBigInteger::divideAndRemainder(EBigInteger& b) {
	EBigInteger remainder; //Remainder
	EBigInteger ZERO(0LL), absB(b < ZERO ? -b : b);
	EString s;
	if (b == ZERO) {
		//The divisor is 0 and returns 0
		EA<EBigInteger*> r(2);
		r[0] = new EBigInteger(0LL);
		r[1] = new EBigInteger(0LL);
		return r;
	}
	//Two get the length of the digital divide
	int lenA = num.size();
	int lenB = b.num.size();
	int len = 0;
	//To order read dividend
	int i = lenA;
	while (i--) {
		if (remainder == ZERO)
			remainder.num.pop();
		remainder.num.insertElementAt(num[i], 0); //Insert a number
		//Start trial run
		EBigInteger tmp2; //Storing temporary results
		int i = RADIX;
		while (i--) //Try out operations from large to small
		{
			tmp2 = absB * i; //Here we make some judgments about the case where the divisor is negative.
                             //We must ensure that the symbols are positive in the operation
			tmp2 = remainder - tmp2; //Subtract the test result
			if (tmp2 >= ZERO) { //The first non negative test results
				s += i; //Quotient
				remainder = tmp2;
				break;
			}
		}
	}
	EBigInteger divisor(s.c_str()); //Result, The constructor automatically removes the leading zeros
	//positive when same sign &  negative when difference sign
	divisor.sign = sign ^ b.sign;
	//The remainder sign is the same as the dividend symbol
	remainder.sign = this->sign;
	EA<EBigInteger*> r(2);
	r[0] = new EBigInteger(divisor);
	r[1] = new EBigInteger(remainder);
	return r;
}

EBigInteger& EBigInteger::operator +=(EBigInteger& b) {
	*this = *this + b;
	return *this;
}

EBigInteger& EBigInteger::operator -=(EBigInteger& b) {
	*this = *this - b;
	return *this;
}

EBigInteger& EBigInteger::operator *=(EBigInteger& b) {
	*this = *this * b;
	return *this;
}

EBigInteger& EBigInteger::operator /=(EBigInteger& b) {
	*this = *this / b;
	return *this;
}

EBigInteger& EBigInteger::operator %=(EBigInteger& b) {
	*this = *this % b;
	return *this;
}

EBigInteger& EBigInteger::operator +=(llong val) {
	*this = *this + val;
	return *this;
}

EBigInteger& EBigInteger::operator -=(llong val) {
	*this = *this - val;
	return *this;
}

EBigInteger& EBigInteger::operator *=(llong val) {
	*this = *this * val;
	return *this;
}

EBigInteger& EBigInteger::operator /=(llong val) {
	*this = *this / val;
	return *this;
}

EBigInteger& EBigInteger::operator %=(llong val) {
	*this = *this % val;
	return *this;
}

boolean EBigInteger::operator <(llong v) {
	if (v == 0) {
		return sign;
	} else {
		EBigInteger b(v);
		return this->operator <(b);
	}
}

boolean EBigInteger::operator <(EBigInteger& b) {
	int bLen = (b.num.size());
	if (!b.sign && sign)
		return true; //Left positive, Right negative
	else if (b.sign && !sign)
		return false; //Left negative, Right positive
	else if (num.size() != bLen) {
		//The length is different, and returns the size directly
		if (!sign)
			return num.size() < bLen;
		else
			return num.size() > bLen;
	}

	//By comparison from the beginning of last
	for (int i = (bLen - 1); i > 0; --i) {
		if (num[i] != b.num[i]) {
			if (!b.sign && !this->sign)
				return num[i] < b.num[i];
			else
				return b.num[i] < num[i];
		}
	}

	if (!b.sign && !sign)
		return num[0] < b.num[0]; //Left positive, Right positive
	else
		return b.num[0] < num[0]; //Left negative, Right negative
}
boolean EBigInteger::operator >(llong v) {
	EBigInteger b(v);
	return this->operator >(b);
}

boolean EBigInteger::operator >(EBigInteger& b) {
	return (b < *this) && !(*this == b);
}

boolean EBigInteger::operator <=(llong v) {
	EBigInteger b(v);
	return this->operator <=(b);
}

boolean EBigInteger::operator <=(EBigInteger& b) {
	return !(*this > b);
}

boolean EBigInteger::operator >=(llong v) {
	EBigInteger b(v);
	return this->operator >=(b);
}

boolean EBigInteger::operator >=(EBigInteger& b) {
	return !(*this < b);
}

boolean EBigInteger::operator !=(llong v) {
	EBigInteger b(v);
	return this->operator !=(b);
}

boolean EBigInteger::operator !=(EBigInteger& b) {
	return !(*this == b);
}

boolean EBigInteger::operator ==(llong v) {
	EBigInteger b(v);
	return this->operator ==(b);
}

boolean EBigInteger::operator ==(EBigInteger& b) {
	if (this->sign != b.sign || this->num.size() != b.num.size()) {
		return false;
	}
	//By comparison from the beginning of last
	for (int i = (b.num.size() - 1); i >= 0; --i) {
		if (this->num[i] != b.num[i]) {
			return false;
		}
	}

	return true;
}

int EBigInteger::hashCode() {
	int hashCode = 0;
	for (int i = 0; i < num.size(); i++) {
		hashCode = (int) ((31 * hashCode + num[i]));
	}

	return hashCode * (sign ? -1 : 1);
}

EString EBigInteger::toString() {
	EString out;
	if (sign) {
		out << '-';
	}
	for (int i = num.size() - 1; i >= 0; --i) {
		out << num[i];
	}
	return out;
}

int EBigInteger::compareTo(EBigInteger* val) {
	if (!val) {
		return 1;
	}
	if (*this == *val) {
		return 0;
	}
	return (*this < *val) ? -1 : 1;
}

boolean EBigInteger::equals(EObject* obj) {
	EBigInteger *b  = dynamic_cast<EBigInteger*>(obj);
	if (!b) {
		return false;
	}
	return this->operator ==(*b);
}

int EBigInteger::intValue() {
	return EInteger::parseInt(this->toString().c_str());
}

llong EBigInteger::llongValue() {
	return ELLong::parseLLong(this->toString().c_str());
}

float EBigInteger::floatValue() {
	return EFloat::parseFloat(this->toString().c_str());
}

double EBigInteger::doubleValue() {
	return EDouble::parseDouble(this->toString().c_str());
}

} /* namespace efc */
