/*
 * EIllegalArgumentException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EILLEGALARGUMENTEXCEPTION_H_
#define EILLEGALARGUMENTEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define EILLEGALARGUMENTEXCEPTION       EIllegalArgumentException(__FILE__, __LINE__, errno)
#define EILLEGALARGUMENTEXCEPTIONS(msg) EIllegalArgumentException(msg, __FILE__, __LINE__, errno)

/**
 * Thrown to indicate that a method has been passed an illegal or 
 * inappropriate argument.
 */

class EIllegalArgumentException: public EException {
public:
	/**
     * Constructs an <code>IllegalArgumentException</code> with no 
     * detail message. 
     *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
     */
	EIllegalArgumentException(const char *_file_, int _line_, int errn = 0) :
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
	EIllegalArgumentException(const char *s, const char *_file_, int _line_,
			int errn = 0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EILLEGALARGUMENTEXCEPTION_H_ */
