/*
 * EArrayDeque.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EARRAYDEQUE_HH_
#define EARRAYDEQUE_HH_

#include "EDeque.hh"
#include "ESystem.hh"
#include "EAbstractCollection.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalStateException.hh"
#include "ENullPointerException.hh"
#include "ENoSuchElementException.hh"
#include "EConcurrentModificationException.hh"

namespace efc {

/**
 * Resizable-array implementation of the {@link Deque} interface.  Array
 * deques have no capacity restrictions; they grow as necessary to support
 * usage.  They are not thread-safe; in the absence of external
 * synchronization, they do not support concurrent access by multiple threads.
 * Null elements are prohibited.  This class is likely to be faster than
 * {@link Stack} when used as a stack, and faster than {@link LinkedList}
 * when used as a queue.
 *
 * <p>Most <tt>ArrayDeque</tt> operations run in amortized constant time.
 * Exceptions include {@link #remove(Object) remove}, {@link
 * #removeFirstOccurrence removeFirstOccurrence}, {@link #removeLastOccurrence
 * removeLastOccurrence}, {@link #contains contains}, {@link #iterator
 * iterator.remove()}, and the bulk operations, all of which run in linear
 * time.
 *
 * <p>The iterators returned by this class's <tt>iterator</tt> method are
 * <i>fail-fast</i>: If the deque is modified at any time after the iterator
 * is created, in any way except through the iterator's own <tt>remove</tt>
 * method, the iterator will generally throw a {@link
 * ConcurrentModificationException}.  Thus, in the face of concurrent
 * modification, the iterator fails quickly and cleanly, rather than risking
 * arbitrary, non-deterministic behavior at an undetermined time in the
 * future.
 *
 * <p>Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw <tt>ConcurrentModificationException</tt> on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness: <i>the fail-fast behavior of iterators
 * should be used only to detect bugs.</i>
 *
 * <p>This class and its iterator implement all of the
 * <em>optional</em> methods of the {@link Collection} and {@link
 * Iterator} interfaces.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since   1.6
 * @param <E> the type of elements held in this collection
 */

//=============================================================================
//Primitive Types.

template<typename E>
class EArrayDeque : public EAbstractCollection<E>,
		virtual public EDeque<E> {
private:
	class DeqIterator : public EIterator<E> {
	private:
		EArrayDeque<E>* ad;

		/**
		 * Index of element to be returned by subsequent call to next.
		 */
		int cursor;// = head;

		/**
		 * Tail recorded at construction (also in remove), to stop
		 * iterator and also to check for comodification.
		 */
		int fence;// = tail;

		/**
		 * Index of element returned by most recent call to next.
		 * Reset to -1 if element is deleted by a call to remove.
		 */
		int lastRet;// = -1;

	public:
		DeqIterator(EArrayDeque<E>* ad) :
				cursor(ad->head), fence(ad->tail), lastRet(-1) {
			this->ad = ad;
		}

		boolean hasNext() {
			return cursor != fence;
		}

		E next() {
			EA<E>* elements = ad->elements;

			if (cursor == fence)
				throw ENoSuchElementException(__FILE__, __LINE__);
			E result = (*elements)[cursor];
			// This check doesn't catch all possible comodifications,
			// but does catch the ones that corrupt traversal
			if (ad->tail != fence)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			lastRet = cursor;
			cursor = (cursor + 1) & (elements->length() - 1);
			return result;
		}

		void remove() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			ad->deleteAt(lastRet, &flag);
			if (flag) { // if left-shifted, undo increment in next()
				cursor = (cursor - 1) & (ad->elements->length() - 1);
				fence = ad->tail;
			}
			lastRet = -1;
		}

		E moveOut() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			E o = ad->deleteAt(lastRet, &flag);
			if (flag) { // if left-shifted, undo increment in next()
				cursor = (cursor - 1) & (ad->elements->length() - 1);
				fence = ad->tail;
			}
			lastRet = -1;
			return o;
		}
	};

	class DescendingIterator : public EIterator<E> {
	private:
		EArrayDeque<E>* ad;

		/*
		 * This class is nearly a mirror-image of DeqIterator, using
		 * tail instead of head for initial cursor, and head instead of
		 * tail for fence.
		 */
		int cursor;// = tail;
		int fence;// = head;
		int lastRet;// = -1;

	public:
		DescendingIterator(EArrayDeque<E>* ad) :
				cursor(ad->tail), fence(ad->head), lastRet(-1) {
			this->ad = ad;
		}

		boolean hasNext() {
			return cursor != fence;
		}

		E next() {
			EA<E>* elements = ad->elements;

			if (cursor == fence)
				throw ENoSuchElementException(__FILE__, __LINE__);
			cursor = (cursor - 1) & (elements->length() - 1);
			E result = (*elements)[cursor];
			if (ad->head != fence)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			lastRet = cursor;
			return result;
		}

		void remove() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			ad->deleteAt(lastRet, &flag);
			if (!flag) {
				cursor = (cursor + 1) & (ad->elements->length() - 1);
				fence = ad->head;
			}
			lastRet = -1;
		}

		E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

