/*
 * EIOException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EIOEXCEPTION_H_
#define EIOEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define EIOEXCEPTION       EIOException(__FILE__, __LINE__, errno)
#define EIOEXCEPTIONS(msg) EIOException(msg, __FILE__, __LINE__, errno)

/**
 * Signals that an I/O exception of some sort has occurred. This
 * class is the general class of exceptions produced by failed or
 * interrupted I/O operations.
 */

class EIOException: public EException {
public:
	/**
	 * Constructs an <code>EIOException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EIOException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EIOException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EIOException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EIOEXCEPTION_H_ */
