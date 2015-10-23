/*
 * EConnectionPendingException.hh
 *
 *  Created on: 2013-12-18
 *      Author: cxxjava@163.com
 */

#ifndef ECONNECTIONPENDINGEXCEPTION_HH_
#define ECONNECTIONPENDINGEXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {
namespace nio {

#define ECONNECTIONPENDINGEXCEPTION        EConnectionPendingException(__FILE__, __LINE__, errno)
#define ECONNECTIONPENDINGEXCEPTIONS(msg)  EConnectionPendingException(msg, __FILE__, __LINE__, errno)

/**
 * Unchecked exception thrown when an attempt is made to connect a {@link
 * SocketChannel} for which a non-blocking connection operation is already in
 * progress.
 */

class EConnectionPendingException : public EIllegalStateException {
public:
	/**
	 * Constructs an <code>EConnectionPendingException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EConnectionPendingException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EConnectionPendingException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EConnectionPendingException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				EIllegalStateException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* ECONNECTIONPENDINGEXCEPTION_HH_ */
