/*
 * EAbstractConcurrentCollection.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EAbstractConcurrentCollection_HH_
#define EAbstractConcurrentCollection_HH_

#include "ESharedArrLst.hh"
#include "EConcurrentCollection.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

/**
 * This class provides a skeletal implementation of the <tt>Collection</tt>
 * interface, to minimize the effort required to implement this interface. <p>
 *
 * To implement an unmodifiable collection, the programmer needs only to
 * extend this class and provide implementations for the <tt>iterator</tt> and
 * <tt>size</tt> methods.  (The iterator returned by the <tt>iterator</tt>
 * method must implement <tt>hasNext</tt> and <tt>next</tt>.)<p>
 *
 * To implement a modifiable collection, the programmer must additionally
 * override this class's <tt>add</tt> method (which otherwise throws an
 * <tt>UnsupportedOperationException</tt>), and the iterator returned by the
 * <tt>iterator</tt> method must additionally implement its <tt>remove</tt>
 * method.<p>
 *
 * The programmer should generally provide a void (no argument) and
 * <tt>Collection</tt> constructor, as per the recommendation in the
 * <tt>Collection</tt> interface specification.<p>
 *
 * The documentation for each non-abstract method in this class describes its
 * implementation in detail.  Each of these methods may be overridden if
 * the collection being implemented admits a more efficient implementation.<p>
 *
 * This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @version 1.38, 06/16/06
 * @see Collection
 * @since 1.2
 */

template<typename E>
abstract class EAbstractConcurrentCollection: virtual public EConcurrentCollection<E> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractConcurrentCollection() {}

