/*
 * EPriorityQueue.hh
 *
 *  Created on: 2013-12-18
 *      Author: cxxjava@163.com
 */

#ifndef EPRIORITYQUEUE_HH_
#define EPRIORITYQUEUE_HH_

#include "EA.hh"
#include "EArrays.hh"
#include "EInteger.hh"
#include "EAbstractQueue.hh"
#include "EComparator.hh"
#include "EOutOfMemoryError.hh"
#include "EIllegalArgumentException.hh"
#include "ENoSuchElementException.hh"
#include "EConcurrentModificationException.hh"

namespace efc {

/**
 * An unbounded priority {@linkplain Queue queue} based on a priority heap.
 * The elements of the priority queue are ordered according to their
 * {@linkplain Comparable natural ordering}, or by a {@link Comparator}
 * provided at queue construction time, depending on which constructor is
 * used.  A priority queue does not permit {@code null} elements.
 * A priority queue relying on natural ordering also does not permit
 * insertion of non-comparable objects (doing so may result in
 * {@code ClassCastException}).
 *
 * <p>The <em>head</em> of this queue is the <em>least</em> element
 * with respect to the specified ordering.  If multiple elements are
 * tied for least value, the head is one of those elements -- ties are
 * broken arbitrarily.  The queue retrieval operations {@code poll},
 * {@code remove}, {@code peek}, and {@code element} access the
 * element at the head of the queue.
 *
 * <p>A priority queue is unbounded, but has an internal
 * <i>capacity</i> governing the size of an array used to store the
 * elements on the queue.  It is always at least as large as the queue
 * size.  As elements are added to a priority queue, its capacity
 * grows automatically.  The details of the growth policy are not
 * specified.
 *
 * <p>This class and its iterator implement all of the
 * <em>optional</em> methods of the {@link Collection} and {@link
 * Iterator} interfaces.  The Iterator provided in method {@link
 * #iterator()} is <em>not</em> guaranteed to traverse the elements of
 * the priority queue in any particular order. If you need ordered
 * traversal, consider using {@code Arrays.sort(pq.toArray())}.
 *
 * <p> <strong>Note that this implementation is not synchronized.</strong>
 * Multiple threads should not access a {@code PriorityQueue}
 * instance concurrently if any of the threads modifies the queue.
 * Instead, use the thread-safe {@link
 * java.util.concurrent.PriorityBlockingQueue} class.
 *
 * <p>Implementation note: this implementation provides
 * O(log(n)) time for the enqueing and dequeing methods
 * ({@code offer}, {@code poll}, {@code remove()} and {@code add});
 * linear time for the {@code remove(Object)} and {@code contains(Object)}
 * methods; and constant time for the retrieval methods
 * ({@code peek}, {@code element}, and {@code size}).
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since 1.5
 * @param <E> the type of elements held in this collection
 */

template<typename E>
class EPriorityQueue: public EAbstractQueue<E> {
public:
	virtual ~EPriorityQueue() {
		delete queue;
	}

	/**
     * Creates a {@code PriorityQueue} with the specified initial
     * capacity that orders its elements according to their
     * {@linkplain Comparable natural ordering}.
     *
     * @param initialCapacity the initial capacity for this priority queue
     * @throws IllegalArgumentException if {@code initialCapacity} is less
     *         than 1
     */
	explicit
    EPriorityQueue() :
    	_size(0), _comparator(null), modCount(0) {
        this->queue = new EA<EObject*>(DEFAULT_INITIAL_CAPACITY, true);
    }

	explicit
    EPriorityQueue(boolean autoFree) :
    	_size(0), _comparator(null), modCount(0) {
        this->queue = new EA<EObject*>(DEFAULT_INITIAL_CAPACITY, autoFree);
    }

	explicit
    EPriorityQueue(int initialCapacity) :
    	_size(0), _comparator(null), modCount(0) {
        this->queue = new EA<EObject*>(initialCapacity, true);
    }

	explicit
    EPriorityQueue(int initialCapacity, boolean autoFree) :
    	_size(0), _comparator(null), modCount(0) {
        this->queue = new EA<EObject*>(initialCapacity, autoFree);
    }
    
	/**
     * Creates a {@code PriorityQueue} with the default initial
     * capacity (11) that orders its elements according to their
     * {@linkplain Comparable natural ordering}.
     */
	explicit
	EPriorityQueue(int initialCapacity, EComparator<E>* comparator, boolean autoFree=true) :
			_size(0), modCount(0) {
    	// Note: This restriction of at least one is not actually needed,
        // but continues for 1.5 compatibility
        if (initialCapacity < 1)
            throw EIllegalArgumentException(__FILE__, __LINE__);
        this->queue = new EA<E>(initialCapacity, autoFree);
        this->_comparator = comparator;
    }
	
