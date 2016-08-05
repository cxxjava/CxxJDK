/*
 * ENullPointerException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ENullPointerException_HH_
#define ENullPointerException_HH_

#include "EException.hh"

namespace efc {

#define ENULLPOINTEREXCEPTION       ENullPointerException(__FILE__, __LINE__, errno)
#define ENULLPOINTEREXCEPTIONS(msg) ENullPointerException(__FILE__, __LINE__, msg)

/**
 * Thrown when an application attempts to use <code>null</code> in a 
 * case where an object is required. These include: 
 * <ul>
 * <li>Calling the instance method of a <code>null</code> object. 
 * <li>Accessing or modifying the field of a <code>null</code> object. 
 * <li>Taking the length of <code>null</code> as if it were an array. 
 * <li>Accessing or modifying the slots of <code>null</code> as if it 
 *     were an array. 
 * <li>Throwing <code>null</code> as if it were a <code>Throwable</code> 
 *     value. 
 * </ul>
 * <p>
 * Applications should throw instances of this class to indicate 
 * other illegal uses of the <code>null</code> object. 
 *
 * @version 1.20, 11/17/05
 * @since   JDK1.0
 */

class ENullPointerException: public EException {
public:
	/**
	 * Constructs an <code>ENullPointerException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ENullPointerException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>NullPointerException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ENullPointerException(const char *_file_, int _line_,
			const char *s) :
			EException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* ENullPointerException_HH_ */
