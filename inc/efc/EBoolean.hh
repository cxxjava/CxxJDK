/*
 * EBoolean.hh
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#ifndef EBOOLEAN_HH_
#define EBOOLEAN_HH_

#include "EObject.hh"
#include "EString.hh"

namespace efc {

/**
 * The Boolean class wraps a value of the primitive type
 * {@code boolean} in an object. An object of type
 * {@code Boolean} contains a single field whose type is
 * {@code boolean}.
 * <p>
 * In addition, this class provides many methods for
 * converting a {@code boolean} to a {@code String} and a
 * {@code String} to a {@code boolean}, as well as other
 * constants and methods useful when dealing with a
 * {@code boolean}.
 *
 * @author  Arthur van Hoff
 * @since   JDK1.0
 */

class EBoolean : public EObject {
public:
	virtual ~EBoolean() {
	}

	/**
	 * Allocates a {@code Boolean} object representing the
	 * {@code value} argument.
	 *
	 * <p><b>Note: It is rarely appropriate to use this constructor.
	 * Unless a <i>new</i> instance is required, the static factory
	 * {@link #valueOf(boolean)} is generally a better choice. It is
	 * likely to yield significantly better space and time performance.</b>
	 *
	 * @param   value   the value of the {@code Boolean}.
	 */
	EBoolean(boolean value);

	/**
	 * Allocates a {@code Boolean} object representing the value
	 * {@code true} if the string argument is not {@code null}
	 * and is equal, ignoring case, to the string {@code "true"}.
	 * Otherwise, allocate a {@code Boolean} object representing the
	 * value {@code false}. Examples:<p>
	 * {@code new Boolean("True")} produces a {@code Boolean} object
	 * that represents {@code true}.<br>
	 * {@code new Boolean("yes")} produces a {@code Boolean} object
	 * that represents {@code false}.
	 *
	 * @param   s   the string to be converted to a {@code Boolean}.
	 */
	EBoolean(const char* s);

	/**
	 * Parses the string argument as a boolean.  The {@code boolean}
	 * returned represents the value {@code true} if the string argument
	 * is not {@code null} and is equal, ignoring case, to the string
	 * {@code "true"}. <p>
	 * Example: {@code Boolean.parseBoolean("True")} returns {@code true}.<br>
	 * Example: {@code Boolean.parseBoolean("yes")} returns {@code false}.
	 *
	 * @param      s   the {@code String} containing the boolean
	 *                 representation to be parsed
	 * @return     the boolean represented by the string argument
	 * @since 1.5
	 */
	static boolean parseBoolean(const char* s);

	/**
	 * Returns the value of this {@code Boolean} object as a boolean
	 * primitive.
	 *
	 * @return  the primitive {@code boolean} value of this object.
	 */
	boolean booleanValue();

	/**
	 * Returns a {@code Boolean} instance representing the specified
	 * {@code boolean} value.  If the specified {@code boolean} value
	 * is {@code true}, this method returns {@code Boolean.TRUE};
	 * if it is {@code false}, this method returns {@code Boolean.FALSE}.
	 * If a new {@code Boolean} instance is not required, this method
	 * should generally be used in preference to the constructor
	 * {@link #Boolean(boolean)}, as this method is likely to yield
	 * significantly better space and time performance.
	 *
	 * @param  b a boolean value.
	 * @return a {@code Boolean} instance representing {@code b}.
	 * @since  1.4
	 */
	static EBoolean valueOf(boolean b);

	/**
	 * Returns a {@code Boolean} with a value represented by the
	 * specified string.  The {@code Boolean} returned represents a
	 * true value if the string argument is not {@code null}
	 * and is equal, ignoring case, to the string {@code "true"}.
	 *
	 * @param   s   a string.
	 * @return  the {@code Boolean} value represented by the string.
	 */
	static EBoolean valueOf(const char* s);

	/**
	 * Returns a {@code String} object representing the specified
	 * boolean.  If the specified boolean is {@code true}, then
	 * the string {@code "true"} will be returned, otherwise the
	 * string {@code "false"} will be returned.
	 *
	 * @param b the boolean to be converted
	 * @return the string representation of the specified {@code boolean}
	 * @since 1.4
	 */
	static EString toString(boolean b);

	/**
	 * Returns a {@code String} object representing this Boolean's
	 * value.  If this object represents the value {@code true},
	 * a string equal to {@code "true"} is returned. Otherwise, a
	 * string equal to {@code "false"} is returned.
	 *
	 * @return  a string representation of this object.
	 */
	EString toString();

	/**
	 * Returns a hash code for this {@code Boolean} object.
	 *
	 * @return  the integer {@code 1231} if this object represents
	 * {@code true}; returns the integer {@code 1237} if this
	 * object represents {@code false}.
	 */
	int hashCode();

	/**
	 * Returns {@code true} if and only if the argument is not
	 * {@code null} and is a {@code Boolean} object that
	 * represents the same {@code boolean} value as this object.
	 *
	 * @param   obj   the object to compare with.
	 * @return  {@code true} if the Boolean objects represent the
	 *          same value; {@code false} otherwise.
	 */
	boolean equals(EBoolean* obj);

	/**
	 * Compares this {@code Boolean} instance with another.
	 *
	 * @param   b the {@code Boolean} instance to be compared
	 * @return  zero if this object represents the same boolean value as the
	 *          argument; a positive value if this object represents true
	 *          and the argument represents false; and a negative value if
	 *          this object represents false and the argument represents true
	 * @throws  NullPointerException if the argument is {@code null}
	 * @see     Comparable
	 * @since  1.5
	 */
	int compareTo(EBoolean* b);

private:
	/**
	 * The value of the Boolean.
	 *
	 * @serial
	 */
	boolean _value;

	static boolean toBoolean(const char* name);
};

} /* namespace efc */
#endif /* EBOOLEAN_HH_ */
