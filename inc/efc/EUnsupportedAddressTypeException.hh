/*
 * EUnsupportedAddressTypeException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EUNSUPPORTEDADDRESSTYPEEXCEPTION_HH_
#define EUNSUPPORTEDADDRESSTYPEEXCEPTION_HH_

#include "EIllegalArgumentException.hh"

namespace efc {

#define EUNSUPPORTEDADDRESSTYPEEXCEPTION       EUnsupportedAddressTypeException(__FILE__, __LINE__, errno)
#define EUNSUPPORTEDADDRESSTYPEEXCEPTIONS(msg) EUnsupportedAddressTypeException(__FILE__, __LINE__, msg)

/**
 * Unchecked exception thrown when an attempt is made to bind or connect
 * to a socket address of a type that is not supported.
 */

class EUnsupportedAddressTypeException: public EIllegalArgumentException {
public:
	/**
	 * Constructs an <code>EUnsupportedAddressTypeException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EUnsupportedAddressTypeException(const char *_file_, int _line_, int errn = 0) :
		EIllegalArgumentException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EUnsupportedAddressTypeException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EUnsupportedAddressTypeException(const char *_file_, int _line_, const char *s, int errn = 0) :
				EIllegalArgumentException(_file_, _line_, s, errn) {
	}
};

} /* namespace efc */
#endif /* EUNSUPPORTEDADDRESSTYPEEXCEPTION_HH_ */
