/*
 * EUnknownServiceException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EUNKNOWNSERVICEEXCEPTION_HH_
#define EUNKNOWNSERVICEEXCEPTION_HH_

#include "EIOException.hh"

namespace efc {

#define EUNKNOWNSERVICEEXCEPTION        EUnknownServiceException(__FILE__, __LINE__, errno)
#define EUNKNOWNSERVICEEXCEPTIONS(msg)  EUnknownServiceException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that an unknown service exception has
 * occurred. Either the MIME type returned by a URL connection does
 * not make sense, or the application is attempting to write to a
 * read-only URL connection.
 *
 * @author  unascribed
 * @since   JDK1.0
 */

class EUnknownServiceException: public EIOException {
public:
	/**
	 * Constructs an <code>EUnknownServiceException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EUnknownServiceException(const char *_file_, int _line_, int errn = 0) :
		EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EUnknownServiceException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EUnknownServiceException(const char *_file_,
			int _line_, const char *s) :
				EIOException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EUNKNOWNSERVICEEXCEPTION_HH_ */
