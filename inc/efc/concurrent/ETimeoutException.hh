/*
 * ETimeoutException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ETIMEOUTEXCEPTION_H_
#define ETIMEOUTEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define ETIMEOUTEXCEPTION       ETimeoutException(__FILE__, __LINE__, errno)
#define ETIMEOUTEXCEPTIONS(msg) ETimeoutException(msg, __FILE__, __LINE__, errno)

/**
 * Exception thrown when a blocking operation times out.  Blocking
 * operations for which a timeout is specified need a means to
 * indicate that the timeout has occurred. For many such operations it
 * is possible to return a value that indicates timeout; when that is
 * not possible or desirable then <tt>TimeoutException</tt> should be
 * declared and thrown.
 *
 * @since 1.5
 * @author Doug Lea
 */

class ETimeoutException: public EException {
public:
	/**
	 * Constructs an <code>ETimeoutException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ETimeoutException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ETimeoutException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ETimeoutException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ETIMEOUTEXCEPTION_H_ */
