/*
 * ELLong.hh
 *
 *  Created on: 2013-3-21
 *      Author: Administrator
 */

#ifndef ELLONG_HH_
#define ELLONG_HH_

#include "EObject.hh"
#include "EString.hh"
#include "ENumberFormatException.hh"

namespace efc {

/**
 * The {@code Long} class wraps a value of the primitive type {@code
 * long} in an object. An object of type {@code Long} contains a
 * single field whose type is {@code long}.
 *
 * <p> In addition, this class provides several methods for converting
 * a {@code long} to a {@code String} and a {@code String} to a {@code
 * long}, as well as other constants and methods useful when dealing
 * with a {@code long}.
 *
 * <p>Implementation note: The implementations of the "bit twiddling"
 * methods (such as {@link #highestOneBit(long) highestOneBit} and
 * {@link #numberOfTrailingZeros(long) numberOfTrailingZeros}) are
 * based on material from Henry S. Warren, Jr.'s <i>Hacker's
 * Delight</i>, (Addison Wesley, 2002).
 *
 * @since   JDK1.0
 */

/**
 * The minimum radix available for conversion to and from Strings.
 */
#define LLONG_MIN_RADIX   2

/**
 * The maximum radix available for conversion to and from Strings.
 */
#define LLONG_MAX_RADIX   36

/**
 * The Long class wraps a value of the primitive type <code>long</code>
 * in an object. An object of type <code>Long</code> contains a single
 * field whose type is <code>long</code>.
 * <p>
 * In addition, this class provides several methods for converting a
 * <code>long</code> to a <code>String</code> and a
 * <code>String</code> to a <code>long</code>, as well as other
 * constants and methods useful when dealing with a
 * <code>long</code>.
 *
 * @version 12/17/01 (CLDC 1.1)
 * @since   JDK1.0, CLDC 1.0
 */

class ELLong : virtual public EComparable<ELLong*>
{
public:
	/**
	 * The smallest value of type <code>long</code>.
	 */
	static const llong MIN_VALUE;// = LLONG(0x8000000000000000);

	/**
	 * The largest value of type <code>long</code>.
	 */
	static const llong MAX_VALUE;// = LLONG(0x7fffffffffffffff);

public:
	virtual ~ELLong(){}

	ELLong(llong value);

	/**
	 * Returns the value of this Long as a long value.
	 *
	 * @return  the <code>long</code> value represented by this object.
	 */
	llong llongValue();

	/**
	 * Returns the value of this Long as a double.
	 *
	 * @return  the <code>long</code> value represented by this object that
	 *          is converted to type <code>double</code> and the result of
	 *          the conversion is returned.
	 * @since   CLDC 1.1
	 */
	double doubleValue();

	/**
	 * Creates a string representation of the first argument in the
	 * radix specified by the second argument.
	 * <p>
	 * If the radix is smaller than <code>Character.MIN_RADIX</code> or
	 * larger than <code>Character.MAX_RADIX</code>, then the radix
	 * <code>10</code> is used instead.
	 * <p>
	 * If the first argument is negative, the first element of the
	 * result is the ASCII minus sign <code>'-'</code>
	 * (<code>'&#92;u002d'</code>. If the first argument is not negative,
	 * no sign character appears in the result.
	 * <p>
	 * The remaining characters of the result represent the magnitude of
	 * the first argument. If the magnitude is zero, it is represented by
	 * a single zero character <code>'0'</code>
	 * (<code>'&#92;u0030'</code>); otherwise, the first character of the
	 * representation of the magnitude will not be the zero character.
	 * The following ASCII characters are used as digits:
	 * <blockquote><pre>
	 *   0123456789abcdefghijklmnopqrstuvwxyz
	 * </pre></blockquote>
	 * These are <tt>'&#92;u0030'</tt> through <tt>'&#92;u0039'</tt>
	 * and <tt>'&#92;u0061'</tt> through <tt>'&#92;u007a'</tt>. If the
	 * radix is <var>N</var>, then the first <var>N</var> of these
	 * characters are used as radix-<var>N</var> digits in the order
	 * shown. Thus, the digits for hexadecimal (radix 16) are
	 * <blockquote><pre>
	 * <tt>0123456789abcdef</tt>.
	 * </pre></blockquote>
	 *
	 * @param   i       a long.
	 * @param   radix   the radix.
	 * @return  a string representation of the argument in the specified radix.
	 * @see     java.lang.Character#MAX_RADIX
	 * @see     java.lang.Character#MIN_RADIX
	 */
	static EString toString(llong i, int radix = 10);

