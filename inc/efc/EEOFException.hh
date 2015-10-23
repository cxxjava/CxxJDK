/*
 * EEOFException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EEOFEXCEPTION_HH_
#define EEOFEXCEPTION_HH_

#include "EIOException.hh"

namespace efc {

#define EEOFEXCEPTION       EEOFException(__FILE__, __LINE__, errno)
#define EEOFEXCEPTIONS(msg) EEOFException(msg, __FILE__, __LINE__, errno)

/**
 * Signals that an end of file or end of stream has been reached
 * unexpectedly during input.
 * <p>
 * This exception is mainly used by data input streams to signal end of
 * stream. Note that many other input operations return a special value on
 * end of stream rather than throwing an exception.
 * <p>
 *
 * @author  Frank Yellin
 * @see     java.io.DataInputStream
 * @see     java.io.IOException
 * @since   JDK1.0
 */

class EEOFException: public EIOException {
public:
	/**
	 * Constructs an <code>EEOFException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EEOFException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EEOFException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EEOFException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EIOException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EEOFEXCEPTION_HH_ */
