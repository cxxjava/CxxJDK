/*
 * ECancelledKeyException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ECANCELLEDKEYEXCEPTION_HH_
#define ECANCELLEDKEYEXCEPTION_HH_

#include "../../inc/EIllegalStateException.hh"

namespace efc {
namespace nio {

#define ECANCELLEDKEYEXCEPTION        ECancelledKeyException(__FILE__, __LINE__, errno)
#define ECANCELLEDKEYEXCEPTIONS(msg)  ECancelledKeyException(__FILE__, __LINE__, msg)

/**
 * Unchecked exception thrown when an attempt is made to use
 * a selection key that is no longer valid.
 *
 * @version 1.9, 01/11/19
 * @since 1.4
 */

class ECancelledKeyException : public EIllegalStateException {
public:
	/**
	 * Constructs an <code>ECancelledKeyException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ECancelledKeyException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ECancelledKeyException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ECancelledKeyException(const char *_file_,
			int _line_, const char *s) :
				EIllegalStateException(_file_, _line_, s) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* ECANCELLEDKEYEXCEPTION_HH_ */
