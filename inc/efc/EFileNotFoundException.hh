/*
 * EFileNotFoundException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EFILENOTFOUNDEXCEPTION_H_
#define EFILENOTFOUNDEXCEPTION_H_

#include "EIOException.hh"

namespace efc {

#define EFILENOTFOUNDEXCEPTION        EFileNotFoundException(__FILE__, __LINE__, errno)
#define EFILENOTFOUNDEXCEPTIONS(msg)  EFileNotFoundException(__FILE__, __LINE__, msg)

/**
 * Signals that an attempt to open the file denoted by a specified pathname
 * has failed.
 *
 * <p> This exception will be thrown by the {@link FileInputStream}, {@link
 * FileOutputStream}, and {@link RandomAccessFile} constructors when a file
 * with the specified pathname does not exist.  It will also be thrown by these
 * constructors if the file does exist but for some reason is inaccessible, for
 * example when an attempt is made to open a read-only file for writing.
 */

class EFileNotFoundException: public EIOException {
public:
	/**
	 * Constructs an <code>EFileNotFoundException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EFileNotFoundException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EFileNotFoundException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EFileNotFoundException(const char *_file_,
			int _line_, const char *s) :
			EIOException(_file_, _line_, s) {
	}
};

} /* namespace efc */
#endif /* EFILENOTFOUNDEXCEPTION_H_ */
