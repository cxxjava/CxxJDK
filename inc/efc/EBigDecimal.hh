/*
 * EBigDecimal.hh
 *
 *  Created on: 2017-6-28
 *      Author: cxxjava@163.com
 */

#ifndef EBIGDECIMAL_HH_
#define EBIGDECIMAL_HH_

#include "ENumber.hh"
#include "EComparable.hh"
#include "EBigInteger.hh"

namespace efc {

/**
 * Immutable, arbitrary-precision signed decimal numbers.  A
 * {@code BigDecimal} consists of an arbitrary precision integer
 * <i>unscaled value</i> and a 32-bit integer <i>scale</i>.  If zero
 * or positive, the scale is the number of digits to the right of the
 * decimal point.  If negative, the unscaled value of the number is
 * multiplied by ten to the power of the negation of the scale.  The
 * value of the number represented by the {@code BigDecimal} is
 * therefore <tt>(unscaledValue &times; 10<sup>-scale</sup>)</tt>.
 *
 * <p>The {@code BigDecimal} class provides operations for
 * arithmetic, scale manipulation, rounding, comparison, hashing, and
 * format conversion.  The {@link #toString} method provides a
 * canonical representation of a {@code BigDecimal}.
 *
 * <p>The {@code BigDecimal} class gives its user complete control
 * over rounding behavior.  If no rounding mode is specified and the
 * exact result cannot be represented, an exception is thrown;
 * otherwise, calculations can be carried out to a chosen precision
 * and rounding mode by supplying an appropriate {@link MathContext}
 * object to the operation.  In either case, eight <em>rounding
 * modes</em> are provided for the control of rounding.  Using the
 * integer fields in this class (such as {@link #ROUND_HALF_UP}) to
 * represent rounding mode is largely obsolete; the enumeration values
 * of the {@code RoundingMode} {@code enum}, (such as {@link
 * RoundingMode#HALF_UP}) should be used instead.
 *
 * <p>When a {@code MathContext} object is supplied with a precision
 * setting of 0 (for example, {@link MathContext#UNLIMITED}),
 * arithmetic operations are exact, as are the arithmetic methods
 * which take no {@code MathContext} object.  (This is the only
 * behavior that was supported in releases prior to 5.)  As a
 * corollary of computing the exact result, the rounding mode setting
 * of a {@code MathContext} object with a precision setting of 0 is
 * not used and thus irrelevant.  In the case of divide, the exact
 * quotient could have an infinitely long decimal expansion; for
 * example, 1 divided by 3.  If the quotient has a nonterminating
 * decimal expansion and the operation is specified to return an exact
 * result, an {@code ArithmeticException} is thrown.  Otherwise, the
 * exact result of the division is returned, as done for other
 * operations.
 *
 * <p>When the precision setting is not 0, the rules of
 * {@code BigDecimal} arithmetic are broadly compatible with selected
 * modes of operation of the arithmetic defined in ANSI X3.274-1996
 * and ANSI X3.274-1996/AM 1-2000 (section 7.4).  Unlike those
 * standards, {@code BigDecimal} includes many rounding modes, which
 * were mandatory for division in {@code BigDecimal} releases prior
 * to 5.  Any conflicts between these ANSI standards and the
 * {@code BigDecimal} specification are resolved in favor of
 * {@code BigDecimal}.
 *
 * <p>Since the same numerical value can have different
 * representations (with different scales), the rules of arithmetic
 * and rounding must specify both the numerical result and the scale
 * used in the result's representation.
 *
 *
 * <p>In general the rounding modes and precision setting determine
 * how operations return results with a limited number of digits when
 * the exact result has more digits (perhaps infinitely many in the
 * case of division) than the number of digits returned.
 *
 * First, the
 * total number of digits to return is specified by the
 * {@code MathContext}'s {@code precision} setting; this determines
 * the result's <i>precision</i>.  The digit count starts from the
 * leftmost nonzero digit of the exact result.  The rounding mode
 * determines how any discarded trailing digits affect the returned
 * result.
 *
 * <p>For all arithmetic operators , the operation is carried out as
 * though an exact intermediate result were first calculated and then
 * rounded to the number of digits specified by the precision setting
 * (if necessary), using the selected rounding mode.  If the exact
 * result is not returned, some digit positions of the exact result
 * are discarded.  When rounding increases the magnitude of the
 * returned result, it is possible for a new digit position to be
 * created by a carry propagating to a leading {@literal "9"} digit.
 * For example, rounding the value 999.9 to three digits rounding up
 * would be numerically equal to one thousand, represented as
 * 100&times;10<sup>1</sup>.  In such cases, the new {@literal "1"} is
 * the leading digit position of the returned result.
 *
 * <p>Besides a logical exact result, each arithmetic operation has a
 * preferred scale for representing a result.  The preferred
 * scale for each operation is listed in the table below.
 *
 * <table border>
 * <caption><b>Preferred Scales for Results of Arithmetic Operations
 * </b></caption>
 * <tr><th>Operation</th><th>Preferred Scale of Result</th></tr>
 * <tr><td>Add</td><td>max(addend.scale(), augend.scale())</td>
 * <tr><td>Subtract</td><td>max(minuend.scale(), subtrahend.scale())</td>
 * <tr><td>Multiply</td><td>multiplier.scale() + multiplicand.scale()</td>
 * <tr><td>Divide</td><td>dividend.scale() - divisor.scale()</td>
 * </table>
 *
 * These scales are the ones used by the methods which return exact
 * arithmetic results; except that an exact divide may have to use a
 * larger scale since the exact result may have more digits.  For
 * example, {@code 1/32} is {@code 0.03125}.
 *
 * <p>Before rounding, the scale of the logical exact intermediate
 * result is the preferred scale for that operation.  If the exact
 * numerical result cannot be represented in {@code precision}
 * digits, rounding selects the set of digits to return and the scale
 * of the result is reduced from the scale of the intermediate result
 * to the least scale which can represent the {@code precision}
 * digits actually returned.  If the exact result can be represented
 * with at most {@code precision} digits, the representation
 * of the result with the scale closest to the preferred scale is
 * returned.  In particular, an exactly representable quotient may be
 * represented in fewer than {@code precision} digits by removing
 * trailing zeros and decreasing the scale.  For example, rounding to
 * three digits using the {@linkplain RoundingMode#FLOOR floor}
 * rounding mode, <br>
 *
 * {@code 19/100 = 0.19   // integer=19,  scale=2} <br>
 *
 * but<br>
 *
 * {@code 21/110 = 0.190  // integer=190, scale=3} <br>
 *
 * <p>Note that for add, subtract, and multiply, the reduction in
 * scale will equal the number of digit positions of the exact result
 * which are discarded. If the rounding causes a carry propagation to
 * create a new high-order digit position, an additional digit of the
 * result is discarded than when no new digit position is created.
 *
 * <p>Other methods may have slightly different rounding semantics.
 * For example, the result of the {@code pow} method using the
 * {@linkplain #pow(int, MathContext) specified algorithm} can
 * occasionally differ from the rounded mathematical result by more
 * than one unit in the last place, one <i>{@linkplain #ulp() ulp}</i>.
 *
 * <p>Two types of operations are provided for manipulating the scale
 * of a {@code BigDecimal}: scaling/rounding operations and decimal
 * point motion operations.  Scaling/rounding operations ({@link
 * #setScale setScale} and {@link #round round}) return a
 * {@code BigDecimal} whose value is approximately (or exactly) equal
 * to that of the operand, but whose scale or precision is the
 * specified value; that is, they increase or decrease the precision
 * of the stored number with minimal effect on its value.  Decimal
 * point motion operations ({@link #movePointLeft movePointLeft} and
 * {@link #movePointRight movePointRight}) return a
 * {@code BigDecimal} created from the operand by moving the decimal
 * point a specified distance in the specified direction.
 *
 * <p>For the sake of brevity and clarity, pseudo-code is used
 * throughout the descriptions of {@code BigDecimal} methods.  The
 * pseudo-code expression {@code (i + j)} is shorthand for "a
 * {@code BigDecimal} whose value is that of the {@code BigDecimal}
 * {@code i} added to that of the {@code BigDecimal}
 * {@code j}." The pseudo-code expression {@code (i == j)} is
 * shorthand for "{@code true} if and only if the
 * {@code BigDecimal} {@code i} represents the same value as the
 * {@code BigDecimal} {@code j}." Other pseudo-code expressions
 * are interpreted similarly.  Square brackets are used to represent
 * the particular {@code BigInteger} and scale pair defining a
 * {@code BigDecimal} value; for example [19, 2] is the
 * {@code BigDecimal} numerically equal to 0.19 having a scale of 2.
 *
 * <p>Note: care should be exercised if {@code BigDecimal} objects
 * are used as keys in a {@link java.util.SortedMap SortedMap} or
 * elements in a {@link java.util.SortedSet SortedSet} since
 * {@code BigDecimal}'s <i>natural ordering</i> is <i>inconsistent
 * with equals</i>.  See {@link Comparable}, {@link
 * java.util.SortedMap} or {@link java.util.SortedSet} for more
 * information.
 *
 * <p>All methods and constructors for this class throw
 * {@code NullPointerException} when passed a {@code null} object
 * reference for any input parameter.
 *
 * @see     BigInteger
 * @see     MathContext
 * @see     RoundingMode
 * @see     java.util.SortedMap
 * @see     java.util.SortedSet
 */

class EBigDecimal: public ENumber, virtual public EComparable<EBigDecimal*> {
public:
	virtual ~EBigDecimal();

