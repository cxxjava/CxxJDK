/*
 * ENoRouteToHostException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ENoRouteToHostException_H_
#define ENoRouteToHostException_H_

#include "ESocketException.hh"

namespace efc {

#define ENOROUTETOHOSTEXCEPTION       ENoRouteToHostException(__FILE__, __LINE__, errno)
#define ENOROUTETOHOSTEXCEPTIONS(msg) ENoRouteToHostException(__FILE__, __LINE__, msg)

/**
 * Signals that an error occurred while attempting to connect a
 * socket to a remote address and port.  Typically, the remote
 * host cannot be reached because of an intervening firewall, or
 * if an intermediate router is down.
 *
 * @since   JDK1.1
 */

class ENoRouteToHostException: public ESocketException {
public:
	/**
	 * Constructs an <code>ENoRouteToHostException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ENoRouteToHostException(const char *_file_, int _line_, int errn = 0) :
			ESocketException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>ENoRouteToHostException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ENoRouteToHostException(const char *_file_, int _line_, const char *s, int errn = 0) :
			ESocketException(_file_, _line_, s, errn) {
	}
};

} /* namespace efc */
#endif /* ENoRouteToHostException_H_ */
