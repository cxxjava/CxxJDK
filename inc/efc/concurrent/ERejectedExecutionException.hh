/*
 * ERejectedExecutionException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EREJECTEDEXECUTIONEXCEPTION_HH_
#define EREJECTEDEXECUTIONEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {

#define EREJECTEDEXECUTIONEXCEPTION        ERejectedExecutionException(__FILE__, __LINE__, errno)
#define EREJECTEDEXECUTIONEXCEPTIONS(msg)  ERejectedExecutionException(msg, __FILE__, __LINE__, errno)

/**
 * Exception thrown by an {@link Executor} when a task cannot be
 * accepted for execution.
 *
 * @since 1.5
 * @author Doug Lea
 */

class ERejectedExecutionException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>ERejectedExecutionException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ERejectedExecutionException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ERejectedExecutionException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ERejectedExecutionException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				ERuntimeException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EREJECTEDEXECUTIONEXCEPTION_HH_ */
