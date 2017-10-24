/*
 * EConcurrentLiteQueue.hh
 *
 *  Created on: 2016-12-29
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTLITEQUEUE_HH_
#define ECONCURRENTLITEQUEUE_HH_

#include "../ESpinLock.hh"
#include "./EAtomicCounter.hh"
#include "./EConcurrentQueue.hh"
#include "../ENoSuchElementException.hh"
#include "../ENullPointerException.hh"
#include "../EIllegalStateException.hh"
#include "../EUnsupportedOperationException.hh"

namespace efc {

/**
 * An unbounded thread-safe {@linkplain Queue queue} based on linked nodes.
 * This queue orders elements FIFO (first-in-first-out).
 * The <em>head</em> of the queue is that element that has been on the
 * queue the longest time.
 * The <em>tail</em> of the queue is that element that has been on the
 * queue the shortest time. New elements
 * are inserted at the tail of the queue, and the queue retrieval
 * operations obtain elements at the head of the queue.
 * A <tt>EConcurrentLiteQueue</tt> is an appropriate choice when
 * many threads will share access to a common collection.
 * This queue does not permit <tt>null</tt> elements.
 *
 * <p>This implementation consider speed first, and only to achieve the most
 * basic functions
 *
 */

template<typename E, typename LOCK=ESpinLock>
class EConcurrentLiteQueue: public EConcurrentQueue<E> {
private:
	typedef struct node_t {
		sp<E> value;
		node_t* volatile next;

		node_t(): next(null) {}
	} Node;

public:
    ~EConcurrentLiteQueue() {
    	// node unlink for recursion
    	Node* node = head;
		while (node != null) {
			Node* n = node->next;
			delete node;
			node = n;
		}
    }

    /**
     * Creates a <tt>EConcurrentLiteQueue</tt> that is initially empty.
     */
    EConcurrentLiteQueue() {
    	Node* node = new Node();
		head = tail = node;
    }

    // Have to override just to update the javadoc

    /**
     * Inserts the specified element at the tail of this queue.
     *
     * @return <tt>true</tt> (as specified by {@link Collection#add})
     * @throws NullPointerException if the specified element is null
     */
    boolean add(E* e) {
        return offer(e);
    }
    boolean add(sp<E> e) {
		return offer(e);
	}

    /**
     * Inserts the specified element at the tail of this queue.
     *
     * @return <tt>true</tt> (as specified by {@link Queue#offer})
     * @throws NullPointerException if the specified element is null
     */
    boolean offer(E* e) {
        sp<E> x(e);
        boolean r = offer(x);
		if (!r) {
			x.dismiss();
		}
		return r;
    }
    boolean offer(sp<E> e) {
    	if (e == null) throw ENullPointerException(__FILE__, __LINE__);
    	Node* node = new Node();
		node->value = e;
		node->next = null;
		tl.lock();
			tail->next = node;
			tail = node;
			size_++;
		tl.unlock();
		return true;
    }

    sp<E> poll() {
    	sp<E> v;
    	Node* node = null;
		hl.lock();
			node = head;
			Node* new_head = node->next;
			if (new_head == null) {
				hl.unlock();
				return null;
			}
			v = new_head->value;
			head = new_head;
			head->value = null;
			size_--;
		hl.unlock();
		delete node;
		return v;
    }

    sp<E> element() {
    	sp<E> x = peek();
		if (x != null)
			return x;
		else
			throw ENoSuchElementException(__FILE__, __LINE__);
    }

    sp<E> peek() { // same as poll except don't remove item
    	sp<E> v;
		hl.lock();
			Node* node = head->next;
			if (node == null) {
				hl.unlock();
				return null;
			}
			v = node->value;
		hl.unlock();
		return v;
    }

    /**
     * Returns <tt>true</tt> if this queue contains no elements.
     *
     * @return <tt>true</tt> if this queue contains no elements
     */
    boolean isEmpty() {
        return head->next == null;
    }

    /**
     * Returns the number of elements in this queue.  If this queue
     * contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
     * <tt>Integer.MAX_VALUE</tt>.
     *
     * <p>Beware that, unlike in most collections, this method is
     * <em>NOT</em> a constant-time operation. Because of the
     * asynchronous nature of these queues, determining the current
     * number of elements requires an O(n) traversal.
     *
     * @return the number of elements in this queue
     */
    int size() {
        return size_.value();
    }

    /**
	 * Returns <tt>true</tt> if this queue contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this queue contains
	 * at least one element <tt>e</tt> such that <tt>o.equals(e)</tt>.
	 *
	 * @param o object to be checked for containment in this queue
	 * @return <tt>true</tt> if this queue contains the specified element
	 */
	boolean contains(E* o) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * Removes a single instance of the specified element from this queue,
	 * if it is present.  More formally, removes an element <tt>e</tt> such
	 * that <tt>o.equals(e)</tt>, if this queue contains one or more such
	 * elements.
	 * Returns <tt>true</tt> if this queue contained the specified element
	 * (or equivalently, if this queue changed as a result of the call).
	 *
	 * @param o element to be removed from this queue, if present
	 * @return <tt>true</tt> if this queue changed as a result of the call
	 */
	boolean remove(E* o) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

    /**
	 * Retrieves and removes the head of this queue.  This method differs
	 * from {@link #poll poll} only in that it throws an exception if this
	 * queue is empty.
	 *
	 * <p>This implementation returns the result of <tt>poll</tt>
	 * unless the queue is empty.
	 *
	 * @return the head of this queue
	 * @throws NoSuchElementException if this queue is empty
	 */
	sp <E> remove() {
		sp<E> x = poll();
		if (x != null)
			return x;
		else
			throw ENoSuchElementException(__FILE__, __LINE__);
	}

    /**
	 * Removes all of the elements from this queue.
	 * The queue will be empty after this call returns.
	 *
	 * <p>This implementation repeatedly invokes {@link #poll poll} until it
	 * returns <tt>null</tt>.
	 */
	void clear() {
		while (poll() != null)
			;
	}

	/**
	 * Returns an iterator over the elements in this queue in proper sequence.
	 * The returned iterator is a "weakly consistent" iterator that
	 * will never throw {@link ConcurrentModificationException},
	 * and guarantees to traverse elements as they existed upon
	 * construction of the iterator, and may (but is not guaranteed to)
	 * reflect any modifications subsequent to construction.
	 *
	 * @return an iterator over the elements in this queue in proper sequence
	 */
	sp<EConcurrentIterator<E> > iterator() {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * Returns an array containing all of the elements in this queue, in
	 * proper sequence.
	 *
	 * <p>The returned array will be "safe" in that no references to it are
	 * maintained by this queue.  (In other words, this method must allocate
	 * a new array).  The caller is thus free to modify the returned array.
	 *
	 * <p>This method acts as bridge between array-based and collection-based
	 * APIs.
	 *
	 * @return an array containing all of the elements in this queue
	 */
	EA<sp<E> > toArray() {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

private:
    /**
	 * Pointer to header node, initialized to a dummy node.  The first
	 * actual node is at head.getNext().
	 */
	Node *head;

	/** Pointer to last node on list **/
	Node* tail;// = head;

	LOCK hl;
	LOCK tl;

	EAtomicCounter size_;
};

} /* namespace efc */
#endif /* ECONCURRENTLITEQUEUE_HH_ */