	/**
	 *
	 */
	EPriorityQueue(const EPriorityQueue& that) {
		EPriorityQueue* t = (EPriorityQueue*)&that;
		this->_size = t->_size;
		this->modCount = 0;
		this->queue = t->queue->clone();
		this->queue->setAutoFree(t->queue->getAutoFree());
		t->queue->setAutoFree(false);
		this->_comparator = t->_comparator;
	}

	/**
	 *
	 */
	EPriorityQueue& operator= (const EPriorityQueue& that) {
		if (this == &that) return *this;

		EPriorityQueue* t = (EPriorityQueue*)&that;

		//1.
		delete queue;

		//2.
		this->_size = t->_size;
		this->modCount = 0;
		this->queue = t->queue->clone();
		this->queue->setAutoFree(t->queue->getAutoFree());
		t->queue->setAutoFree(false);
		this->_comparator = t->_comparator;

		return *this;
	}

	/**
     * Inserts the specified element into this priority queue.
     *
     * @return {@code true} (as specified by {@link Collection#add})
     * @throws ClassCastException if the specified element cannot be
     *         compared with elements currently in this priority queue
     *         according to the priority queue's ordering
     * @throws NullPointerException if the specified element is null
     */
    boolean add(E e) {
        return offer(e);
    }

    /**
     * Inserts the specified element into this priority queue.
     *
     * @return {@code true} (as specified by {@link Queue#offer})
     * @throws ClassCastException if the specified element cannot be
     *         compared with elements currently in this priority queue
     *         according to the priority queue's ordering
     * @throws NullPointerException if the specified element is null
     */
    boolean offer(E e) {
        if (e == null)
            throw ENullPointerException(__FILE__, __LINE__);
        modCount++;
        int i = _size;
        if (i >= queue->length())
            grow(i + 1);
        _size = i + 1;
        if (i == 0)
            (*queue)[0] = e;
        else
            siftUp(i, e);
        return true;
    }

	/**
     * Retrieves, but does not remove, the head of this queue,
     * or returns <tt>null</tt> if this queue is empty.
     *
     * @return the head of this queue, or <tt>null</tt> if this queue is empty
     */
    E peek() {
        if (_size == 0)
            return null;
        return (*queue)[0];
    }

	/**
     * Removes a single instance of the specified element from this queue,
     * if it is present.  More formally, removes an element {@code e} such
     * that {@code o.equals(e)}, if this queue contains one or more such
     * elements.  Returns {@code true} if and only if this queue contained
     * the specified element (or equivalently, if this queue changed as a
     * result of the call).
     *
     * @param o element to be removed from this queue, if present
     * @return {@code true} if this queue changed as a result of the call
     */
    boolean remove(E o) {
        int i = indexOf(o);
        if (i == -1)
            return false;
        else {
            removeAt(i);
            return true;
        }
    }

	/**
     * Returns {@code true} if this queue contains the specified element.
     * More formally, returns {@code true} if and only if this queue contains
     * at least one element {@code e} such that {@code o.equals(e)}.
     *
     * @param o object to be checked for containment in this queue
     * @return {@code true} if this queue contains the specified element
     */
    boolean contains(E o) {
        return indexOf(o) != -1;
    }
    
    /**
     * Returns an iterator over the elements in this queue. The iterator
     * does not return the elements in any particular order.
     *
     * @return an iterator over the elements in this queue
     */
    sp<EIterator<E> > iterator(int index=0) {
        return new Itr(this);
    }
    
    int size() {
        return _size;
    }

    /**
     * Removes all of the elements from this priority queue.
     * The queue will be empty after this call returns.
     */
    void clear() {
        modCount++;
        queue->clear();
        _size = 0;
    }

	/**
     * Retrieves and removes the head of this queue,
     * or returns <tt>null</tt> if this queue is empty.
     *
     * @return the head of this queue, or <tt>null</tt> if this queue is empty
     */
    E poll() {
        if (_size == 0)
            return null;
        int s = --_size;
        modCount++;
        E result = (E) (*queue)[0];
        E x = (E) (*queue)[s];
        (*queue)[s] = null;
        if (s != 0)
            siftDown(0, x);
        return result;
    }
    
    /**
     * Returns the comparator used to order the elements in this
     * queue, or {@code null} if this queue is sorted according to
     * the {@linkplain Comparable natural ordering} of its elements.
     *
     * @return the comparator used to order this queue, or
     *         {@code null} if this queue is sorted according to the
     *         natural ordering of its elements
     */
    EComparator<E>* comparator() {
        return _comparator;
    }

    /**
     *
     */
    void setAutoFree(boolean autoFree = true ) {
    	queue->setAutoFree(autoFree);
    }

    /**
     *
     */
    boolean getAutoFree() {
    	return queue->getAutoFree();
    }

private:
	static const int DEFAULT_INITIAL_CAPACITY = 11;
	