	/**
     * Returns a string representation of the <code>long</code>
     * argument as an unsigned integer in base&nbsp;16.
     * <p>
     * The unsigned <code>long</code> value is the argument plus
     * 2<sup>64</sup> if the argument is negative; otherwise, it is
     * equal to the argument.  This value is converted to a string of
     * ASCII digits in hexadecimal (base&nbsp;16) with no extra
     * leading <code>0</code>s.  If the unsigned magnitude is zero, it
     * is represented by a single zero character <code>'0'</code>
     * (<code>'&#92;u0030'</code>); otherwise, the first character of
     * the representation of the unsigned magnitude will not be the
     * zero character. The following characters are used as
     * hexadecimal digits:
     * <blockquote><pre>
     * 0123456789abcdef
     * </pre></blockquote>
     * These are the characters <code>'&#92;u0030'</code> through
     * <code>'&#92;u0039'</code> and  <code>'&#92;u0061'</code> through
     * <code>'&#92;u0066'</code>.  If uppercase letters are desired,
     * the {@link java.lang.String#toUpperCase()} method may be called
     * on the result:
     * <blockquote><pre>
     * Long.toHexString(n).toUpperCase()
     * </pre></blockquote>
     *
     * @param   i   a <code>long</code> to be converted to a string.
     * @return  the string representation of the unsigned <code>long</code>
     * 		value represented by the argument in hexadecimal
     *		(base&nbsp;16).
     * @since   JDK 1.0.2
     */
	static EString toHexString(llong i);
	
	/**
     * Returns a string representation of the <code>long</code>
     * argument as an unsigned integer in base&nbsp;8.
     * <p>
     * The unsigned <code>long</code> value is the argument plus
     * 2<sup>64</sup> if the argument is negative; otherwise, it is
     * equal to the argument.  This value is converted to a string of
     * ASCII digits in octal (base&nbsp;8) with no extra leading
     * <code>0</code>s.
     * <p>
     * If the unsigned magnitude is zero, it is represented by a
     * single zero character <code>'0'</code>
     * (<code>'&#92;u0030'</code>); otherwise, the first character of
     * the representation of the unsigned magnitude will not be the
     * zero character. The following characters are used as octal
     * digits:
     * <blockquote><pre>
     * 01234567
     * </pre></blockquote>
     * These are the characters <code>'&#92;u0030'</code> through 
     * <code>'&#92;u0037'</code>. 
     *
     * @param   i   a <code>long</code> to be converted to a string.
     * @return  the string representation of the unsigned <code>long</code> 
     *		value represented by the argument in octal (base&nbsp;8).
     * @since   JDK 1.0.2
     */
    static EString toOctalString(llong i);
    
    /**
     * Returns a string representation of the <code>long</code>
     * argument as an unsigned integer in base&nbsp;2.
     * <p>
     * The unsigned <code>long</code> value is the argument plus
     * 2<sup>64</sup> if the argument is negative; otherwise, it is
     * equal to the argument.  This value is converted to a string of
     * ASCII digits in binary (base&nbsp;2) with no extra leading
     * <code>0</code>s.  If the unsigned magnitude is zero, it is
     * represented by a single zero character <code>'0'</code>
     * (<code>'&#92;u0030'</code>); otherwise, the first character of
     * the representation of the unsigned magnitude will not be the
     * zero character. The characters <code>'0'</code>
     * (<code>'&#92;u0030'</code>) and <code>'1'</code>
     * (<code>'&#92;u0031'</code>) are used as binary digits.
     *
     * @param   i   a <code>long</code> to be converted to a string.
     * @return  the string representation of the unsigned <code>long</code> 
     *          value represented by the argument in binary (base&nbsp;2).
     * @since   JDK 1.0.2
     */
    static EString toBinaryString(llong i);
    
    /**
     * Convert the integer to an unsigned number.
     */
    static EString toUnsignedString(llong i, int shift);
    
