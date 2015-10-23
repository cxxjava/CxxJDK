/*
 * ECancellationException.hh
 *
 *  Created on: 2013-12-16
 *      Author: cxxjava@163.com
 */

#ifndef ECANCELLATIONEXCEPTION_HH_
#define ECANCELLATIONEXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {

#define ECANCELLATIONEXCEPTION       ECancellationException(__FILE__, __LINE__, errno)
#define ECANCELLATIONEXCEPTIONS(msg) ECancellationException(msg, __FILE__, __LINE__, errno)

/**
 * Unchecked exception thrown when an attempt is made to read
 * from a channel that was not originally opened for reading.
 */

class ECancellationException: public EIllegalStateException {
public:
	/**
	 * Constructs an <code>ECancellationException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ECancellationException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ECancellationException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ECancellationException(const char *s, const char *_file_, int _line_,
			int errn = 0) :
			EIllegalStateException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ECANCELLATIONEXCEPTION_HH_ */
