/*
 * EAsynchronousCloseException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EAsynchronousCloseException_HH_
#define EAsynchronousCloseException_HH_

#include "./EClosedChannelException.hh"

namespace efc {
namespace nio {

#define EASYNCHRONOUSCLOSEEXCEPTION       EAsynchronousCloseException(__FILE__, __LINE__, errno)
#define EASYNCHRONOUSCLOSEEXCEPTIONS(msg) EAsynchronousCloseException(__FILE__, __LINE__, msg)


/**
 * Checked exception received by a thread when another thread closes the
 * channel or the part of the channel upon which it is blocked in an I/O
 * operation.
 */

class EAsynchronousCloseException: public EClosedChannelException {
public:
	/**
	 * Constructs an <code>EAsynchronousCloseException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EAsynchronousCloseException(const char *_file_, int _line_, int errn = 0) :
		EClosedChannelException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EAsynchronousCloseException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EAsynchronousCloseException(const char *_file_, int _line_, const char *s, int errn = 0) :
				EClosedChannelException(_file_, _line_, s,  errn) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EAsynchronousCloseException_HH_ */
