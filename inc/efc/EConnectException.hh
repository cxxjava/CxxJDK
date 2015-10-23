/*
 * EConnectException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EConnectException_H_
#define EConnectException_H_

#include "ESocketException.hh"

namespace efc {

#define ECONNECTEXCEPTION       EConnectException(__FILE__, __LINE__, errno)
#define ECONNECTEXCEPTIONS(msg) EConnectException(msg, __FILE__, __LINE__, errno)

/**
 * Signals that an error occurred while attempting to connect a
 * socket to a remote address and port.  Typically, the connection
 * was refused remotely (e.g., no process is listening on the
 * remote address/port).
 *
 * @since   JDK1.1
 */

class EConnectException: public ESocketException {
public:
	/**
	 * Constructs an <code>EConnectException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EConnectException(const char *_file_, int _line_, int errn = 0) :
			ESocketException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EConnectException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EConnectException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			ESocketException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EConnectException_H_ */
