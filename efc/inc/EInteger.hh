#ifndef __EInteger_H__
#define __EInteger_H__

#include "ENumber.hh"
#include "EString.hh"
#include "EComparable.hh"
#include "ENumberFormatException.hh"

namespace efc {

/**
 * The {@code Integer} class wraps a value of the primitive type
 * {@code int} in an object. An object of type {@code Integer}
 * contains a single field whose type is {@code int}.
 *
 * <p>In addition, this class provides several methods for converting
 * an {@code int} to a {@code String} and a {@code String} to an
 * {@code int}, as well as other constants and methods useful when
 * dealing with an {@code int}.
 *
 * <p>Implementation note: The implementations of the "bit twiddling"
 * methods (such as {@link #highestOneBit(int) highestOneBit} and
 * {@link #numberOfTrailingZeros(int) numberOfTrailingZeros}) are
 * based on material from Henry S. Warren, Jr.'s <i>Hacker's
 * Delight</i>, (Addison Wesley, 2002).
 *
 * @since JDK1.0
 */

class EInteger : public ENumber, virtual public EComparable<EInteger*>
{
public:
	/**
	 * The smallest value of type <code>int</code>. The constant
	 * value of this field is <tt>-2147483648</tt>.
	 */
	static const int MIN_VALUE = 0x80000000;

	/**
	 * The largest value of type <code>int</code>. The constant
	 * value of this field is <tt>2147483647</tt>.
	 */
	static const int MAX_VALUE = 0x7fffffff;

	/**
	 * The number of bits used to represent an {@code int} value in two's
	 * complement binary form.
	 *
	 * @since 1.5
	 */
	static const int SIZE = 32;

public:
	virtual ~EInteger(){}

	EInteger(int value);

public:
    static EString toString(int i, int radix=10);
    static EString toHexString(int i);
    static EString toOctalString(int i);
    static EString toBinaryString(int i);
    
    /**
	 * Returns a string representation of the first argument as an
	 * unsigned integer value in the radix specified by the second
	 * argument.
	 *
	 * <p>If the radix is smaller than {@code Character.MIN_RADIX}
	 * or larger than {@code Character.MAX_RADIX}, then the radix
	 * {@code 10} is used instead.
	 *
	 * <p>Note that since the first argument is treated as an unsigned
	 * value, no leading sign character is printed.
	 *
	 * <p>If the magnitude is zero, it is represented by a single zero
	 * character {@code '0'} ({@code '\u005Cu0030'}); otherwise,
	 * the first character of the representation of the magnitude will
	 * not be the zero character.
	 *
	 * <p>The behavior of radixes and the characters used as digits
	 * are the same as {@link #toString(int, int) toString}.
	 *
	 * @param   i       an integer to be converted to an unsigned string.
	 * @param   radix   the radix to use in the string representation.
	 * @return  an unsigned string representation of the argument in the specified radix.
	 * @see     #toString(int, int)
	 * @since 1.8
	 */
    static EString toUnsignedString(int i, int radix);

    /**
     * Parses the string argument as a signed integer in the radix
     * specified by the second argument. The characters in the string
     * must all be digits of the specified radix (as determined by
     * whether {@link java.lang.Character#digit(char, int)} returns a
     * nonnegative value), except that the first character may be an
     * ASCII minus sign <code>'-'</code> (<code>'&#92;u002d'</code>) to
     * indicate a negative value. The resulting integer value is returned.
     * <p>
     * An exception of type <tt>NumberFormatException</tt> is thrown if any
     * of the following situations occurs:
     * <ul>
     * <li>The first argument is <tt>null</tt> or is a string of length zero.
     * <li>The radix is either smaller than
     * {@link java.lang.Character#MIN_RADIX} or
     * larger than {@link java.lang.Character#MAX_RADIX}.
     * <li>Any character of the string is not a digit of the specified radix,
     * except that the first character may be a minus sign <tt>'-'</tt>
     * (<tt>'&#92;u002d'</tt>) provided that the string is longer than length 1.
     * <li>The integer value represented by the string is not a value of type
     * <tt>int</tt>.
     * </ul><p>
     * Examples:
     * <blockquote><pre>
     * parseInt("0", 10) returns 0
     * parseInt("473", 10) returns 473
     * parseInt("-0", 10) returns 0
     * parseInt("-FF", 16) returns -255
     * parseInt("1100110", 2) returns 102
     * parseInt("2147483647", 10) returns 2147483647
     * parseInt("-2147483648", 10) returns -2147483648
     * parseInt("2147483648", 10) throws a NumberFormatException
     * parseInt("99", 8) throws a NumberFormatException
     * parseInt("Kona", 10) throws a NumberFormatException
     * parseInt("Kona", 27) returns 411787
     * </pre></blockquote>
     *
     * @param      s   the <code>String</code> containing the integer.
     * @param      radix   the radix to be used.
     * @return     the integer represented by the string argument in the
     *             specified radix.
     * @exception  NumberFormatException  if the string does not contain a
     *               parsable integer.

     */
    static int parseInt(const char* s, int radix=10) THROWS(ENumberFormatException);
	static int toDigit(char ch, int radix);
    
