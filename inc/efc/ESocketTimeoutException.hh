/*
 * ESocketTimeoutException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ESOCKETTIMEOUTEXCEPTION_HH_
#define ESOCKETTIMEOUTEXCEPTION_HH_

#include "EInterruptedIOException.hh"

namespace efc {

#define ESOCKETTIMEOUTEXCEPTION        ESocketTimeoutException(__FILE__, __LINE__, errno)
#define ESOCKETTIMEOUTEXCEPTIONS(msg)  ESocketTimeoutException(msg, __FILE__, __LINE__, errno)

/**
 * Signals that a timeout has occurred on a socket read or accept.
 *
 * @since   1.4
 */

class ESocketTimeoutException: public EInterruptedIOException {
public:
	/**
	 * Constructs an <code>ESocketTimeoutException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ESocketTimeoutException(const char *_file_, int _line_, int errn = 0) :
		EInterruptedIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ESocketTimeoutException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ESocketTimeoutException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				EInterruptedIOException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ESOCKETTIMEOUTEXCEPTION_HH_ */
