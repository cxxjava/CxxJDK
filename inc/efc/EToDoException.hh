/*
 * EToDoException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ETODOEXCEPTION_H_
#define ETODOEXCEPTION_H_

#include "EException.hh"

namespace efc {

#define ETODOEXCEPTION       EToDoException(__FILE__, __LINE__, errno)
#define ETODOEXCEPTIONS(msg) EToDoException(msg, __FILE__, __LINE__, errno)

/**
 * <code>EToDoException</code> means this need to do.
 */

class EToDoException: public EException {
public:
	/**
	 * Constructs an <code>EToDoException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EToDoException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EToDoException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EToDoException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ETODOEXCEPTION_H_ */
