/*
 * EByte.hh
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#ifndef EBYTE_HH_
#define EBYTE_HH_

#include "ENumber.hh"
#include "EString.hh"
#include "EComparable.hh"
#include "ENumberFormatException.hh"

namespace efc {

/**
 *
 * The {@code Byte} class wraps a value of primitive type {@code byte}
 * in an object.  An object of type {@code Byte} contains a single
 * field whose type is {@code byte}.
 *
 * <p>In addition, this class provides several methods for converting
 * a {@code byte} to a {@code String} and a {@code String} to a {@code
 * byte}, as well as other constants and methods useful when dealing
 * with a {@code byte}.
 *
 * @see     java.lang.Number
 * @since   JDK1.1
 */

class EByte : public ENumber, virtual public EComparable<EByte*> {
public:
	/**
	 * The constant value of this field is the smallest value of type
	 * <code>byte</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const byte MIN_VALUE = -128;

	/**
	 * The constant value of this field is the largest value of type
	 * <code>byte</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const byte MAX_VALUE = 127;

	/**
	 * The number of bits used to represent a {@code byte} value in two's
	 * complement binary form.
	 *
	 * @since 1.5
	 */
	static const int SIZE = 8;

public:
	virtual ~EByte() {
	}

	EByte(byte value);

	/**
	 * Compares this object against the specified object.
	 * The result is <code>true</code> if and only if the argument is not
	 * <code>null</code> and is a <code>Character</code> object that
	 * represents the same <code>byte</code> value as this object.
	 *
	 * @param   obj   the object to compare with.
	 * @return  <code>true</code> if the objects are the same;
	 *          <code>false</code> otherwise.
	 */
	boolean equals(byte that);

	/**
	 * Returns a String object representing this byte's value.
	 * Converts this <code>Character</code> object to a string. The
	 * result is a string whose length is <code>1</code>. The string's
	 * sole component is the primitive <code>byte</code> value represented
	 * by this object.
	 *
	 * @return  a string representation of this object.
	 */
	virtual EString toString();

	/**
     * Assuming the specified String represents a byte, returns
     * that byte's value. Throws an exception if the String cannot
     * be parsed as a byte.  The radix is assumed to be 10.
     *
     * @param s       the String containing the byte
     * @return        the parsed value of the byte
     * @exception     NumberFormatException If the string does not
     *                contain a parsable byte.
     */
    static byte parseByte(const char *s, int radix=10) THROWS(ENumberFormatException);

    /**
	 * Compares two {@code byte} values numerically.
	 * The value returned is identical to what would be returned by:
	 * <pre>
	 *    Byte.valueOf(x).compareTo(Byte.valueOf(y))
	 * </pre>
	 *
	 * @param  x the first {@code byte} to compare
	 * @param  y the second {@code byte} to compare
	 * @return the value {@code 0} if {@code x == y};
	 *         a value less than {@code 0} if {@code x < y}; and
	 *         a value greater than {@code 0} if {@code x > y}
	 * @since 1.7
	 */
	static int compare(byte x, byte y);

	/**
	 * Returns a hash code for this <code>Byte</code>.
	 */
	virtual int hashCode();

	/**
	 * Compares this object to the specified object.  The result is
	 * <code>true</code> if and only if the argument is not
	 * <code>null</code> and is a <code>Byte</code> object that
	 * contains the same <code>byte</code> value as this object.
	 *
	 * @param obj	the object to compare with
	 * @return 		<code>true</code> if the objects are the same;
	 * 			<code>false</code> otherwise.
	 */
	boolean equals(EByte *obj);
	virtual boolean equals(EObject* obj);

	/**
	 * Compares two {@code Byte} objects numerically.
	 *
	 * @param   anotherByte   the {@code Byte} to be compared.
	 * @return  the value {@code 0} if this {@code Byte} is
	 *          equal to the argument {@code Byte}; a value less than
	 *          {@code 0} if this {@code Byte} is numerically less
	 *          than the argument {@code Byte}; and a value greater than
	 *           {@code 0} if this {@code Byte} is numerically
	 *           greater than the argument {@code Byte} (signed
	 *           comparison).
	 * @since   1.2
	 */
	virtual int compareTo(EByte* anotherByte);

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
	byte value;
};

} /* namespace efc */
#endif /* EBYTE_HH_ */
