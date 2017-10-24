/*
 * EExecutionException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EEXECUTIONEXCEPTION_H_
#define EEXECUTIONEXCEPTION_H_

#include "../EException.hh"

namespace efc {

#define EEXECUTIONEXCEPTION       EExecutionException(__FILE__, __LINE__, errno)
#define EEXECUTIONEXCEPTIONS(msg) EExecutionException(__FILE__, __LINE__, msg)

/**
 * Exception thrown when attempting to retrieve the result of a task
 * that aborted by throwing an exception. This exception can be
 * inspected using the {@link #getCause()} method.
 *
 * @see Future
 * @since 1.5
 */

class EExecutionException: public EException {
public:
	/**
	 * Constructs an <code>EExecutionException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EExecutionException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EExecutionException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EExecutionException(const char *_file_, int _line_, const char *s, int errn = 0) :
			EException(_file_, _line_, s, errn) {
	}

	/**
	 * Constructs an <code>EExecutionException</code> with the specified detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	EExecutionException(const char *_file_, int _line_, EThrowable* cause) :
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
	EExecutionException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
			EException(_file_, _line_, s, cause) {
	}
};

} /* namespace efc */
#endif /* EEXECUTIONEXCEPTION_H_ */