	static EInteger valueOf(int i);
    static EInteger valueOf(const char* s, int radix=10);
    
    /**
	 * Decodes a {@code String} into an {@code Integer}.
	 * Accepts decimal, hexadecimal, and octal numbers given
	 * by the following grammar:
	 *
	 * <blockquote>
	 * <dl>
	 * <dt><i>DecodableString:</i>
	 * <dd><i>Sign<sub>opt</sub> DecimalNumeral</i>
	 * <dd><i>Sign<sub>opt</sub></i> {@code 0x} <i>HexDigits</i>
	 * <dd><i>Sign<sub>opt</sub></i> {@code 0X} <i>HexDigits</i>
	 * <dd><i>Sign<sub>opt</sub></i> {@code #} <i>HexDigits</i>
	 * <dd><i>Sign<sub>opt</sub></i> {@code 0} <i>OctalDigits</i>
	 *
	 * <dt><i>Sign:</i>
	 * <dd>{@code -}
	 * <dd>{@code +}
	 * </dl>
	 * </blockquote>
	 *
	 * <i>DecimalNumeral</i>, <i>HexDigits</i>, and <i>OctalDigits</i>
	 * are as defined in section 3.10.1 of
	 * <cite>The Java&trade; Language Specification</cite>,
	 * except that underscores are not accepted between digits.
	 *
	 * <p>The sequence of characters following an optional
	 * sign and/or radix specifier ("{@code 0x}", "{@code 0X}",
	 * "{@code #}", or leading zero) is parsed as by the {@code
	 * Integer.parseInt} method with the indicated radix (10, 16, or
	 * 8).  This sequence of characters must represent a positive
	 * value or a {@link NumberFormatException} will be thrown.  The
	 * result is negated if first character of the specified {@code
	 * String} is the minus sign.  No whitespace characters are
	 * permitted in the {@code String}.
	 *
	 * @param     nm the {@code String} to decode.
	 * @return    an {@code Integer} object holding the {@code int}
	 *             value represented by {@code nm}
	 * @exception NumberFormatException  if the {@code String} does not
	 *            contain a parsable integer.
	 * @see java.lang.Integer#parseInt(java.lang.String, int)
	 */
    static EInteger decode(const char* nm) THROWS(ENumberFormatException);

    /**
	 * Compares two {@code int} values numerically.
	 * The value returned is identical to what would be returned by:
	 * <pre>
	 *    Integer.valueOf(x).compareTo(Integer.valueOf(y))
	 * </pre>
	 *
	 * @param  x the first {@code int} to compare
	 * @param  y the second {@code int} to compare
	 * @return the value {@code 0} if {@code x == y};
	 *         a value less than {@code 0} if {@code x < y}; and
	 *         a value greater than {@code 0} if {@code x > y}
	 * @since 1.7
	 */
	static int compare(int x, int y);

	/**
	 * Compares two {@code int} values numerically treating the values
	 * as unsigned.
	 *
	 * @param  x the first {@code int} to compare
	 * @param  y the second {@code int} to compare
	 * @return the value {@code 0} if {@code x == y}; a value less
	 *         than {@code 0} if {@code x < y} as unsigned values; and
	 *         a value greater than {@code 0} if {@code x > y} as
	 *         unsigned values
	 * @since 1.8
	 */
	static int compareUnsigned(int x, int y);

	/**
	 * Returns the unsigned quotient of dividing the first argument by
	 * the second where each argument and the result is interpreted as
	 * an unsigned value.
	 *
	 * <p>Note that in two's complement arithmetic, the three other
	 * basic arithmetic operations of add, subtract, and multiply are
	 * bit-wise identical if the two operands are regarded as both
	 * being signed or both being unsigned.  Therefore separate {@code
	 * addUnsigned}, etc. methods are not provided.
	 *
	 * @param dividend the value to be divided
	 * @param divisor the value doing the dividing
	 * @return the unsigned quotient of the first argument divided by
	 * the second argument
	 * @see #remainderUnsigned
	 * @since 1.8
	 */
	static int divideUnsigned(int dividend, int divisor);