	/**
     * The maximum size of array to allocate.
     * Some VMs reserve some header words in an array.
     * Attempts to allocate larger arrays may result in
     * OutOfMemoryError: Requested array size exceeds VM limit
     */
    static const int MAX_ARRAY_SIZE = 0x7fffffff - 8; //Integer.MAX_VALUE - 8;
    
	/**
     * Priority queue represented as a balanced binary heap: the two
     * children of queue[n] are queue[2*n+1] and queue[2*(n+1)].  The
     * priority queue is ordered by comparator, or by the elements'
     * natural ordering, if comparator is null: For each node n in the
     * heap and each descendant d of n, n <= d.  The element with the
     * lowest value is in queue[0], assuming the queue is nonempty.
     */
    EA<E>* queue;

    /**
     * The number of elements in the priority queue.
     */
    int _size;// = 0;

    /**
     * The comparator, or null if priority queue uses elements'
     * natural ordering.
     */
    EComparator<E>* _comparator;

    /**
     * The number of times this priority queue has been
     * <i>structurally modified</i>.  See AbstractList for gory details.
     */
    int modCount;// = 0;
	
	class Itr : public EIterator<E> {
	public:
		~Itr() {
			delete forgetMeNot;
		}
		
		Itr(EPriorityQueue<E>* q) : queue(q), cursor(0), lastRet(-1), forgetMeNot(null), lastRetElt(null) {
			expectedModCount = queue->modCount;
		}
		
		boolean hasNext() {
			return cursor < queue->_size ||
                (forgetMeNot != null && !forgetMeNot->isEmpty());
		}
		
		E next() {
			if (expectedModCount != queue->modCount)
                throw EConcurrentModificationException(__FILE__, __LINE__);
            if (cursor < queue->_size) {
                return (*queue->queue)[lastRet = cursor++];
            }
            if (forgetMeNot != null) {
                lastRet = -1;
                lastRetElt = forgetMeNot->poll();
                if (lastRetElt != null)
                    return lastRetElt;
            }
            throw ENoSuchElementException(__FILE__, __LINE__);
		}
		
		void remove() {
			if (expectedModCount != queue->modCount)
                throw EConcurrentModificationException(__FILE__, __LINE__);
            if (lastRet != -1) {
                E moved = queue->removeAt(lastRet);
                lastRet = -1;
                if (moved == null)
                    cursor--;
                else {
                    if (forgetMeNot == null)
                        forgetMeNot = new EArrayDeque<E>();
                    forgetMeNot->add(moved);
                }
            } else if (lastRetElt != null) {
                queue->removeEq(lastRetElt);
                lastRetElt = null;
            } else {
                throw EIllegalStateException(__FILE__, __LINE__);
            }
            expectedModCount = queue->modCount;
		}
		
		E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

	private:
		EPriorityQueue<E>* queue;
		
		/**
         * Index (into queue array) of element to be returned by
         * subsequent call to next.
         */
        int cursor;// = 0;

        /**
         * Index of element returned by most recent call to next,
         * unless that element came from the forgetMeNot list.
         * Set to -1 if element is deleted by a call to remove.
         */
        int lastRet;// = -1;

        /**
         * A queue of elements that were moved from the unvisited portion of
         * the heap into the visited portion as a result of "unlucky" element
         * removals during the iteration.  (Unlucky element removals are those
         * that require a siftup instead of a siftdown.)  We must visit all of
         * the elements in this list to complete the iteration.  We do this
         * after we've completed the "normal" iteration.
         *
         * We expect that most iterations, even those involving removals,
         * will not need to store elements in this field.
         */
        EArrayDeque<E>* forgetMeNot;// = null;

        /**
         * Element returned by the most recent call to next iff that
         * element was drawn from the forgetMeNot list.
         */
        E lastRetElt;// = null;

        /**
         * The modCount value that the iterator believes that the backing
         * Queue should have.  If this expectation is violated, the iterator
         * has detected concurrent modification.
         */
        int expectedModCount;// = modCount;
	};
	
private:
	static int hugeCapacity(int minCapacity) {
        if (minCapacity < 0) // overflow
            throw EOutOfMemoryError(__FILE__, __LINE__);
        return (minCapacity > MAX_ARRAY_SIZE) ?
            EInteger::MAX_VALUE :
            MAX_ARRAY_SIZE;
    }
    
	/**
     * Increases the capacity of the array.
     *
     * @param minCapacity the desired minimum capacity
     */
    void grow(int minCapacity) {
    	int oldCapacity = queue->length();
        // Double size if small; else grow by 50%
        int newCapacity = oldCapacity + ((oldCapacity < 64) ?
                                         (oldCapacity + 2) :
                                         (oldCapacity >> 1));
        // overflow-conscious code
        if (newCapacity - MAX_ARRAY_SIZE > 0)
            newCapacity = hugeCapacity(minCapacity);
        EA<E>* old = queue;
        queue = EArrays::copyOf(queue, newCapacity);
        queue->setAutoFree(old->getAutoFree());
        old->setAutoFree(false);
        delete old; //!
    }
    
