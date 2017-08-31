/*
 * EBigInteger.hh
 *
 *  Created on: 2017-6-26
 *      Author: cxxjava@163.com
 */

#ifndef EBIGINTEGER_HH_
#define EBIGINTEGER_HH_

#include "ENumber.hh"
#include "EComparable.hh"
#include "EStack.hh"

namespace efc {

/**
 * Immutable arbitrary-precision integers.  All operations behave as if
 * BigIntegers were represented in two's-complement notation (like Java's
 * primitive integer types).  BigInteger provides analogues to all of Java's
 * primitive integer operators, and all relevant methods from java.lang.Math.
 * Additionally, BigInteger provides operations for modular arithmetic, GCD
 * calculation, primality testing, prime generation, bit manipulation,
 * and a few other miscellaneous operations.
 *
 * <p>Semantics of arithmetic operations exactly mimic those of Java's integer
 * arithmetic operators, as defined in <i>The Java Language Specification</i>.
 * For example, division by zero throws an {@code ArithmeticException}, and
 * division of a negative by a positive yields a negative (or zero) remainder.
 * All of the details in the Spec concerning overflow are ignored, as
 * BigIntegers are made as large as necessary to accommodate the results of an
 * operation.
 *
 * <p>Semantics of shift operations extend those of Java's shift operators
 * to allow for negative shift distances.  A right-shift with a negative
 * shift distance results in a left shift, and vice-versa.  The unsigned
 * right shift operator ({@code >>>}) is omitted, as this operation makes
 * little sense in combination with the "infinite word size" abstraction
 * provided by this class.
 *
 * <p>Semantics of bitwise logical operations exactly mimic those of Java's
 * bitwise integer operators.  The binary operators ({@code and},
 * {@code or}, {@code xor}) implicitly perform sign extension on the shorter
 * of the two operands prior to performing the operation.
 *
 * <p>Comparison operations perform signed integer comparisons, analogous to
 * those performed by Java's relational and equality operators.
 *
 * <p>Modular arithmetic operations are provided to compute residues, perform
 * exponentiation, and compute multiplicative inverses.  These methods always
 * return a non-negative result, between {@code 0} and {@code (modulus - 1)},
 * inclusive.
 *
 * <p>Bit operations operate on a single bit of the two's-complement
 * representation of their operand.  If necessary, the operand is sign-
 * extended so that it contains the designated bit.  None of the single-bit
 * operations can produce a BigInteger with a different sign from the
 * BigInteger being operated on, as they affect only a single bit, and the
 * "infinite word size" abstraction provided by this class ensures that there
 * are infinitely many "virtual sign bits" preceding each BigInteger.
 *
 * <p>For the sake of brevity and clarity, pseudo-code is used throughout the
 * descriptions of BigInteger methods.  The pseudo-code expression
 * {@code (i + j)} is shorthand for "a BigInteger whose value is
 * that of the BigInteger {@code i} plus that of the BigInteger {@code j}."
 * The pseudo-code expression {@code (i == j)} is shorthand for
 * "{@code true} if and only if the BigInteger {@code i} represents the same
 * value as the BigInteger {@code j}."  Other pseudo-code expressions are
 * interpreted similarly.
 *
 * <p>All methods and constructors in this class throw
 * {@code NullPointerException} when passed
 * a null object reference for any input parameter.
 *
 * BigInteger must support values in the range
 * -2<sup>{@code Integer.MAX_VALUE}</sup> (exclusive) to
 * +2<sup>{@code Integer.MAX_VALUE}</sup> (exclusive)
 * and may support values outside of that range.
 *
 * The range of probable prime values is limited and may be less than
 * the full supported positive range of {@code BigInteger}.
 * The range must be at least 1 to 2<sup>500000000</sup>.
 *
 * @implNote
 * BigInteger constructors and operations throw {@code ArithmeticException} when
 * the result is out of the supported range of
 * -2<sup>{@code Integer.MAX_VALUE}</sup> (exclusive) to
 * +2<sup>{@code Integer.MAX_VALUE}</sup> (exclusive).
 *
 * @see     BigDecimal
 * @since JDK1.1
 */

class EBigInteger: public ENumber, virtual public EComparable<EBigInteger*> {
public:
	virtual ~EBigInteger();