public:
	virtual ~EAbstractConcurrentCollection() {}

	// Query Operations

	/**
	 * Returns an iterator over the elements contained in this collection.
	 *
	 * @return an iterator over the elements contained in this collection
	 */
	virtual sp<EConcurrentIterator<E> > iterator() = 0;

	virtual int size() = 0;

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns <tt>size() == 0</tt>.
	 */
	virtual boolean isEmpty() {
		return size() == 0;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over the elements in the collection,
	 * checking each element in turn for equality with the specified element.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean contains(E* o) {
		sp<EConcurrentIterator<E> > e = iterator();
		while (e->hasNext())
			if (o->equals(e->next().get())) {
				return true;
			}
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns an array containing all the elements
	 * returned by this collection's iterator, in the same order, stored in
	 * consecutive elements of the array, starting with index {@code 0}.
	 * The length of the returned array is equal to the number of elements
	 * returned by the iterator, even if the size of this collection changes
	 * during iteration, as might happen if the collection permits
	 * concurrent modification during iteration.  The {@code size} method is
	 * called only as an optimization hint; the correct result is returned
	 * even if the iterator returns a different number of elements.
	 *
	 * <p>This method is equivalent to:
	 *
	 *  <pre> {@code
	 * List<E> list = new ArrayList<E>(size());
	 * for (E e : this)
	 *     list.add(e);
	 * return list.toArray();
	 * }</pre>
	 */
	virtual ea<E> toArray() {
		// Estimate size of array; be prepared to see more or fewer elements
		eal<E> r(size());
		sp<EConcurrentIterator<E> > it = iterator();
		for (int i = 0; i < r.size(); i++) {
			if (! it->hasNext()) { // fewer elements than expected
				break;
			}
			r.add(it->next());
		}
		return it->hasNext() ? finishToArray(r, it) : r.toArray();
	}

	// Modification Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * <tt>UnsupportedOperationException</tt>.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IllegalStateException         {@inheritDoc}
	 */
	virtual boolean add(E* e) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}
	virtual boolean add(sp<E> e) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over the collection looking for the
	 * specified element.  If it finds the element, it removes the element
	 * from the collection using the iterator's remove method.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if the iterator returned by this
	 * collection's iterator method does not implement the <tt>remove</tt>
	 * method and this collection contains the specified object.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 */
	virtual boolean remove(E* o) {
		sp<EConcurrentIterator<E> > e = iterator();
		while (e->hasNext()) {
			if (o->equals(e->next().get())) {
				e->remove();
				return true;
			}
		}
		return false;
	}

	// Bulk Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over the specified collection,
	 * checking each element returned by the iterator in turn to see
	 * if it's contained in this collection.  If all elements are so
	 * contained <tt>true</tt> is returned, otherwise <tt>false</tt>.
	 *
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @see #contains(Object)
	 */
	virtual boolean containsAll(EConcurrentCollection<E>* c) {
		sp<EConcurrentIterator<E> > e = c->iterator();
		while (e->hasNext())
			if (!contains(e->next().get())) {
				return false;
			}
		return true;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over the specified collection, and adds
	 * each object returned by the iterator to this collection, in turn.
	 *
	 * <p>Note that this implementation will throw an
	 * <tt>UnsupportedOperationException</tt> unless <tt>add</tt> is
	 * overridden (assuming the specified collection is non-empty).
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IllegalStateException         {@inheritDoc}
	 *
	 * @see #add(Object)
	 */
	virtual boolean addAll(EConcurrentCollection<E> *c) {
		boolean modified = false;
		sp<EConcurrentIterator<E> > e = c->iterator();
		while (e->hasNext()) {
			if (add(e->next()))
				modified = true;
		}
		return modified;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over this collection, checking each
	 * element returned by the iterator in turn to see if it's contained
	 * in the specified collection.  If it's so contained, it's removed from
	 * this collection with the iterator's <tt>remove</tt> method.
	 *
	 * <p>Note that this implementation will throw an
	 * <tt>UnsupportedOperationException</tt> if the iterator returned by the
	 * <tt>iterator</tt> method does not implement the <tt>remove</tt> method
	 * and this collection contains one or more elements in common with the
	 * specified collection.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 *
	 * @see #remove(Object)
	 * @see #contains(Object)
	 */
	virtual boolean removeAll(EConcurrentCollection<E> *c) {
		boolean modified = false;
		sp<EConcurrentIterator<E> > e = iterator();
		while (e->hasNext()) {
			if (c->contains(e->next().get())) {
				e->remove();
				modified = true;
			}
		}
		return modified;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over this collection, checking each
	 * element returned by the iterator in turn to see if it's contained
	 * in the specified collection.  If it's not so contained, it's removed
	 * from this collection with the iterator's <tt>remove</tt> method.
	 *
	 * <p>Note that this implementation will throw an
	 * <tt>UnsupportedOperationException</tt> if the iterator returned by the
	 * <tt>iterator</tt> method does not implement the <tt>remove</tt> method
	 * and this collection contains one or more elements not present in the
	 * specified collection.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 *
	 * @see #remove(Object)
	 * @see #contains(Object)
	 */
	virtual boolean retainAll(EConcurrentCollection<E> *c) {
		boolean modified = false;
		sp<EConcurrentIterator<E> > e = iterator();
		while (e->hasNext()) {
			if (!c->contains(e->next().get())) {
				e->remove();
				modified = true;
			}
		}
		return modified;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over this collection, removing each
	 * element using the <tt>Iterator.remove</tt> operation.  Most
	 * implementations will probably choose to override this method for
	 * efficiency.
	 *
	 * <p>Note that this implementation will throw an
	 * <tt>UnsupportedOperationException</tt> if the iterator returned by this
	 * collection's <tt>iterator</tt> method does not implement the
	 * <tt>remove</tt> method and this collection is non-empty.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 */
	virtual void clear() {
		sp<EConcurrentIterator<E> > e = iterator();
		while (e->hasNext()) {
			e->next();
			e->remove();
		}
	}

private:
	/**
	 * The maximum size of array to allocate.
	 * Some VMs reserve some header words in an array.
	 * Attempts to allocate larger arrays may result in
	 * OutOfMemoryError: Requested array size exceeds VM limit
	 */
	static const int MAX_ARRAY_SIZE = EInteger::MAX_VALUE - 8;

	/**
	 * Reallocates the array being used within toArray when the iterator
	 * returned more elements than expected, and finishes filling it from
	 * the iterator.
	 *
	 * @param r the array, replete with previously stored elements
	 * @param it the in-progress iterator over this collection
	 * @return array containing the elements in the given array, plus any
	 *         further elements returned by the iterator, trimmed to size
	 */
	static ea<E> finishToArray(eal<E>& r, sp<EConcurrentIterator<E> >& it) {
		while (it->hasNext()) {
			r.add(it->next());
		}
		return r.toArray();
	}
};

} /* namespace efc */
#endif //!EAbstractConcurrentCollection_HH_
