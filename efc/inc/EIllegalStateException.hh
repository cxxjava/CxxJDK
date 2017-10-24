/*
 * EIllegalStateException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EILLEGALSTATEEXCEPTION_H_
#define EILLEGALSTATEEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define EILLEGALSTATEEXCEPTION        EIllegalStateException(__FILE__, __LINE__, errno)
#define EILLEGALSTATEEXCEPTIONS(msg)  EIllegalStateException(__FILE__, __LINE__, msg)

/**
 * Signals that a method has been invoked at an illegal or
 * inappropriate time.  In other words, the Java environment or
 * Java application is not in an appropriate state for the requested
 * operation.
 *
 * @version 1.16, 11/17/05
 * @since   JDK1.1
 */

class EIllegalStateException: public EException {
public:
	/**
	 * Constructs an <code>EIllegalStateException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EIllegalStateException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EIllegalStateException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EIllegalStateException(const char *_file_,
			int _line_, const char *s) :
			EException(_file_, _line_, s) {
	}

	/**
	 * Constructs an <code>EIllegalStateException</code> with the specified detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	EIllegalStateException(const char *_file_, int _line_, EThrowable* cause) :
			EException(_file_, _line_, cause) {
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
	EIllegalStateException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
			EException(_file_, _line_, s, cause) {
	}
};

} /* namespace efc */
#endif /* EILLEGALSTATEEXCEPTION_H_ */
