/*
 * EInvalidMarkException.hh
 *
 *  Created on: 2013-12-16
 *      Author: cxxjava@163.com
 */

#ifndef EINVALIDMARKEXCEPTION_HH_
#define EINVALIDMARKEXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {

#define EINVALIDMARKEXCEPTION       EInvalidMarkException(__FILE__, __LINE__, errno)
#define EINVALIDMARKEXCEPTIONS(msg) EInvalidMarkException(__FILE__, __LINE__, msg)

/**
 * Unchecked exception thrown when an attempt is made to reset a buffer
 * when its mark is not defined.
 *
 * @version 1.14, 01/05/02
 * @since 1.4
 */

class EInvalidMarkException: public EIllegalStateException {
public:
	/**
	 * Constructs an <code>EInvalidMarkException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EInvalidMarkException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EInvalidMarkException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EInvalidMarkException(const char *_file_, int _line_,
			const char *s) :
			EIllegalStateException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EINVALIDMARKEXCEPTION_HH_ */
