/*
 * ENotYetConnectedException.hh
 *
 *  Created on: 2013-12-18
 *      Author: cxxjava@163.com
 */

#ifndef ENOTYETCONNECTEDEXCEPTION_HH_
#define ENOTYETCONNECTEDEXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {
namespace nio {

#define ENOTYETCONNECTEDEXCEPTION        ENotYetConnectedException(__FILE__, __LINE__, errno)
#define ENOTYETCONNECTEDEXCEPTIONS(msg)  ENotYetConnectedException(msg, __FILE__, __LINE__, errno)

/**
 * Unchecked exception thrown when an attempt is made to invoke an I/O
 * operation upon a socket channel that is not yet connected.
 */

class ENotYetConnectedException : public EIllegalStateException {
public:
	/**
	 * Constructs an <code>ENotYetConnectedException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ENotYetConnectedException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ENotYetConnectedException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ENotYetConnectedException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				EIllegalStateException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* ENOTYETCONNECTEDEXCEPTION_HH_ */
