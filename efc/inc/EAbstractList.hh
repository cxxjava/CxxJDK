/*
 * EList.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EABSTRACTLIST_HH_
#define EABSTRACTLIST_HH_

#include "EList.hh"
#include "EListIterator.hh"
#include "EAbstractCollection.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EUnsupportedOperationException.hh"
#include "ENoSuchElementException.hh"
#include "EIllegalStateException.hh"

namespace efc {

/**
 * This class provides a skeletal implementation of the {@link List}
 * interface to minimize the effort required to implement this interface
 * backed by a "random access" data store (such as an array).  For sequential
 * access data (such as a linked list), {@link AbstractSequentialList} should
 * be used in preference to this class.
 *
 * <p>To implement an unmodifiable list, the programmer needs only to extend
 * this class and provide implementations for the {@link #get(int)} and
 * {@link List#size() size()} methods.
 *
 * <p>To implement a modifiable list, the programmer must additionally
 * override the {@link #set(int, Object) set(int, E)} method (which otherwise
 * throws an {@code UnsupportedOperationException}).  If the list is
 * variable-size the programmer must additionally override the
 * {@link #add(int, Object) add(int, E)} and {@link #remove(int)} methods.
 *
 * <p>The programmer should generally provide a void (no argument) and collection
 * constructor, as per the recommendation in the {@link Collection} interface
 * specification.
 *
 * <p>Unlike the other abstract collection implementations, the programmer does
 * <i>not</i> have to provide an iterator implementation; the iterator and
 * list iterator are implemented by this class, on top of the "random access"
 * methods:
 * {@link #get(int)},
 * {@link #set(int, Object) set(int, E)},
 * {@link #add(int, Object) add(int, E)} and
 * {@link #remove(int)}.
 *
 * <p>The documentation for each non-abstract method in this class describes its
 * implementation in detail.  Each of these methods may be overridden if the
 * collection being implemented admits a more efficient implementation.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @version 1.52, 06/16/06
 * @since 1.2
 */

//=============================================================================
//Primitive Types.

template<typename E>
abstract class EAbstractList: virtual public EAbstractCollection<E>,
		virtual public EList<E> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractList() {
	}

private:
	class ListIterator: public EListIterator<E> {
	public:
		ListIterator(EAbstractList<E> *list, int index=0) {
			abslist = list;

			cursor = index;
			lastRet = -1;
		}

		boolean hasNext() {
			return cursor != abslist->size();
		}

		E next() {
			E next = abslist->getAt(cursor);
			lastRet = cursor++;
			return next;
		}

		void remove() {
			if (lastRet >= 0) {
				abslist->removeAt(lastRet);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
			}
		}

		E moveOut() {
			if (lastRet >= 0) {
				E o = abslist->removeAt(lastRet);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
				return o;
			}
			throw ENoSuchElementException(__FILE__, __LINE__);
		}

		boolean hasPrevious() {
			return cursor != 0;
		}

		E previous() {
			int i = cursor - 1;
			E previous = abslist->getAt(i);
			lastRet = cursor = i;
			return previous;
		}

		int nextIndex() {
			return cursor;
		}

		int previousIndex() {
		    return cursor-1;
		}

		void set(E e) {
			if (lastRet == -1)
				return;

			abslist->setAt(lastRet, e);
		}

		void add(E e) {
			abslist->addAt(cursor++, e);
			lastRet = -1;
		}

	private:
		EAbstractList<E> *abslist;

		/**
		 * Index of element to be returned by subsequent call to next.
		 */
		int cursor;

		/**
		 * Index of element returned by most recent call to next or
		 * previous.  Reset to -1 if this element is deleted by a call
		 * to remove.
		 */
		int lastRet;

	};

