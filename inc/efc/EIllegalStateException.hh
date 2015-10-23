/*
 * EIllegalStateException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EILLEGALSTATEEXCEPTION_H_
#define EILLEGALSTATEEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define EILLEGALSTATEEXCEPTION        EIllegalStateException(__FILE__, __LINE__, errno)
#define EILLEGALSTATEEXCEPTIONS(msg)  EIllegalStateException(msg, __FILE__, __LINE__, errno)

/**
 * Signals that a method has been invoked at an illegal or
 * inappropriate time.  In other words, the Java environment or
 * Java application is not in an appropriate state for the requested
 * operation.
 *
 * @author  Jonni Kanerva
 * @version 1.16, 11/17/05
 * @since   JDK1.1
 */

class EIllegalStateException: public EException {
public:
	/**
	 * Constructs an <code>EIllegalStateException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EIllegalStateException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EIllegalStateException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EIllegalStateException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EILLEGALSTATEEXCEPTION_H_ */
