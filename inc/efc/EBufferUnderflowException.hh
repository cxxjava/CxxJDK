/*
 * EBufferUnderflowException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EBUFFERUNDERFLOWEXCEPTION_HH_
#define EBUFFERUNDERFLOWEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {
namespace nio {

#define EBUFFERUNDERFLOWEXCEPTION        EBufferUnderflowException(__FILE__, __LINE__, errno)
#define EBUFFERUNDERFLOWEXCEPTIONS(msg)  EBufferUnderflowException(msg, __FILE__, __LINE__, errno)


/**
 * Unchecked exception thrown when a relative <i>get</i> operation reaches
 * the source buffer's limit.
 */

class EBufferUnderflowException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EBufferUnderflowException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EBufferUnderflowException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EBufferUnderflowException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EBufferUnderflowException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				ERuntimeException(s, _file_, _line_, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EBUFFERUNDERFLOWEXCEPTION_HH_ */