	// Constructors

	EBigDecimal(llong val = 0);
	EBigDecimal(const char *val);
	EBigDecimal(const EBigInteger& val);
	EBigDecimal(const EBigDecimal& that);

	// Modular Arithmetic Operations

	EBigDecimal operator -();

	EBigDecimal& operator = (const llong val);
	EBigDecimal& operator = (const char* val);
	EBigDecimal& operator = (const EBigInteger& b);
	EBigDecimal& operator = (const EBigDecimal& d);
	boolean operator == (llong val);
	boolean operator == (EBigInteger& b);
	boolean operator == (EBigDecimal& d);
	boolean operator != (llong val);
	boolean operator != (EBigInteger& b);
	boolean operator != (EBigDecimal& d);
	boolean operator > (llong val);
	boolean operator > (EBigInteger& b);
	boolean operator > (EBigDecimal& d);
	boolean operator < (llong val);
	boolean operator < (EBigInteger& b);
	boolean operator < (EBigDecimal& d);
	boolean operator >= (llong val);
	boolean operator >= (EBigInteger& b);
	boolean operator >= (EBigDecimal& d);
	boolean operator <= (llong val);
	boolean operator <= (EBigInteger& b);
	boolean operator <= (EBigDecimal& d);
	EBigDecimal operator + (llong val);
	EBigDecimal operator + (EBigInteger& b);
	EBigDecimal operator + (EBigDecimal& d);
	EBigDecimal add(llong val);
	EBigDecimal add(EBigInteger& val);
	EBigDecimal add(EBigDecimal& val);
	EBigDecimal operator - (llong val);
	EBigDecimal operator - (EBigInteger& b);
	EBigDecimal operator - (EBigDecimal& d);
	EBigDecimal subtract(llong val);
	EBigDecimal subtract(EBigInteger& b);
	EBigDecimal subtract(EBigDecimal& d);
	EBigDecimal operator * (llong val);
	EBigDecimal operator * (EBigInteger& b);
	EBigDecimal operator * (EBigDecimal& d);
	EBigDecimal multiply(llong val);
	EBigDecimal multiply(EBigInteger& b);
	EBigDecimal multiply(EBigDecimal& d);
	EBigDecimal operator / (llong val);
	EBigDecimal operator / (EBigInteger& b);
	EBigDecimal operator / (EBigDecimal& d);
	EBigDecimal divide(llong val);
	EBigDecimal divide(EBigInteger& b);
	EBigDecimal divide(EBigDecimal& d);
	EBigDecimal operator % (llong val);
	EBigDecimal operator % (EBigInteger& b);
	EBigDecimal operator % (EBigDecimal& d);
	EBigDecimal remainder(llong val);
	EBigDecimal remainder(EBigInteger& b);
	EBigDecimal remainder(EBigDecimal& d);
	EBigDecimal& operator += (llong val);
	EBigDecimal& operator += (EBigInteger& b);
	EBigDecimal& operator += (EBigDecimal& d);
	EBigDecimal& operator -= (llong val);
	EBigDecimal& operator -= (EBigInteger& b);
	EBigDecimal& operator -= (EBigDecimal& d);
	EBigDecimal& operator *= (llong val);
	EBigDecimal& operator *= (EBigInteger& b);
	EBigDecimal& operator *= (EBigDecimal& d);
	EBigDecimal& operator /= (llong val);
	EBigDecimal& operator /= (EBigInteger& b);
	EBigDecimal& operator /= (EBigDecimal& d);
	EBigDecimal& operator %= (llong val);
	EBigDecimal& operator %= (EBigInteger& b);
	EBigDecimal& operator %= (EBigDecimal& d);

