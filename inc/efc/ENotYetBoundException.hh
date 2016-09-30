/*
 * ENotYetBoundException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ENOTYETBOUNDEXCEPTION_HH_
#define ENOTYETBOUNDEXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {

#define ENOTYETBOUNDEXCEPTION       ENotYetBoundException(__FILE__, __LINE__, errno)
#define ENOTYETBOUNDEXCEPTIONS(msg) ENotYetBoundException(__FILE__, __LINE__, msg)

/**
 * Unchecked exception thrown when an attempt is made to invoke an I/O
 * operation upon a server socket channel that is not yet bound.
 */

class ENotYetBoundException: public EIllegalStateException {
public:
	/**
	 * Constructs an <code>ENotYetBoundException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ENotYetBoundException(const char *_file_, int _line_, int errn = 0) :
			EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ENotYetBoundException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ENotYetBoundException(const char *_file_, int _line_, const char *s) :
			EIllegalStateException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* ENOTYETBOUNDEXCEPTION_HH_ */
