/*
 * EConcurrentIntrusiveDeque.hh
 *
 *  Created on: 2016-12-29
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTINTRUSIVEDEQUE_HH_
#define ECONCURRENTINTRUSIVEDEQUE_HH_

#include "./EReentrantLock.hh"
#include "../ESynchronizeable.hh"
#include "./EAtomicCounter.hh"
#include "./EConcurrentDeque.hh"
#include "../ENoSuchElementException.hh"
#include "../ENullPointerException.hh"
#include "../EIllegalArgumentException.hh"
#include "../EUnsupportedOperationException.hh"

namespace efc {

/**
 * An unbounded thread-safe {@linkplain Deque queue} based on the object
 * is extends <em>EQueueEntry</em>.
 *
 */

template<typename E, typename LOCK=EReentrantLock>
class EConcurrentIntrusiveDeque: public EConcurrentDeque<E> {
public:
    ~EConcurrentIntrusiveDeque() {
    	// node unlink for recursion

    }

    /**
     * Creates a <tt>EConcurrentLiteQueue</tt> that is initially empty.
     */
    EConcurrentIntrusiveDeque() {
		head = tail = new EQueueEntry();
    }

	// *** Queue methods ***

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
    	sp<EQueueEntry> entry = dynamic_pointer_cast<EQueueEntry>(e);
    	if (!entry || entry->getOwner()) {
    		throw EIllegalArgumentException(__FILE__, __LINE__);
    	}

    	entry->setOwner((EObject*)this);

    	SYNCBLOCK(&lock) {
			tail->setNext(entry);
			entry->setPrev(tail);
			entry->setNext(null);
			tail = entry;
			size_++;
    	}}

		return true;
    }

    sp<E> poll() {
    	sp<EQueueEntry> entry;
    	SYNCBLOCK(&lock) {
			if (head == tail) {
				return null;
			}
			entry = head->getNext();
			if (entry == tail) tail = head;
			sp<EQueueEntry> entry_next = entry->getNext();
			head->setNext(entry_next);
			if (entry_next != null) entry_next->setPrev(head);
			entry->setPrev(null);
			entry->setNext(null);
			entry->setOwner(null);
			size_--;
    	}}
		return dynamic_pointer_cast<E>(entry);
    }

    sp<E> element() {
    	sp<E> x = peek();
		if (x != null)
			return x;
		else
			throw ENoSuchElementException(__FILE__, __LINE__);
    }

    sp<E> peek() { // same as poll except don't remove item
    	sp<EQueueEntry> entry;
    	SYNCBLOCK(&lock) {
			entry = head->getNext();
    	}}
		return dynamic_pointer_cast<E>(entry);
    }

    /**
     * Returns <tt>true</tt> if this queue contains no elements.
     *
     * @return <tt>true</tt> if this queue contains no elements
     */
    boolean isEmpty() {
    	boolean r = false;
    	SYNCBLOCK(&lock) {
    		r = (head->getNext() == null);
    	}}
        return r;
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
		EQueueEntry* entry = dynamic_cast<EQueueEntry*>(o);
		if (!entry) {
			return false;
		}

		SYNCBLOCK(&lock) {
			EObject* owner = entry->getOwner();
			if (!owner) {
				return false;
			}

			EObject* this_obj = (EObject*)this;
			return (owner == this_obj);
		}}
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
		EQueueEntry* entry = dynamic_cast<EQueueEntry*>(o);
		if (!entry) {
			return false;
		}

		SYNCBLOCK(&lock) {
			EObject* owner = entry->getOwner();
			if (!owner) {
				return false;
			}

			EObject* this_obj = (EObject*)this;
			if (owner != this_obj) {
				return false;
			}

			sp<EQueueEntry> entry_prev = entry->getPrev();
			sp<EQueueEntry> entry_next = entry->getNext();
			if (entry_prev != null) entry_prev->setNext(entry->getNext());
			if (entry_next != null) {
				entry_next->setPrev(entry_prev);
			} else if (entry == tail.get()) {
				tail = tail->getPrev();
			}
			entry->setPrev(null);
			entry->setNext(null);
			entry->setOwner(null);
			size_--;
			return true;
		}}
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
		SYNCBLOCK(&lock) {
			if (head == tail) {
				return;
			}

			sp<EQueueEntry> entry = head->getNext();
			while (entry != null) {
				entry->setPrev(null);
				entry->setNext(null);
				entry->setOwner(null);

				entry = entry->getNext();
			}

			head = tail = new EQueueEntry();
			size_ = 0;
		}}
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

	// *** Deque methods ***

	void addFirst(E* e) {
		(void)offerFirst(e);
	}

	void addFirst(sp<E> e) {
		(void)offerFirst(e);
	}

	void addLast(E* e) {
		this->offer(e);
	}

	void addLast(sp<E> e) {
		this->offer(e);
	}

	boolean offerFirst(E* e) {
		sp<E> x(e);
		boolean r = offerFirst(x);
		if (!r) {
			x.dismiss();
		}
		return r;
	}

	boolean offerFirst(sp<E> e) {
		sp<EQueueEntry> entry = dynamic_pointer_cast<EQueueEntry>(e);
		if (!entry || entry->getOwner()) {
			throw EIllegalArgumentException(__FILE__, __LINE__);
		}

		entry->setOwner((EObject*)this);

		SYNCBLOCK(&lock) {
			entry->setPrev(head);
			entry->setNext(head->getNext());
			head->setNext(entry);
			size_++;
		}}

		return true;
	}

	boolean offerLast(E* e) {
		return this->offer(e);
	}

	boolean offerLast(sp<E> e) {
		return this->offer(e);
	}

	sp<E> removeFirst() {
		return pollFirst();
	}

	sp<E> removeLast() {
		return pollLast();
	}

	sp<E> pollFirst() {
		return poll();
	}

	sp<E> pollLast() {
		sp<EQueueEntry> entry;
		SYNCBLOCK(&lock) {
			if (head == tail) {
				return null;
			}
			entry = tail->getPrev();
			if (entry == head) head = tail;
			sp<EQueueEntry> entry_prev = entry->getPrev();
			tail->setPrev(entry_prev);
			if (entry_prev != null) entry_prev->setNext(tail);
			entry->setPrev(null);
			entry->setNext(null);
			entry->setOwner(null);
			size_--;
		}}
		return dynamic_pointer_cast<E>(entry);
	}

	sp<E> getFirst() {
		return peekFirst();
	}

	sp<E> getLast() {
		return peekLast();
	}

	sp<E> peekFirst() {
		return peek();
	}

	sp<E> peekLast() {
		sp<EQueueEntry> entry;
		SYNCBLOCK(&lock) {
			entry = tail->getPrev();
		}}
		return dynamic_pointer_cast<E>(entry);
	}

	boolean removeFirstOccurrence(E* o) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	boolean removeLastOccurrence(E* o) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	// *** Stack methods ***

	void push(E* e) {
		addFirst(e);
	}

	void push(sp<E> e) {
		addFirst(e);
	}

	sp<E> pop() {
		return removeFirst();
	}

private:
    /**
	 * Pointer to header node, initialized to a dummy node.  The first
	 * actual node is at head.getNext().
	 */
	sp<EQueueEntry> head;

	/** Pointer to last node on list **/
	sp<EQueueEntry> tail;// = head;

	LOCK lock;

	EAtomicCounter size_;
};

} /* namespace efc */
#endif /* ECONCURRENTINTRUSIVEDEQUE_HH_ */