	/**
	 * Returns a {@code BigDecimal} whose value is {@code (this /
	 * divisor)}, and whose scale is as specified.  If rounding must
	 * be performed to generate a result with the specified scale, the
	 * specified rounding mode is applied.
	 *
	 * <p>The new {@link #divide(BigDecimal, int, RoundingMode)} method
	 * should be used in preference to this legacy method.
	 *
	 * @param  divisor value by which this {@code BigDecimal} is to be divided.
	 * @param  scale scale of the {@code BigDecimal} quotient to be returned.
	 * @param  roundingMode rounding mode to apply.
	 * @return {@code this / divisor}
	 * @throws ArithmeticException if {@code divisor} is zero,
	 *         {@code roundingMode==ROUND_UNNECESSARY} and
	 *         the specified scale is insufficient to represent the result
	 *         of the division exactly.
	 * @throws IllegalArgumentException if {@code roundingMode} does not
	 *         represent a valid rounding mode.
	 * @see    #ROUND_UP
	 * @see    #ROUND_DOWN
	 * @see    #ROUND_CEILING
	 * @see    #ROUND_FLOOR
	 * @see    #ROUND_HALF_UP
	 * @see    #ROUND_HALF_DOWN
	 * @see    #ROUND_HALF_EVEN
	 * @see    #ROUND_UNNECESSARY
	 */
	EBigDecimal divide(llong val, int scale, int roundingMode=ROUND_DOWN);
	EBigDecimal divide(EBigInteger& b, int scale, int roundingMode=ROUND_DOWN);
	EBigDecimal divide(EBigDecimal& d, int scale, int roundingMode=ROUND_DOWN);

