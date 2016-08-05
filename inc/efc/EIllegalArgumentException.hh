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
#define EILLEGALARGUMENTEXCEPTIONS(msg) EIllegalArgumentException(__FILE__, __LINE__, msg)

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
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EIllegalArgumentException(const char *_file_, int _line_, const char *s, int errn = 0) :
			EException(_file_, _line_, s, errn) {
	}

	/**
	 * Constructs an <code>EIllegalArgumentException</code> with the specified detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	EIllegalArgumentException(const char *_file_, int _line_, EThrowable* cause) :
			EException(_file_, _line_, cause) {
	}

	/**
	 * Constructs a new exception with the specified detail message and
	 * cause.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   s   the detail message.
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	EIllegalArgumentException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
			EException(_file_, _line_, s, cause) {
	}
};

} /* namespace efc */
#endif /* EILLEGALARGUMENTEXCEPTION_H_ */
