/*
 * EReadOnlyBufferException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EREADONLYBUFFEREXCEPTION_HH_
#define EREADONLYBUFFEREXCEPTION_HH_

#include "EUnsupportedOperationException.hh"

namespace efc {
namespace nio {

#define EREADONLYBUFFEREXCEPTION       EReadOnlyBufferException(__FILE__, __LINE__, errno)
#define EREADONLYBUFFEREXCEPTIONS(msg) EReadOnlyBufferException(msg, __FILE__, __LINE__, errno)

/**
 * Unchecked exception thrown when a content-mutation method such as
 * <tt>put</tt> or <tt>compact</tt> is invoked upon a read-only buffer.
 */

class EReadOnlyBufferException: public EUnsupportedOperationException {
public:
	/**
	 * Constructs an <code>EReadOnlyBufferException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EReadOnlyBufferException(const char *_file_, int _line_, int errn = 0) :
		EUnsupportedOperationException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EReadOnlyBufferException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EReadOnlyBufferException(const char *s, const char *_file_, int _line_, int errn =
			0) :
				EUnsupportedOperationException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EREADONLYBUFFEREXCEPTION_HH_ */
