#ifndef __EInteger_H__
#define __EInteger_H__

#include "EObject.hh"
#include "EString.hh"
#include "EComparable.hh"
#include "ENumberFormatException.hh"

namespace efc {

/**
 * The minimum radix available for conversion to and from Strings.
 */
#define INTEGER_MIN_RADIX   2

/**
 * The maximum radix available for conversion to and from Strings.
 */
#define INTEGER_MAX_RADIX   36

class EInteger : virtual public EComparable<EInteger*>
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
    static EString toUnsignedString(int i, int shift);
    
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
    
    byte byteValue();
    short shortValue();
    int intValue();
    long longValue();
    
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

	/**
	 * Returns a hash code for this <code>Integer</code>.
	 *
	 * @return  a hash code value for this object, equal to the
	 *          primitive <code>int</code> value represented by this
	 *          <code>Integer</code> object.
	 */
	int hashCode();

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
	int compareTo(EInteger* anotherInteger);

public:
	int value;
};

} /* namespace efc */
#endif //!__EInteger_H__
