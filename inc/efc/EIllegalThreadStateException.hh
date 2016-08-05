/*
 * EIllegalThreadStateException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EILLEGALTHREADSTATEEXCEPTION_HH_
#define EILLEGALTHREADSTATEEXCEPTION_HH_

#include "EIllegalArgumentException.hh"

namespace efc {

#define EILLEGALTHREADSTATEEXCEPTION       EIllegalThreadStateException(__FILE__, __LINE__, errno)
#define EILLEGALTHREADSTATEEXCEPTIONS(msg) EIllegalThreadStateException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that a thread is not in an appropriate state
 * for the requested operation. See, for example, the
 * <code>suspend</code> and <code>resume</code> methods in class
 * <code>Thread</code>.
 *
 * @see     java.lang.Thread#resume()
 * @see     java.lang.Thread#suspend()
 * @since   JDK1.0
 */

class EIllegalThreadStateException: public EIllegalArgumentException {
public:
	/**
	 * Constructs an <code>EIllegalThreadStateException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EIllegalThreadStateException(const char *_file_, int _line_, int errn = 0) :
		EIllegalArgumentException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EIllegalThreadStateException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EIllegalThreadStateException(const char *_file_, int _line_, const char *s, int errn = 0) :
				EIllegalArgumentException(_file_, _line_, s, errn) {
	}
};

} /* namespace efc */
#endif /* EILLEGALTHREADSTATEEXCEPTION_HH_ */
