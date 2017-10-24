/*
 * EIterator.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EITERATOR_HH_
#define EITERATOR_HH_

#include "ESharedPtr.hh"
#include "ENoSuchElementException.hh"
#include "EIllegalStateException.hh"

namespace efc {

/**
 * An iterator over a collection.  Iterator takes the place of Enumeration in
 * the Java collections framework.  Iterators differ from enumerations in two
 * ways: <ul>
 *	<li> Iterators allow the caller to remove elements from the
 *	     underlying collection during the iteration with well-defined
 * 	     semantics.
 *	<li> Method names have been improved.
 * </ul><p>
 *
 * This interface is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @version 1.27, 07/24/06
 * @see Collection
 * @see ListIterator
 * @see Enumeration
 * @since 1.2
 */

template<typename E>
interface EIterator : virtual public EObject
{
	virtual ~EIterator(){}
	
	/**
	 * Returns <tt>true</tt> if the iteration has more elements. (In other
	 * words, returns <tt>true</tt> if <tt>next</tt> would return an element
	 * rather than throwing an exception.)
	 *
	 * @return <tt>true</tt> if the iterator has more elements.
	 */
	virtual boolean hasNext() = 0;

	/**
	 * Returns the next element in the iteration.
	 *
	 * @return the next element in the iteration.
	 * @exception NoSuchElementException iteration has no more elements.
	 */
	virtual E next() = 0;

	/**
	 *
	 * Removes from the underlying collection the last element returned by the
	 * iterator (optional operation).  This method can be called only once per
	 * call to <tt>next</tt>.  The behavior of an iterator is unspecified if
	 * the underlying collection is modified while the iteration is in
	 * progress in any way other than by calling this method.
	 *
	 * @exception UnsupportedOperationException if the <tt>remove</tt>
	 *		  operation is not supported by this Iterator.

	 * @exception IllegalStateException if the <tt>next</tt> method has not
	 *		  yet been called, or the <tt>remove</tt> method has already
	 *		  been called after the last call to the <tt>next</tt>
	 *		  method.
	 */
	virtual void remove() = 0;
	virtual E moveOut() = 0;
};

template<typename E>
class EEmptyIterator: public EIterator<E> {
public:
	virtual ~EEmptyIterator() {}
	virtual boolean hasNext() { return false; }
	virtual E next() { throw ENoSuchElementException(__FILE__, __LINE__); }
	virtual void remove() { throw EIllegalStateException(__FILE__, __LINE__); }
	virtual E moveOut() { throw EIllegalStateException(__FILE__, __LINE__); }
};

} /* namespace efc */
#endif /* EITERATOR_HH_ */
