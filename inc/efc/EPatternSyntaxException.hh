/*
 * EPatternSyntaxException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EPatternSyntaxException_HH_
#define EPatternSyntaxException_HH_

#include "EException.hh"

namespace efc {

#define EPATTERNSYNTAXEXCEPTION       EPatternSyntaxException(__FILE__, __LINE__, errno)
#define EPATTERNSYNTAXEXCEPTIONS(msg) EPatternSyntaxException(msg, __FILE__, __LINE__, errno)

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
 * @author  unascribed
 * @version 1.20, 11/17/05
 * @since   JDK1.0
 */

class EPatternSyntaxException: public EException {
public:
	/**
	 * Constructs an <code>EPatternSyntaxException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EPatternSyntaxException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EPatternSyntaxException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EPatternSyntaxException(const char *s, const char *_file_, int _line_,
			int errn = 0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EPatternSyntaxException_HH_ */
