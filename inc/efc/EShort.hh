/*
 * EShort.hh
 *
 *  Created on: 2017-6-21
 *      Author: cxxjava@163.com
 */

#ifndef ESHORT_HH_
#define ESHORT_HH_

#include "ENumber.hh"
#include "EString.hh"
#include "EComparable.hh"
#include "ENumberFormatException.hh"

namespace efc {

/**
 * The {@code Short} class wraps a value of primitive type {@code
 * short} in an object.  An object of type {@code Short} contains a
 * single field whose type is {@code short}.
 *
 * <p>In addition, this class provides several methods for converting
 * a {@code short} to a {@code String} and a {@code String} to a
 * {@code short}, as well as other constants and methods useful when
 * dealing with a {@code short}.
 *
 * @see     java.lang.Number
 * @since   JDK1.1
 */

class EShort : public ENumber, virtual public EComparable<EShort*> {
public:
	/**
	 * The constant value of this field is the smallest value of type
	 * <code>short</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const short MIN_VALUE = -32768;

	/**
	 * The constant value of this field is the largest value of type
	 * <code>short</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const short MAX_VALUE = 32767;

	/**
	 * The number of bits used to represent a {@code short} value in two's
	 * complement binary form.
	 *
	 * @since 1.5
	 */
	static const int SIZE = 16;

public:
	virtual ~EShort() {
	}

	EShort(short value);

	/**
	 * Compares this object against the specified object.
	 * The result is <code>true</code> if and only if the argument is not
	 * <code>null</code> and is a <code>Character</code> object that
	 * represents the same <code>short</code> value as this object.
	 *
	 * @param   obj   the object to compare with.
	 * @return  <code>true</code> if the objects are the same;
	 *          <code>false</code> otherwise.
	 */
	boolean equals(short that);

	/**
	 * Returns a String object representing this short's value.
	 * Converts this <code>Character</code> object to a string. The
	 * result is a string whose length is <code>1</code>. The string's
	 * sole component is the primitive <code>short</code> value represented
	 * by this object.
	 *
	 * @return  a string representation of this object.
	 */
	virtual EStringBase toString();

	/**
	 * Parses the string argument as a signed {@code short} in the
	 * radix specified by the second argument. The characters in the
	 * string must all be digits, of the specified radix (as
	 * determined by whether {@link java.lang.Character#digit(char,
	 * int)} returns a nonnegative value) except that the first
	 * character may be an ASCII minus sign {@code '-'}
	 * ({@code '\u005Cu002D'}) to indicate a negative value or an
	 * ASCII plus sign {@code '+'} ({@code '\u005Cu002B'}) to
	 * indicate a positive value.  The resulting {@code short} value
	 * is returned.
	 *
	 * <p>An exception of type {@code NumberFormatException} is
	 * thrown if any of the following situations occurs:
	 * <ul>
	 * <li> The first argument is {@code null} or is a string of
	 * length zero.
	 *
	 * <li> The radix is either smaller than {@link
	 * java.lang.Character#MIN_RADIX} or larger than {@link
	 * java.lang.Character#MAX_RADIX}.
	 *
	 * <li> Any character of the string is not a digit of the
	 * specified radix, except that the first character may be a minus
	 * sign {@code '-'} ({@code '\u005Cu002D'}) or plus sign
	 * {@code '+'} ({@code '\u005Cu002B'}) provided that the
	 * string is longer than length 1.
	 *
	 * <li> The value represented by the string is not a value of type
	 * {@code short}.
	 * </ul>
	 *
	 * @param s         the {@code String} containing the
	 *                  {@code short} representation to be parsed
	 * @param radix     the radix to be used while parsing {@code s}
	 * @return          the {@code short} represented by the string
	 *                  argument in the specified radix.
	 * @throws          NumberFormatException If the {@code String}
	 *                  does not contain a parsable {@code short}.
	 */
    static short parseShort(const char *s, int radix=10) THROWS(ENumberFormatException);

    /**
	 * Returns a {@code Short} object holding the value
	 * extracted from the specified {@code String} when parsed
	 * with the radix given by the second argument. The first argument
	 * is interpreted as representing a signed {@code short} in
	 * the radix specified by the second argument, exactly as if the
	 * argument were given to the {@link #parseShort(java.lang.String,
	 * int)} method. The result is a {@code Short} object that
	 * represents the {@code short} value specified by the string.
	 *
	 * <p>In other words, this method returns a {@code Short} object
	 * equal to the value of:
	 *
	 * <blockquote>
	 *  {@code new Short(Short.parseShort(s, radix))}
	 * </blockquote>
	 *
	 * @param s         the string to be parsed
	 * @param radix     the radix to be used in interpreting {@code s}
	 * @return          a {@code Short} object holding the value
	 *                  represented by the string argument in the
	 *                  specified radix.
	 * @throws          NumberFormatException If the {@code String} does
	 *                  not contain a parsable {@code short}.
	 */
	static EShort valueOf(const char* s, int radix=10) THROWS(ENumberFormatException);

    /**
	 * Compares two {@code short} values numerically.
	 * The value returned is identical to what would be returned by:
	 * <pre>
	 *    Short.valueOf(x).compareTo(Short.valueOf(y))
	 * </pre>
	 *
	 * @param  x the first {@code short} to compare
	 * @param  y the second {@code short} to compare
	 * @return the value {@code 0} if {@code x == y};
	 *         a value less than {@code 0} if {@code x < y}; and
	 *         a value greater than {@code 0} if {@code x > y}
	 * @since 1.7
	 */
	static int compare(short x, short y);

	/**
	 * Returns a hash code for this <code>Short</code>.
	 */
	virtual int hashCode();

	/**
	 * Compares this object to the specified object.  The result is
	 * <code>true</code> if and only if the argument is not
	 * <code>null</code> and is a <code>Short</code> object that
	 * contains the same <code>short</code> value as this object.
	 *
	 * @param obj	the object to compare with
	 * @return 		<code>true</code> if the objects are the same;
	 * 			<code>false</code> otherwise.
	 */
	boolean equals(EShort *obj);
	virtual boolean equals(EObject* obj);

	/**
	 * Compares two {@code Short} objects numerically.
	 *
	 * @param   anotherShort   the {@code Short} to be compared.
	 * @return  the value {@code 0} if this {@code Short} is
	 *          equal to the argument {@code Short}; a value less than
	 *          {@code 0} if this {@code Short} is numerically less
	 *          than the argument {@code Short}; and a value greater than
	 *           {@code 0} if this {@code Short} is numerically
	 *           greater than the argument {@code Short} (signed
	 *           comparison).
	 * @since   1.2
	 */
	virtual int compareTo(EShort* anotherShort);

	virtual byte byteValue();
	virtual short shortValue();
	virtual int intValue();
	virtual llong llongValue();
	virtual float floatValue();
	virtual double doubleValue();

public:
	/**
	 * The value of the Character.
	 */
	short value;
};

} /* namespace efc */
#endif /* ESHORT_HH_ */
