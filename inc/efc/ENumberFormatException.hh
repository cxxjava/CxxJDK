/*
 * ENumberFormatException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ENUMBERFORMATEXCEPTION_H_
#define ENUMBERFORMATEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define ENUMBERFORMATEXCEPTION       ENumberFormatException(__FILE__, __LINE__, errno)
#define ENUMBERFORMATEXCEPTIONS(msg) ENumberFormatException(msg, __FILE__, __LINE__, errno)

/**
 * Thrown to indicate that the application has attempted to convert 
 * a string to one of the numeric types, but that the string does not 
 * have the appropriate format. 
 */

class ENumberFormatException: public EException {
public:
	/**
	 * Constructs an <code>ENumberFormatException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ENumberFormatException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>IllegalArgumentException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	ENumberFormatException(const char *s, const char *_file_, int _line_,
			int errn = 0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ENUMBERFORMATEXCEPTION_H_ */
