/*
 * EBrokenBarrierException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EBrokenBarrierException_HH_
#define EBrokenBarrierException_HH_

#include "EException.hh"

namespace efc {

#define EBROKENBARRIEREXCEPTION        EBROKENBARRIEREXCEPTION(__FILE__, __LINE__, errno)
#define EBROKENBARRIEREXCEPTIONS(msg)  EBROKENBARRIEREXCEPTION(msg, __FILE__, __LINE__, errno)

/**
 * Exception thrown when a thread tries to wait upon a barrier that is
 * in a broken state, or which enters the broken state while the thread
 * is waiting.
 *
 * @see CyclicBarrier
 *
 * @since 1.5
 * @author Doug Lea
 *
 */

class EBrokenBarrierException: public EException {
public:
	/**
	 * Constructs an <code>EBrokenBarrierException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EBrokenBarrierException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EBrokenBarrierException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EBrokenBarrierException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EBrokenBarrierException_HH_ */
