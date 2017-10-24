/*
 * EUnsupportedOperationException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EUNSUPPORTEDOPERATIONEXCEPTION_HH_
#define EUNSUPPORTEDOPERATIONEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {

#define EUNSUPPORTEDOPERATIONEXCEPTION        EUnsupportedOperationException(__FILE__, __LINE__, errno)
#define EUNSUPPORTEDOPERATIONEXCEPTIONS(msg)  EUnsupportedOperationException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that the requested operation is not supported.<p>
 *
 * This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since   1.2
 */

class EUnsupportedOperationException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EUnsupportedOperationException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EUnsupportedOperationException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EUnsupportedOperationException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EUnsupportedOperationException(const char *_file_, int _line_, const char *s, int errn = 0) :
		ERuntimeException(_file_, _line_, s, errn) {
	}

	/**
	 * Constructs an <code>EUnsupportedOperationException</code> with the specified detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	EUnsupportedOperationException(const char *_file_, int _line_, EThrowable* cause) :
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
	EUnsupportedOperationException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
		ERuntimeException(_file_, _line_, s, cause) {
	}
};

} /* namespace efc */
#endif /* EUNSUPPORTEDOPERATIONEXCEPTION_HH_ */
