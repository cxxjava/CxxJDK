/*
 * EClosedChannelException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EClosedChannelException_H_
#define EClosedChannelException_H_

#include "EIOException.hh"

namespace efc {
namespace nio {

#define ECLOSEDCHANNELEXCEPTION       EClosedChannelException(__FILE__, __LINE__, errno)
#define ECLOSEDCHANNELEXCEPTIONS(msg) EClosedChannelException(__FILE__, __LINE__, msg)

/**
 * Checked exception thrown when an attempt is made to invoke or complete an
 * I/O operation upon channel that is closed, or at least closed to that
 * operation.  That this exception is thrown does not necessarily imply that
 * the channel is completely closed.  A socket channel whose write half has
 * been shut down, for example, may still be open for reading.
 *
 * @version 1.9, 01/11/19
 * @since 1.4
 */

class EClosedChannelException: public EIOException {
public:
	/**
	 * Constructs an <code>EClosedChannelException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EClosedChannelException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EClosedChannelException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EClosedChannelException(const char *_file_, int _line_, const char *s, int errn = 0) :
			EIOException(_file_, _line_, s, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EClosedChannelException_H_ */
