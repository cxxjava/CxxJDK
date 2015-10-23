/*
 * EClosedSelectorException.hh
 *
 *  Created on: 2013-12-18
 *      Author: cxxjava@163.com
 */

#ifndef ECLOSEDSELECTOREXCEPTION_HH_
#define ECLOSEDSELECTOREXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {
namespace nio {

#define ECLOSEDSELECTOREXCEPTION        EClosedSelectorException(__FILE__, __LINE__, errno)
#define ECLOSEDSELECTOREXCEPTIONS(msg)  EClosedSelectorException(msg, __FILE__, __LINE__, errno)

/**
 * Unchecked exception thrown when an attempt is made to invoke an I/O
 * operation upon a closed selector.
 *
 * @version 1.9, 01/11/19
 * @since 1.4
 */

class EClosedSelectorException : public EIllegalStateException {
public:
	/**
	 * Constructs an <code>EClosedSelectorException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EClosedSelectorException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EClosedSelectorException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EClosedSelectorException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				EIllegalStateException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* ECLOSEDSELECTOREXCEPTION_HH_ */
