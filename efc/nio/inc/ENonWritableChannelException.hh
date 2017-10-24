/*
 * ENonWritableChannelException.hh
 *
 *  Created on: 2013-12-16
 *      Author: cxxjava@163.com
 */

#ifndef ENONWRITABLECHANNELEXCEPTION_HH_
#define ENONWRITABLECHANNELEXCEPTION_HH_

#include "../../inc/EIllegalStateException.hh"

namespace efc {
namespace nio {

#define ENONWRITABLECHANNELEXCEPTION       ENonWritableChannelException(__FILE__, __LINE__, errno)
#define ENONWRITABLECHANNELEXCEPTIONS(msg) ENonWritableChannelException(__FILE__, __LINE__, msg)

/**
 * Unchecked exception thrown when an attempt is made to write
 * to a channel that was not originally opened for writing.
 */

class ENonWritableChannelException: public EIllegalStateException {
public:
	/**
	 * Constructs an <code>ENonWritableChannelException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ENonWritableChannelException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ENonWritableChannelException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ENonWritableChannelException(const char *_file_, int _line_,
			const char *s) :
			EIllegalStateException(_file_, _line_, s) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* ENONWRITABLECHANNELEXCEPTION_HH_ */