	/**
	 * Returns a two-element {@code BigDecimal} array containing the
	 * result of {@code divideToIntegralValue} followed by the result of
	 * {@code remainder} on the two operands.
	 *
	 * <p>Note that if both the integer quotient and remainder are
	 * needed, this method is faster than using the
	 * {@code divideToIntegralValue} and {@code remainder} methods
	 * separately because the division need only be carried out once.
	 *
	 * @param  divisor value by which this {@code BigDecimal} is to be divided,
	 *         and the remainder computed.
	 * @return a two element {@code BigDecimal} array: the quotient
	 *         (the result of {@code divideToIntegralValue}) is the initial element
	 *         and the remainder is the final element.
	 * @throws ArithmeticException if {@code divisor==0}
	 * @see    #divideToIntegralValue(java.math.BigDecimal, java.math.MathContext)
	 * @see    #remainder(java.math.BigDecimal, java.math.MathContext)
	 * @since  1.5
	 */
	EA<EBigDecimal*> divideAndRemainder(EBigDecimal& d);

	/**
	 * Returns a {@code BigDecimal} whose value is the absolute value
	 * of this {@code BigDecimal}, and whose scale is
	 * {@code this.scale()}.
	 *
	 * @return {@code abs(this)}
	 */
	EBigDecimal abs();

	/**
	 * Returns the <i>scale</i> of this {@code BigDecimal}.  If zero
	 * or positive, the scale is the number of digits to the right of
	 * the decimal point.  If negative, the unscaled value of the
	 * number is multiplied by ten to the power of the negation of the
	 * scale.  For example, a scale of {@code -3} means the unscaled
	 * value is multiplied by 1000.
	 *
	 * @return the scale of this {@code BigDecimal}.
	 */
	int scale();