public:
	/**
	 * count of references.
	 */
	virtual ~EAbstractList(){
	}

	virtual int size() {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * Appends the specified element to the end of this list (optional
	 * operation).
	 *
	 * <p>Lists that support this operation may place limitations on what
	 * elements may be added to this list.  In particular, some
	 * lists will refuse to add null elements, and others will impose
	 * restrictions on the type of elements that may be added.  List
	 * classes should clearly specify in their documentation any restrictions
	 * on what elements may be added.
	 *
	 * <p>This implementation calls {@code add(size(), e)}.
	 *
	 * <p>Note that this implementation throws an
	 * {@code UnsupportedOperationException} unless
	 * {@link #add(int, Object) add(int, E)} is overridden.
	 *
	 * @param e element to be appended to this list
	 * @return {@code true} (as specified by {@link Collection#add})
	 * @throws UnsupportedOperationException if the {@code add} operation
	 *         is not supported by this list
	 * @throws ClassCastException if the class of the specified element
	 *         prevents it from being added to this list
	 * @throws NullPointerException if the specified element is null and this
	 *         list does not permit null elements
	 * @throws IllegalArgumentException if some property of this element
	 *         prevents it from being added to this list
	 */
	virtual boolean add(E e) {
		addAt(size(), e);
		return true;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E getAt(int index) = 0;

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual E setAt(int index, E element) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual void addAt(int index, E element) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual E removeAt(int index) THROWS(EUnsupportedOperationException) {
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
		return EAbstractCollection<E>::remove(o);
	}

	// Search Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation first gets a list iterator (with
	 * {@code listIterator()}).  Then, it iterates over the list until the
	 * specified element is found or the end of the list is reached.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual int indexOf(E o) {
		ListIterator e(this);
		while (e.hasNext())
			if (o == e.next())
				return e.previousIndex();
		return -1;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation first gets a list iterator that points to the end
	 * of the list (with {@code listIterator(size())}).  Then, it iterates
	 * backwards over the list until the specified element is found, or the
	 * beginning of the list is reached.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual int lastIndexOf(E o) {
		ListIterator e(this);
		while (e.hasPrevious())
			if (o == e.previous())
				return e.nextIndex();

		return -1;
	}

	// Bulk Operations

	/**
	 * Removes all of the elements from this list (optional operation).
	 * The list will be empty after this call returns.
	 *
	 * <p>This implementation calls {@code removeRange(0, size())}.
	 *
	 * <p>Note that this implementation throws an
	 * {@code UnsupportedOperationException} unless {@code remove(int
	 * index)} or {@code removeRange(int fromIndex, int toIndex)} is
	 * overridden.
	 *
	 * @throws UnsupportedOperationException if the {@code clear} operation
	 *         is not supported by this list
	 */
	virtual void clear() THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	// Iterators

	/**
	 * Returns an iterator over the elements in this list in proper sequence.
	 *
	 * <p>This implementation returns a straightforward implementation of the
	 * iterator interface, relying on the backing list's {@code size()},
	 * {@code get(int)}, and {@code remove(int)} methods.
	 *
	 * <p>Note that the iterator returned by this method will throw an
	 * {@code UnsupportedOperationException} in response to its
	 * {@code remove} method unless the list's {@code remove(int)} method is
	 * overridden.
	 *
	 * <p>This implementation can be made to throw runtime exceptions in the
	 * face of concurrent modification, as described in the specification
	 * for the (protected) {@code modCount} field.
	 *
	 * @return an iterator over the elements in this list in proper sequence
	 *
	 * @see #modCount
	 */
	virtual sp<EIterator<E> > iterator(int index=0) {
		return new ListIterator(this, index);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a straightforward implementation of the
	 * {@code ListIterator} interface that extends the implementation of the
	 * {@code Iterator} interface returned by the {@code iterator()} method.
	 * The {@code ListIterator} implementation relies on the backing list's
	 * {@code get(int)}, {@code set(int, E)}, {@code add(int, E)}
	 * and {@code remove(int)} methods.
	 *
	 * <p>Note that the list iterator returned by this implementation will
	 * throw an {@link UnsupportedOperationException} in response to its
	 * {@code remove}, {@code set} and {@code add} methods unless the
	 * list's {@code remove(int)}, {@code set(int, E)}, and
	 * {@code add(int, E)} methods are overridden.
	 *
	 * <p>This implementation can be made to throw runtime exceptions in the
	 * face of concurrent modification, as described in the specification for
	 * the (protected) {@link #modCount} field.
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	sp<EListIterator<E> > listIterator(int index = 0) {
		if (index < 0 || index > size()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__,
					EString::formatOf("Index: %d, Size: %d", index, size()).c_str());
		}
		return new ListIterator(this, index);
	}
};

//=============================================================================
//Native pointer type.

template<typename T>
abstract class EAbstractList<T*>: virtual public EAbstractCollection<T*>,
		virtual public EList<T*> {
public:
	typedef T* E;

protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractList() {
	}

private:
	class ListIterator: public EListIterator<E> {
	public:
		ListIterator(EAbstractList<E> *list, int index=0) {
			abslist = list;

			cursor = index;
			lastRet = -1;
		}

		virtual ~ListIterator() {
		}

		boolean hasNext() {
			return cursor != abslist->size();
		}

		E next() {
			E next = abslist->getAt(cursor);
			lastRet = cursor++;
			return next;
		}

		void remove() {
			if (lastRet >= 0) {
				E o = abslist->getAt(lastRet);
				abslist->remove(o);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
			}
		}
		E moveOut() {
			if (lastRet >= 0) {
				E o = abslist->removeAt(lastRet);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
				return o;
			}
			return null;
		}

		boolean hasPrevious() {
			return cursor != 0;
		}

		E previous() {
			int i = cursor - 1;
			E previous = abslist->getAt(i);
			lastRet = cursor = i;
			return previous;
		}

		int nextIndex() {
			return cursor;
		}

		int previousIndex() {
		    return cursor-1;
		}

		void set(E e) {
			if (lastRet == -1)
				return;

			abslist->setAt(lastRet, e);
		}

		void add(E e) {
			abslist->addAt(cursor++, e);
			lastRet = -1;
		}

	private:
		EAbstractList<E> *abslist;

		/**
		 * Index of element to be returned by subsequent call to next.
		 */
		int cursor;

		/**
		 * Index of element returned by most recent call to next or
		 * previous.  Reset to -1 if this element is deleted by a call
		 * to remove.
		 */
		int lastRet;

	};

public:
	/**
	 * count of references.
	 */
	virtual ~EAbstractList(){
	}

	virtual int size() {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * Appends the specified element to the end of this list (optional
	 * operation).
	 *
	 * <p>Lists that support this operation may place limitations on what
	 * elements may be added to this list.  In particular, some
	 * lists will refuse to add null elements, and others will impose
	 * restrictions on the type of elements that may be added.  List
	 * classes should clearly specify in their documentation any restrictions
	 * on what elements may be added.
	 *
	 * <p>This implementation calls {@code add(size(), e)}.
	 *
	 * <p>Note that this implementation throws an
	 * {@code UnsupportedOperationException} unless
	 * {@link #add(int, Object) add(int, E)} is overridden.
	 *
	 * @param e element to be appended to this list
	 * @return {@code true} (as specified by {@link Collection#add})
	 * @throws UnsupportedOperationException if the {@code add} operation
	 *         is not supported by this list
	 * @throws ClassCastException if the class of the specified element
	 *         prevents it from being added to this list
	 * @throws NullPointerException if the specified element is null and this
	 *         list does not permit null elements
	 * @throws IllegalArgumentException if some property of this element
	 *         prevents it from being added to this list
	 */
	virtual boolean add(E e) {
		addAt(size(), e);
		return true;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E getAt(int index) = 0;

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual E setAt(int index, E element) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual void addAt(int index, E element) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual E removeAt(int index) THROWS(EUnsupportedOperationException) {
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
		return EAbstractCollection<E>::remove(o);
	}

	// Search Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation first gets a list iterator (with
	 * {@code listIterator()}).  Then, it iterates over the list until the
	 * specified element is found or the end of the list is reached.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual int indexOf(E o) {
		ListIterator e(this);
		while (e.hasNext())
			if (o == e.next())
				return e.previousIndex();
		return -1;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation first gets a list iterator that points to the end
	 * of the list (with {@code listIterator(size())}).  Then, it iterates
	 * backwards over the list until the specified element is found, or the
	 * beginning of the list is reached.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual int lastIndexOf(E o) {
		ListIterator e(this);
		while (e.hasPrevious())
			if (o == e.previous())
				return e.nextIndex();

		return -1;
	}

	// Bulk Operations

	/**
	 * Removes all of the elements from this list (optional operation).
	 * The list will be empty after this call returns.
	 *
	 * <p>This implementation calls {@code removeRange(0, size())}.
	 *
	 * <p>Note that this implementation throws an
	 * {@code UnsupportedOperationException} unless {@code remove(int
	 * index)} or {@code removeRange(int fromIndex, int toIndex)} is
	 * overridden.
	 *
	 * @throws UnsupportedOperationException if the {@code clear} operation
	 *         is not supported by this list
	 */
	virtual void clear() THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	// Iterators

	/**
	 * Returns an iterator over the elements in this list in proper sequence.
	 *
	 * <p>This implementation returns a straightforward implementation of the
	 * iterator interface, relying on the backing list's {@code size()},
	 * {@code get(int)}, and {@code remove(int)} methods.
	 *
	 * <p>Note that the iterator returned by this method will throw an
	 * {@code UnsupportedOperationException} in response to its
	 * {@code remove} method unless the list's {@code remove(int)} method is
	 * overridden.
	 *
	 * <p>This implementation can be made to throw runtime exceptions in the
	 * face of concurrent modification, as described in the specification
	 * for the (protected) {@code modCount} field.
	 *
	 * @return an iterator over the elements in this list in proper sequence
	 *
	 * @see #modCount
	 */
	virtual sp<EIterator<E> > iterator(int index=0) {
		return new ListIterator(this, index);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a straightforward implementation of the
	 * {@code ListIterator} interface that extends the implementation of the
	 * {@code Iterator} interface returned by the {@code iterator()} method.
	 * The {@code ListIterator} implementation relies on the backing list's
	 * {@code get(int)}, {@code set(int, E)}, {@code add(int, E)}
	 * and {@code remove(int)} methods.
	 *
	 * <p>Note that the list iterator returned by this implementation will
	 * throw an {@link UnsupportedOperationException} in response to its
	 * {@code remove}, {@code set} and {@code add} methods unless the
	 * list's {@code remove(int)}, {@code set(int, E)}, and
	 * {@code add(int, E)} methods are overridden.
	 *
	 * <p>This implementation can be made to throw runtime exceptions in the
	 * face of concurrent modification, as described in the specification for
	 * the (protected) {@link #modCount} field.
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	sp<EListIterator<E> > listIterator(int index = 0) {
		if (index < 0 || index > size()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__,
					EString::formatOf("Index: %d, Size: %d", index, size()).c_str());
		}
		return new ListIterator(this, index);
	}
};

//=============================================================================
//Shared pointer type.

template<typename T>
abstract class EAbstractList<sp<T> >: virtual public EAbstractCollection<sp<T> >,
		virtual public EList<sp<T> > {
public:
	typedef sp<T> E;

protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractList() {
	}

private:
	class ListIterator: public EListIterator<E> {
	public:
		ListIterator(EAbstractList<E> *list, int index=0) {
			abslist = list;

			cursor = index;
			lastRet = -1;
		}

		virtual ~ListIterator() {
		}

		boolean hasNext() {
			return cursor != abslist->size();
		}

		E next() {
			E next = abslist->getAt(cursor);
			lastRet = cursor++;
			return next;
		}

		void remove() {
			if (lastRet >= 0) {
				E o = abslist->getAt(lastRet);
				abslist->remove(o.get());
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
			}
		}
		E moveOut() {
			if (lastRet >= 0) {
				E o = abslist->removeAt(lastRet);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
				return o;
			}
			throw EILLEGALSTATEEXCEPTION;
		}

		boolean hasPrevious() {
			return cursor != 0;
		}

		E previous() {
			int i = cursor - 1;
			E previous = abslist->getAt(i);
			lastRet = cursor = i;
			return previous;
		}

		int nextIndex() {
			return cursor;
		}

		int previousIndex() {
		    return cursor-1;
		}

		void set(E e) {
			if (lastRet == -1)
				return;

			abslist->setAt(lastRet, e);
		}

		void add(E e) {
			abslist->addAt(cursor++, e);
			lastRet = -1;
		}

	private:
		EAbstractList<E> *abslist;

		/**
		 * Index of element to be returned by subsequent call to next.
		 */
		int cursor;

		/**
		 * Index of element returned by most recent call to next or
		 * previous.  Reset to -1 if this element is deleted by a call
		 * to remove.
		 */
		int lastRet;

	};

public:
	/**
	 * count of references.
	 */
	virtual ~EAbstractList(){
	}

	virtual int size() {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * Appends the specified element to the end of this list (optional
	 * operation).
	 *
	 * <p>Lists that support this operation may place limitations on what
	 * elements may be added to this list.  In particular, some
	 * lists will refuse to add null elements, and others will impose
	 * restrictions on the type of elements that may be added.  List
	 * classes should clearly specify in their documentation any restrictions
	 * on what elements may be added.
	 *
	 * <p>This implementation calls {@code add(size(), e)}.
	 *
	 * <p>Note that this implementation throws an
	 * {@code UnsupportedOperationException} unless
	 * {@link #add(int, Object) add(int, E)} is overridden.
	 *
	 * @param e element to be appended to this list
	 * @return {@code true} (as specified by {@link Collection#add})
	 * @throws UnsupportedOperationException if the {@code add} operation
	 *         is not supported by this list
	 * @throws ClassCastException if the class of the specified element
	 *         prevents it from being added to this list
	 * @throws NullPointerException if the specified element is null and this
	 *         list does not permit null elements
	 * @throws IllegalArgumentException if some property of this element
	 *         prevents it from being added to this list
	 */
	virtual boolean add(E e) {
		addAt(size(), e);
		return true;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E getAt(int index) = 0;

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual E setAt(int index, E element) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual void addAt(int index, E element) THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * {@code UnsupportedOperationException}.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws IndexOutOfBoundsException     {@inheritDoc}
	 */
	virtual E removeAt(int index) THROWS(EUnsupportedOperationException) {
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
	virtual boolean remove(T* o) {
		return EAbstractCollection<E>::remove(o);
	}

	// Search Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation first gets a list iterator (with
	 * {@code listIterator()}).  Then, it iterates over the list until the
	 * specified element is found or the end of the list is reached.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual int indexOf(T* o) {
		ListIterator e(this);
		while (e.hasNext())
			if (e.next() == o)
				return e.previousIndex();
		return -1;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation first gets a list iterator that points to the end
	 * of the list (with {@code listIterator(size())}).  Then, it iterates
	 * backwards over the list until the specified element is found, or the
	 * beginning of the list is reached.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual int lastIndexOf(T* o) {
		ListIterator e(this);
		while (e.hasPrevious())
			if (e.previous() == o)
				return e.nextIndex();

		return -1;
	}

	// Bulk Operations

	/**
	 * Removes all of the elements from this list (optional operation).
	 * The list will be empty after this call returns.
	 *
	 * <p>This implementation calls {@code removeRange(0, size())}.
	 *
	 * <p>Note that this implementation throws an
	 * {@code UnsupportedOperationException} unless {@code remove(int
	 * index)} or {@code removeRange(int fromIndex, int toIndex)} is
	 * overridden.
	 *
	 * @throws UnsupportedOperationException if the {@code clear} operation
	 *         is not supported by this list
	 */
	virtual void clear() THROWS(EUnsupportedOperationException) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	// Iterators

	/**
	 * Returns an iterator over the elements in this list in proper sequence.
	 *
	 * <p>This implementation returns a straightforward implementation of the
	 * iterator interface, relying on the backing list's {@code size()},
	 * {@code get(int)}, and {@code remove(int)} methods.
	 *
	 * <p>Note that the iterator returned by this method will throw an
	 * {@code UnsupportedOperationException} in response to its
	 * {@code remove} method unless the list's {@code remove(int)} method is
	 * overridden.
	 *
	 * <p>This implementation can be made to throw runtime exceptions in the
	 * face of concurrent modification, as described in the specification
	 * for the (protected) {@code modCount} field.
	 *
	 * @return an iterator over the elements in this list in proper sequence
	 *
	 * @see #modCount
	 */
	virtual sp<EIterator<E> > iterator(int index=0) {
		return new ListIterator(this, index);
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a straightforward implementation of the
	 * {@code ListIterator} interface that extends the implementation of the
	 * {@code Iterator} interface returned by the {@code iterator()} method.
	 * The {@code ListIterator} implementation relies on the backing list's
	 * {@code get(int)}, {@code set(int, E)}, {@code add(int, E)}
	 * and {@code remove(int)} methods.
	 *
	 * <p>Note that the list iterator returned by this implementation will
	 * throw an {@link UnsupportedOperationException} in response to its
	 * {@code remove}, {@code set} and {@code add} methods unless the
	 * list's {@code remove(int)}, {@code set(int, E)}, and
	 * {@code add(int, E)} methods are overridden.
	 *
	 * <p>This implementation can be made to throw runtime exceptions in the
	 * face of concurrent modification, as described in the specification for
	 * the (protected) {@link #modCount} field.
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	sp<EListIterator<E> > listIterator(int index = 0) {
		if (index < 0 || index > size()) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__,
					EString::formatOf("Index: %d, Size: %d", index, size()).c_str());
		}
		return new ListIterator(this, index);
	}
};

} /* namespace efc */
#endif //!EABSTRACTLIST_HH_
