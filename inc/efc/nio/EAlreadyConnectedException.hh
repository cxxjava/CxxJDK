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
#define EALREADYCONNECTEDEXCEPTIONS(msg)  EAlreadyConnectedException(__FILE__, __LINE__, msg)

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
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EAlreadyConnectedException(const char *_file_,
			int _line_, const char *s) :
				EIllegalStateException(_file_, _line_, s) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EALREADYCONNECTEDEXCEPTION_HH_ */
