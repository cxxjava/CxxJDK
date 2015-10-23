/*
 * EConcurrentModificationException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTMODIFICATIONEXCEPTION_HH_
#define ECONCURRENTMODIFICATIONEXCEPTION_HH_

#include "ERuntimeException.hh"

namespace efc {

#define ECONCURRENTMODIFICATIONEXCEPTION        EConcurrentModificationException(__FILE__, __LINE__, errno)
#define ECONCURRENTMODIFICATIONEXCEPTIONS(msg)  EConcurrentModificationException(msg, __FILE__, __LINE__, errno)

/**
 * This exception may be thrown by methods that have detected concurrent
 * modification of an object when such modification is not permissible.
 * <p>
 * For example, it is not generally permissible for one thread to modify a Collection
 * while another thread is iterating over it.  In general, the results of the
 * iteration are undefined under these circumstances.  Some Iterator
 * implementations (including those of all the general purpose collection implementations
 * provided by the JRE) may choose to throw this exception if this behavior is
 * detected.  Iterators that do this are known as <i>fail-fast</i> iterators,
 * as they fail quickly and cleanly, rather that risking arbitrary,
 * non-deterministic behavior at an undetermined time in the future.
 * <p>
 * Note that this exception does not always indicate that an object has
 * been concurrently modified by a <i>different</i> thread.  If a single
 * thread issues a sequence of method invocations that violates the
 * contract of an object, the object may throw this exception.  For
 * example, if a thread modifies a collection directly while it is
 * iterating over the collection with a fail-fast iterator, the iterator
 * will throw this exception.
 *
 * <p>Note that fail-fast behavior cannot be guaranteed as it is, generally
 * speaking, impossible to make any hard guarantees in the presence of
 * unsynchronized concurrent modification.  Fail-fast operations
 * throw <tt>ConcurrentModificationException</tt> on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness: <i><tt>ConcurrentModificationException</tt>
 * should be used only to detect bugs.</i>
 *
 * @author  Josh Bloch
 * @see     Collection
 * @see     Iterator
 * @see     ListIterator
 * @see     Vector
 * @see     LinkedList
 * @see     HashSet
 * @see     Hashtable
 * @see     TreeMap
 * @see     AbstractList
 * @since   1.2
 */

class EConcurrentModificationException: public ERuntimeException {
public:
	/**
	 * Constructs an <code>EConcurrentModificationException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EConcurrentModificationException(const char *_file_, int _line_, int errn = 0) :
		ERuntimeException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EConcurrentModificationException</code> with the
	 * specified detail message.
	 *
	 * @param   s   the detail message.
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   errn   the errno.
	 */
	EConcurrentModificationException(const char *s, const char *_file_,
			int _line_, int errn = 0) :
				ERuntimeException(s, _file_, _line_, errn) {
	}
};

} /* namespace efc */
#endif /* ECONCURRENTMODIFICATIONEXCEPTION_HH_ */
