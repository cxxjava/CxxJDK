/*
 * EVector.hh
 *
 *  Created on: 2013-7-25
 *      Author: cxxjava@163.com
 */

#ifndef EVECTOR_HH_
#define EVECTOR_HH_

#include "EArrayList.hh"
#include "ESharedPtr.hh"
#include "EEnumeration.hh"
#include "ESynchronizeable.hh"
#include "EIndexOutOfBoundsException.hh"
#include "ENoSuchElementException.hh"

namespace efc {

/**
 * The {@code Vector} class implements a growable array of
 * objects. Like an array, it contains components that can be
 * accessed using an integer index. However, the size of a
 * {@code Vector} can grow or shrink as needed to accommodate
 * adding and removing items after the {@code Vector} has been created.
 *
 * <p>Each vector tries to optimize storage management by maintaining a
 * {@code capacity} and a {@code capacityIncrement}. The
 * {@code capacity} is always at least as large as the vector
 * size; it is usually larger because as components are added to the
 * vector, the vector's storage increases in chunks the size of
 * {@code capacityIncrement}. An application can increase the
 * capacity of a vector before inserting a large number of
 * components; this reduces the amount of incremental reallocation.
 *
 * <p>The Iterators returned by Vector's iterator and listIterator
 * methods are <em>fail-fast</em>: if the Vector is structurally modified
 * at any time after the Iterator is created, in any way except through the
 * Iterator's own remove or add methods, the Iterator will throw a
 * ConcurrentModificationException.  Thus, in the face of concurrent
 * modification, the Iterator fails quickly and cleanly, rather than risking
 * arbitrary, non-deterministic behavior at an undetermined time in the future.
 * The Enumerations returned by Vector's elements method are <em>not</em>
 * fail-fast.
 *
 * <p>Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw {@code ConcurrentModificationException} on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness:  <i>the fail-fast behavior of iterators
 * should be used only to detect bugs.</i>
 *
 * <p>As of the Java 2 platform v1.2, this class was retrofitted to
 * implement the {@link List} interface, making it a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html"> Java
 * Collections Framework</a>.  Unlike the new collection
 * implementations, {@code Vector} is synchronized.
 *
 * @version 1.106, 06/16/06
 * @see Collection
 * @see List
 * @see ArrayList
 * @see LinkedList
 * @since   JDK1.0
 */

#define SYNC_IF(b) { \
		ESentry __synchronizer__((b) ? this->getLock() : null);

template<typename E>
class EVector: public EArrayList<E>, public ESynchronizeable {
protected:
	boolean _threadSafe;

public:
	virtual ~EVector() {
	}

	EVector(const EVector<E>& that) :
		EArrayList<E>(that), _threadSafe(that._threadSafe) {
	}

	/**
	 * Constructs an empty vector with the specified initial capacity and
	 * with its capacity increment equal to zero.
	 *
	 * @param   initialCapacity   the initial capacity of the vector
	 * @throws IllegalArgumentException if the specified initial capacity
	 *         is negative
	 */
	explicit
	EVector(boolean autoFree=true) :
		EArrayList<E>(autoFree), _threadSafe(true) {
	}

	explicit
	EVector(int initialCapacity) :
		EArrayList<E>(initialCapacity), _threadSafe(true) {
	}

	explicit
	EVector(boolean autoFree, int initialCapacity) :
		EArrayList<E>(autoFree, initialCapacity), _threadSafe(true) {
	}

	/**
	 *
	 */
	void setThreadSafe(boolean safe=true) {
		SYNCHRONIZED(this) {
			_threadSafe = safe;
        }}
	}