	/**
	 * Returns the unsigned remainder from dividing the first argument
	 * by the second where each argument and the result is interpreted
	 * as an unsigned value.
	 *
	 * @param dividend the value to be divided
	 * @param divisor the value doing the dividing
	 * @return the unsigned remainder of the first argument divided by
	 * the second argument
	 * @see #divideUnsigned
	 * @since 1.8
	 */
	static int remainderUnsigned(int dividend, int divisor);

	/**
	 * Returns the number of zero bits preceding the highest-order
	 * ("leftmost") one-bit in the two's complement binary representation
	 * of the specified {@code int} value.  Returns 32 if the
	 * specified value has no one-bits in its two's complement representation,
	 * in other words if it is equal to zero.
	 *
	 * <p>Note that this method is closely related to the logarithm base 2.
	 * For all positive {@code int} values x:
	 * <ul>
	 * <li>floor(log<sub>2</sub>(x)) = {@code 31 - numberOfLeadingZeros(x)}
	 * <li>ceil(log<sub>2</sub>(x)) = {@code 32 - numberOfLeadingZeros(x - 1)}
	 * </ul>
	 *
	 * @param i the value whose number of leading zeros is to be computed
	 * @return the number of zero bits preceding the highest-order
	 *     ("leftmost") one-bit in the two's complement binary representation
	 *     of the specified {@code int} value, or 32 if the value
	 *     is equal to zero.
	 * @since 1.5
	 */
	static int numberOfLeadingZeros(int i);

	/**
	 * Returns the number of zero bits following the lowest-order ("rightmost")
	 * one-bit in the two's complement binary representation of the specified
	 * {@code int} value.  Returns 32 if the specified value has no
	 * one-bits in its two's complement representation, in other words if it is
	 * equal to zero.
	 *
	 * @param i the value whose number of trailing zeros is to be computed
	 * @return the number of zero bits following the lowest-order ("rightmost")
	 *     one-bit in the two's complement binary representation of the
	 *     specified {@code int} value, or 32 if the value is equal
	 *     to zero.
	 * @since 1.5
	 */
	static int numberOfTrailingZeros(int i);

	/**
	 * Returns the number of one-bits in the two's complement binary
	 * representation of the specified {@code int} value.  This function is
	 * sometimes referred to as the <i>population count</i>.
	 *
	 * @param i the value whose bits are to be counted
	 * @return the number of one-bits in the two's complement binary
	 *     representation of the specified {@code int} value.
	 * @since 1.5
	 */
	static int bitCount(int i);

	/**
	 * Returns the value obtained by rotating the two's complement binary
	 * representation of the specified {@code int} value left by the
	 * specified number of bits.  (Bits shifted out of the left hand, or
	 * high-order, side reenter on the right, or low-order.)
	 *
	 * <p>Note that left rotation with a negative distance is equivalent to
	 * right rotation: {@code rotateLeft(val, -distance) == rotateRight(val,
	 * distance)}.  Note also that rotation by any multiple of 32 is a
	 * no-op, so all but the last five bits of the rotation distance can be
	 * ignored, even if the distance is negative: {@code rotateLeft(val,
	 * distance) == rotateLeft(val, distance & 0x1F)}.
	 *
	 * @param i the value whose bits are to be rotated left
	 * @param distance the number of bit positions to rotate left
	 * @return the value obtained by rotating the two's complement binary
	 *     representation of the specified {@code int} value left by the
	 *     specified number of bits.
	 * @since 1.5
	 */
	static int rotateLeft(int i, int distance);

	/**
	 * Returns the value obtained by rotating the two's complement binary
	 * representation of the specified {@code int} value right by the
	 * specified number of bits.  (Bits shifted out of the right hand, or
	 * low-order, side reenter on the left, or high-order.)
	 *
	 * <p>Note that right rotation with a negative distance is equivalent to
	 * left rotation: {@code rotateRight(val, -distance) == rotateLeft(val,
	 * distance)}.  Note also that rotation by any multiple of 32 is a
	 * no-op, so all but the last five bits of the rotation distance can be
	 * ignored, even if the distance is negative: {@code rotateRight(val,
	 * distance) == rotateRight(val, distance & 0x1F)}.
	 *
	 * @param i the value whose bits are to be rotated right
	 * @param distance the number of bit positions to rotate right
	 * @return the value obtained by rotating the two's complement binary
	 *     representation of the specified {@code int} value right by the
	 *     specified number of bits.
	 * @since 1.5
	 */
	static int rotateRight(int i, int distance);

