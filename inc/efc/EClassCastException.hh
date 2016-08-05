/*
 * EClassCastException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ECLASSCASTEXCEPTION_H_
#define ECLASSCASTEXCEPTION_H_

#include "ERuntimeException.hh"

namespace efc {

#define ECLASSCASTEXCEPTION        EClassCastException(__FILE__, __LINE__, errno)
#define ECLASSCASTEXCEPTIONS(msg)  EClassCastException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that the code has attempted to cast an object
 * to a subclass of which it is not an instance. For example, the
 * following code generates a <code>ClassCastException</code>:
 * <p><blockquote><pre>
 *     Object x = new Integer(0);
 *     System.out.println((String)x);
 * </pre></blockquote>
 *
 * @since   JDK1.0
 */

class EClassCastException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EClassCastException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EClassCastException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EClassCastException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EClassCastException(const char *_file_,
			int _line_, const char *s) :
				ERuntimeException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* ECLASSCASTEXCEPTION_H_ */
