/*
 * EUnsupportedEncodingException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EUNSUPPORTEDENCODINGEXCEPTION_H_
#define EUNSUPPORTEDENCODINGEXCEPTION_H_

#include "EIOException.hh"

namespace efc {

#define EUNSUPPORTEDENCODINGEXCEPTION        EUnsupportedEncodingException(__FILE__, __LINE__, errno)
#define EUNSUPPORTEDENCODINGEXCEPTIONS(msg)  EUnsupportedEncodingException(__FILE__, __LINE__, msg)

/**
 * The Character Encoding is not supported.
 *
 * @author  Asmus Freytag
 * @since   JDK1.1
 */

class EUnsupportedEncodingException: public EIOException {
public:
	/**
	 * Constructs an <code>EUnsupportedEncodingException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EUnsupportedEncodingException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EUnsupportedEncodingException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EUnsupportedEncodingException(const char *_file_,
			int _line_, const char *s) :
			EIOException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EUNSUPPORTEDENCODINGEXCEPTION_H_ */
