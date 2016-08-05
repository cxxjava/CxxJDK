/*
 * ENoSuchElementException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ENOSUCHELEMENTEXCEPTION_H_
#define ENOSUCHELEMENTEXCEPTION_H_

#include "ERuntimeException.hh"

namespace efc {

#define ENOSUCHELEMENTEXCEPTION        ENoSuchElementException(__FILE__, __LINE__, errno)
#define ENOSUCHELEMENTEXCEPTIONS(msg)  ENoSuchElementException(__FILE__, __LINE__, msg)

/**
 * Thrown by the <code>nextElement</code> method of an
 * <code>Enumeration</code> to indicate that there are no more
 * elements in the enumeration.
 *
 * @see     java.util.Enumeration
 * @see     java.util.Enumeration#nextElement()
 * @since   JDK1.0
 */

class ENoSuchElementException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>ENoSuchElementException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ENoSuchElementException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ENoSuchElementException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ENoSuchElementException(const char *_file_,
			int _line_, const char *s) :
				ERuntimeException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* ENOSUCHELEMENTEXCEPTION_H_ */
