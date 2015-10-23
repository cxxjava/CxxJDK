/*
 * EUnresolvedAddressException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EUNRESOLVEDADDRESSEXCEPTION_HH_
#define EUNRESOLVEDADDRESSEXCEPTION_HH_

#include "EIllegalArgumentException.hh"

namespace efc {
namespace nio {

#define EUNRESOLVEDADDRESSEXCEPTION       EUnresolvedAddressException(__FILE__, __LINE__, errno)
#define EUNRESOLVEDADDRESSEXCEPTIONS(msg) EUnresolvedAddressException(msg, __FILE__, __LINE__, errno)

/**
 * Unchecked exception thrown when an attempt is made to invoke a network
 * operation upon an unresolved socket address.
 */

class EUnresolvedAddressException: public EIllegalArgumentException {
public:
	/**
	 * Constructs an <code>EUnresolvedAddressException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EUnresolvedAddressException(const char *_file_, int _line_, int errn = 0) :
		EIllegalArgumentException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EUnresolvedAddressException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EUnresolvedAddressException(const char *s, const char *_file_, int _line_, int errn =
			0) :
				EIllegalArgumentException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EUNRESOLVEDADDRESSEXCEPTION_HH_ */
