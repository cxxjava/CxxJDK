/*
 * EInterruptedException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EInterruptedException_HH_
#define EInterruptedException_HH_

#include "EException.hh"

namespace efc {

#define EINTERRUPTEDEXCEPTION        EInterruptedException(__FILE__, __LINE__, errno)
#define EINTERRUPTEDEXCEPTIONS(msg)  EInterruptedException(msg, __FILE__, __LINE__, errno)

/**
 * Thrown when a thread is waiting, sleeping, or otherwise occupied,
 * and the thread is interrupted, either before or during the activity.
 * Occasionally a method may wish to test whether the current
 * thread has been interrupted, and if so, to immediately throw
 * this exception.  The following code can be used to achieve
 * this effect:
 * <pre>
 *  if (Thread.interrupted())  // Clears interrupted status!
 *      throw new InterruptedException();
 * </pre>
 *
 * @author  Frank Yellin
 * @see     java.lang.Object#wait()
 * @see     java.lang.Object#wait(long)
 * @see     java.lang.Object#wait(long, int)
 * @see     java.lang.Thread#sleep(long)
 * @see     java.lang.Thread#interrupt()
 * @see     java.lang.Thread#interrupted()
 * @since   JDK1.0
 */

class EInterruptedException: public EException {
public:
	/**
	 * Constructs an <code>EInterruptedException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EInterruptedException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EInterruptedException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EInterruptedException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
			EException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* EInterruptedException_HH_ */
