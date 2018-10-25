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
#include "../EDeque.hh"
#include "../ENoSuchElementException.hh"
#include "../ENullPointerException.hh"
#include "../EIllegalArgumentException.hh"
#include "../EUnsupportedOperationException.hh"

namespace efc {

/**
 * A helper class for intrusive queue.
 */

class EQueueEntry: virtual public EObject {
public:
	EQueueEntry() : queuePtr(null) {
	}

	ALWAYS_INLINE void setPrev(sp<EQueueEntry> e) { queue_entry_prev = e; }
	ALWAYS_INLINE sp<EQueueEntry> getPrev() { return queue_entry_prev; }

	ALWAYS_INLINE void setNext(sp<EQueueEntry> e) { queue_entry_next = e; }
	ALWAYS_INLINE sp<EQueueEntry> getNext() { return queue_entry_next; }

	ALWAYS_INLINE void setOwner(EObject* p) { queuePtr = p; }
	ALWAYS_INLINE EObject* getOwner() { return queuePtr; }

protected:
	sp<EQueueEntry> queue_entry_prev;
	sp<EQueueEntry> queue_entry_next;
	EObject* queuePtr; // for owner check.
};

//=============================================================================

/**
 * An unbounded thread-safe {@linkplain Deque queue} based on the object
 * is extends <em>EQueueEntry</em>.
 *
 */

template<typename E_, typename LOCK=EReentrantLock>
class EConcurrentIntrusiveDeque: public EDeque<sp<E_> > {
public:
	typedef sp<E_> E;

    virtual ~EConcurrentIntrusiveDeque() {
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
    boolean add(E e) {
		return offer(e);
	}

    /**
     * Inserts the specified element at the tail of this queue.
     *
     * @return <tt>true</tt> (as specified by {@link Queue#offer})
     * @throws NullPointerException if the specified element is null
     */
    boolean offer(E e) {
    	sp<EQueueEntry> entry = dynamic_pointer_cast<EQueueEntry>(e);
        if (!entry) {
            throw EIllegalArgumentException(__FILE__, __LINE__);
        }
        SYNCBLOCK(&lock) {
        	EObject* owner = entry->getOwner();
        	if (owner && (owner != (EObject*)this)) {
        		throw ERuntimeException(__FILE__, __LINE__);
        	}

			tail->setNext(entry);
			entry->setPrev(tail);
			entry->setNext(null);
            entry->setOwner((EObject*)this);
			tail = entry;
			size_++;
    	}}

		return true;
    }

    E poll() {
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
		return dynamic_pointer_cast<E_>(entry);
    }

    E element() {
    	E x = peek();
		if (x != null)
			return x;
		else
			throw ENoSuchElementException(__FILE__, __LINE__);
    }

    E peek() { // same as poll except don't remove item
    	sp<EQueueEntry> entry;
    	SYNCBLOCK(&lock) {
			entry = head->getNext();
    	}}
		return dynamic_pointer_cast<E_>(entry);
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
	boolean contains(E_* o) {
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
	boolean remove(E_* o) {
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
	E remove() {
		E x = poll();
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
	sp<EIterator<E> > iterator(int index=0) {
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
	EA<E> toArray() {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	// *** Deque methods ***

	void addFirst(E e) {
		(void)offerFirst(e);
	}

	void addLast(E e) {
		this->offer(e);
	}

	boolean offerFirst(E e) {
		sp<EQueueEntry> entry = dynamic_pointer_cast<EQueueEntry>(e);
        if (!entry) {
            throw EIllegalArgumentException(__FILE__, __LINE__);
        }
        SYNCBLOCK(&lock) {
        	EObject* owner = entry->getOwner();
			if (owner && (owner != (EObject*)this)) {
				throw ERuntimeException(__FILE__, __LINE__);
			}

			entry->setPrev(head);
			entry->setNext(head->getNext());
            entry->setOwner((EObject*)this);
			head->setNext(entry);
			size_++;
		}}

		return true;
	}

	boolean offerLast(E e) {
		return this->offer(e);
	}

	E removeFirst() {
		return pollFirst();
	}

	E removeLast() {
		return pollLast();
	}

	E pollFirst() {
		return poll();
	}

	E pollLast() {
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
		return dynamic_pointer_cast<E_>(entry);
	}

	E getFirst() {
		return peekFirst();
	}

	E getLast() {
		return peekLast();
	}

	E peekFirst() {
		return peek();
	}

	E peekLast() {
		sp<EQueueEntry> entry;
		SYNCBLOCK(&lock) {
			entry = tail->getPrev();
		}}
		return dynamic_pointer_cast<E_>(entry);
	}

	boolean removeFirstOccurrence(E_* o) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	boolean removeLastOccurrence(E_* o) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	// *** Stack methods ***

	void push(E_* e) {
		addFirst(e);
	}

	void push(E e) {
		addFirst(e);
	}

	E pop() {
		return removeFirst();
	}

	virtual sp<EIterator<E> > descendingIterator() {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 */
	virtual boolean containsAll(ECollection<E> *c) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 */
	virtual boolean removeAll(ECollection<E> *c) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 */
	virtual boolean retainAll(ECollection<E> *c) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
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
