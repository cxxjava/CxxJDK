/*
 * EMalformedURLException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EMALFORMEDURLEXCEPTION_H_
#define EMALFORMEDURLEXCEPTION_H_

#include "EIOException.hh"

namespace efc {

#define EMALFORMEDURLEXCEPTION       EMalformedURLException(__FILE__, __LINE__, errno)
#define EMALFORMEDURLEXCEPTIONS(msg) EMalformedURLException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that a malformed URL has occurred. Either no
 * legal protocol could be found in a specification string or the
 * string could not be parsed.
 *
 * @since   JDK1.0
 */

class EMalformedURLException: public EIOException {
public:
	/**
	 * Constructs an <code>EMalformedURLException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EMalformedURLException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EMalformedURLException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EMalformedURLException(const char *_file_, int _line_, const char *s, int errn = 0) :
			EIOException(_file_, _line_, s, errn) {
	}
};

} /* namespace efc */
#endif /* EMALFORMEDURLEXCEPTION_H_ */