    /**
     * Removes the ith element from queue.
     *
     * Normally this method leaves the elements at up to i-1,
     * inclusive, untouched.  Under these circumstances, it returns
     * null.  Occasionally, in order to maintain the heap invariant,
     * it must swap a later element of the list with one earlier than
     * i.  Under these circumstances, this method returns the element
     * that was previously at the end of the list and is now at some
     * position before i. This fact is used by iterator.remove so as to
     * avoid missing traversing elements.
     */
    E removeAt(int i) {
    	ES_ASSERT(i >= 0 && i < _size);
    	E removed = (E) (*queue)[i]; //!
        modCount++;
        int s = --_size;
        if (s == i) { // removed last element
            (*queue)[i] = null;
        }
        else {
            E moved = (E) (*queue)[s];
            (*queue)[s] = null;
            siftDown(i, moved);
            if ((*queue)[i] == moved) {
                siftUp(i, moved);
                if ((*queue)[i] != moved) {
                	delete removed; //!
                    return moved;
                }
            }
        }
    	delete removed; //!
        return null;
    }
    
    /**
     * Inserts item x at position k, maintaining heap invariant by
     * promoting x up the tree until it is greater than or equal to
     * its parent, or is the root.
     *
     * To simplify and speed up coercions and comparisons. the
     * Comparable and Comparator versions are separated into different
     * methods that are otherwise identical. (Similarly for siftDown.)
     *
     * @param k the position to fill
     * @param x the item to insert
     */
    void siftUp(int k, E x) {
        if (_comparator != null)
            siftUpUsingComparator(k, x);
        else
            siftUpComparable(k, x);
    }

    void siftUpComparable(int k, E x) {
        EComparable<E>* key = (EComparable<E>*) x;
        while (k > 0) {
            int parent = (uint)(k - 1) >> 1;
            E e = (*queue)[parent];
            if (key->compareTo((E) e) >= 0)
                break;
            (*queue)[k] = e;
            k = parent;
        }
        (*queue)[k] = x;
    }

    void siftUpUsingComparator(int k, E x) {
        while (k > 0) {
            int parent = (uint)(k - 1) >> 1;
            E e = (*queue)[parent];
            if (_comparator->compare(x, (E) e) >= 0)
                break;
            (*queue)[k] = e;
            k = parent;
        }
        (*queue)[k] = x;
    }

    /**
     * Inserts item x at position k, maintaining heap invariant by
     * demoting x down the tree repeatedly until it is less than or
     * equal to its children or is a leaf.
     *
     * @param k the position to fill
     * @param x the item to insert
     */
    void siftDown(int k, E x) {
        if (_comparator != null)
            siftDownUsingComparator(k, x);
        else
            siftDownComparable(k, x);
    }

    void siftDownComparable(int k, E x) {
        EComparable<E>* key = (EComparable<E>*) x;
        int half = (uint)_size >> 1;        // loop while a non-leaf
        while (k < half) {
            int child = (k << 1) + 1; // assume left child is least
            E c = (*queue)[child];
            int right = child + 1;
            if (right < _size &&
                ((EComparable<E>*) c)->compareTo((E) (*queue)[right]) > 0)
                c = (*queue)[child = right];
            if (key->compareTo((E) c) <= 0)
                break;
            (*queue)[k] = c;
            k = child;
        }
        (*queue)[k] = x;
    }

    void siftDownUsingComparator(int k, E x) {
        int half = (uint)_size >> 1;
        while (k < half) {
            int child = (k << 1) + 1;
            E c = (*queue)[child];
            int right = child + 1;
            if (right < _size &&
                _comparator->compare((E) c, (E) (*queue)[right]) > 0)
                c = (*queue)[child = right];
            if (_comparator->compare(x, (E) c) <= 0)
                break;
            (*queue)[k] = c;
            k = child;
        }
        (*queue)[k] = x;
    }
    
    int indexOf(E o) {
        if (o != null) {
            for (int i = 0; i < _size; i++)
                if (o->equals((*queue)[i]))
                    return i;
        }
        return -1;
    }
    
    /**
     * Version of remove using reference equality, not equals.
     * Needed by iterator.remove.
     *
     * @param o element to be removed from this queue, if present
     * @return {@code true} if removed
     */
    boolean removeEq(E o) {
        for (int i = 0; i < _size; i++) {
            if (o == (*queue)[i]) {
                removeAt(i);
                return true;
            }
        }
        return false;
    }
};

} /* namespace efc */
#endif /* EPRIORITYQUEUE_HH_ */