	/**
	 * Parses the string argument as a signed <code>long</code> in the
	 * radix specified by the second argument. The characters in the
	 * string must all be digits of the specified radix (as determined by
	 * whether <code>Character.digit</code> returns a
	 * nonnegative value), except that the first character may be an
	 * ASCII minus sign <code>'-'</code> (<tt>'&#92;u002d'</tt> to indicate
	 * a negative value. The resulting <code>long</code> value is returned.
	 * <p>
	 * Note that neither <tt>L</tt> nor <tt>l</tt> is permitted to appear at
	 * the end of the string as a type indicator, as would be permitted in
	 * Java programming language source code - except that either <tt>L</tt>
	 * or <tt>l</tt> may appear as a digit for a radix greater than 22.
	 * <p>
	 * An exception of type <tt>NumberFormatException</tt> is thrown if any of
	 * the following situations occurs:
	 * <ul>
	 * <li>The first argument is <tt>null</tt> or is a string of length zero.
	 * <li>The <tt>radix</tt> is either smaller than
	 *     {@link java.lang.Character#MIN_RADIX} or larger than
	 *     {@link java.lang.Character#MAX_RADIX}.
	 * <li>The first character of the string is not a digit of the
	 *     specified <tt>radix</tt> and is not a minus sign <tt>'-'</tt>
	 *     (<tt>'&#92;u002d'</tt>).
	 * <li>The first character of the string is a minus sign and the
	 *     string is of length 1.
	 * <li>Any character of the string after the first is not a digit of
	 *     the specified <tt>radix</tt>.
	 * <li>The integer value represented by the string cannot be
	 *     represented as a value of type <tt>long</tt>.
	 * </ul><p>
	 * Examples:
	 * <blockquote><pre>
	 * parseLong("0", 10) returns 0L
	 * parseLong("473", 10) returns 473L
	 * parseLong("-0", 10) returns 0L
	 * parseLong("-FF", 16) returns -255L
	 * parseLong("1100110", 2) returns 102L
	 * parseLong("99", 8) throws a NumberFormatException
	 * parseLong("Hazelnut", 10) throws a NumberFormatException
	 * parseLong("Hazelnut", 36) returns 1356099454469L
	 * </pre></blockquote>
	 *
	 * @param      s       the <code>String</code> containing the
	 *                     <code>long</code>.
	 * @param      radix   the radix to be used.
	 * @return     the <code>long</code> represented by the string argument in
	 *             the specified radix.
	 * @exception  NumberFormatException  if the string does not contain a
	 *                                    parsable integer.
	 */
	static llong parseLLong(const char* s, int radix = 10) THROWS(ENumberFormatException);

	static ELLong valueOf(llong ll);
	static ELLong valueOf(const char* s, int radix=10);

	/**
	 * Compares this object to the specified object.  The result is
	 * <code>true</code> if and only if the argument is not
	 * <code>null</code> and is a <code>Long</code> object that
	 * contains the same <code>long</code> value as this object.
	 *
	 * @param   obj   the object to compare with.
	 * @return  <code>true</code> if the objects are the same;
	 *          <code>false</code> otherwise.
	 */
	boolean equals(ELLong *obj);
	virtual boolean equals(EObject* obj);

	/**
	 * Returns a String object representing this Long's value.
	 * The long integer value represented by this Long object is converted
	 * to signed decimal representation and returned as a string, exactly
	 * as if the long value were given as an argument to the
	 * {@link #toString(long)} method that takes one argument.
	 *
	 * @return  a string representation of this object in base&nbsp;10.
	 */
	virtual EStringBase toString();

	/**
	 * Returns a hash code for this <code>Long</code>. The result is
	 * the exclusive OR of the two halves of the primitive
	 * <code>long</code> value held by this <code>Long</code>
	 * object. That is, the hashcode is the value of the expression:
	 * <blockquote><pre>
	 * (int)(this.longValue()^(this.longValue()&gt;&gt;&gt;32))
	 * </pre></blockquote>
	 *
	 * @return  a hash code value for this object.
	 */
	virtual int hashCode();

	/**
	 * Compares two {@code Long} objects numerically.
	 *
	 * @param   anotherLong   the {@code Long} to be compared.
	 * @return  the value {@code 0} if this {@code Long} is
	 *          equal to the argument {@code Long}; a value less than
	 *          {@code 0} if this {@code Long} is numerically less
	 *          than the argument {@code Long}; and a value greater
	 *          than {@code 0} if this {@code Long} is numerically
	 *           greater than the argument {@code Long} (signed
	 *           comparison).
	 * @since   1.2
	 */
	virtual int compareTo(ELLong* anotherLong);

public:
	/**
	 * The value of the Long.
	 */
	llong value;

	/**
	 * Returns a {@code long} value with at most a single one-bit, in the
	 * position of the highest-order ("leftmost") one-bit in the specified
	 * {@code long} value.  Returns zero if the specified value has no
	 * one-bits in its two's complement binary representation, that is, if it
	 * is equal to zero.
	 *
	 * @param i the value whose highest one bit is to be computed
	 * @return a {@code long} value with a single one-bit, in the position
	 *     of the highest-order one-bit in the specified value, or zero if
	 *     the specified value is itself equal to zero.
	 * @since 1.5
	 */
	static llong highestOneBit(llong i);

	/**
	 * Returns a {@code long} value with at most a single one-bit, in the
	 * position of the lowest-order ("rightmost") one-bit in the specified
	 * {@code long} value.  Returns zero if the specified value has no
	 * one-bits in its two's complement binary representation, that is, if it
	 * is equal to zero.
	 *
	 * @param i the value whose lowest one bit is to be computed
	 * @return a {@code long} value with a single one-bit, in the position
	 *     of the lowest-order one-bit in the specified value, or zero if
	 *     the specified value is itself equal to zero.
	 * @since 1.5
	 */
	static llong lowestOneBit(llong i);
};

} /* namespace efc */
#endif /* ELLONG_HH_ */