public:
	virtual ~EArrayDeque() {
		delete elements;
	}

	/**
	 * Constructs an empty array deque with an initial capacity
	 * sufficient to hold 16 elements.
	 */
	explicit
	EArrayDeque() : head(0), tail(0) {
		elements = new EA<E>(16);
	}

	/**
	 * Constructs an empty array deque with an initial capacity
	 * sufficient to hold the specified number of elements.
	 *
	 * @param numElements  lower bound on initial capacity of the deque
	 */
	explicit
	EArrayDeque(int numElements) : head(0), tail(0) {
		allocateElements(numElements);
	}

	/**
	 * Constructs a deque containing the elements of the specified
	 * collection, in the order they are returned by the collection's
	 * iterator.  (The first element returned by the collection's
	 * iterator becomes the first element, or <i>front</i> of the
	 * deque.)
	 *
	 * @param c the collection whose elements are to be placed into the deque
	 * @throws NullPointerException if the specified collection is null
	 */
	EArrayDeque(ECollection<E>* c) : head(0), tail(0) {
		allocateElements(c->size(), false);
		EAbstractCollection<E>::addAll(c);
	}

	EArrayDeque(const EArrayDeque<E>& that) {
		EArrayDeque<E>* t = (EArrayDeque<E>*)&that;

		elements = new EA<E>(*(t->elements));
		head = t->head;
		tail = t->tail;
	}

	EArrayDeque<E>& operator= (const EArrayDeque<E>& that) {
		if (this == &that) return *this;

		EArrayDeque<E>* t = (EArrayDeque<E>*)&that;

		//1.
		delete elements;

		//2.
		elements = new EA<E>(*(t->elements));
		head = t->head;
		tail = t->tail;

		return *this;
	}

	// The main insertion and extraction methods are addFirst,
	// addLast, pollFirst, pollLast. The other methods are defined in
	// terms of these.

	/**
	 * Inserts the specified element at the front of this deque.
	 *
	 * @param e the element to add
	 * @throws NullPointerException if the specified element is null
	 */
	void addFirst(E e) {
		(*elements)[head = (head - 1) & (elements->length() - 1)] = e;
		if (head == tail)
			doubleCapacity();
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 * @throws NullPointerException if the specified element is null
	 */
	void addLast(E e) {
		(*elements)[tail] = e;
		if ( (tail = (tail + 1) & (elements->length() - 1)) == head)
			doubleCapacity();
	}

	/**
	 * Inserts the specified element at the front of this deque.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Deque#offerFirst})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offerFirst(E e) {
		addFirst(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Deque#offerLast})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offerLast(E e) {
		addLast(e);
		return true;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E removeFirst() {
		return pollFirst();
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E removeLast() {
		return pollLast();
	}

	E pollFirst() {
		if (head == tail) {
			throw ENoSuchElementException(__FILE__, __LINE__);
		}
		int h = head;
		E result = (*elements)[h]; // Element is null if deque empty
		head = (h + 1) & (elements->length() - 1);
		return result;
	}

	E pollLast() {
		if (head == tail) {
			throw ENoSuchElementException(__FILE__, __LINE__);
		}
		int t = (tail - 1) & (elements->length() - 1);
		E result = (*elements)[t];
		tail = t;
		return result;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E getFirst() {
		if (head == tail) {
			throw ENoSuchElementException(__FILE__, __LINE__);
		}
		return (*elements)[head];
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E getLast() {
		if (head == tail) {
			throw ENoSuchElementException(__FILE__, __LINE__);
		}
		return (*elements)[(tail - 1) & (elements->length() - 1)];
	}

	E peekFirst() {
		if (head == tail) {
			throw ENoSuchElementException(__FILE__, __LINE__);
		}
		return (*elements)[head];
	}

	E peekLast() {
		if (head == tail) {
			throw ENoSuchElementException(__FILE__, __LINE__);
		}
		return (*elements)[(tail - 1) & (elements->length() - 1)];
	}

	/**
	 * Removes the first occurrence of the specified element in this
	 * deque (when traversing the deque from head to tail).
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the first element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if the deque contained the specified element
	 */
	boolean removeFirstOccurrence(E o) {
		int mask = elements->length() - 1;
		int i = head;
		E x;
		while (i != tail) {
			x = (*elements)[i];
			if (o == x) {
				deleteAt(i, null);
				return true;
			}
			i = (i + 1) & mask;
		}
		return false;
	}

	/**
	 * Removes the last occurrence of the specified element in this
	 * deque (when traversing the deque from head to tail).
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the last element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if the deque contained the specified element
	 */
	boolean removeLastOccurrence(E o) {
		int mask = elements->length() - 1;
		int i = (tail - 1) & mask;
		E x;
		while (i != head) {
			x = (*elements)[i];
			if (o == x) {
				deleteAt(i, null);
				return true;
			}
			i = (i - 1) & mask;
		}
		return false;
	}

	// *** Queue methods ***

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #addLast}.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean add(E e) {
		addLast(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #offerLast}.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offer(E e) {
		return offerLast(e);
	}

	/**
	 * Retrieves and removes the head of the queue represented by this deque.
	 *
	 * This method differs from {@link #poll poll} only in that it throws an
	 * exception if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #removeFirst}.
	 *
	 * @return the head of the queue represented by this deque
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E remove() {
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head of the queue represented by this deque
	 * (in other words, the first element of this deque), or returns
	 * <tt>null</tt> if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #pollFirst}.
	 *
	 * @return the head of the queue represented by this deque, or
	 *         <tt>null</tt> if this deque is empty
	 */
	E poll() {
		return pollFirst();
	}

	/**
	 * Retrieves, but does not remove, the head of the queue represented by
	 * this deque.  This method differs from {@link #peek peek} only in
	 * that it throws an exception if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #getFirst}.
	 *
	 * @return the head of the queue represented by this deque
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E element() {
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head of the queue represented by
	 * this deque, or returns <tt>null</tt> if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #peekFirst}.
	 *
	 * @return the head of the queue represented by this deque, or
	 *         <tt>null</tt> if this deque is empty
	 */
	E peek() {
		return peekFirst();
	}

	// *** Stack methods ***

	/**
	 * Pushes an element onto the stack represented by this deque.  In other
	 * words, inserts the element at the front of this deque.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 * @throws NullPointerException if the specified element is null
	 */
	void push(E e) {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this deque.  In other
	 * words, removes and returns the first element of this deque.
	 *
	 * <p>This method is equivalent to {@link #removeFirst()}.
	 *
	 * @return the element at the front of this deque (which is the top
	 *         of the stack represented by this deque)
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E pop() {
		return removeFirst();
	}

	// *** Collection Methods ***

	/**
	 * Returns the number of elements in this deque.
	 *
	 * @return the number of elements in this deque
	 */
	int size() {
		return (tail - head) & (elements->length() - 1);
	}

	/**
	 * Returns <tt>true</tt> if this deque contains no elements.
	 *
	 * @return <tt>true</tt> if this deque contains no elements
	 */
	boolean isEmpty() {
		return head == tail;
	}

	/**
	 * Returns an iterator over the elements in this deque.  The elements
	 * will be ordered from first (head) to last (tail).  This is the same
	 * order that elements would be dequeued (via successive calls to
	 * {@link #remove} or popped (via successive calls to {@link #pop}).
	 *
	 * @return an iterator over the elements in this deque
	 */
	sp<EIterator<E> > iterator(int index=0) {
		ES_ASSERT(index == 0);
		return new DeqIterator(this);
	}

	sp<EIterator<E> > descendingIterator() {
		return new DescendingIterator(this);
	}

	/**
	 * Returns <tt>true</tt> if this deque contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this deque contains
	 * at least one element <tt>e</tt> such that <tt>o.equals(e)</tt>.
	 *
	 * @param o object to be checked for containment in this deque
	 * @return <tt>true</tt> if this deque contains the specified element
	 */
	boolean contains(E o) {
		int mask = elements->length() - 1;
		int i = head;
		E x;
		while (i != tail) {
			x = (*elements)[i];
			if (o == x)
				return true;
			i = (i + 1) & mask;
		}
		return false;
	}

	/**
	 * Removes a single instance of the specified element from this deque.
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the first element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * <p>This method is equivalent to {@link #removeFirstOccurrence}.
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if this deque contained the specified element
	 */
	boolean remove(E o) {
		return removeFirstOccurrence(o);
	}

	/**
	 * Removes all of the elements from this deque.
	 * The deque will be empty after this call returns.
	 */
	void clear() {
		head = tail = 0;
	}

private:
	/**
	 * The array in which the elements of the deque are stored.
	 * The capacity of the deque is the length of this array, which is
	 * always a power of two. The array is never allowed to become
	 * full, except transiently within an addX method where it is
	 * resized (see doubleCapacity) immediately upon becoming full,
	 * thus avoiding head and tail wrapping around to equal each
	 * other.  We also guarantee that all array cells not holding
	 * deque elements are always null.
	 */
	EA<E>* elements;

	/**
	 * The index of the element at the head of the deque (which is the
	 * element that would be removed by remove() or pop()); or an
	 * arbitrary number equal to tail if the deque is empty.
	 */
	int head;

	/**
	 * The index at which the next element would be added to the tail
	 * of the deque (via addLast(E), add(E), or push(E)).
	 */
	int tail;

	/**
	 * The minimum capacity that we'll use for a newly created deque.
	 * Must be a power of 2.
	 */
	static const int MIN_INITIAL_CAPACITY = 8;

	// ******  Array allocation and resizing utilities ******

	/**
	 * Allocate empty array to hold the given number of elements.
	 *
	 * @param numElements  the number of elements to hold
	 */
	void allocateElements(int numElements) {
		unsigned int initialCapacity = MIN_INITIAL_CAPACITY;
		// Find the best power of two to hold elements.
		// Tests "<=" because arrays aren't kept full.
		if (numElements >= initialCapacity) {
			initialCapacity = numElements;
			initialCapacity |= (initialCapacity >>  1);
			initialCapacity |= (initialCapacity >>  2);
			initialCapacity |= (initialCapacity >>  4);
			initialCapacity |= (initialCapacity >>  8);
			initialCapacity |= (initialCapacity >> 16);
			initialCapacity++;
		}
		elements = new EA<E>(initialCapacity);
	}

	/**
	 * Double the capacity of this deque.  Call only when full, i.e.,
	 * when head and tail have wrapped around to become equal.
	 */
	void doubleCapacity() {
		ES_ASSERT(head == tail);
		int p = head;
		int n = elements->length();
		int r = n - p; // number of elements to the right of p
		int newCapacity = n << 1;
		if (newCapacity < 0)
			throw EIllegalStateException(__FILE__, __LINE__, "Sorry, deque too big");
		EA<E>* a = new EA<E>(newCapacity);
		ESystem::arraycopy((*elements), p, (*a), 0, r);
		ESystem::arraycopy((*elements), 0, (*a), r, p);
		delete elements; //!
		elements = a;
		head = 0;
		tail = n;
	}

    /**
     * Removes the element at the specified position in the elements array,
     * adjusting head and tail as necessary.  This can result in motion of
     * elements backwards or forwards in the array.
     *
     * <p>This method is called delete rather than remove to emphasize
     * that its semantics differ from those of {@link List#remove(int)}.
     *
     * @param flag true if elements moved backwards
     * @return the deleted element
     */
    E deleteAt(int i, boolean* flag) {
        EA<E>* elements = this->elements;
        int mask = elements->length() - 1;
        int h = head;
        int t = tail;
        int front = (i - h) & mask;
        int back  = (t - i) & mask;

        // Invariant: head <= i < tail mod circularity
        if (front >= ((t - h) & mask))
            throw EConcurrentModificationException(__FILE__, __LINE__);

    	E o = (*elements)[i]; //1

        // Optimize for least element motion
        if (front < back) {
            if (h <= i) {
                ESystem::arraycopy((*elements), h, (*elements), h + 1, front);
            } else { // Wrap around
            	ESystem::arraycopy((*elements), 0, (*elements), 1, i);
                (*elements)[0] = (*elements)[mask];
                ESystem::arraycopy((*elements), h, (*elements), h + 1, mask - h);
            }
            head = (h + 1) & mask;
            if (flag) *flag = false;
        } else {
            if (i < t) { // Copy the null tail as well
            	ESystem::arraycopy((*elements), i + 1, (*elements), i, back);
                tail = t - 1;
            } else { // Wrap around
            	ESystem::arraycopy((*elements), i + 1, (*elements), i, mask - i);
                (*elements)[mask] = (*elements)[0];
                ESystem::arraycopy((*elements), 1, (*elements), 0, t);
                tail = (t - 1) & mask;
            }
            if (flag) *flag = true;
        }

        return o; //!
   }
};

//=============================================================================
//Native pointer Types.

template<typename T>
class EArrayDeque<T*> : public EAbstractCollection<T*>,
		virtual public EDeque<T*> {
public:
	typedef T* E;

private:
	class DeqIterator : public EIterator<E> {
	private:
		EArrayDeque<E>* ad;

		/**
		 * Index of element to be returned by subsequent call to next.
		 */
		int cursor;// = head;

		/**
		 * Tail recorded at construction (also in remove), to stop
		 * iterator and also to check for comodification.
		 */
		int fence;// = tail;

		/**
		 * Index of element returned by most recent call to next.
		 * Reset to -1 if element is deleted by a call to remove.
		 */
		int lastRet;// = -1;

	public:
		DeqIterator(EArrayDeque<E>* ad) :
				cursor(ad->head), fence(ad->tail), lastRet(-1) {
			this->ad = ad;
		}

		boolean hasNext() {
			return cursor != fence;
		}

		E next() {
			EA<E>* elements = ad->elements;

			if (cursor == fence)
				throw ENoSuchElementException(__FILE__, __LINE__);
			E result = (*elements)[cursor];
			// This check doesn't catch all possible comodifications,
			// but does catch the ones that corrupt traversal
			if (ad->tail != fence || result == null)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			lastRet = cursor;
			cursor = (cursor + 1) & (elements->length() - 1);
			return result;
		}

		void remove() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			delete ad->deleteAt(lastRet, &flag);
			if (flag) { // if left-shifted, undo increment in next()
				cursor = (cursor - 1) & (ad->elements->length() - 1);
				fence = ad->tail;
			}
			lastRet = -1;
		}

		E moveOut() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			E o = ad->deleteAt(lastRet, &flag);
			if (flag) { // if left-shifted, undo increment in next()
				cursor = (cursor - 1) & (ad->elements->length() - 1);
				fence = ad->tail;
			}
			lastRet = -1;
			return o;
		}
	};

	class DescendingIterator : public EIterator<E> {
	private:
		EArrayDeque<E>* ad;

		/*
		 * This class is nearly a mirror-image of DeqIterator, using
		 * tail instead of head for initial cursor, and head instead of
		 * tail for fence.
		 */
		int cursor;// = tail;
		int fence;// = head;
		int lastRet;// = -1;

	public:
		DescendingIterator(EArrayDeque<E>* ad) :
				cursor(ad->tail), fence(ad->head), lastRet(-1) {
			this->ad = ad;
		}

		boolean hasNext() {
			return cursor != fence;
		}

		E next() {
			EA<E>* elements = ad->elements;

			if (cursor == fence)
				throw ENoSuchElementException(__FILE__, __LINE__);
			cursor = (cursor - 1) & (elements->length() - 1);
			E result = (*elements)[cursor];
			if (ad->head != fence || result == null)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			lastRet = cursor;
			return result;
		}

		void remove() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			delete ad->deleteAt(lastRet, &flag);
			if (!flag) {
				cursor = (cursor + 1) & (ad->elements->length() - 1);
				fence = ad->head;
			}
			lastRet = -1;
		}

		E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

public:
	virtual ~EArrayDeque() {
		delete elements;
	}

	/**
	 * Constructs an empty array deque with an initial capacity
	 * sufficient to hold 16 elements.
	 */
	explicit
	EArrayDeque(boolean autoFree=true) : head(0), tail(0) {
		elements = new EA<E>(16, autoFree);
	}

	/**
	 * Constructs an empty array deque with an initial capacity
	 * sufficient to hold the specified number of elements.
	 *
	 * @param numElements  lower bound on initial capacity of the deque
	 */
	explicit
	EArrayDeque(int numElements, boolean autoFree=true) : head(0), tail(0) {
		allocateElements(numElements, autoFree);
	}

	/**
	 * Constructs a deque containing the elements of the specified
	 * collection, in the order they are returned by the collection's
	 * iterator.  (The first element returned by the collection's
	 * iterator becomes the first element, or <i>front</i> of the
	 * deque.)
	 *
	 * @param c the collection whose elements are to be placed into the deque
	 * @throws NullPointerException if the specified collection is null
	 */
	EArrayDeque(ECollection<E>* c) : head(0), tail(0) {
		allocateElements(c->size(), false);
		EAbstractCollection<E>::addAll(c);
	}

	EArrayDeque(const EArrayDeque<E>& that) {
		EArrayDeque<E>* t = (EArrayDeque<E>*)&that;

		elements = new EA<E>(*(t->elements));
		head = t->head;
		tail = t->tail;
	}

	EArrayDeque<E>& operator= (const EArrayDeque<E>& that) {
		if (this == &that) return *this;

		EArrayDeque<E>* t = (EArrayDeque<E>*)&that;

		//1.
		delete elements;

		//2.
		elements = new EA<E>(*(t->elements));
		head = t->head;
		tail = t->tail;

		return *this;
	}

	// The main insertion and extraction methods are addFirst,
	// addLast, pollFirst, pollLast. The other methods are defined in
	// terms of these.

	/**
	 * Inserts the specified element at the front of this deque.
	 *
	 * @param e the element to add
	 * @throws NullPointerException if the specified element is null
	 */
	void addFirst(E e) {
		if (e == null)
			throw ENullPointerException(__FILE__, __LINE__);
		(*elements)[head = (head - 1) & (elements->length() - 1)] = e;
		if (head == tail)
			doubleCapacity();
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 * @throws NullPointerException if the specified element is null
	 */
	void addLast(E e) {
		if (e == null)
			throw ENullPointerException(__FILE__, __LINE__);
		(*elements)[tail] = e;
		if ( (tail = (tail + 1) & (elements->length() - 1)) == head)
			doubleCapacity();
	}

	/**
	 * Inserts the specified element at the front of this deque.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Deque#offerFirst})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offerFirst(E e) {
		addFirst(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Deque#offerLast})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offerLast(E e) {
		addLast(e);
		return true;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E removeFirst() {
		E x = pollFirst();
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E removeLast() {
		E x = pollLast();
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	E pollFirst() {
		int h = head;
		E result = (*elements)[h]; // Element is null if deque empty
		if (result == null)
			return null;
		(*elements)[h] = null;     // Must null out slot
		head = (h + 1) & (elements->length() - 1);
		return result;
	}

	E pollLast() {
		int t = (tail - 1) & (elements->length() - 1);
		E result = (*elements)[t];
		if (result == null)
			return null;
		(*elements)[t] = null;
		tail = t;
		return result;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E getFirst() {
		E x = (*elements)[head];
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E getLast() {
		E x = (*elements)[(tail - 1) & (elements->length() - 1)];
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	E peekFirst() {
		return (*elements)[head]; // elements[head] is null if deque empty
	}

	E peekLast() {
		return (*elements)[(tail - 1) & (elements->length() - 1)];
	}

	/**
	 * Removes the first occurrence of the specified element in this
	 * deque (when traversing the deque from head to tail).
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the first element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if the deque contained the specified element
	 */
	boolean removeFirstOccurrence(E o) {
		if (o == null)
			return false;
		int mask = elements->length() - 1;
		int i = head;
		E x;
		while ( (x = (*elements)[i]) != null) {
			if (o->equals(x)) {
				delete deleteAt(i, null);
				return true;
			}
			i = (i + 1) & mask;
		}
		return false;
	}

	/**
	 * Removes the last occurrence of the specified element in this
	 * deque (when traversing the deque from head to tail).
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the last element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if the deque contained the specified element
	 */
	boolean removeLastOccurrence(E o) {
		if (o == null)
			return false;
		int mask = elements->length() - 1;
		int i = (tail - 1) & mask;
		E x;
		while ( (x = (*elements)[i]) != null) {
			if (o->equals(x)) {
				delete deleteAt(i, null);
				return true;
			}
			i = (i - 1) & mask;
		}
		return false;
	}

	// *** Queue methods ***

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #addLast}.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean add(E e) {
		addLast(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #offerLast}.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offer(E e) {
		return offerLast(e);
	}

	/**
	 * Retrieves and removes the head of the queue represented by this deque.
	 *
	 * This method differs from {@link #poll poll} only in that it throws an
	 * exception if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #removeFirst}.
	 *
	 * @return the head of the queue represented by this deque
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E remove() {
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head of the queue represented by this deque
	 * (in other words, the first element of this deque), or returns
	 * <tt>null</tt> if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #pollFirst}.
	 *
	 * @return the head of the queue represented by this deque, or
	 *         <tt>null</tt> if this deque is empty
	 */
	E poll() {
		return pollFirst();
	}

	/**
	 * Retrieves, but does not remove, the head of the queue represented by
	 * this deque.  This method differs from {@link #peek peek} only in
	 * that it throws an exception if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #getFirst}.
	 *
	 * @return the head of the queue represented by this deque
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E element() {
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head of the queue represented by
	 * this deque, or returns <tt>null</tt> if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #peekFirst}.
	 *
	 * @return the head of the queue represented by this deque, or
	 *         <tt>null</tt> if this deque is empty
	 */
	E peek() {
		return peekFirst();
	}

	// *** Stack methods ***

	/**
	 * Pushes an element onto the stack represented by this deque.  In other
	 * words, inserts the element at the front of this deque.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 * @throws NullPointerException if the specified element is null
	 */
	void push(E e) {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this deque.  In other
	 * words, removes and returns the first element of this deque.
	 *
	 * <p>This method is equivalent to {@link #removeFirst()}.
	 *
	 * @return the element at the front of this deque (which is the top
	 *         of the stack represented by this deque)
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E pop() {
		return removeFirst();
	}

	// *** Collection Methods ***

	/**
	 * Returns the number of elements in this deque.
	 *
	 * @return the number of elements in this deque
	 */
	int size() {
		return (tail - head) & (elements->length() - 1);
	}

	/**
	 * Returns <tt>true</tt> if this deque contains no elements.
	 *
	 * @return <tt>true</tt> if this deque contains no elements
	 */
	boolean isEmpty() {
		return head == tail;
	}

	/**
	 * Returns an iterator over the elements in this deque.  The elements
	 * will be ordered from first (head) to last (tail).  This is the same
	 * order that elements would be dequeued (via successive calls to
	 * {@link #remove} or popped (via successive calls to {@link #pop}).
	 *
	 * @return an iterator over the elements in this deque
	 */
	sp<EIterator<E> > iterator(int index=0) {
		ES_ASSERT(index == 0);
		return new DeqIterator(this);
	}

	sp<EIterator<E> > descendingIterator() {
		return new DescendingIterator(this);
	}

	/**
	 * Returns <tt>true</tt> if this deque contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this deque contains
	 * at least one element <tt>e</tt> such that <tt>o.equals(e)</tt>.
	 *
	 * @param o object to be checked for containment in this deque
	 * @return <tt>true</tt> if this deque contains the specified element
	 */
	boolean contains(E o) {
		if (o == null)
			return false;
		int mask = elements->length() - 1;
		int i = head;
		E x;
		while ( (x = (*elements)[i]) != null) {
			if (o->equals(x))
				return true;
			i = (i + 1) & mask;
		}
		return false;
	}

	/**
	 * Removes a single instance of the specified element from this deque.
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the first element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * <p>This method is equivalent to {@link #removeFirstOccurrence}.
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if this deque contained the specified element
	 */
	boolean remove(E o) {
		return removeFirstOccurrence(o);
	}

	/**
	 * Removes all of the elements from this deque.
	 * The deque will be empty after this call returns.
	 */
	void clear() {
		int h = head;
		int t = tail;
		if (h != t) { // clear all cells
			head = tail = 0;
			int i = h;
			int mask = elements->length() - 1;
			do {
				(*elements)[i] = null;
				i = (i + 1) & mask;
			} while (i != t);
		}
	}

	/**
	 *
	 */
	void setAutoFree(boolean autoFree=true) {
		elements->setAutoFree(autoFree);
	}

	/**
	 *
	 */
	boolean getAutoFree() {
		return elements->getAutoFree();
	}

private:
	/**
	 * The array in which the elements of the deque are stored.
	 * The capacity of the deque is the length of this array, which is
	 * always a power of two. The array is never allowed to become
	 * full, except transiently within an addX method where it is
	 * resized (see doubleCapacity) immediately upon becoming full,
	 * thus avoiding head and tail wrapping around to equal each
	 * other.  We also guarantee that all array cells not holding
	 * deque elements are always null.
	 */
	EA<E>* elements;

	/**
	 * The index of the element at the head of the deque (which is the
	 * element that would be removed by remove() or pop()); or an
	 * arbitrary number equal to tail if the deque is empty.
	 */
	int head;

	/**
	 * The index at which the next element would be added to the tail
	 * of the deque (via addLast(E), add(E), or push(E)).
	 */
	int tail;

	/**
	 * The minimum capacity that we'll use for a newly created deque.
	 * Must be a power of 2.
	 */
	static const int MIN_INITIAL_CAPACITY = 8;

	// ******  Array allocation and resizing utilities ******

	/**
	 * Allocate empty array to hold the given number of elements.
	 *
	 * @param numElements  the number of elements to hold
	 */
	void allocateElements(int numElements, boolean autoFree) {
		unsigned int initialCapacity = MIN_INITIAL_CAPACITY;
		// Find the best power of two to hold elements.
		// Tests "<=" because arrays aren't kept full.
		if (numElements >= initialCapacity) {
			initialCapacity = numElements;
			initialCapacity |= (initialCapacity >>  1);
			initialCapacity |= (initialCapacity >>  2);
			initialCapacity |= (initialCapacity >>  4);
			initialCapacity |= (initialCapacity >>  8);
			initialCapacity |= (initialCapacity >> 16);
			initialCapacity++;
		}
		elements = new EA<E>(initialCapacity, autoFree);
	}

	/**
	 * Double the capacity of this deque.  Call only when full, i.e.,
	 * when head and tail have wrapped around to become equal.
	 */
	void doubleCapacity() {
		ES_ASSERT(head == tail);
		int p = head;
		int n = elements->length();
		int r = n - p; // number of elements to the right of p
		int newCapacity = n << 1;
		if (newCapacity < 0)
			throw EIllegalStateException(__FILE__, __LINE__, "Sorry, deque too big");
		EA<E>* a = new EA<E>(newCapacity);
		ESystem::arraycopy((*elements), p, (*a), 0, r);
		ESystem::arraycopy((*elements), 0, (*a), r, p);
		elements->setAutoFree(false); //!
		delete elements; //!
		elements = a;
		head = 0;
		tail = n;
	}

    void checkInvariants() {
        ES_ASSERT((*elements)[tail] == null);
        ES_ASSERT(head == tail ? (*elements)[head] == null :
            ((*elements)[head] != null &&
             (*elements)[(tail - 1) & (elements->length() - 1)] != null));
        ES_ASSERT((*elements)[(head - 1) & (elements->length() - 1)] == null);
    }

    /**
     * Removes the element at the specified position in the elements array,
     * adjusting head and tail as necessary.  This can result in motion of
     * elements backwards or forwards in the array.
     *
     * <p>This method is called delete rather than remove to emphasize
     * that its semantics differ from those of {@link List#remove(int)}.
     *
     * @param flag true if elements moved backwards
     * @return the deleted element
     */
    E deleteAt(int i, boolean* flag) {
        checkInvariants();
        EA<E>* elements = this->elements;
        int mask = elements->length() - 1;
        int h = head;
        int t = tail;
        int front = (i - h) & mask;
        int back  = (t - i) & mask;

        // Invariant: head <= i < tail mod circularity
        if (front >= ((t - h) & mask))
            throw EConcurrentModificationException(__FILE__, __LINE__);

    	E o = (*elements)[i]; //1

        // Optimize for least element motion
        if (front < back) {
            if (h <= i) {
                ESystem::arraycopy((*elements), h, (*elements), h + 1, front);
            } else { // Wrap around
            	ESystem::arraycopy((*elements), 0, (*elements), 1, i);
                (*elements)[0] = (*elements)[mask];
                ESystem::arraycopy((*elements), h, (*elements), h + 1, mask - h);
            }
            (*elements)[h] = null;
            head = (h + 1) & mask;
            if (flag) *flag = false;
        } else {
            if (i < t) { // Copy the null tail as well
            	ESystem::arraycopy((*elements), i + 1, (*elements), i, back);
                tail = t - 1;
            } else { // Wrap around
            	ESystem::arraycopy((*elements), i + 1, (*elements), i, mask - i);
                (*elements)[mask] = (*elements)[0];
                ESystem::arraycopy((*elements), 1, (*elements), 0, t);
                tail = (t - 1) & mask;
            }
            if (flag) *flag = true;
        }

        return o; //!
   }
};

//=============================================================================
//Shared pointer Types.

template<typename T>
class EArrayDeque<sp<T> > : public EAbstractCollection<sp<T> >,
		virtual public EDeque<sp<T> > {
public:
	typedef sp<T> E;

private:
	class DeqIterator : public EIterator<E> {
	private:
		EArrayDeque<E>* ad;

		/**
		 * Index of element to be returned by subsequent call to next.
		 */
		int cursor;// = head;

		/**
		 * Tail recorded at construction (also in remove), to stop
		 * iterator and also to check for comodification.
		 */
		int fence;// = tail;

		/**
		 * Index of element returned by most recent call to next.
		 * Reset to -1 if element is deleted by a call to remove.
		 */
		int lastRet;// = -1;

	public:
		DeqIterator(EArrayDeque<E>* ad) :
				cursor(ad->head), fence(ad->tail), lastRet(-1) {
			this->ad = ad;
		}

		boolean hasNext() {
			return cursor != fence;
		}

		E next() {
			EA<E>* elements = ad->elements;

			if (cursor == fence)
				throw ENoSuchElementException(__FILE__, __LINE__);
			E result = (*elements)[cursor];
			// This check doesn't catch all possible comodifications,
			// but does catch the ones that corrupt traversal
			if (ad->tail != fence || result == null)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			lastRet = cursor;
			cursor = (cursor + 1) & (elements->length() - 1);
			return result;
		}

		void remove() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			ad->deleteAt(lastRet, &flag);
			if (flag) { // if left-shifted, undo increment in next()
				cursor = (cursor - 1) & (ad->elements->length() - 1);
				fence = ad->tail;
			}
			lastRet = -1;
		}

		E moveOut() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			E o = ad->deleteAt(lastRet, &flag);
			if (flag) { // if left-shifted, undo increment in next()
				cursor = (cursor - 1) & (ad->elements->length() - 1);
				fence = ad->tail;
			}
			lastRet = -1;
			return o;
		}
	};

	class DescendingIterator : public EIterator<E> {
	private:
		EArrayDeque<E>* ad;

		/*
		 * This class is nearly a mirror-image of DeqIterator, using
		 * tail instead of head for initial cursor, and head instead of
		 * tail for fence.
		 */
		int cursor;// = tail;
		int fence;// = head;
		int lastRet;// = -1;

	public:
		DescendingIterator(EArrayDeque<E>* ad) :
				cursor(ad->tail), fence(ad->head), lastRet(-1) {
			this->ad = ad;
		}

		boolean hasNext() {
			return cursor != fence;
		}

		E next() {
			EA<E>* elements = ad->elements;

			if (cursor == fence)
				throw ENoSuchElementException(__FILE__, __LINE__);
			cursor = (cursor - 1) & (elements->length() - 1);
			E result = (*elements)[cursor];
			if (ad->head != fence || result == null)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			lastRet = cursor;
			return result;
		}

		void remove() {
			if (lastRet < 0)
				throw EIllegalStateException(__FILE__, __LINE__);
			boolean flag;
			ad->deleteAt(lastRet, &flag);
			if (!flag) {
				cursor = (cursor + 1) & (ad->elements->length() - 1);
				fence = ad->head;
			}
			lastRet = -1;
		}

		E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

public:
	virtual ~EArrayDeque() {
		delete elements;
	}

	/**
	 * Constructs an empty array deque with an initial capacity
	 * sufficient to hold 16 elements.
	 */
	explicit
	EArrayDeque() : head(0), tail(0) {
		elements = new EA<E>(16);
	}

	/**
	 * Constructs an empty array deque with an initial capacity
	 * sufficient to hold the specified number of elements.
	 *
	 * @param numElements  lower bound on initial capacity of the deque
	 */
	explicit
	EArrayDeque(int numElements) : head(0), tail(0) {
		allocateElements(numElements);
	}

	/**
	 * Constructs a deque containing the elements of the specified
	 * collection, in the order they are returned by the collection's
	 * iterator.  (The first element returned by the collection's
	 * iterator becomes the first element, or <i>front</i> of the
	 * deque.)
	 *
	 * @param c the collection whose elements are to be placed into the deque
	 * @throws NullPointerException if the specified collection is null
	 */
	EArrayDeque(ECollection<E>* c) : head(0), tail(0) {
		allocateElements(c->size(), false);
		EAbstractCollection<E>::addAll(c);
	}

	EArrayDeque(const EArrayDeque<E>& that) {
		EArrayDeque<E>* t = (EArrayDeque<E>*)&that;

		elements = new EA<E>(*(t->elements));
		head = t->head;
		tail = t->tail;
	}

	EArrayDeque<E>& operator= (const EArrayDeque<E>& that) {
		if (this == &that) return *this;

		EArrayDeque<E>* t = (EArrayDeque<E>*)&that;

		//1.
		delete elements;

		//2.
		elements = new EA<E>(*(t->elements));
		head = t->head;
		tail = t->tail;

		return *this;
	}

	// The main insertion and extraction methods are addFirst,
	// addLast, pollFirst, pollLast. The other methods are defined in
	// terms of these.

	/**
	 * Inserts the specified element at the front of this deque.
	 *
	 * @param e the element to add
	 * @throws NullPointerException if the specified element is null
	 */
	void addFirst(E e) {
		if (e == null)
			throw ENullPointerException(__FILE__, __LINE__);
		(*elements)[head = (head - 1) & (elements->length() - 1)] = e;
		if (head == tail)
			doubleCapacity();
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 * @throws NullPointerException if the specified element is null
	 */
	void addLast(E e) {
		if (e == null)
			throw ENullPointerException(__FILE__, __LINE__);
		(*elements)[tail] = e;
		if ( (tail = (tail + 1) & (elements->length() - 1)) == head)
			doubleCapacity();
	}

	/**
	 * Inserts the specified element at the front of this deque.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Deque#offerFirst})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offerFirst(E e) {
		addFirst(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Deque#offerLast})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offerLast(E e) {
		addLast(e);
		return true;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E removeFirst() {
		E x = pollFirst();
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E removeLast() {
		E x = pollLast();
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	E pollFirst() {
		int h = head;
		E result = (*elements)[h]; // Element is null if deque empty
		if (result == null)
			return null;
		(*elements)[h] = null;     // Must null out slot
		head = (h + 1) & (elements->length() - 1);
		return result;
	}

	E pollLast() {
		int t = (tail - 1) & (elements->length() - 1);
		E result = (*elements)[t];
		if (result == null)
			return null;
		(*elements)[t] = null;
		tail = t;
		return result;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E getFirst() {
		E x = (*elements)[head];
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E getLast() {
		E x = (*elements)[(tail - 1) & (elements->length() - 1)];
		if (x == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return x;
	}

	E peekFirst() {
		return (*elements)[head]; // elements[head] is null if deque empty
	}

	E peekLast() {
		return (*elements)[(tail - 1) & (elements->length() - 1)];
	}

	/**
	 * Removes the first occurrence of the specified element in this
	 * deque (when traversing the deque from head to tail).
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the first element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if the deque contained the specified element
	 */
	boolean removeFirstOccurrence(T* o) {
		if (o == null)
			return false;
		int mask = elements->length() - 1;
		int i = head;
		E x;
		while ( (x = (*elements)[i]) != null) {
			if (x->equals(o)) {
				deleteAt(i, null);
				return true;
			}
			i = (i + 1) & mask;
		}
		return false;
	}

	/**
	 * Removes the last occurrence of the specified element in this
	 * deque (when traversing the deque from head to tail).
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the last element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if the deque contained the specified element
	 */
	boolean removeLastOccurrence(T* o) {
		if (o == null)
			return false;
		int mask = elements->length() - 1;
		int i = (tail - 1) & mask;
		E x;
		while ( (x = (*elements)[i]) != null) {
			if (x->equals(o)) {
				deleteAt(i, null);
				return true;
			}
			i = (i - 1) & mask;
		}
		return false;
	}

	// *** Queue methods ***

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #addLast}.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean add(E e) {
		addLast(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this deque.
	 *
	 * <p>This method is equivalent to {@link #offerLast}.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @throws NullPointerException if the specified element is null
	 */
	boolean offer(E e) {
		return offerLast(e);
	}

	/**
	 * Retrieves and removes the head of the queue represented by this deque.
	 *
	 * This method differs from {@link #poll poll} only in that it throws an
	 * exception if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #removeFirst}.
	 *
	 * @return the head of the queue represented by this deque
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E remove() {
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head of the queue represented by this deque
	 * (in other words, the first element of this deque), or returns
	 * <tt>null</tt> if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #pollFirst}.
	 *
	 * @return the head of the queue represented by this deque, or
	 *         <tt>null</tt> if this deque is empty
	 */
	E poll() {
		return pollFirst();
	}

	/**
	 * Retrieves, but does not remove, the head of the queue represented by
	 * this deque.  This method differs from {@link #peek peek} only in
	 * that it throws an exception if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #getFirst}.
	 *
	 * @return the head of the queue represented by this deque
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E element() {
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head of the queue represented by
	 * this deque, or returns <tt>null</tt> if this deque is empty.
	 *
	 * <p>This method is equivalent to {@link #peekFirst}.
	 *
	 * @return the head of the queue represented by this deque, or
	 *         <tt>null</tt> if this deque is empty
	 */
	E peek() {
		return peekFirst();
	}

	// *** Stack methods ***

	/**
	 * Pushes an element onto the stack represented by this deque.  In other
	 * words, inserts the element at the front of this deque.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 * @throws NullPointerException if the specified element is null
	 */
	void push(E e) {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this deque.  In other
	 * words, removes and returns the first element of this deque.
	 *
	 * <p>This method is equivalent to {@link #removeFirst()}.
	 *
	 * @return the element at the front of this deque (which is the top
	 *         of the stack represented by this deque)
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E pop() {
		return removeFirst();
	}

	// *** Collection Methods ***

	/**
	 * Returns the number of elements in this deque.
	 *
	 * @return the number of elements in this deque
	 */
	int size() {
		return (tail - head) & (elements->length() - 1);
	}

	/**
	 * Returns <tt>true</tt> if this deque contains no elements.
	 *
	 * @return <tt>true</tt> if this deque contains no elements
	 */
	boolean isEmpty() {
		return head == tail;
	}

	/**
	 * Returns an iterator over the elements in this deque.  The elements
	 * will be ordered from first (head) to last (tail).  This is the same
	 * order that elements would be dequeued (via successive calls to
	 * {@link #remove} or popped (via successive calls to {@link #pop}).
	 *
	 * @return an iterator over the elements in this deque
	 */
	sp<EIterator<E> > iterator(int index=0) {
		ES_ASSERT(index == 0);
		return new DeqIterator(this);
	}

	sp<EIterator<E> > descendingIterator() {
		return new DescendingIterator(this);
	}

	/**
	 * Returns <tt>true</tt> if this deque contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this deque contains
	 * at least one element <tt>e</tt> such that <tt>o.equals(e)</tt>.
	 *
	 * @param o object to be checked for containment in this deque
	 * @return <tt>true</tt> if this deque contains the specified element
	 */
	boolean contains(T* o) {
		if (o == null)
			return false;
		int mask = elements->length() - 1;
		int i = head;
		E x;
		while ( (x = (*elements)[i]) != null) {
			if (x->equals(o))
				return true;
			i = (i + 1) & mask;
		}
		return false;
	}

	/**
	 * Removes a single instance of the specified element from this deque.
	 * If the deque does not contain the element, it is unchanged.
	 * More formally, removes the first element <tt>e</tt> such that
	 * <tt>o.equals(e)</tt> (if such an element exists).
	 * Returns <tt>true</tt> if this deque contained the specified element
	 * (or equivalently, if this deque changed as a result of the call).
	 *
	 * <p>This method is equivalent to {@link #removeFirstOccurrence}.
	 *
	 * @param o element to be removed from this deque, if present
	 * @return <tt>true</tt> if this deque contained the specified element
	 */
	boolean remove(T* o) {
		return removeFirstOccurrence(o);
	}

	/**
	 * Removes all of the elements from this deque.
	 * The deque will be empty after this call returns.
	 */
	void clear() {
		int h = head;
		int t = tail;
		if (h != t) { // clear all cells
			head = tail = 0;
			int i = h;
			int mask = elements->length() - 1;
			do {
				(*elements)[i] = null;
				i = (i + 1) & mask;
			} while (i != t);
		}
	}

private:
	/**
	 * The array in which the elements of the deque are stored.
	 * The capacity of the deque is the length of this array, which is
	 * always a power of two. The array is never allowed to become
	 * full, except transiently within an addX method where it is
	 * resized (see doubleCapacity) immediately upon becoming full,
	 * thus avoiding head and tail wrapping around to equal each
	 * other.  We also guarantee that all array cells not holding
	 * deque elements are always null.
	 */
	EA<E>* elements;

	/**
	 * The index of the element at the head of the deque (which is the
	 * element that would be removed by remove() or pop()); or an
	 * arbitrary number equal to tail if the deque is empty.
	 */
	int head;

	/**
	 * The index at which the next element would be added to the tail
	 * of the deque (via addLast(E), add(E), or push(E)).
	 */
	int tail;

	/**
	 * The minimum capacity that we'll use for a newly created deque.
	 * Must be a power of 2.
	 */
	static const int MIN_INITIAL_CAPACITY = 8;

	// ******  Array allocation and resizing utilities ******

	/**
	 * Allocate empty array to hold the given number of elements.
	 *
	 * @param numElements  the number of elements to hold
	 */
	void allocateElements(int numElements) {
		unsigned int initialCapacity = MIN_INITIAL_CAPACITY;
		// Find the best power of two to hold elements.
		// Tests "<=" because arrays aren't kept full.
		if (numElements >= initialCapacity) {
			initialCapacity = numElements;
			initialCapacity |= (initialCapacity >>  1);
			initialCapacity |= (initialCapacity >>  2);
			initialCapacity |= (initialCapacity >>  4);
			initialCapacity |= (initialCapacity >>  8);
			initialCapacity |= (initialCapacity >> 16);
			initialCapacity++;
		}
		elements = new EA<E>(initialCapacity);
	}

	/**
	 * Double the capacity of this deque.  Call only when full, i.e.,
	 * when head and tail have wrapped around to become equal.
	 */
	void doubleCapacity() {
		ES_ASSERT(head == tail);
		int p = head;
		int n = elements->length();
		int r = n - p; // number of elements to the right of p
		int newCapacity = n << 1;
		if (newCapacity < 0)
			throw EIllegalStateException(__FILE__, __LINE__, "Sorry, deque too big");
		EA<E>* a = new EA<E>(newCapacity);
		ESystem::arraycopy((*elements), p, (*a), 0, r);
		ESystem::arraycopy((*elements), 0, (*a), r, p);
		delete elements; //!
		elements = a;
		head = 0;
		tail = n;
	}

    void checkInvariants() {
        ES_ASSERT((*elements)[tail] == null);
        ES_ASSERT(head == tail ? (*elements)[head] == null :
            ((*elements)[head] != null &&
             (*elements)[(tail - 1) & (elements->length() - 1)] != null));
        ES_ASSERT((*elements)[(head - 1) & (elements->length() - 1)] == null);
    }

    /**
     * Removes the element at the specified position in the elements array,
     * adjusting head and tail as necessary.  This can result in motion of
     * elements backwards or forwards in the array.
     *
     * <p>This method is called delete rather than remove to emphasize
     * that its semantics differ from those of {@link List#remove(int)}.
     *
     * @param flag true if elements moved backwards
     * @return the deleted element
     */
    E deleteAt(int i, boolean* flag) {
        checkInvariants();
        EA<E>* elements = this->elements;
        int mask = elements->length() - 1;
        int h = head;
        int t = tail;
        int front = (i - h) & mask;
        int back  = (t - i) & mask;

        // Invariant: head <= i < tail mod circularity
        if (front >= ((t - h) & mask))
            throw EConcurrentModificationException(__FILE__, __LINE__);

    	E o = (*elements)[i]; //1

        // Optimize for least element motion
        if (front < back) {
            if (h <= i) {
                ESystem::arraycopy((*elements), h, (*elements), h + 1, front);
            } else { // Wrap around
            	ESystem::arraycopy((*elements), 0, (*elements), 1, i);
                (*elements)[0] = (*elements)[mask];
                ESystem::arraycopy((*elements), h, (*elements), h + 1, mask - h);
            }
            (*elements)[h] = null;
            head = (h + 1) & mask;
            if (flag) *flag = false;
        } else {
            if (i < t) { // Copy the null tail as well
            	ESystem::arraycopy((*elements), i + 1, (*elements), i, back);
                tail = t - 1;
            } else { // Wrap around
            	ESystem::arraycopy((*elements), i + 1, (*elements), i, mask - i);
                (*elements)[mask] = (*elements)[0];
                ESystem::arraycopy((*elements), 1, (*elements), 0, t);
                tail = (t - 1) & mask;
            }
            if (flag) *flag = true;
        }

        return o; //!
   }
};

} /* namespace efc */
#endif //!EARRAYDEQUE_HH_
