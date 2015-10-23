/*
 * EAlreadyConnectedException.hh
 *
 *  Created on: 2013-12-18
 *      Author: cxxjava@163.com
 */

#ifndef EALREADYCONNECTEDEXCEPTION_HH_
#define EALREADYCONNECTEDEXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {
namespace nio {

#define EALREADYCONNECTEDEXCEPTION        EAlreadyConnectedException(__FILE__, __LINE__, errno)
#define EALREADYCONNECTEDEXCEPTIONS(msg)  EAlreadyConnectedException(msg, __FILE__, __LINE__, errno)

/**
 * Unchecked exception thrown when an attempt is made to connect a {@link
 * SocketChannel} that is already connected.
 */

class EAlreadyConnectedException : public EIllegalStateException {
public:
	/**
	 * Constructs an <code>EAlreadyConnectedException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EAlreadyConnectedException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EAlreadyConnectedException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EAlreadyConnectedException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				EIllegalStateException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EALREADYCONNECTEDEXCEPTION_HH_ */
