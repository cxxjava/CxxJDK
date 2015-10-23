/*
 * EExecutionException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EEXECUTIONEXCEPTION_H_
#define EEXECUTIONEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define EEXECUTIONEXCEPTION       EExecutionException(__FILE__, __LINE__, errno)
#define EEXECUTIONEXCEPTIONS(msg) EExecutionException(msg, __FILE__, __LINE__, errno)

/**
 * Exception thrown when attempting to retrieve the result of a task
 * that aborted by throwing an exception. This exception can be
 * inspected using the {@link #getCause()} method.
 *
 * @see Future
 * @since 1.5
 * @author Doug Lea
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
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EExecutionException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EEXECUTIONEXCEPTION_H_ */
