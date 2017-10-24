/*
 * ESecurityException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ESECURITYEXCEPTION_HH_
#define ESECURITYEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {

#define ESECURITYEXCEPTION        ESecurityException(__FILE__, __LINE__, errno)
#define ESECURITYEXCEPTIONS(msg)  ESecurityException(__FILE__, __LINE__, msg)

/**
 * Thrown by the security manager to indicate a security violation.
 *
 * @see     java.lang.SecurityManager
 * @since   JDK1.0
 */

class ESecurityException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>ESecurityException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ESecurityException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ESecurityException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ESecurityException(const char *_file_, int _line_, const char *s, int errn = 0) :
		ERuntimeException(_file_, _line_, s, errn) {
	}

	/**
	 * Constructs an <code>ESecurityException</code> with the specified detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	ESecurityException(const char *_file_, int _line_, EThrowable* cause) :
		ERuntimeException(_file_, _line_, cause) {
	}

	/**
	 * Constructs a new exception with the specified detail message and
	 * cause.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   s   the detail message.
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	ESecurityException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
		ERuntimeException(_file_, _line_, s, cause) {
	}
};

} /* namespace efc */
#endif /* ESECURITYEXCEPTION_HH_ */
