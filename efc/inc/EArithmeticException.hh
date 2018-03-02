/*
 * EArithmeticException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EARITHMETICEXCEPTION_H_
#define EARITHMETICEXCEPTION_H_

#include "ERuntimeException.hh"

namespace efc {

#define EARITHMETICEXCEPTION        EArithmeticException(__FILE__, __LINE__, errno)
#define EARITHMETICEXCEPTIONS(msg)  EArithmeticException(__FILE__, __LINE__, msg)

/**
 * Thrown when an exceptional arithmetic condition has occurred. For
 * example, an integer "divide by zero" throws an
 * instance of this class.
 *
 * {@code ArithmeticException} objects may be constructed by the
 * virtual machine as if {@linkplain Throwable#Throwable(String,
 * Throwable, boolean, boolean) suppression were disabled and/or the
 * stack trace was not writable}.
 *
 * @since   JDK1.0
 */

class EArithmeticException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EArithmeticException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EArithmeticException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EArithmeticException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EArithmeticException(const char *_file_,
			int _line_, const char *s) :
				ERuntimeException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EARITHMETICEXCEPTION_H_ */
