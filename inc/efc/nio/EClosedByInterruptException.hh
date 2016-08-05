/*
 * EClosedByInterruptException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ECLOSEDBYINTERRUPTEXCEPTION_HH_
#define ECLOSEDBYINTERRUPTEXCEPTION_HH_

#include "EAsynchronousCloseException.hh"

namespace efc {
namespace nio {

#define ECLOSEDBYINTERRUPTEXCEPTION       EClosedByInterruptException(__FILE__, __LINE__, errno)
#define ECLOSEDBYINTERRUPTEXCEPTIONS(msg) EClosedByInterruptException(__FILE__, __LINE__, msg)


/**
 * * Checked exception received by a thread when another thread interrupts it
 * while it is blocked in an I/O operation upon a channel.  Before this
 * exception is thrown the channel will have been closed and the interrupt
 * status of the previously-blocked thread will have been set."
 */

class EClosedByInterruptException: public EAsynchronousCloseException {
public:
	/**
	 * Constructs an <code>EClosedByInterruptException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EClosedByInterruptException(const char *_file_, int _line_, int errn = 0) :
		EAsynchronousCloseException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EClosedByInterruptException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EClosedByInterruptException(const char *_file_, int _line_, const char *s, int errn = 0) :
		EAsynchronousCloseException(_file_, _line_, s, errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* ECLOSEDBYINTERRUPTEXCEPTION_HH_ */
