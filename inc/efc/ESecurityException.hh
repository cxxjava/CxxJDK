/*
 * ESecurityException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ESECURITYEXCEPTION_HH_
#define ESECURITYEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {

#define ESECURITYEXCEPTION        ESecurityException(__FILE__, __LINE__, errno)
#define ESECURITYEXCEPTIONS(msg)  ESecurityException(msg, __FILE__, __LINE__, errno)

/**
 * Thrown by the security manager to indicate a security violation.
 *
 * @author  unascribed
 * @see     java.lang.SecurityManager
 * @since   JDK1.0
 */

class ESecurityException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>ESecurityException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ESecurityException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ESecurityException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ESecurityException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				ERuntimeException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ESECURITYEXCEPTION_HH_ */
