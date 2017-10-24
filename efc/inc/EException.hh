/*
 * EException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EEXCEPTION_H_
#define EEXCEPTION_H_

#include "EThrowable.hh"

namespace efc {

#define EEXCEPTION       EException(__FILE__, __LINE__, errno)
#define EEXCEPTIONS(msg) EException(__FILE__, __LINE__, msg)

/**
 * The class <code>Exception</code> and its subclasses are a form of 
 * <code>Throwable</code> that indicates conditions that a reasonable 
 * application might want to catch.
 */

class EException : public EThrowable {
public:
	/**
     * Constructs an <code>Exception</code> with no specified detail message. 
     *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
     */
	EException(const char *_file_, int _line_, int errn = 0) :
			EThrowable(_file_, _line_, errn) {
	}
	
	/**
     * Constructs an <code>Exception</code> with the specified detail message. 
     *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
     * @param   s   the detail message.
     */
	EException(const char *_file_, int _line_, const char *s, int errn = 0) :
			EThrowable(_file_, _line_, s, errn) {
	}

	/**
	 * Constructs an <code>Exception</code> with the specified detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   cause    the cause (which is saved for later retrieval by the
     *         {@link #getCause()} method).  (A {@code null} value is
     *         permitted, and indicates that the cause is nonexistent or
     *         unknown.)
	 */
	EException(const char *_file_, int _line_, EThrowable* cause) :
			EThrowable(_file_, _line_, cause) {
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
	EException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
			EThrowable(_file_, _line_, s, cause) {
	}
};

} /* namespace efc */
#endif /* EEXCEPTION_H_ */
