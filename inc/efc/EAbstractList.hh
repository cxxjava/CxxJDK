/*
 * EList.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EABSTRACTLIST_HH_
#define EABSTRACTLIST_HH_

#include "EList.hh"
#include "EAbstractCollection.hh"
#include "EUnsupportedOperationException.hh"

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
 * @author  Josh Bloch
 * @author  Neal Gafter
 * @version 1.52, 06/16/06
 * @since 1.2
 */

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

	template<typename ELI>
	class ListIterator: public EIterator<ELI> {
	public:
		ListIterator(EAbstractList<ELI> *list, int index=0) {
			abslist = list;

			cursor = index;
			lastRet = -1;
		}

		virtual ~ListIterator() {
		}

		boolean hasNext() {
			return cursor != abslist->size();
		}

		ELI next() {
			ELI next = abslist->getAt(cursor);
			lastRet = cursor++;
			return next;
		}

		void remove() {
			if (lastRet >= 0) {
				ELI o = abslist->getAt(lastRet);
				abslist->remove(o);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
			}
		}

		boolean hasPrevious() {
			return cursor != 0;
		}

		ELI previous() {
			int i = cursor - 1;
			ELI previous = abslist->getAt(i);
			lastRet = cursor = i;
			return previous;
		}

		int nextIndex() {
			return cursor;
		}

		int previousIndex() {
		    return cursor-1;
		}

		void set(ELI e) {
			if (lastRet == -1)
				return;

			abslist->set(lastRet, e);
		}

		void add(ELI e) {
			abslist->add(cursor++, e);
			lastRet = -1;
		}

	private:
		EAbstractList<ELI> *abslist;

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
		ListIterator<E> e(this);
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
		ListIterator<E> e(this);
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
	virtual EIterator<E>* iterator(int index=0) {
		return new ListIterator<E>(this, index);
	}
};

//=============================================================================

template<>
abstract class EAbstractList<int>: virtual public EAbstractCollection<int>,
		virtual public EList<int> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractList() {
	}

	template<typename ELI>
	class ListIterator: public EIterator<ELI> {
	public:
		ListIterator(EAbstractList<ELI> *list, int index=0) {
			abslist = list;

			cursor = index;
			lastRet = -1;
		}

		boolean hasNext() {
			return cursor != abslist->size();
		}

		ELI next() {
			ELI next = abslist->getAt(cursor);
			lastRet = cursor++;
			return next;
		}

		void remove() {
			if (lastRet >= 0) {
				ELI o = abslist->getAt(lastRet);
				abslist->remove(o);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
			}
		}

		boolean hasPrevious() {
			return cursor != 0;
		}

		ELI previous() {
			int i = cursor - 1;
			ELI previous = abslist->getAt(i);
			lastRet = cursor = i;
			return previous;
		}

		int nextIndex() {
			return cursor;
		}

		int previousIndex() {
		    return cursor-1;
		}

		void set(ELI e) {
			if (lastRet == -1)
				return;

			abslist->set(lastRet, e);
		}

		void add(ELI e) {
			abslist->add(cursor++, e);
			lastRet = -1;
		}

	private:
		EAbstractList<ELI> *abslist;

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
	virtual boolean add(int e) {
		addAt(size(), e);
		return true;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual int getAt(int index) = 0;

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
	virtual int setAt(int index, int element) THROWS(EUnsupportedOperationException) {
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
	virtual void addAt(int index, int element) THROWS(EUnsupportedOperationException) {
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
	virtual int removeAt(int index) THROWS(EUnsupportedOperationException) {
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
		return EAbstractCollection<int>::remove(o);
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
	virtual int indexOf(int o) {
		ListIterator<int> e(this);
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
	virtual int lastIndexOf(int o) {
		ListIterator<int> e(this);
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
	virtual EIterator<int>* iterator(int index=0) {
		return new ListIterator<int>(this, index);
	}
};

//=============================================================================

template<>
abstract class EAbstractList<llong>: virtual public EAbstractCollection<llong>,
		virtual public EList<llong> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractList() {
	}

	template<typename ELI>
	class ListIterator: public EIterator<ELI> {
	public:
		ListIterator(EAbstractList<ELI> *list, int index=0) {
			abslist = list;

			cursor = index;
			lastRet = -1;
		}

		boolean hasNext() {
			return cursor != abslist->size();
		}

		ELI next() {
			ELI next = abslist->getAt(cursor);
			lastRet = cursor++;
			return next;
		}

		void remove() {
			if (lastRet >= 0) {
				ELI o = abslist->getAt(lastRet);
				abslist->remove(o);
				if (lastRet < cursor)
					cursor--;
				lastRet = -1;
			}
		}

		boolean hasPrevious() {
			return cursor != 0;
		}

		ELI previous() {
			int i = cursor - 1;
			ELI previous = abslist->getAt(i);
			lastRet = cursor = i;
			return previous;
		}

		int nextIndex() {
			return cursor;
		}

		int previousIndex() {
		    return cursor-1;
		}

		void set(ELI e) {
			if (lastRet == -1)
				return;

			abslist->set(lastRet, e);
		}

		void add(ELI e) {
			abslist->add(cursor++, e);
			lastRet = -1;
		}

	private:
		EAbstractList<ELI> *abslist;

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
	virtual boolean add(llong e) {
		addAt(size(), e);
		return true;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual llong getAt(int index) = 0;

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
	virtual llong setAt(int index, llong element) THROWS(EUnsupportedOperationException) {
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
	virtual void addAt(int index, llong element) THROWS(EUnsupportedOperationException) {
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
	virtual llong removeAt(int index) THROWS(EUnsupportedOperationException) {
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
		return EAbstractCollection<llong>::remove(o);
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
	virtual int indexOf(llong o) {
		ListIterator<llong> e(this);
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
	virtual int lastIndexOf(llong o) {
		ListIterator<llong> e(this);
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
	virtual EIterator<llong>* iterator(int index=0) {
		return new ListIterator<llong>(this, index);
	}
};

} /* namespace efc */
#endif //!EABSTRACTLIST_HH_