	/**
	 *
	 */
	boolean getThreadSafe() {
		SYNCHRONIZED(this) {
			return _threadSafe;
        }}
	}

	/**
	 * Returns the current capacity of this vector.
	 *
	 * @return  the current capacity (the length of its internal
	 *          data array, kept in the field {@code elementData}
	 *          of this vector)
	 */
	virtual int capacity() {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::capacity();
		}}
	}

	/**
	 * Returns the number of components in this vector.
	 *
	 * @return  the number of components in this vector
	 */
	virtual int size() {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::size();
		}}
	}

	/**
	 * Tests if this vector has no components.
	 *
	 * @return  {@code true} if and only if this vector has
	 *          no components, that is, its size is zero;
	 *          {@code false} otherwise.
	 */
	virtual boolean isEmpty() {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::isEmpty();
		}}
	}

protected:
	class EVEnumeration : public EEnumeration<E> {
	public:
		~EVEnumeration() {
			//
		}
		EVEnumeration(EVector<E>* v) {
			_iter = ((EAbstractList<E>*)v)->iterator();
		}
		boolean hasMoreElements() {
			return _iter->hasNext();
		}
		E nextElement() {
			return _iter->next();
		}
	private:
		sp<EIterator<E> > _iter;
	};

public:
	/**
	 * Returns an enumeration of the components of this vector. The
	 * returned {@code Enumeration} object will generate all items in
	 * this vector. The first item generated is the item at index {@code 0},
	 * then the item at index {@code 1}, and so on.
	 *
	 * @return  an enumeration of the components of this vector
	 * @see     Iterator
	 */
	sp<EEnumeration<E> > elements() {
		return new EVEnumeration(this);
	}

	/**
	 * Returns {@code true} if this vector contains the specified element.
	 * More formally, returns {@code true} if and only if this vector
	 * contains at least one element {@code e} such that
	 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>.
	 *
	 * @param o element whose presence in this vector is to be tested
	 * @return {@code true} if this vector contains the specified element
	 */
	boolean contains(E o) {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::contains(o);
		}}
	}

	/**
	 * Returns the index of the first occurrence of the specified element in
	 * this vector, searching forwards from {@code index}, or returns -1 if
	 * the element is not found.
	 * More formally, returns the lowest index {@code i} such that
	 * <tt>(i&nbsp;&gt;=&nbsp;index&nbsp;&amp;&amp;&nbsp;(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i))))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @param index index to start searching from
	 * @return the index of the first occurrence of the element in
	 *         this vector at position {@code index} or later in the vector;
	 *         {@code -1} if the element is not found.
	 * @throws IndexOutOfBoundsException if the specified index is negative
	 * @see     Object#equals(Object)
	 */
	virtual int indexOf(E o, int index = 0) {
		SYNC_IF(_threadSafe) {
			if (index < 0 || index >= size()) {
				throw EINDEXOUTOFBOUNDSEXCEPTION;
			}
			for (int i = index; i < size(); i++) {
				if (getAt(i)->equals(o))
					return i;
			}
			return -1;
		}}
	}

	/**
	 * Returns the index of the last occurrence of the specified element in
	 * this vector, searching backwards from {@code index}, or returns -1 if
	 * the element is not found.
	 * More formally, returns the highest index {@code i} such that
	 * <tt>(i&nbsp;&lt;=&nbsp;index&nbsp;&amp;&amp;&nbsp;(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i))))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @param index index to start searching backwards from
	 * @return the index of the last occurrence of the element at position
	 *         less than or equal to {@code index} in this vector;
	 *         -1 if the element is not found.
	 * @throws IndexOutOfBoundsException if the specified index is greater
	 *         than or equal to the current size of this vector
	 */
	virtual int lastIndexOf(E o, int index = -1) {
		SYNC_IF(_threadSafe) {
			if (size() == 0) {
				return -1;
			}
			if (index < 0) {
				index = size() - 1;
			}
			if (index >= size()) {
				throw EINDEXOUTOFBOUNDSEXCEPTION;
			}
			for (int i = index; i >= 0; i--) {
				if (getAt(i)->equals(o))
					return i;
			}
			return -1;
		}}
	}

	/**
	 * Returns the component at the specified index.
	 *
	 * <p>This method is identical in functionality to the {@link #get(int)}
	 * method (which is part of the {@link List} interface).
	 *
	 * @param      index   an index into this vector
	 * @return     the component at the specified index
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *	       ({@code index < 0 || index >= size()})
	 */
	virtual E elementAt(int index) {
		SYNC_IF(_threadSafe) {
			return getAt(index);
		}}
	}

	/**
	 * Returns the first component (the item at index {@code 0}) of
	 * this vector.
	 *
	 * @return     the first component of this vector
	 * @throws NoSuchElementException if this vector has no components
	 */
	virtual E firstElement() {
		SYNC_IF(_threadSafe) {
			if (size() == 0) {
				throw ENOSUCHELEMENTEXCEPTION;
			}
			return getAt(0);
		}}
	}

	/**
	 * Returns the last component of the vector.
	 *
	 * @return  the last component of the vector, i.e., the component at index
	 *          <code>size()&nbsp;-&nbsp;1</code>.
	 * @throws NoSuchElementException if this vector is empty
	 */
	virtual E lastElement() {
		SYNC_IF(_threadSafe) {
			if (size() == 0) {
				throw ENOSUCHELEMENTEXCEPTION;
			}
			return getAt(size() - 1);
		}}
	}

	/**
	 * Sets the component at the specified {@code index} of this
	 * vector to be the specified object. The previous component at that
	 * position is discarded.
	 *
	 * <p>The index must be a value greater than or equal to {@code 0}
	 * and less than the current size of the vector.
	 *
	 * <p>This method is identical in functionality to the
	 * {@link #set(int, Object) set(int, E)}
	 * method (which is part of the {@link List} interface). Note that the
	 * {@code set} method reverses the order of the parameters, to more closely
	 * match array usage.  Note also that the {@code set} method returns the
	 * old value that was stored at the specified position.
	 *
	 * @param      obj     what the component is to be set to
	 * @param      index   the specified index
	 * @return     the old component
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *	       ({@code index < 0 || index >= size()})
	 */
	virtual E setElementAt(E obj, int index) {
		SYNC_IF(_threadSafe) {
			return setAt(index, obj);
		}}
	}

	/**
	 * Deletes the component at the specified index. Each component in
	 * this vector with an index greater or equal to the specified
	 * {@code index} is shifted downward to have an index one
	 * smaller than the value it had previously. The size of this vector
	 * is decreased by {@code 1}.
	 *
	 * <p>The index must be a value greater than or equal to {@code 0}
	 * and less than the current size of the vector.
	 *
	 * <p>This method is identical in functionality to the {@link #remove(int)}
	 * method (which is part of the {@link List} interface).  Note that the
	 * {@code remove} method returns the old value that was stored at the
	 * specified position.
	 *
	 * @param      index   the index of the object to remove
	 * @return  the moved component
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *	       ({@code index < 0 || index >= size()})
	 */
	virtual E removeElementAt(int index) {
		SYNC_IF(_threadSafe) {
			return removeAt(index);
		}}
	}

	/**
	 * Inserts the specified object as a component in this vector at the
	 * specified {@code index}. Each component in this vector with
	 * an index greater or equal to the specified {@code index} is
	 * shifted upward to have an index one greater than the value it had
	 * previously.
	 *
	 * <p>The index must be a value greater than or equal to {@code 0}
	 * and less than or equal to the current size of the vector. (If the
	 * index is equal to the current size of the vector, the new element
	 * is appended to the Vector.)
	 *
	 * <p>This method is identical in functionality to the
	 * {@link #add(int, Object) add(int, E)}
	 * method (which is part of the {@link List} interface).  Note that the
	 * {@code add} method reverses the order of the parameters, to more closely
	 * match array usage.
	 *
	 * @param      obj     the component to insert
	 * @param      index   where to insert the new component
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *	       ({@code index < 0 || index > size()})
	 */
	virtual void insertElementAt(E obj, int index) {
		SYNC_IF(_threadSafe) {
			addAt(index, obj);
		}}
	}

	/**
	 * Adds the specified component to the end of this vector,
	 * increasing its size by one. The capacity of this vector is
	 * increased if its size becomes greater than its capacity.
	 *
	 * <p>This method is identical in functionality to the
	 * {@link #add(Object) add(E)}
	 * method (which is part of the {@link List} interface).
	 *
	 * @param   obj   the component to be added
	 */
	virtual void addElement(E obj) {
		SYNC_IF(_threadSafe) {
			add(obj);
		}}
	}

	/**
	 * Removes the first (lowest-indexed) occurrence of the argument
	 * from this vector. If the object is found in this vector, each
	 * component in the vector with an index greater or equal to the
	 * object's index is shifted downward to have an index one smaller
	 * than the value it had previously.
	 *
	 * <p>This method is identical in functionality to the
	 * {@link #remove(Object)} method (which is part of the
	 * {@link List} interface).
	 *
	 * @param   obj   the component to be removed
	 * @return  {@code true} if the argument was a component of this
	 *          vector; {@code false} otherwise.
	 */
	virtual boolean removeElement(E obj) {
		SYNC_IF(_threadSafe) {
			return remove(obj);
		}}
	}

	/**
	 * Removes all components from this vector and sets its size to zero.
	 *
	 * <p>This method is identical in functionality to the {@link #clear}
	 * method (which is part of the {@link List} interface).
	 */
	virtual void removeAllElements() {
		clear();
	}

	// Positional Access Operations

	/**
	 * Returns the element at the specified position in this Vector.
	 *
	 * @param index index of the element to return
	 * @return object at the specified index
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *            ({@code index < 0 || index >= size()})
	 * @since 1.2
	 */
	virtual E getAt(int index) {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::getAt(index);
		}}
	}

	/**
	 * Replaces the element at the specified position in this Vector with the
	 * specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *	       ({@code index < 0 || index >= size()})
	 * @since 1.2
	 */
	virtual E setAt(int index, E element) {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::setAt(index, element);
		}}
	}

	/**
	 * Appends the specified element to the end of this Vector.
	 *
	 * @param e element to be appended to this Vector
	 * @return {@code true} (as specified by {@link Collection#add})
	 * @since 1.2
	 */
	virtual boolean add(E e) {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::add(e);
		}}
	}

	/**
	 * Removes the first occurrence of the specified element in this Vector
	 * If the Vector does not contain the element, it is unchanged.  More
	 * formally, removes the element with the lowest index i such that
	 * {@code (o==null ? get(i)==null : o.equals(get(i)))} (if such
	 * an element exists).
	 *
	 * @param o element to be removed from this Vector, if present
	 * @return true if the Vector contained the specified element
	 * @since 1.2
	 */
	virtual boolean remove(E o) {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::remove(o);
		}}
	}

	/**
	 * Inserts the specified element at the specified position in this Vector.
	 * Shifts the element currently at that position (if any) and any
	 * subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *         ({@code index < 0 || index > size()})
	 * @since 1.2
	 */
	virtual void addAt(int index, E element) {
		SYNC_IF(_threadSafe) {
			EArrayList<E>::addAt(index, element);
		}}
	}

	/**
	 * Removes the element at the specified position in this Vector.
	 * Shifts any subsequent elements to the left (subtracts one from their
	 * indices).  Returns the element that was removed from the Vector.
	 *
	 * @throws ArrayIndexOutOfBoundsException if the index is out of range
	 *         ({@code index < 0 || index >= size()})
	 * @param index the index of the element to be removed
	 * @return element that was removed
	 * @since 1.2
	 */
	virtual E removeAt(int index) {
		SYNC_IF(_threadSafe) {
			return EArrayList<E>::removeAt(index);
		}}
	}

	/**
	 * Removes all of the elements from this Vector.  The Vector will
	 * be empty after this call returns (unless it throws an exception).
	 *
	 * @since 1.2
	 */
	virtual void clear() {
		SYNC_IF(_threadSafe) {
			EArrayList<E>::clear();
		}}
	}
};

} /* namespace efc */
#endif /* EVECTOR_HH_ */
