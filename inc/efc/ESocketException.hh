/*
 * ESocketException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ESocketException_H_
#define ESocketException_H_

#include "EIOException.hh"

namespace efc {

#define ESOCKETEXCEPTION       ESocketException(__FILE__, __LINE__, errno)
#define ESOCKETEXCEPTIONS(msg) ESocketException(msg, __FILE__, __LINE__, errno)

/**
 * Thrown to indicate that there is an error in the underlying 
 * protocol, such as a TCP error. 
 *
 * @author  Jonathan Payne
 * @version 1.18, 11/17/05
 * @since   JDK1.0
 */

class ESocketException: public EIOException {
public:
	/**
	 * Constructs an <code>ESocketException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ESocketException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ESocketException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ESocketException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EIOException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ESocketException_H_ */
