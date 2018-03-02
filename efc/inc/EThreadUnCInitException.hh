/*
 * EThreadUnCInitException.hh
 *
 *  Created on: 2017-10-26
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADUNCINITEXCEPTION_H_
#define ETHREADUNCINITEXCEPTION_H_

#include "ERuntimeException.hh"

namespace efc {

#define ETHREADUNCINITEXCEPTION        EThreadUnCInitException(__FILE__, __LINE__, errno)
#define ETHREADUNCINITEXCEPTIONS(msg)  EThreadUnCInitException(__FILE__, __LINE__, msg)

/**
 * Thrown this exception when a native thread have not called EThread::c_init().
 */

class EThreadUnCInitException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EThreadUnCInitException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EThreadUnCInitException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EThreadUnCInitException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EThreadUnCInitException(const char *_file_,
			int _line_, const char *s) :
				ERuntimeException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* ETHREADUNCINITEXCEPTION_H_ */