	/**
	 * Converts this {@code BigDecimal} to a {@code BigInteger}.
	 * This conversion is analogous to the
	 * <i>narrowing primitive conversion</i> from {@code double} to
	 * {@code long} as defined in section 5.1.3 of
	 * <cite>The Java&trade; Language Specification</cite>:
	 * any fractional part of this
	 * {@code BigDecimal} will be discarded.  Note that this
	 * conversion can lose information about the precision of the
	 * {@code BigDecimal} value.
	 * <p>
	 * To have an exception thrown if the conversion is inexact (in
	 * other words if a nonzero fractional part is discarded), use the
	 * {@link #toBigIntegerExact()} method.
	 *
	 * @return this {@code BigDecimal} converted to a {@code BigInteger}.
	 */
	EBigInteger toBigInteger();

	virtual int hashCode();
	virtual EStringBase toString();
	virtual int compareTo(EBigDecimal* val);
	virtual boolean equals(EObject* obj);

	virtual int intValue();
	virtual llong llongValue();
	virtual float floatValue();
	virtual double doubleValue();

public:
	// Rounding Modes

	/**
	 * Rounding mode to round towards zero.  Never increments the digit
	 * prior to a discarded fraction (i.e., truncates).  Note that this
	 * rounding mode never increases the magnitude of the calculated value.
	 */
	static const int ROUND_DOWN =         0;

	/**
	 * Rounding mode to round away from zero.  Always increments the
	 * digit prior to a nonzero discarded fraction.  Note that this rounding
	 * mode never decreases the magnitude of the calculated value.
	 */
	static const int ROUND_UP =           1;

	/**
	 * Rounding mode to round towards positive infinity.  If the
	 * {@code BigDecimal} is positive, behaves as for
	 * {@code ROUND_UP}; if negative, behaves as for
	 * {@code ROUND_DOWN}.  Note that this rounding mode never
	 * decreases the calculated value.
	 */
	static const int ROUND_CEILING =      2;

	/**
	 * Rounding mode to round towards negative infinity.  If the
	 * {@code BigDecimal} is positive, behave as for
	 * {@code ROUND_DOWN}; if negative, behave as for
	 * {@code ROUND_UP}.  Note that this rounding mode never
	 * increases the calculated value.
	 */
	static const int ROUND_FLOOR =        3;

	/**
	 * Rounding mode to round towards {@literal "nearest neighbor"}
	 * unless both neighbors are equidistant, in which case round up.
	 * Behaves as for {@code ROUND_UP} if the discarded fraction is
	 * &ge; 0.5; otherwise, behaves as for {@code ROUND_DOWN}.  Note
	 * that this is the rounding mode that most of us were taught in
	 * grade school.
	 */
	static const int ROUND_HALF_UP =      4;

	/**
	 * Rounding mode to round towards {@literal "nearest neighbor"}
	 * unless both neighbors are equidistant, in which case round
	 * down.  Behaves as for {@code ROUND_UP} if the discarded
	 * fraction is {@literal >} 0.5; otherwise, behaves as for
	 * {@code ROUND_DOWN}.
	 */
	static const int ROUND_HALF_DOWN =    5;

	/**
	 * Rounding mode to round towards the {@literal "nearest neighbor"}
	 * unless both neighbors are equidistant, in which case, round
	 * towards the even neighbor.  Behaves as for
	 * {@code ROUND_HALF_UP} if the digit to the left of the
	 * discarded fraction is odd; behaves as for
	 * {@code ROUND_HALF_DOWN} if it's even.  Note that this is the
	 * rounding mode that minimizes cumulative error when applied
	 * repeatedly over a sequence of calculations.
	 */
	static const int ROUND_HALF_EVEN =    6;

	/**
	 * Rounding mode to assert that the requested operation has an exact
	 * result, hence no rounding is necessary.  If this rounding mode is
	 * specified on an operation that yields an inexact result, an
	 * {@code ArithmeticException} is thrown.
	 */
	static const int ROUND_UNNECESSARY =  7;

private:
	EBigInteger intVal;
	int decimalDigits;
};

} /* namespace efc */
#endif /* EBIGDECIMAL_HH_ */
