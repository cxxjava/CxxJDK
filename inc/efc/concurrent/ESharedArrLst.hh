/*
 * ESharedArrLst.hh
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#ifndef ESHAREDARRLST_HH_
#define ESHAREDARRLST_HH_

#include "ESharedPtr.hh"
#include "EComparable.hh"
#include "EInteger.hh"
#include "ESystem.hh"
#include "EToDoException.hh"
#include "EConcurrentCollection.hh"
#include "EOutOfMemoryError.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalArgumentException.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {


template<typename E>
class eal : public EConcurrentCollection<E>
{
public:
	virtual ~eal() {
		delete elementData;
	}

	/**
	 * Constructs an empty list with the specified initial capacity.
	 *
	 * @param  initialCapacity  the initial capacity of the list
	 * @throws IllegalArgumentException if the specified initial capacity
	 *         is negative
	 */
	eal(int initialCapacity = DEFAULT_CAPACITY) :
			size_(0) {
		if (initialCapacity < 0) {
			char msg[64];
			eso_snprintf(msg, sizeof(msg), "Illegal Capacity: %d",
					initialCapacity);
			throw EIllegalArgumentException(msg, __FILE__, __LINE__);
		}
		this->elementData = new ea<E>(initialCapacity);
	}

	eal(eal<E>& that) {
		size_ = that.size();
		elementData = new ea<E>(size_);
		for (int i = 0; i < size_; i++) {
			(*elementData)[i] = that.get(i);
		}
	}

	eal(const eal<E>& that) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	int size() {
		return size_;
	}

	/**
	 * Returns <tt>true</tt> if this list contains no elements.
	 *
	 * @return <tt>true</tt> if this list contains no elements
	 */
	boolean isEmpty() {
		return size_ == 0;
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
	boolean contains(E* o) {
		return indexOf(o) >= 0;
	}

	/**
	 * Returns the index of the first occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 */
	int indexOf(E* o) {
		if (o == null) {
			for (int i = 0; i < size_; i++)
				if ((*elementData)[i] == null)
					return i;
		} else {
			for (int i = 0; i < size_; i++)
				if (o->equals((*elementData)[i].get()))
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
	int lastIndexOf(E* o) {
		if (o == null) {
			for (int i = size_ - 1; i >= 0; i--)
				if ((*elementData)[i] == null)
					return i;
		} else {
			for (int i = size_ - 1; i >= 0; i--)
				if (o->equals((*elementData)[i].get()))
					return i;
		}
		return -1;
	}

	/**
	 * Returns an array containing all of the elements in this list
	 * in proper sequence (from first to last element).
	 *
	 * <p>The returned array will be "safe" in that no references to it are
	 * maintained by this list.  (In other words, this method must allocate
	 * a new array).  The caller is thus free to modify the returned array.
	 *
	 * <p>This method acts as bridge between array-based and collection-based
	 * APIs.
	 *
	 * @return an array containing all of the elements in this list in
	 *         proper sequence
	 */
	ea<E> toArray() {
		ea<E> result(size_);

		for (int i = 0; i < size_; i++) {
			result[i] = get(i);
		}

		return result;
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param  index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	sp<E> get(int index) {
		rangeCheck(index);

		return (*elementData)[index];
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
	sp<E> set(int index, E* element) {
		rangeCheck(index);

		sp<E> oldValue = (*elementData)[index];
		(*elementData)[index] = element;
		return oldValue;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * @param e element to be appended to this list
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 */
	boolean add(E* e) {
		ensureExplicitCapacity(size_ + 1);  // Increments modCount!!
		(*elementData)[size_++] = e;
		return true;
	}
	boolean add(sp<E> e) {
		ensureExplicitCapacity(size_ + 1);  // Increments modCount!!
		(*elementData)[size_++] = e;
		return true;
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
	sp<E> remove(int index) {
		rangeCheck(index);

		sp<E> oldValue = (*elementData)[index];

		int numMoved = size_ - index - 1;
		if (numMoved > 0)
			ESystem::arraycopy(*elementData, index+1, *elementData, index,
							 numMoved);
		(*elementData)[--size_] = null; // clear to let GC do its work

		return oldValue;
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
	boolean remove(E* o) {
		if (o == null) {
			for (int index = 0; index < size_; index++)
				if ((*elementData)[index] == null) {
					fastRemove(index);
					return true;
				}
		} else {
			for (int index = 0; index < size_; index++)
				if (o->equals((*elementData)[index].get())) {
					fastRemove(index);
					return true;
				}
		}
		return false;
	}

	/**
	 * Removes all of the elements from this list.  The list will
	 * be empty after this call returns.
	 */
	void clear() {
		// clear to let GC do its work
		for (int i = 0; i < size_; i++)
			(*elementData)[i] = null;

		size_ = 0;
	}

	/**
	 * Returns an iterator over the elements in this list in proper sequence.
	 *
	 * <p>The returned iterator is <a href="#fail-fast"><i>fail-fast</i></a>.
	 *
	 * @return an iterator over the elements in this list in proper sequence
	 */
	sp<EConcurrentIterator<E> > iterator() {
		throw EToDoException(__FILE__, __LINE__);
	}

private:
	/**
	 * Default initial capacity.
	 */
	static const int DEFAULT_CAPACITY = 10;

	/**
	 * The maximum size of array to allocate.
	 * Some VMs reserve some header words in an array.
	 * Attempts to allocate larger arrays may result in
	 * OutOfMemoryError: Requested array size exceeds VM limit
	 */
	static const int MAX_ARRAY_SIZE = EInteger::MAX_VALUE - 8;

	/**
	 * The array buffer into which the elements of the ArrayList are stored.
	 * The capacity of the ArrayList is the length of this array buffer. Any
	 * empty ArrayList with elementData == EMPTY_ELEMENTDATA will be expanded to
	 * DEFAULT_CAPACITY when the first element is added.
	 */
	ea<E>* elementData;

	/**
	 * The size of the ArrayList (the number of elements it contains).
	 *
	 * @serial
	 */
	int size_;

	/**
	 * Checks if the given index is in range.  If not, throws an appropriate
	 * runtime exception.  This method does *not* check if the index is
	 * negative: It is always used immediately prior to an array access,
	 * which throws an ArrayIndexOutOfBoundsException if index is negative.
	 */
	void rangeCheck(int index) {
		if (index >= size_)
			throw EIndexOutOfBoundsException("outOfBoundsMsg(index)", __FILE__, __LINE__);
	}

	/**
	 * A version of rangeCheck used by add and addAll.
	 */
	void rangeCheckForAdd(int index) {
		if (index > size_ || index < 0)
			throw EIndexOutOfBoundsException("outOfBoundsMsg(index)", __FILE__, __LINE__);
	}

	void ensureExplicitCapacity(int minCapacity) {
		// overflow-conscious code
		if (minCapacity - elementData->length() > 0)
			grow(minCapacity);
	}

	/**
	 * Increases the capacity to ensure that it can hold at least the
	 * number of elements specified by the minimum capacity argument.
	 *
	 * @param minCapacity the desired minimum capacity
	 */
	void grow(int minCapacity) {
		// overflow-conscious code
		int oldCapacity = elementData->length();
		int newCapacity = oldCapacity + (oldCapacity >> 1);
		if (newCapacity - minCapacity < 0)
			newCapacity = minCapacity;
		if (newCapacity - MAX_ARRAY_SIZE > 0)
			newCapacity = hugeCapacity(minCapacity);
		// minCapacity is usually close to size, so this is a win:
		ea<E>* old = elementData;
		elementData = old->clone(0, newCapacity);// Arrays.copyOf(elementData, newCapacity);
		delete old;
	}

	/*
	 * Private remove method that skips bounds checking and does not
	 * return the value removed.
	 */
	void fastRemove(int index) {
		int numMoved = size_ - index - 1;
		if (numMoved > 0)
			ESystem::arraycopy(*elementData, index+1, *elementData, index,
							 numMoved);
		(*elementData)[--size_] = null; // clear to let GC do its work
	}

	static int hugeCapacity(int minCapacity) {
		if (minCapacity < 0) // overflow
			throw EOutOfMemoryError(__FILE__, __LINE__);
		return (minCapacity > MAX_ARRAY_SIZE) ?
			EInteger::MAX_VALUE :
			MAX_ARRAY_SIZE;
	}
};

} /* namespace efc */
#endif //!ESHAREDARRLST_HH_
