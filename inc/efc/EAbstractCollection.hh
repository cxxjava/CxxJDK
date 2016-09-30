/*
 * EAbstractCollection.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EAbstractCollection_HH_
#define EAbstractCollection_HH_

#include "EBase.hh"
#include "ECollection.hh"
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
abstract class EAbstractCollection: virtual public ECollection<E> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractCollection() {}

public:
	virtual ~EAbstractCollection() {}

	// Query Operations

	/**
	 * Returns an iterator over the elements contained in this collection.
	 *
	 * @return an iterator over the elements contained in this collection
	 */
	virtual sp<EIterator<E> > iterator(int index=0) = 0;

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
	virtual boolean contains(E o) {
		sp<EIterator<E> > e = iterator();
		while (e->hasNext())
			if (o->equals(e->next())) {
				return true;
			}
		return false;
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
	virtual boolean add(E e) THROWS(EUnsupportedOperationException) {
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
	virtual boolean remove(E o) {
		sp<EIterator<E> > e = iterator();
		while (e->hasNext()) {
			if (o->equals(e->next())) {
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
	virtual boolean containsAll(ECollection<E> *c) {
		sp<EIterator<E> > e = c->iterator();
		while (e->hasNext())
			if (!contains(e->next())) {
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
	virtual boolean addAll(ECollection<E> *c) {
		boolean modified = false;
		sp<EIterator<E> > e = c->iterator();
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
	virtual boolean removeAll(ECollection<E> *c) {
		boolean modified = false;
		sp<EIterator<E> > e = iterator();
		while (e->hasNext()) {
			if (c->contains(e->next())) {
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
	virtual boolean retainAll(ECollection<E> *c) {
		boolean modified = false;
		sp<EIterator<E> > e = iterator();
		while (e->hasNext()) {
			if (!c->contains(e->next())) {
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
		sp<EIterator<E> > it = iterator();
		while (it->hasNext()) {
			it->next();
			it->remove();
		}
	}

	//  String conversion

	/**
	 * Returns a string representation of this collection.  The string
	 * representation consists of a list of the collection's elements in the
	 * order they are returned by its iterator, enclosed in square brackets
	 * (<tt>"[]"</tt>).  Adjacent elements are separated by the characters
	 * <tt>", "</tt> (comma and space).  Elements are converted to strings as
	 * by {@link String#valueOf(Object)}.
	 *
	 * @return a string representation of this collection
	 */
	virtual EStringBase toString() {
		sp<EIterator<E> > it = iterator();
		if (! it->hasNext()) {
			return "[]";
		}

		EStringBase sb;
		sb.append('[');
		for (;;) {
			E e = it->next();
			sb.append((void*)e == (void*)this ? "(this Collection)" : e->toString().c_str());
			if (! it->hasNext()) {
				return sb.append(']');
			}
			sb.append(',').append(' ');
		}
	}
};

//=============================================================================

template<>
abstract class EAbstractCollection<int>: virtual public ECollection<int> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractCollection() {}

public:
	virtual ~EAbstractCollection() {}

	// Query Operations

	/**
	 * Returns an iterator over the elements contained in this collection.
	 *
	 * @return an iterator over the elements contained in this collection
	 */
	virtual sp<EIterator<int> > iterator(int index=0) = 0;

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
	virtual boolean contains(int o) {
		sp<EIterator<int> > e = iterator();
		while (e->hasNext())
			if (o == e->next()) {
				return true;
			}
		return false;
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
	virtual boolean add(int e) THROWS(EUnsupportedOperationException) {
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
	virtual boolean remove(int o) {
		sp<EIterator<int> > e = iterator();
		while (e->hasNext()) {
			if (o == (e->next())) {
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
	virtual boolean containsAll(ECollection<int> *c) {
		sp<EIterator<int> > e = c->iterator();
		while (e->hasNext())
			if (!contains(e->next())) {
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
	virtual boolean addAll(ECollection<int> *c) {
		boolean modified = false;
		sp<EIterator<int> > e = c->iterator();
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
	virtual boolean removeAll(ECollection<int> *c) {
		boolean modified = false;
		sp<EIterator<int> > e = iterator();
		while (e->hasNext()) {
			if (c->contains(e->next())) {
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
	virtual boolean retainAll(ECollection<int> *c) {
		boolean modified = false;
		sp<EIterator<int> > e = iterator();
		while (e->hasNext()) {
			if (!c->contains(e->next())) {
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
		sp<EIterator<int> > it = iterator();
		while (it->hasNext()) {
			it->next();
			it->remove();
		}
	}

	//  String conversion

	/**
	 * Returns a string representation of this collection.  The string
	 * representation consists of a list of the collection's elements in the
	 * order they are returned by its iterator, enclosed in square brackets
	 * (<tt>"[]"</tt>).  Adjacent elements are separated by the characters
	 * <tt>", "</tt> (comma and space).  Elements are converted to strings as
	 * by {@link String#valueOf(Object)}.
	 *
	 * @return a string representation of this collection
	 */
	virtual EStringBase toString() {
		sp<EIterator<int> > it = iterator();
		if (! it->hasNext()) {
			return "[]";
		}

		EStringBase sb;
		sb.append('[');
		for (;;) {
			int e = it->next();
			sb.append(e);
			if (! it->hasNext()) {
				return sb.append(']');
			}
			sb.append(',').append(' ');
		}
	}
};

//=============================================================================

template<>
abstract class EAbstractCollection<llong>: virtual public ECollection<llong> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractCollection() {}

public:
	virtual ~EAbstractCollection() {}

	// Query Operations

	/**
	 * Returns an iterator over the elements contained in this collection.
	 *
	 * @return an iterator over the elements contained in this collection
	 */
	virtual sp<EIterator<llong> > iterator(int index=0) = 0;

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
	virtual boolean contains(llong o) {
		sp<EIterator<llong> > e = iterator();
		while (e->hasNext())
			if (o == (e->next())) {
				return true;
			}
		return false;
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
	virtual boolean add(llong e) THROWS(EUnsupportedOperationException) {
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
	virtual boolean remove(llong o) {
		sp<EIterator<llong> > e = iterator();
		while (e->hasNext()) {
			if (o == (e->next())) {
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
	virtual boolean containsAll(ECollection<llong> *c) {
		sp<EIterator<llong> > e = c->iterator();
		while (e->hasNext())
			if (!contains(e->next())) {
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
	virtual boolean addAll(ECollection<llong> *c) {
		boolean modified = false;
		sp<EIterator<llong> > e = c->iterator();
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
	virtual boolean removeAll(ECollection<llong> *c) {
		boolean modified = false;
		sp<EIterator<llong> > e = iterator();
		while (e->hasNext()) {
			if (c->contains(e->next())) {
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
	virtual boolean retainAll(ECollection<llong> *c) {
		boolean modified = false;
		sp<EIterator<llong> > e = iterator();
		while (e->hasNext()) {
			if (!c->contains(e->next())) {
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
		sp<EIterator<llong> > it = iterator();
		while (it->hasNext()) {
			it->next();
			it->remove();
		}
	}

	//  String conversion

	/**
	 * Returns a string representation of this collection.  The string
	 * representation consists of a list of the collection's elements in the
	 * order they are returned by its iterator, enclosed in square brackets
	 * (<tt>"[]"</tt>).  Adjacent elements are separated by the characters
	 * <tt>", "</tt> (comma and space).  Elements are converted to strings as
	 * by {@link String#valueOf(Object)}.
	 *
	 * @return a string representation of this collection
	 */
	virtual EStringBase toString() {
		sp<EIterator<llong> > it = iterator();
		if (! it->hasNext()) {
			return "[]";
		}

		EStringBase sb;
		sb.append('[');
		for (;;) {
			llong e = it->next();
			sb.append(e);
			if (! it->hasNext()) {
				return sb.append(']');
			}
			sb.append(',').append(' ');
		}
	}
};

} /* namespace efc */
#endif //!EAbstractCollection_HH_
