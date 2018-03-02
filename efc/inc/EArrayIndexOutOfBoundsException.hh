/*
 * EArrayIndexOutOfBoundsException.hh
 *
 *  Created on: 2018-1-1
 *      Author: cxxjava@163.com
 */

#ifndef EARRAYINDEXOUTOFBOUNDSEXCEPTION_HH_
#define EARRAYINDEXOUTOFBOUNDSEXCEPTION_HH_

#include "EIndexOutOfBoundsException.hh"

namespace efc {

#define EARRAYINDEXOUTOFBOUNDSEXCEPTION       EArrayIndexOutOfBoundsException(__FILE__, __LINE__, errno)
#define EARRAYINDEXOUTOFBOUNDSEXCEPTIONS(msg) EArrayIndexOutOfBoundsException(__FILE__, __LINE__, msg)

/**
 * Thrown to indicate that an array has been accessed with an
 * illegal index. The index is either negative or greater than or
 * equal to the size of the array.
 *
 * @since   JDK1.0
 */

class EArrayIndexOutOfBoundsException: public EIndexOutOfBoundsException {
public:
	/**
	 * Constructs a new <code>ArrayIndexOutOfBoundsException</code>
	 * class with an argument indicating the illegal index.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   index   the illegal index.
	 */
	EArrayIndexOutOfBoundsException(const char *_file_, int _line_, int index) :
		EIndexOutOfBoundsException(_file_, _line_, (EString("Array index out of range: ") + index).c_str()) {
	}

	/**
	 * Constructs an <code>EArrayIndexOutOfBoundsException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EArrayIndexOutOfBoundsException(const char *_file_, int _line_, const char *s) :
		EIndexOutOfBoundsException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EARRAYINDEXOUTOFBOUNDSEXCEPTION_HH_ */
