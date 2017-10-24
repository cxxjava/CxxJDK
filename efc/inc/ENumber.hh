/*
 * ENumber.hh
 *
 *  Created on: 2016-9-19
 *      Author: cxxjava@163.com
 */

#ifndef ENUMBER_HH_
#define ENUMBER_HH_

#include "EObject.hh"

namespace efc {

/**
 * The abstract class {@code Number} is the superclass of platform
 * classes representing numeric values that are convertible to the
 * primitive types {@code byte}, {@code double}, {@code float}, {@code
 * int}, {@code long}, and {@code short}.
 *
 * The specific semantics of the conversion from the numeric value of
 * a particular {@code Number} implementation to a given primitive
 * type is defined by the {@code Number} implementation in question.
 *
 * For platform classes, the conversion is often analogous to a
 * narrowing primitive conversion or a widening primitive conversion
 * as defining in <cite>The Java&trade; Language Specification</cite>
 * for converting between primitive types.  Therefore, conversions may
 * lose information about the overall magnitude of a numeric value, may
 * lose precision, and may even return a result of a different sign
 * than the input.
 *
 * See the documentation of a given {@code Number} implementation for
 * conversion details.
 *
 * @author      Lee Boynton
 * @author      Arthur van Hoff
 * @jls 5.1.2 Widening Primitive Conversions
 * @jls 5.1.3 Narrowing Primitive Conversions
 * @since   JDK1.0
 */

abstract class ENumber: virtual public EObject {
public:
	virtual ~ENumber(){
	}

	/**
	 * Returns the value of the specified number as an {@code int},
	 * which may involve rounding or truncation.
	 *
	 * @return  the numeric value represented by this object after conversion
	 *          to type {@code int}.
	 */
	virtual int intValue() = 0;

	/**
	 * Returns the value of the specified number as a {@code long},
	 * which may involve rounding or truncation.
	 *
	 * @return  the numeric value represented by this object after conversion
	 *          to type {@code long}.
	 */
	virtual llong llongValue() = 0;

	/**
	 * Returns the value of the specified number as a {@code float},
	 * which may involve rounding.
	 *
	 * @return  the numeric value represented by this object after conversion
	 *          to type {@code float}.
	 */
	virtual float floatValue() = 0;

	/**
	 * Returns the value of the specified number as a {@code double},
	 * which may involve rounding.
	 *
	 * @return  the numeric value represented by this object after conversion
	 *          to type {@code double}.
	 */
	virtual double doubleValue() = 0;

	/**
	 * Returns the value of the specified number as a {@code byte},
	 * which may involve rounding or truncation.
	 *
	 * <p>This implementation returns the result of {@link #intValue} cast
	 * to a {@code byte}.
	 *
	 * @return  the numeric value represented by this object after conversion
	 *          to type {@code byte}.
	 * @since   JDK1.1
	 */
	virtual byte byteValue() {
		return (byte)intValue();
	}

	/**
	 * Returns the value of the specified number as a {@code short},
	 * which may involve rounding or truncation.
	 *
	 * <p>This implementation returns the result of {@link #intValue} cast
	 * to a {@code short}.
	 *
	 * @return  the numeric value represented by this object after conversion
	 *          to type {@code short}.
	 * @since   JDK1.1
	 */
	virtual short shortValue() {
		return (short)intValue();
	}
};

} /* namespace efc */
#endif /* ENUMBER_HH_ */