	/**
	 * Returns the value obtained by reversing the order of the bits in the
	 * two's complement binary representation of the specified {@code int}
	 * value.
	 *
	 * @param i the value to be reversed
	 * @return the value obtained by reversing order of the bits in the
	 *     specified {@code int} value.
	 * @since 1.5
	 */
	static int reverse(int i);

	/**
	 * Returns the signum function of the specified {@code int} value.  (The
	 * return value is -1 if the specified value is negative; 0 if the
	 * specified value is zero; and 1 if the specified value is positive.)
	 *
	 * @param i the value whose signum is to be computed
	 * @return the signum function of the specified {@code int} value.
	 * @since 1.5
	 */
	static int signum(int i);

	/**
	 * Returns the value obtained by reversing the order of the bytes in the
	 * two's complement representation of the specified {@code int} value.
	 *
	 * @param i the value whose bytes are to be reversed
	 * @return the value obtained by reversing the bytes in the specified
	 *     {@code int} value.
	 * @since 1.5
	 */
	static int reverseBytes(int i);

    virtual byte byteValue();
    virtual short shortValue();
    virtual int intValue();
    virtual llong llongValue();
    virtual float floatValue();
    virtual double doubleValue();
    
    /**
	 * Compares this object to the specified object.  The result is
	 * <code>true</code> if and only if the argument is not
	 * <code>null</code> and is an <code>Integer</code> object that
	 * contains the same <code>int</code> value as this object.
	 *
	 * @param   obj   the object to compare with.
	 * @return  <code>true</code> if the objects are the same;
	 *          <code>false</code> otherwise.
	 */
	boolean equals(EInteger* obj);
	virtual boolean equals(EObject* obj);

	/**
	 * Returns a {@code String} object representing this
	 * {@code Integer}'s value. The value is converted to signed
	 * decimal representation and returned as a string, exactly as if
	 * the integer value were given as an argument to the {@link
	 * java.lang.Integer#toString(int)} method.
	 *
	 * @return  a string representation of the value of this object in
	 *          base&nbsp;10.
	 */
	virtual EString toString();

	/**
	 * Returns a hash code for this <code>Integer</code>.
	 *
	 * @return  a hash code value for this object, equal to the
	 *          primitive <code>int</code> value represented by this
	 *          <code>Integer</code> object.
	 */
	virtual int hashCode();

	/**
	 * Compares two {@code Integer} objects numerically.
	 *
	 * @param   anotherInteger   the {@code Integer} to be compared.
	 * @return  the value {@code 0} if this {@code Integer} is
	 *          equal to the argument {@code Integer}; a value less than
	 *          {@code 0} if this {@code Integer} is numerically less
	 *          than the argument {@code Integer}; and a value greater
	 *          than {@code 0} if this {@code Integer} is numerically
	 *           greater than the argument {@code Integer} (signed
	 *           comparison).
	 * @since   1.2
	 */
	virtual int compareTo(EInteger* anotherInteger);

public:
	int value;

	/**
	 * Returns an {@code int} value with at most a single one-bit, in the
	 * position of the highest-order ("leftmost") one-bit in the specified
	 * {@code int} value.  Returns zero if the specified value has no
	 * one-bits in its two's complement binary representation, that is, if it
	 * is equal to zero.
	 *
	 * @param i the value whose highest one bit is to be computed
	 * @return an {@code int} value with a single one-bit, in the position
	 *     of the highest-order one-bit in the specified value, or zero if
	 *     the specified value is itself equal to zero.
	 * @since 1.5
	 */
	static int highestOneBit(int i);

	/**
	 * Returns an {@code int} value with at most a single one-bit, in the
	 * position of the lowest-order ("rightmost") one-bit in the specified
	 * {@code int} value.  Returns zero if the specified value has no
	 * one-bits in its two's complement binary representation, that is, if it
	 * is equal to zero.
	 *
	 * @param i the value whose lowest one bit is to be computed
	 * @return an {@code int} value with a single one-bit, in the position
	 *     of the lowest-order one-bit in the specified value, or zero if
	 *     the specified value is itself equal to zero.
	 * @since 1.5
	 */
	static int lowestOneBit(int i);

protected:
	/**
	 * Format a integer (treated as unsigned) into a String.
	 * @param val the value to format
	 * @param shift the log2 of the base to format in (4 for hex, 3 for octal, 1 for binary)
	 */
	static EString toUnsignedString0(int i, int shift);
};

} /* namespace efc */
#endif //!__EInteger_H__