	// Constructors
	EBigInteger(llong val = 0);
	EBigInteger(const char* val);
	EBigInteger(const EBigInteger& that);

	/**
	 *
	 */
	boolean getSign();
	void setSign(boolean negative);

	/**
	 *
	 */
	EVector<int>* getNumber();

	// Modular Arithmetic Operations

	EBigInteger pow10(int n);
	EBigInteger operator -();

	EBigInteger& operator = (const llong val);
	EBigInteger& operator = (const char* val);
	EBigInteger& operator = (const EBigInteger& b);
	boolean operator == (llong val);
	boolean operator == (EBigInteger& b);
	boolean operator != (llong val);
	boolean operator != (EBigInteger& b);
	boolean operator > (llong val);
	boolean operator > (EBigInteger& b);
	boolean operator < (llong val);
	boolean operator < (EBigInteger& b);
	boolean operator >= (llong val);
	boolean operator >= (EBigInteger& b);
	boolean operator <= (llong val);
	boolean operator <= (EBigInteger& b);
	EBigInteger operator + (llong val);
	EBigInteger operator + (EBigInteger& b);
	EBigInteger add(llong val);
	EBigInteger add(EBigInteger& val);
	EBigInteger operator - (llong val);
	EBigInteger operator - (EBigInteger& b);
	EBigInteger subtract(llong val);
	EBigInteger subtract(EBigInteger& b);
	EBigInteger operator * (llong val);
	EBigInteger operator * (EBigInteger& b);
	EBigInteger multiply(llong val);
	EBigInteger multiply(EBigInteger& b);
	EBigInteger operator / (llong val);
	EBigInteger operator / (EBigInteger& b);
	EBigInteger divide(llong val);
	EBigInteger divide(EBigInteger& b);
	EBigInteger operator % (llong val);
	EBigInteger operator % (EBigInteger& b);
	EBigInteger remainder(llong val);
	EBigInteger remainder(EBigInteger& b);
	EBigInteger mod(llong val);
	EBigInteger mod(EBigInteger& b);
	EBigInteger& operator += (llong val);
	EBigInteger& operator += (EBigInteger& b);
	EBigInteger& operator -= (llong val);
	EBigInteger& operator -= (EBigInteger& b);
	EBigInteger& operator *= (llong val);
	EBigInteger& operator *= (EBigInteger& b);
	EBigInteger& operator /= (llong val);
	EBigInteger& operator /= (EBigInteger& b);
	EBigInteger& operator %= (llong val);
	EBigInteger& operator %= (EBigInteger& b);

	/**
	 * Returns an array of two BigIntegers containing {@code (this / val)}
	 * followed by {@code (this % val)}.
	 *
	 * @param  val value by which this BigInteger is to be divided, and the
	 *         remainder computed.
	 * @return an array of two BigIntegers: the quotient {@code (this / val)}
	 *         is the initial element, and the remainder {@code (this % val)}
	 *         is the final element.
	 * @throws ArithmeticException if {@code val} is zero.
	 */
	EA<EBigInteger*> divideAndRemainder(EBigInteger& b);

	/**
	 * Returns a BigInteger whose value is the absolute value of this
	 * BigInteger.
	 *
	 * @return {@code abs(this)}
	 */
	EBigInteger abs();

	virtual int hashCode();
	virtual EStringBase toString();
	virtual int compareTo(EBigInteger* val);
	virtual boolean equals(EObject* obj);

	virtual int intValue();
	virtual llong llongValue();
	virtual float floatValue();
	virtual double doubleValue();

private:
	boolean sign;		//true - negative | false - positive
	EStack<int> num;	//reverse integer array

	void ltrimZero(EStack<int>& num);
};

} /* namespace efc */
#endif /* EBIGINTEGER_HH_ */
