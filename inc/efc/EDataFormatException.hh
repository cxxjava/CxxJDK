/*
 * EDataFormateException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EDATAFORMATEEXCEPTION_HH_
#define EDATAFORMATEEXCEPTION_HH_

#include "EException.hh"

namespace efc {

#define EDATAFORMATEEXCEPTION        EDataFormateException(__FILE__, __LINE__, errno)
#define EDATAFORMATEEXCEPTIONS(msg)  EDataFormateException(msg, __FILE__, __LINE__, errno)

/**
 * Signals that a data format error has occurred.
 *
 * @version 	1.14, 11/17/05
 * @author 	David Connelly
 */

class EDataFormateException: public EException {
public:
	/**
	 * Constructs an <code>EDataFormateException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EDataFormateException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EDataFormateException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EDataFormateException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EDATAFORMATEEXCEPTION_HH_ */
