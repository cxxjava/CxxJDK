/*
 * EByte.hh
 *
 *  Created on: 2013-3-21
 *      Author: Administrator
 */

#ifndef EBYTE_HH_
#define EBYTE_HH_

#include "EObject.hh"
#include "EString.hh"
#include "ENumberFormatException.hh"

namespace efc {

/**
 * The minimum radix available for conversion to and from Strings.
 */
#define BYTE_MIN_RADIX   2

/**
 * The maximum radix available for conversion to and from Strings.
 */
#define BYTE_MAX_RADIX   36

class EByte : public EObject {
public:
	/**
	 * The constant value of this field is the smallest value of type
	 * <code>byte</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const byte MIN_VALUE;// = -128;

	/**
	 * The constant value of this field is the largest value of type
	 * <code>byte</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const byte MAX_VALUE;// = 127;

	/**
	 * The number of bits used to represent a {@code byte} value in two's
	 * complement binary form.
	 *
	 * @since 1.5
	 */
	static const int SIZE;// = 8;

public:
	virtual ~EByte() {
	}

	EByte(byte value);

	/**
	 * Returns the value of this Character object.
	 * @return  the primitive <code>byte</code> value represented by
	 *          this object.
	 */
	byte byteValue();

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
	virtual EStringBase toString();

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

private:
	/**
	 * The value of the Character.
	 */
	byte m_value;
};

} /* namespace efc */
#endif /* EBYTE_HH_ */
