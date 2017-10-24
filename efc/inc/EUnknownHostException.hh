/*
 * EUnknownHostException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EUNKNOWNHOSTEXCEPTION_HH_
#define EUNKNOWNHOSTEXCEPTION_HH_

#include "EIOException.hh"

namespace efc {

#define EUNKNOWNHOSTEXCEPTION        EUnknownHostException(__FILE__, __LINE__, errno)
#define EUNKNOWNHOSTEXCEPTIONS(msg)  EUnknownHostException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that the IP address of a host could not be determined.
 *
 * @since   JDK1.0
 */

class EUnknownHostException: public EIOException {
public:
	/**
	 * Constructs an <code>EUnknownHostException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EUnknownHostException(const char *_file_, int _line_, int errn = 0) :
		EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EUnknownHostException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EUnknownHostException(const char *_file_,
			int _line_, const char *s) :
				EIOException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EUNKNOWNHOSTEXCEPTION_HH_ */
