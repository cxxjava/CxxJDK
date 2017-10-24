/*
 * EDataFormatException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EDATAFORMATEXCEPTION_HH_
#define EDATAFORMATEXCEPTION_HH_

#include "EException.hh"

namespace efc {

#define EDATAFORMATEXCEPTION        EDataFormatException(__FILE__, __LINE__, errno)
#define EDATAFORMATEXCEPTIONS(msg)  EDataFormatException(__FILE__, __LINE__, msg)

/**
 * Signals that a data format error has occurred.
 *
 * @version 	1.14, 11/17/05
 */

class EDataFormatException: public EException {
public:
	/**
	 * Constructs an <code>EDataFormatException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EDataFormatException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EDataFormatException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EDataFormatException(const char *_file_,
			int _line_, const char *s) :
			EException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EDATAFORMATEXCEPTION_HH_ */
