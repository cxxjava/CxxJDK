/*
 * EUnsupportedOperationException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EUNSUPPORTEDOPERATIONEXCEPTION_HH_
#define EUNSUPPORTEDOPERATIONEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {

#define EUNSUPPORTEDOPERATIONEXCEPTION        EUnsupportedOperationException(__FILE__, __LINE__, errno)
#define EUNSUPPORTEDOPERATIONEXCEPTIONS(msg)  EUnsupportedOperationException(msg, __FILE__, __LINE__, errno)

/**
 * Thrown to indicate that the requested operation is not supported.<p>
 *
 * This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @author  Josh Bloch
 * @since   1.2
 */

class EUnsupportedOperationException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EUnsupportedOperationException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EUnsupportedOperationException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EUnsupportedOperationException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EUnsupportedOperationException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				ERuntimeException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EUNSUPPORTEDOPERATIONEXCEPTION_HH_ */
