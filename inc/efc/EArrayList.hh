/*
 * EArrayList.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EARRAYLIST_HH_
#define EARRAYLIST_HH_

#include "EA.hh"
#include "EList.hh"
#include "EAbstractList.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

/**
 * Resizable-array implementation of the <tt>List</tt> interface.  Implements
 * all optional list operations, and permits all elements, including
 * <tt>null</tt>.  In addition to implementing the <tt>List</tt> interface,
 * this class provides methods to manipulate the size of the array that is
 * used internally to store the list.  (This class is roughly equivalent to
 * <tt>Vector</tt>, except that it is unsynchronized.)<p>
 *
 * The <tt>size</tt>, <tt>isEmpty</tt>, <tt>get</tt>, <tt>set</tt>,
 * <tt>iterator</tt>, and <tt>listIterator</tt> operations run in constant
 * time.  The <tt>add</tt> operation runs in <i>amortized constant time</i>,
 * that is, adding n elements requires O(n) time.  All of the other operations
 * run in linear time (roughly speaking).  The constant factor is low compared
 * to that for the <tt>LinkedList</tt> implementation.<p>
 *
 * Each <tt>ArrayList</tt> instance has a <i>capacity</i>.  The capacity is
 * the size of the array used to store the elements in the list.  It is always
 * at least as large as the list size.  As elements are added to an ArrayList,
 * its capacity grows automatically.  The details of the growth policy are not
 * specified beyond the fact that adding an element has constant amortized
 * time cost.<p>
 *
 * An application can increase the capacity of an <tt>ArrayList</tt> instance
 * before adding a large number of elements using the <tt>ensureCapacity</tt>
 * operation.  This may reduce the amount of incremental reallocation.
 *
 * <p><strong>Note that this implementation is not synchronized.</strong>
 * If multiple threads access an <tt>ArrayList</tt> instance concurrently,
 * and at least one of the threads modifies the list structurally, it
 * <i>must</i> be synchronized externally.  (A structural modification is
 * any operation that adds or deletes one or more elements, or explicitly
 * resizes the backing array; merely setting the value of an element is not
 * a structural modification.)  This is typically accomplished by
 * synchronizing on some object that naturally encapsulates the list.
 *
 * If no such object exists, the list should be "wrapped" using the
 * {@link Collections#synchronizedList Collections.synchronizedList}
 * method.  This is best done at creation time, to prevent accidental
 * unsynchronized access to the list:<pre>
 *   List list = Collections.synchronizedList(new ArrayList(...));</pre>
 *
 * <p>The iterators returned by this class's <tt>iterator</tt> and
 * <tt>listIterator</tt> methods are <i>fail-fast</i>: if the list is
 * structurally modified at any time after the iterator is created, in any way
 * except through the iterator's own <tt>remove</tt> or <tt>add</tt> methods,
 * the iterator will throw a {@link ConcurrentModificationException}.  Thus, in
 * the face of concurrent modification, the iterator fails quickly and cleanly,
 * rather than risking arbitrary, non-deterministic behavior at an undetermined
 * time in the future.<p>
 *
 * Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw <tt>ConcurrentModificationException</tt> on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness: <i>the fail-fast behavior of iterators
 * should be used only to detect bugs.</i><p>
 *
 * This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @version 1.56, 04/21/06
 * @see	    Collection
 * @see	    List
 * @see	    LinkedList
 * @see	    Vector
 * @since   1.2
 */

template<typename E>
class EArrayList : public EAbstractList<E> {
public:
	virtual ~EArrayList() {
		clear();
		eso_array_free(&arrayBuffer);
	}

	explicit
	EArrayList(boolean autoFree=true) : _autoFree(autoFree) {
		arrayBuffer = eso_array_make(32, sizeof(E));
	}

	explicit
	EArrayList(int initialCapacity) : _autoFree(true) {
		arrayBuffer = eso_array_make(initialCapacity, sizeof(E));
	}

