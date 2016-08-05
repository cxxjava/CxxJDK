/*
 * EPortUnreachableException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EPORTUNREACHABLEEXCEPTION_H_
#define EPORTUNREACHABLEEXCEPTION_H_

#include "ESocketException.hh"

namespace efc {

#define EPORTUNREACHABLEEXCEPTION       EPortUnreachableException(__FILE__, __LINE__, errno)
#define EPORTUNREACHABLEEXCEPTIONS(msg) EPortUnreachableException(__FILE__, __LINE__, msg)

/**
 * Signals that an ICMP Port Unreachable message has been
 * received on a connected datagram.
 *
 * @since   1.4
 */

class EPortUnreachableException: public ESocketException {
public:
	/**
	 * Constructs an <code>EPortUnreachableException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EPortUnreachableException(const char *_file_, int _line_, int errn = 0) :
			ESocketException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EPortUnreachableException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EPortUnreachableException(const char *_file_, int _line_, const char *s, int errn = 0) :
			ESocketException(_file_, _line_, s, errn) {
	}
};

} /* namespace efc */
#endif /* EPORTUNREACHABLEEXCEPTION_H_ */
