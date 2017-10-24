/*
 * EOutOfMemoryError.hh
 *
 *  Created on: 2014-2-16
 *      Author: cxxjava@163.com
 */

#ifndef EOUTOFMEMORYERROR_HH_
#define EOUTOFMEMORYERROR_HH_

#include "EThrowable.hh"

namespace efc {

#define EOUTOFMEMORYERROR       EOutOfMemoryError(__FILE__, __LINE__, errno)
#define EOUTOFMEMORYERRORS(msg) EOutOfMemoryError(__FILE__, __LINE__, msg)

/**
 * The class <code>EOutOfMemoryError</code> and its subclasses are a form of
 * <code>Throwable</code> that indicates conditions that a reasonable 
 * application out of memory.
 */

class EOutOfMemoryError : public EThrowable {
public:
	/**
     * Constructs an <code>EOutOfMemoryError</code> with no specified detail message.
     *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
     */
	EOutOfMemoryError(const char *_file_, int _line_, int errn = 0) :
			EThrowable(_file_, _line_, errn) {
	}
	
	/**
     * Constructs an <code>EOutOfMemoryError</code> with the specified detail message.
     *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
     * @param   s   the detail message.
     */
	EOutOfMemoryError(const char *_file_, int _line_, const char *s, int errn = 0) :
			EThrowable(_file_, _line_, s, errn) {
	}
};

} /* namespace efc */
#endif /* EOUTOFMEMORYERROR_HH_ */
