/*
 * EInterruptedIOException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EINTERRUPTEDIOEXCEPTION_HH_
#define EINTERRUPTEDIOEXCEPTION_HH_

#include "EIOException.hh"

namespace efc {

#define EINTERRUPTEDIOEXCEPTION       EInterruptedIOException(__FILE__, __LINE__, errno)
#define EINTERRUPTEDIOEXCEPTIONS(msg) EInterruptedIOException(msg, __FILE__, __LINE__, errno)

/**
 * Signals that an I/O operation has been interrupted. An
 * <code>InterruptedIOException</code> is thrown to indicate that an
 * input or output transfer has been terminated because the thread
 * performing it was interrupted. The field {@link #bytesTransferred}
 * indicates how many bytes were successfully transferred before
 * the interruption occurred.
 *
 * @author  unascribed
 * @see     java.io.InputStream
 * @see     java.io.OutputStream
 * @see     java.lang.Thread#interrupt()
 * @since   JDK1.0
 */

class EInterruptedIOException: public EIOException {
public:
	/**
	 * Constructs an <code>EInterruptedIOException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EInterruptedIOException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EInterruptedIOException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EInterruptedIOException(const char *s, const char *_file_, int _line_, int errn =
			0) :
			EIOException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EINTERRUPTEDIOEXCEPTION_HH_ */
