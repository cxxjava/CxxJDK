/*
 * EBufferOverflowException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EBUFFEROVERFLOWEXCEPTION_HH_
#define EBUFFEROVERFLOWEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {
namespace nio {

#define EBUFFEROVERFLOWEXCEPTION        EBufferOverflowException(__FILE__, __LINE__, errno)
#define EBUFFEROVERFLOWEXCEPTIONS(msg)  EBufferOverflowException(msg, __FILE__, __LINE__, errno)


/**
 * Unchecked exception thrown when a relative <i>put</i> operation reaches
 * the target buffer's limit.
 */

class EBufferOverflowException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EBufferOverflowException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EBufferOverflowException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EBufferOverflowException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EBufferOverflowException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				ERuntimeException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EBUFFEROVERFLOWEXCEPTION_HH_ */
