/*
 * EEmptyStackException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EEMPTYSTACKEXCEPTION_H_
#define EEMPTYSTACKEXCEPTION_H_

#include "ERuntimeException.hh"

namespace efc {

#define EEMPTYSTACKEXCEPTION        EEmptyStackException(__FILE__, __LINE__, errno)
#define EEMPTYSTACKEXCEPTIONS(msg)  EEmptyStackException(msg, __FILE__, __LINE__, errno)

/**
 * Thrown by methods in the <code>Stack</code> class to indicate
 * that the stack is empty.
 *
 * @author  Jonathan Payne
 * @see     java.util.Stack
 * @since   JDK1.0
 */

class EEmptyStackException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EEmptyStackException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EEmptyStackException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EEmptyStackException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EEmptyStackException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				ERuntimeException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EEMPTYSTACKEXCEPTION_H_ */