	explicit
	EArrayList(boolean autoFree, int initialCapacity) :
			_autoFree(autoFree) {
		arrayBuffer = eso_array_make(initialCapacity, sizeof(E));
	}

	EArrayList(const EArrayList<E>& that) : _autoFree(false) {
		EArrayList<E>* t = (EArrayList<E>*)&that;

		arrayBuffer = eso_array_make(t->size(), sizeof(E));
		for (int i = 0; i < t->size(); i++) {
			E o = *(E*) eso_array_get(t->arrayBuffer, i);
			eso_array_push(arrayBuffer, (void*)&o, sizeof(E));
		}
		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);
	}

	EArrayList<E>& operator= (const EArrayList<E>& that) {
		if (this == &that) return *this;

		EArrayList<E>* t = (EArrayList<E>*)&that;

		//1.
		clear();
		eso_array_free(&arrayBuffer);

		//2.
		arrayBuffer = eso_array_make(t->size(), sizeof(E));
		for (int i = 0; i < t->size(); i++) {
			E o = *(E*) eso_array_get(t->arrayBuffer, i);
			eso_array_push(arrayBuffer, (void*)&o, sizeof(E));
		}
		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);

		return *this;
	}

	/**
	 *
	 */
	void setAutoFree(boolean autoFree=true) {
		_autoFree = autoFree;
	}

	/**
	 *
	 */
	boolean getAutoFree() {
		return _autoFree;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * @param e element to be appended to this list
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 */
	virtual boolean add(E e) {
		eso_array_push(arrayBuffer, (void*)&e, sizeof(E));
		return true;
	}

	/**
	 * Removes the first occurrence of the specified element from this list,
	 * if it is present.  If the list does not contain the element, it is
	 * unchanged.  More formally, removes the element with the lowest index
	 * <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>
	 * (if such an element exists).  Returns <tt>true</tt> if this list
	 * contained the specified element (or equivalently, if this list
	 * changed as a result of the call).
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if this list contained the specified element
	 */
	virtual boolean remove(E obj) {
		for (int i = 0; i < size(); i++) {
			E o = *(E*) eso_array_get(arrayBuffer, i);
			if (o->equals(obj)) {
				eso_array_delete(arrayBuffer, i);
				if (_autoFree) {
					delete o;
				}
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	virtual int size() {
		return eso_array_count(arrayBuffer);
	}

	/**
	 *
	 */
	virtual int capacity() {
		return eso_array_capacity(arrayBuffer);
	}

	/**
	 * Returns <tt>true</tt> if this list contains no elements.
	 *
	 * @return <tt>true</tt> if this list contains no elements
	 */
	virtual boolean isEmpty() {
		return eso_array_is_empty(arrayBuffer);
	}

	/**
	 * Returns <tt>true</tt> if this list contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this list contains
	 * at least one element <tt>e</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>.
	 *
	 * @param o element whose presence in this list is to be tested
	 * @return <tt>true</tt> if this list contains the specified element
	 */
	virtual boolean contains(E o) {
		return indexOf(o) >= 0;
	}

	virtual boolean removeAll(ECollection<E> *c) {
		//TODO...
		return false;
	}

	virtual boolean retainAll(ECollection<E> *c) {
		//TODO...
		return false;
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param  index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E getAt(int index) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		return *(E*)eso_array_get(arrayBuffer, index);
	}

	/**
	 * Inserts the specified element at the specified position in this
	 * list. Shifts the element currently at that position (if any) and
	 * any subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual void addAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		eso_array_insert(arrayBuffer, index, (void*)&element, sizeof(E));
	}

	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		E old = *(E*)eso_array_get(arrayBuffer, index);
		eso_array_set(arrayBuffer, index, (void*)&element, sizeof(E));
		return old;
	}

	/**
	 * Removes the element at the specified position in this list.
	 * Shifts any subsequent elements to the left (subtracts one from their
	 * indices).
	 *
	 * @param index the index of the element to be removed
	 * @return the element that was removed from the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E removeAt(int index) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		void *p = eso_array_get(arrayBuffer, index);
		E r = *(E*)p;
		eso_array_delete(arrayBuffer, index);
		return r;
	}

	/**
	 * Removes all of the elements from this list.  The list will
	 * be empty after this call returns.
	 */
	virtual void clear() {
		if (_autoFree) {
			for (int i = 0; i < size(); i++) {
				E obj = *(E*) eso_array_get(arrayBuffer, i);
				delete obj;
			}
		}
		eso_array_clear(arrayBuffer);
	}

	/**
	 * Returns the index of the first occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 */
	virtual int indexOf(E obj) {
		for (int i = 0; i < size(); i++) {
			if (getAt(i)->equals(obj))
				return i;
		}
		return -1;
	}

	/**
	 * Returns the index of the last occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the highest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 */
	virtual int lastIndexOf(E obj) {
		for (int i = size() - 1; i >= 0; i--) {
			if (getAt(i)->equals(obj))
				return i;
		}
		return -1;
	}

	/**
	 * Returns a shallow copy of this <tt>ArrayList</tt> instance.  (The
	 * elements themselves are not copied.)
	 *
	 * @return a clone of this <tt>ArrayList</tt> instance
	 */
	virtual EArrayList<E>* clone(int fromIndex=0, int toIndex=-1) {
		int size_;

		if (fromIndex == 0 && toIndex < 0) {
			size_ = this->size();
		}
		else {
			RangeCheck2(fromIndex, toIndex);
			size_ = toIndex - fromIndex;
		}

		EArrayList<E> *ac = new EArrayList<E>();

		ac->arrayBuffer = eso_array_make(size_, sizeof(E));
		for (int i = 0; i < size_; i++) {
			E o = *(E*) eso_array_get(this->arrayBuffer, fromIndex + i);
			eso_array_push(ac->arrayBuffer, (void*) &o, sizeof(E));
		}
		ac->setAutoFree(false);

		return ac;
	}

	virtual EA<E> toArray() {
		int size_ = size();
		EA<E> result(size_, false);

		for (int i = 0; i < size_; i++) {
			E o = *(E*) eso_array_get(this->arrayBuffer, i);
			result[i] = o;
		}

		return result;
	}

protected:
	es_array_t* arrayBuffer;
	boolean _autoFree;

private:
	/**
	 * Checks if the given index is in range.  If not, throws an appropriate
	 * runtime exception.  This method does *not* check if the index is
	 * negative: It is always used immediately prior to an array access,
	 * which throws an ArrayIndexOutOfBoundsException if index is negative.
	 */
	void RangeCheck(int index) {
		int _size = size();
		if (index >= _size) {
			EString str;
			str.format("Index: %d, Size: %d", index, _size);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, str.c_str());
		}
	}

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void RangeCheck2(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > this->size()) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}
};

//=============================================================================

template<>
class EArrayList<int> : virtual public EObject, public EAbstractList<int> {
public:
	virtual ~EArrayList() {
		clear();
		eso_array_free(&arrayBuffer);
	}

	EArrayList(int initialCapacity = 32) {
		arrayBuffer = eso_array_make(initialCapacity, sizeof(int));
	}

	EArrayList(const EArrayList<int>& that) {
		EArrayList<int>* t = (EArrayList<int>*)&that;

		arrayBuffer = eso_array_make(t->size(), sizeof(int));
		for (int i = 0; i < t->size(); i++) {
			int o = *(int*) eso_array_get(t->arrayBuffer, i);
			eso_array_push(arrayBuffer, (void*)&o, sizeof(int));
		}
	}

	EArrayList<int>& operator= (const EArrayList<int>& that) {
		if (this == &that) return *this;

		EArrayList<int>* t = (EArrayList<int>*)&that;

		//1.
		clear();
		eso_array_free(&arrayBuffer);

		//2.
		arrayBuffer = eso_array_make(t->size(), sizeof(int));
		for (int i = 0; i < t->size(); i++) {
			int o = *(int*) eso_array_get(t->arrayBuffer, i);
			eso_array_push(arrayBuffer, (void*)&o, sizeof(int));
		}

		return *this;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * @param e element to be appended to this list
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 */
	virtual boolean add(int e) {
		eso_array_push(arrayBuffer, (void*)&e, sizeof(int));
		return true;
	}

	/**
	 * Removes the first occurrence of the specified element from this list,
	 * if it is present.  If the list does not contain the element, it is
	 * unchanged.  More formally, removes the element with the lowest index
	 * <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>
	 * (if such an element exists).  Returns <tt>true</tt> if this list
	 * contained the specified element (or equivalently, if this list
	 * changed as a result of the call).
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if this list contained the specified element
	 */
	virtual boolean remove(int obj) {
		for (int i = 0; i < size(); i++) {
			int o = *(int*) eso_array_get(arrayBuffer, i);
			if (o == (obj)) {
				eso_array_delete(arrayBuffer, i);
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	virtual int size() {
		return eso_array_count(arrayBuffer);
	}

	/**
	 *
	 */
	virtual int capacity() {
		return eso_array_capacity(arrayBuffer);
	}

	/**
	 * Returns <tt>true</tt> if this list contains no elements.
	 *
	 * @return <tt>true</tt> if this list contains no elements
	 */
	virtual boolean isEmpty() {
		return eso_array_is_empty(arrayBuffer);
	}

	/**
	 * Returns <tt>true</tt> if this list contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this list contains
	 * at least one element <tt>e</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>.
	 *
	 * @param o element whose presence in this list is to be tested
	 * @return <tt>true</tt> if this list contains the specified element
	 */
	virtual boolean contains(int o) {
		return indexOf(o) >= 0;
	}

	virtual boolean removeAll(ECollection<int> *c) {
		//TODO...
		return false;
	}

	virtual boolean retainAll(ECollection<int> *c) {
		//TODO...
		return false;
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param  index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual int getAt(int index) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		return *(int*)eso_array_get(arrayBuffer, index);
	}

	/**
	 * Inserts the specified element at the specified position in this
	 * list. Shifts the element currently at that position (if any) and
	 * any subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual void addAt(int index, int element) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		eso_array_insert(arrayBuffer, index, (void*)&element, sizeof(int));
	}

	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual int setAt(int index, int element) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		int old = *(int*)eso_array_get(arrayBuffer, index);
		eso_array_set(arrayBuffer, index, (void*)&element, sizeof(int));
		return old;
	}

	/**
	 * Removes the element at the specified position in this list.
	 * Shifts any subsequent elements to the left (subtracts one from their
	 * indices).
	 *
	 * @param index the index of the element to be removed
	 * @return the element that was removed from the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual int removeAt(int index) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		void *p = eso_array_get(arrayBuffer, index);
		int r = *(int*)p;
		eso_array_delete(arrayBuffer, index);
		return r;
	}

	/**
	 * Removes all of the elements from this list.  The list will
	 * be empty after this call returns.
	 */
	virtual void clear() {
		eso_array_clear(arrayBuffer);
	}

	/**
	 * Returns the index of the first occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 */
	virtual int indexOf(int obj) {
		for (int i = 0; i < size(); i++) {
			if (getAt(i) == (obj))
				return i;
		}
		return -1;
	}

	/**
	 * Returns the index of the last occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the highest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 */
	virtual int lastIndexOf(int obj) {
		for (int i = size() - 1; i >= 0; i--) {
			if (getAt(i) == (obj))
				return i;
		}
		return -1;
	}

	/**
	 * Returns a shallow copy of this <tt>ArrayList</tt> instance.  (The
	 * elements themselves are not copied.)
	 *
	 * @return a clone of this <tt>ArrayList</tt> instance
	 */
	virtual EArrayList<int>* clone(int fromIndex=0, int toIndex=-1) {
		EArrayList<int> *ac = new EArrayList<int>();
		int size_;

		if (fromIndex == 0 && toIndex < 0) {
			size_ = this->size();
		}
		else {
			RangeCheck2(fromIndex, toIndex);
			size_ = toIndex - fromIndex;
		}

		ac->arrayBuffer = eso_array_make(size_, sizeof(int));
		for (int i = 0; i < size_; i++) {
			int o = *(int*) eso_array_get(this->arrayBuffer, fromIndex + i);
			eso_array_push(ac->arrayBuffer, (void*) &o, sizeof(int));
		}

		return ac;
	}

protected:
	es_array_t* arrayBuffer;

private:
	/**
	 * Checks if the given index is in range.  If not, throws an appropriate
	 * runtime exception.  This method does *not* check if the index is
	 * negative: It is always used immediately prior to an array access,
	 * which throws an ArrayIndexOutOfBoundsException if index is negative.
	 */
	void RangeCheck(int index) {
		int _size = size();
		if (index >= _size) {
			EString str;
			str.format("Index: %d, Size: %d", index, _size);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, str.c_str());
		}
	}

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void RangeCheck2(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > this->size()) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}
};

//=============================================================================

template<>
class EArrayList<llong> : virtual public EObject, public EAbstractList<llong> {
public:
	virtual ~EArrayList() {
		clear();
		eso_array_free(&arrayBuffer);
	}

	EArrayList(int initialCapacity = 32) {
		arrayBuffer = eso_array_make(initialCapacity, sizeof(llong));
	}

	EArrayList(const EArrayList<llong>& that) {
		EArrayList<llong>* t = (EArrayList<llong>*)&that;

		arrayBuffer = eso_array_make(t->size(), sizeof(llong));
		for (int i = 0; i < t->size(); i++) {
			llong o = *(llong*) eso_array_get(t->arrayBuffer, i);
			eso_array_push(arrayBuffer, (void*)&o, sizeof(llong));
		}
	}

	EArrayList<llong>& operator= (const EArrayList<llong>& that) {
		if (this == &that) return *this;

		EArrayList<llong>* t = (EArrayList<llong>*)&that;

		//1.
		clear();
		eso_array_free(&arrayBuffer);

		//2.
		arrayBuffer = eso_array_make(t->size(), sizeof(llong));
		for (int i = 0; i < t->size(); i++) {
			llong o = *(llong*) eso_array_get(t->arrayBuffer, i);
			eso_array_push(arrayBuffer, (void*)&o, sizeof(llong));
		}

		return *this;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * @param e element to be appended to this list
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 */
	virtual boolean add(llong e) {
		eso_array_push(arrayBuffer, (void*)&e, sizeof(llong));
		return true;
	}

	/**
	 * Removes the first occurrence of the specified element from this list,
	 * if it is present.  If the list does not contain the element, it is
	 * unchanged.  More formally, removes the element with the lowest index
	 * <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>
	 * (if such an element exists).  Returns <tt>true</tt> if this list
	 * contained the specified element (or equivalently, if this list
	 * changed as a result of the call).
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if this list contained the specified element
	 */
	virtual boolean remove(llong obj) {
		for (int i = 0; i < size(); i++) {
			llong o = *(llong*) eso_array_get(arrayBuffer, i);
			if (o == (obj)) {
				eso_array_delete(arrayBuffer, i);
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	virtual int size() {
		return eso_array_count(arrayBuffer);
	}

	/**
	 *
	 */
	virtual int capacity() {
		return eso_array_capacity(arrayBuffer);
	}

	/**
	 * Returns <tt>true</tt> if this list contains no elements.
	 *
	 * @return <tt>true</tt> if this list contains no elements
	 */
	virtual boolean isEmpty() {
		return eso_array_is_empty(arrayBuffer);
	}

	/**
	 * Returns <tt>true</tt> if this list contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this list contains
	 * at least one element <tt>e</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>.
	 *
	 * @param o element whose presence in this list is to be tested
	 * @return <tt>true</tt> if this list contains the specified element
	 */
	virtual boolean contains(llong o) {
		return indexOf(o) >= 0;
	}

	virtual boolean removeAll(ECollection<llong> *c) {
		//TODO...
		return false;
	}

	virtual boolean retainAll(ECollection<llong> *c) {
		//TODO...
		return false;
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param  index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual llong getAt(int index) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		return *(llong*)eso_array_get(arrayBuffer, index);
	}

	/**
	 * Inserts the specified element at the specified position in this
	 * list. Shifts the element currently at that position (if any) and
	 * any subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual void addAt(int index, llong element) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		eso_array_insert(arrayBuffer, index, (void*)&element, sizeof(llong));
	}

	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual llong setAt(int index, llong element) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		llong old = *(llong*)eso_array_get(arrayBuffer, index);
		eso_array_set(arrayBuffer, index, (void*)&element, sizeof(llong));
		return old;
	}

	/**
	 * Removes the element at the specified position in this list.
	 * Shifts any subsequent elements to the left (subtracts one from their
	 * indices).
	 *
	 * @param index the index of the element to be removed
	 * @return the element that was removed from the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual llong removeAt(int index) THROWS(EIndexOutOfBoundsException) {
		RangeCheck(index);
		void *p = eso_array_get(arrayBuffer, index);
		llong r = *(llong*)p;
		eso_array_delete(arrayBuffer, index);
		return r;
	}

	/**
	 * Removes all of the elements from this list.  The list will
	 * be empty after this call returns.
	 */
	virtual void clear() {
		eso_array_clear(arrayBuffer);
	}

	/**
	 * Returns the index of the first occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 */
	virtual int indexOf(llong obj) {
		for (int i = 0; i < size(); i++) {
			if (getAt(i) == (obj))
				return i;
		}
		return -1;
	}

	/**
	 * Returns the index of the last occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the highest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 */
	virtual int lastIndexOf(llong obj) {
		for (int i = size() - 1; i >= 0; i--) {
			if (getAt(i) == (obj))
				return i;
		}
		return -1;
	}

	/**
	 * Returns a shallow copy of this <tt>ArrayList</tt> instance.  (The
	 * elements themselves are not copied.)
	 *
	 * @return a clone of this <tt>ArrayList</tt> instance
	 */
	virtual EArrayList<llong>* clone(int fromIndex=0, int toIndex=-1) {
		EArrayList<llong> *ac = new EArrayList<llong>();
		int size_;

		if (fromIndex == 0 && toIndex < 0) {
			size_ = this->size();
		}
		else {
			RangeCheck2(fromIndex, toIndex);
			size_ = toIndex - fromIndex;
		}

		ac->arrayBuffer = eso_array_make(size_, sizeof(llong));
		for (int i = 0; i < size_; i++) {
			llong o = *(llong*) eso_array_get(this->arrayBuffer, fromIndex + i);
			eso_array_push(ac->arrayBuffer, (void*) &o, sizeof(llong));
		}

		return ac;
	}

protected:
	es_array_t* arrayBuffer;

private:
	/**
	 * Checks if the given index is in range.  If not, throws an appropriate
	 * runtime exception.  This method does *not* check if the index is
	 * negative: It is always used immediately prior to an array access,
	 * which throws an ArrayIndexOutOfBoundsException if index is negative.
	 */
	void RangeCheck(int index) {
		int _size = size();
		if (index >= _size) {
			EString str;
			str.format("Index: %d, Size: %d", index, _size);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, str.c_str());
		}
	}

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void RangeCheck2(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > this->size()) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}
};

} /* namespace efc */
#endif //!EARRAYLIST_HH_
