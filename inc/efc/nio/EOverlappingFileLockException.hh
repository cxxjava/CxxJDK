/*
 * EOverlappingFileLockException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EOVERLAPPINGFILELOCKEXCEPTION_HH_
#define EOVERLAPPINGFILELOCKEXCEPTION_HH_

#include "EIllegalStateException.hh"

namespace efc {
namespace nio {

#define EOVERLAPPINGFILELOCKEXCEPTION        EOverlappingFileLockException(__FILE__, __LINE__, errno)
#define EOVERLAPPINGFILELOCKEXCEPTIONS(msg)  EOverlappingFileLockException(__FILE__, __LINE__, msg)

/**
 * Unchecked exception thrown when an attempt is made to acquire a lock on a
 * region of a file that overlaps a region already locked by the same Java
 * virtual machine, or when another thread is already waiting to lock an
 * overlapping region of the same file."
 */

class EOverlappingFileLockException : public EIllegalStateException {
public:
	/**
	 * Constructs an <code>EOverlappingFileLockException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EOverlappingFileLockException(const char *_file_, int _line_, int errn = 0) :
		EIllegalStateException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EOverlappingFileLockException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EOverlappingFileLockException(const char *_file_,
			int _line_, const char *s) :
				EIllegalStateException(_file_, _line_, s) {
	}
};

} /* namespace nio */
} /* namespace efc */
#endif /* EOVERLAPPINGFILELOCKEXCEPTION_HH_ */
