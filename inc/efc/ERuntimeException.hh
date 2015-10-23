/*
 * ERuntimeException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ERUNTIMEEXCEPTION_H_
#define ERUNTIMEEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define ERUNTIMEEXCEPTION       ERuntimeException(__FILE__, __LINE__, errno)
#define ERUNTIMEEXCEPTIONS(msg) ERuntimeException(msg, __FILE__, __LINE__, errno)

/**
 * <code>RuntimeException</code> is the superclass of those
 * exceptions that can be thrown during the normal operation of the
 * Java Virtual Machine.
 * <p>
 * A method is not required to declare in its <code>throws</code>
 * clause any subclasses of <code>RuntimeException</code> that might
 * be thrown during the execution of the method but not caught.
 */

class ERuntimeException: public EException {
public:
	/**
	 * Constructs an <code>ERuntimeException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ERuntimeException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ERuntimeException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ERuntimeException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ERUNTIMEEXCEPTION_H_ */
