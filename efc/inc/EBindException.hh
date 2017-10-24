/*
 * EBindException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EBindException_H_
#define EBindException_H_

#include "ESocketException.hh"

namespace efc {

#define EBINDEXCEPTION       EBindException(__FILE__, __LINE__, errno)
#define EBINDEXCEPTIONS(msg) EBindException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that there is an error in the underlying 
 * protocol, such as a TCP error. 
 *
 * @version 1.18, 11/17/05
 * @since   JDK1.0
 */

class EBindException: public ESocketException {
public:
	/**
	 * Constructs an <code>EBindException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EBindException(const char *_file_, int _line_, int errn = 0) :
			ESocketException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EBindException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EBindException(const char *_file_, int _line_, const char *s, int errn = 0) :
			ESocketException(_file_, _line_, s, errn) {
	}
};

} /* namespace efc */
#endif /* EBindException_H_ */
