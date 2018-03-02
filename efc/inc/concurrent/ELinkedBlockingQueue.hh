/*
 * ELinkedBlockingQueue.hh
 *
 *  Created on: 2014-6-27
 *      Author: cxxjava@163.com
 */

#ifndef ELINKEDBLOCKINGQUEUE_HH_
#define ELINKEDBLOCKINGQUEUE_HH_

#include "../EInteger.hh"
#include "../ETimeUnit.hh"
#include "./EBlockingQueue.hh"
#include "./EAtomicInteger.hh"
#include "../EAbstractQueue.hh"
#include "../EInterruptedException.hh"
#include "../EIllegalArgumentException.hh"
#include "../ENoSuchElementException.hh"

namespace efc {

/**
 * An optionally-bounded {@linkplain BlockingQueue blocking queue} based on
 * linked nodes.
 * This queue orders elements FIFO (first-in-first-out).
 * The <em>head</em> of the queue is that element that has been on the
 * queue the longest time.
 * The <em>tail</em> of the queue is that element that has been on the
 * queue the shortest time. New elements
 * are inserted at the tail of the queue, and the queue retrieval
 * operations obtain elements at the head of the queue.
 * Linked queues typically have higher throughput than array-based queues but
 * less predictable performance in most concurrent applications.
 *
 * <p> The optional capacity bound constructor argument serves as a
 * way to prevent excessive queue expansion. The capacity, if unspecified,
 * is equal to {@link Integer#MAX_VALUE}.  Linked nodes are
 * dynamically created upon each insertion unless this would bring the
 * queue above capacity.
 *
 * <p>This class and its iterator implement all of the
 * <em>optional</em> methods of the {@link Collection} and {@link
 * Iterator} interfaces.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since 1.5
 * @param <E> the type of elements held in this collection
 *
 */

template<typename E>
class ELinkedBlockingQueue: virtual public EAbstractQueue<sp<E> >,
		virtual public EBlockingQueue<E> {
public:
	/*
	 * A variant of the "two lock queue" algorithm.  The putLock gates
	 * entry to put (and offer), and has an associated condition for
	 * waiting puts.  Similarly for the takeLock.  The "count" field
	 * that they both rely on is maintained as an atomic to avoid
	 * needing to get both locks in most cases. Also, to minimize need
	 * for puts to get takeLock and vice-versa, cascading notifies are
	 * used. When a put notices that it has enabled at least one take,
	 * it signals taker. That taker in turn signals others if more
	 * items have been entered since the signal. And symmetrically for
	 * takes signalling puts. Operations such as remove(Object) and
	 * iterators acquire both locks.
	 *
     * Visibility between writers and readers is provided as follows:
     *
     * Whenever an element is enqueued, the putLock is acquired and
     * count updated.  A subsequent reader guarantees visibility to the
     * enqueued Node by either acquiring the putLock (via fullyLock)
     * or by acquiring the takeLock, and then reading n = count.get();
     * this gives visibility to the first n items.
     *
     * To implement weakly consistent iterators, it appears we need to
     * keep all Nodes GC-reachable from a predecessor dequeued Node.
     * That would cause two problems:
     * - allow a rogue Iterator to cause unbounded memory retention
     * - cause cross-generational linking of old Nodes to new Nodes if
     *   a Node was tenured while live, which generational GCs have a
     *   hard time dealing with, causing repeated major collections.
     * However, only non-deleted Nodes need to be reachable from
     * dequeued Nodes, and reachability does not necessarily have to
     * be of the kind understood by the GC.  We use the trick of
     * linking a Node that has just been dequeued to itself.  Such a
     * self-link implicitly means to advance to head.next.
	 */

	virtual ~ELinkedBlockingQueue() {
		// node unlink for recursion
		sp<Node> node = head;
		while (node != null) {
			sp<Node> next = node->next;
			node->next = null;
			node = next;
		}

		delete notEmpty;
		delete notFull;
	}

	/**
	 * Creates a <tt>LinkedBlockingQueue</tt> with a capacity of
	 * {@link Integer#MAX_VALUE}.
	 */
	ELinkedBlockingQueue() {
		init(EInteger::MAX_VALUE);
	}

	/**
	 * Creates a <tt>LinkedBlockingQueue</tt> with the given (fixed) capacity.
	 *
	 * @param capacity the capacity of this queue
	 * @throws IllegalArgumentException if <tt>capacity</tt> is not greater
	 *         than zero
	 */
	ELinkedBlockingQueue(int capacity) {
		init(capacity);
	}

	// this doc comment is overridden to remove the reference to collections
	// greater in size than Integer.MAX_VALUE
	/**
	 * Returns the number of elements in this queue.
	 *
	 * @return the number of elements in this queue
	 */
	virtual int size() {
		return count.get();
	}

	// this doc comment is a modified copy of the inherited doc comment,
	// without the reference to unlimited queues.
	/**
	 * Returns the number of additional elements that this queue can ideally
	 * (in the absence of memory or resource constraints) accept without
	 * blocking. This is always equal to the initial capacity of this queue
	 * less the current <tt>size</tt> of this queue.
	 *
	 * <p>Note that you <em>cannot</em> always tell if an attempt to insert
	 * an element will succeed by inspecting <tt>remainingCapacity</tt>
	 * because it may be the case that another thread is about to
	 * insert or remove an element.
	 */
	virtual int remainingCapacity() {
		return capacity_ - count.get();
	}

	virtual int capacity() {
		return capacity_;
	}

	/**
	 * Inserts the specified element at the tail of this queue, waiting if
	 * necessary for space to become available.
	 *
	 * @throws InterruptedException {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual void put(sp<E> e) THROWS(EInterruptedException) {
		if (e == null) throw ENullPointerException(__FILE__, __LINE__);
		// Note: convention in all put/take/etc is to preset
		// local var holding count  negative to indicate failure unless set.
		int c = -1;
		sp<Node> node = new Node(e);
		putLock.lockInterruptibly();
		try {
			/*
			 * Note that count is used in wait guard even though it is
			 * not protected by lock. This works because count can
			 * only decrease at this point (all other puts are shut
			 * out by lock), and we (or some other waiting put) are
			 * signalled if it ever changes from capacity. Similarly
			 * for all other uses of count in other wait guards.
			 */
			while (count.get() == capacity_) {
				notFull->await();
			}
			enqueue(node);
			c = count.getAndIncrement();
			if (c + 1 < capacity_)
				notFull->signal();
		} catch(...) {
			putLock.unlock();
			throw; //!
		} finally {
			putLock.unlock();
		}
		if (c == 0)
			signalNotEmpty();
	}

	/**
	 * Inserts the specified element at the tail of this queue, waiting if
	 * necessary up to the specified wait time for space to become available.
	 *
	 * @return <tt>true</tt> if successful, or <tt>false</tt> if
	 *         the specified waiting time elapses before space is available.
	 * @throws InterruptedException {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean offer(sp<E> e, llong timeout, ETimeUnit* unit)
			THROWS(EInterruptedException) {
		if (e == null) throw ENullPointerException(__FILE__, __LINE__);
		llong nanos = unit->toNanos(timeout);
		int c = -1;
		putLock.lockInterruptibly();
		boolean r = true;
		try {
			while (count.get() == capacity_) {
				if (nanos <= 0) {
					r = false;
					goto FINALLY;
				}
				nanos = notFull->awaitNanos(nanos);
			}
			sp<Node> x(new Node(e));
			enqueue(x);
			c = count.getAndIncrement();
			if (c + 1 < capacity_)
				notFull->signal();
		} catch(...) {
			putLock.unlock();
			throw; //!
		}
		FINALLY:
		finally {
			putLock.unlock();
		}
		if (c == 0)
			signalNotEmpty();
		return r;
	}

	/**
	 * Inserts the specified element at the tail of this queue if it is
	 * possible to do so immediately without exceeding the queue's capacity,
	 * returning <tt>true</tt> upon success and <tt>false</tt> if this queue
	 * is full.
	 * When using a capacity-restricted queue, this method is generally
	 * preferable to method {@link BlockingQueue#add add}, which can fail to
	 * insert an element only by throwing an exception.
	 *
	 * @throws NullPointerException if the specified element is null
	 */
	virtual boolean offer(sp<E> e) {
		if (e == null) throw ENullPointerException(__FILE__, __LINE__);
		if (count.get() == capacity_)
			return false;
		int c = -1;
		sp<Node> node(new Node(e));
		putLock.lock();
		try {
			if (count.get() < capacity_) {
				enqueue(node);
				c = count.getAndIncrement();
				if (c + 1 < capacity_)
					notFull->signal();
			}
		} catch(...) {
			putLock.unlock();
			throw; //!
		} finally {
			putLock.unlock();
		}
		if (c == 0)
			signalNotEmpty();
		return c >= 0;
	}

	virtual sp<E> take() THROWS(EInterruptedException) {
		sp<E> x;
		int c = -1;
		takeLock.lockInterruptibly();
		try {
			while (count.get() == 0) {
				notEmpty->await();
			}
			x = dequeue();
			c = count.getAndDecrement();
			if (c > 1)
				notEmpty->signal();
		} catch(...) {
			takeLock.unlock();
			throw; //!
		} finally {
			takeLock.unlock();
		}
		if (c == capacity_)
			signalNotFull();
		return x;
	}

	virtual sp<E> poll(llong timeout, ETimeUnit* unit) THROWS(EInterruptedException) {
		sp<E> x;
		int c = -1;
		llong nanos = unit->toNanos(timeout);
		takeLock.lockInterruptibly();
		try {
			while (count.get() == 0) {
				if (nanos <= 0) {
					x = null;
					goto FINALLY;
				}
				nanos = notEmpty->awaitNanos(nanos);
			}
			x = dequeue();
			c = count.getAndDecrement();
			if (c > 1)
				notEmpty->signal();
		} catch(...) {
			takeLock.unlock();
			throw; //!
		}
		FINALLY:
		finally {
			takeLock.unlock();
		}
		if (c == capacity_)
			signalNotFull();
		return x;
	}

	virtual sp<E> poll() {
		if (count.get() == 0)
			return null;
		sp<E> x;
		int c = -1;
		takeLock.lock();
		try {
			if (count.get() > 0) {
				x = dequeue();
				c = count.getAndDecrement();
				if (c > 1)
					notEmpty->signal();
			}
		} catch(...) {
			takeLock.unlock();
			throw; //!
		} finally {
			takeLock.unlock();
		}
		if (c == capacity_)
			signalNotFull();
		return x;
	}

	virtual sp<E> peek() {
		if (count.get() == 0)
			return null;
		takeLock.lock();
		sp<E> p;
		try {
			sp<Node> first = head->next;
			if (first == null)
				p = null;
			else
				p = first->item;
		} catch(...) {
			takeLock.unlock();
			throw; //!
		} finally {
			takeLock.unlock();
		}
		return p;
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
	virtual boolean remove(E* o) {
		if (o == null) return false;
		boolean removed = false;
		fullyLock();
		try {
			for (sp<Node> trail = head, p = trail->next;
				 p != null;
				 trail = p, p = p->next) {
				if (o->equals(p->item.get())) {
					unlink(p, trail);
					removed = true;
				}
			}
		} catch(...) {
			fullyUnlock();
			throw; //!
		} finally {
			fullyUnlock();
		}
		return removed;
	}

	/**
	 * Atomically removes all of the elements from this queue.
	 * The queue will be empty after this call returns.
	 */
	virtual void clear() {
		fullyLock();
		try {
			for (sp<Node> p, h = head; (p = h->next) != null; h = p) {
				h->next = null; //!! h->next = h;
				p->item = null;
			}
			head = last;
			// assert head.item == null && head.next == null;
			if (count.getAndSet(0) == capacity_)
				notFull->signal();
		} catch(...) {
			fullyUnlock();
			throw; //!
		} finally {
			fullyUnlock();
		}
	}

	/**
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	virtual int drainTo(ECollection<sp<E> >* c) {
		return drainTo(c, EInteger::MAX_VALUE);
	}

	/**
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	virtual int drainTo(ECollection<sp<E> >* c, int maxElements) {
		if (c == null)
			throw ENullPointerException(__FILE__, __LINE__);
		if (c == dynamic_cast<ECollection<sp<E> >*>(this))
			throw EIllegalArgumentException(__FILE__, __LINE__);
		boolean signalNotFull_ = false;
		takeLock.lock(); //lock!
		int n = ES_MIN(maxElements, count.get());
		// count.get provides visibility to first n Nodes
		sp<Node> h = head;
		int i = 0;
		try {
			while (i < n) {
				sp<Node> p = h->next;
				c->add(p->item);
				p->item = null;
				h->next = null; //!! h->next = h;
				h = p;
				++i;
			}
		} catch(...) {
			if (i > 0) {
				head = h;
				signalNotFull_ = (count.getAndAdd(-i) == capacity_);
			}
			takeLock.unlock(); //unlock!
			if (signalNotFull_)
				signalNotFull();
			throw; //!
		} finally {
			// Restore invariants even if c.add() threw
			if (i > 0) {
				// assert h.item == null;
				head = h;
				signalNotFull_ = (count.getAndAdd(-i) == capacity_);
			}
			takeLock.unlock(); //unlock!
			if (signalNotFull_)
				signalNotFull();
		}
		return n;
	}

	//extends AbstractConcurrentQueue

	/**
	 * Inserts the specified element into this queue if it is possible to do so
	 * immediately without violating capacity restrictions, returning
	 * <tt>true</tt> upon success and throwing an <tt>IllegalStateException</tt>
	 * if no space is currently available.
	 *
	 * <p>This implementation returns <tt>true</tt> if <tt>offer</tt> succeeds,
	 * else throws an <tt>IllegalStateException</tt>.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 * @throws IllegalStateException if the element cannot be added at this
	 *         time due to capacity restrictions
	 * @throws ClassCastException if the class of the specified element
	 *         prevents it from being added to this queue
	 * @throws NullPointerException if the specified element is null and
	 *         this queue does not permit null elements
	 * @throws IllegalArgumentException if some property of this element
	 *         prevents it from being added to this queue
	 */
	virtual boolean add(sp<E> e) {
		return EAbstractQueue<sp<E> >::add(e);
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
	virtual sp<E> remove() {
		return EAbstractQueue<sp<E> >::remove();
	}

	/**
	 * Retrieves, but does not remove, the head of this queue.  This method
	 * differs from {@link #peek peek} only in that it throws an exception if
	 * this queue is empty.
	 *
	 * <p>This implementation returns the result of <tt>peek</tt>
	 * unless the queue is empty.
	 *
	 * @return the head of this queue
	 * @throws NoSuchElementException if this queue is empty
	 */
	virtual sp<E> element() {
		return EAbstractQueue<sp<E> >::element();
	}

	/**
	 * Returns <tt>true</tt> if this collection contains no elements.
	 *
	 * @return <tt>true</tt> if this collection contains no elements
	 */
	virtual boolean isEmpty() {
		return EAbstractCollection<sp<E> >::isEmpty();
	}

	/**
	 * Returns {@code true} if this queue contains the specified element.
	 * More formally, returns {@code true} if and only if this queue contains
	 * at least one element {@code e} such that {@code o.equals(e)}.
	 *
	 * @param o object to be checked for containment in this queue
	 * @return {@code true} if this queue contains the specified element
	 */
	virtual boolean contains(E* o) {
		if (o == null) return false;
		boolean r = false;
		fullyLock();
		try {
			for (sp<Node> p = head->next; p != null; p = p->next)
				if (o->equals(p->item.get()))
					r = true;
		} catch(...) {
			fullyUnlock();
			throw; //!
		} finally {
			fullyUnlock();
		}
		return r;
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
	virtual EA<sp<E> > toArray() {
		fullyLock();
		int size = count.get();
		EA<sp<E> > a(size);
		try {
			int k = 0;
			for (sp<Node> p = head->next; p != null; p = p->next)
				a[k++] = p->item;
		} catch (...) {
			fullyUnlock();
			throw; //!
		} finally {
			fullyUnlock();
		}
		return a;
	}

	/**
	 * Returns an iterator over the elements in this collection.  There are no
	 * guarantees concerning the order in which the elements are returned
	 * (unless this collection is an instance of some class that provides a
	 * guarantee).
	 *
	 * @return an <tt>Iterator</tt> over the elements in this collection
	 */
	virtual sp<EIterator<sp<E> > > iterator(int index=0) {
		return new Itr(this);
	}

protected:

	/**
	 * Linked list node class
	 */
	class Node {
	public:
		sp<E> item;

		/**
		 * One of:
		 * - the real successor Node
		 * - this Node, meaning the successor is head.next
		 * - null, meaning there is no successor (this is the last node)
		 */
		sp<Node> next;
		Node(){};
		Node(sp<E> x) { item = x; }
	};

	/**
	 * Unlinks interior Node p with predecessor trail.
	 */
	void unlink(sp<Node>& p, sp<Node>& trail) {
		// assert isFullyLocked();
		// p.next is not changed, to allow iterators that are
		// traversing p to maintain their weak-consistency guarantee.
		p->item = null;
		trail->next = p->next;
		if (last == p)
			last = trail;
		if (count.getAndDecrement() == capacity_)
			notFull->signal();
	}

private:
	/** The capacity bound, or Integer.MAX_VALUE if none */
	int capacity_;

	/** Current number of elements */
	EAtomicInteger count;

	/**
	 * Head of linked list.
	 * Invariant: head.item == null
	 */
	sp<Node> head;

	/**
	 * Tail of linked list.
	 * Invariant: last.next == null
	 */
	sp<Node> last;

	/** Lock held by take, poll, etc */
	EReentrantLock takeLock;

	/** Wait queue for waiting takes */
	ECondition* notEmpty;// = takeLock.newCondition();

	/** Lock held by put, offer, etc */
	EReentrantLock putLock;

	/** Wait queue for waiting puts */
	ECondition* notFull;// = putLock.newCondition();

	void init(int capacity) {
		if (capacity <= 0) throw EIllegalArgumentException(__FILE__, __LINE__);
		this->capacity_ = capacity;
		last = head = new Node();

		notEmpty = takeLock.newCondition();
		notFull = putLock.newCondition();
	}

	/**
	 * Signals a waiting take. Called only from put/offer (which do not
	 * otherwise ordinarily lock takeLock.)
	 */
	void signalNotEmpty() {
		SYNCBLOCK(&takeLock) {
			notEmpty->signal();
        }}
	}

	/**
	 * Signals a waiting put. Called only from take/poll.
	 */
	void signalNotFull() {
		SYNCBLOCK(&putLock) {
			notFull->signal();
        }}
	}

	/**
     * Links node at end of queue.
     *
     * @param node the node
     */
    void enqueue(sp<Node>& node) {
        // assert putLock.isHeldByCurrentThread();
        // assert last.next == null;
        last = last->next = node;
    }

    /**
     * Removes a node from head of queue.
     *
     * @return the node
     */
    sp<E> dequeue() {
        // assert takeLock.isHeldByCurrentThread();
        // assert head.item == null;
    	sp<Node> h = head;
        sp<Node> first = h->next;
        h->next = null; //!! h->next = h; // help GC
        head = first;
        sp<E> x = first->item;
        first->item = null;
        return x;
    }

	/**
	 * Lock to prevent both puts and takes.
	 */
	void fullyLock() {
		putLock.lock();
		takeLock.lock();
	}

	/**
	 * Unlock to allow both puts and takes.
	 */
	void fullyUnlock() {
		takeLock.unlock();
		putLock.unlock();
	}

	class Itr : public EIterator<sp<E> > {
	private:
		ELinkedBlockingQueue<E>* self;

		/*
		 * Basic weakly-consistent iterator.  At all times hold the next
		 * item to hand out so that if hasNext() reports true, we will
		 * still have it to return even if lost race with a take etc.
		 */
		sp<Node> current;
		sp<Node> lastRet;
		sp<E> currentElement;


		/**
		 * Returns the next live successor of p, or null if no such.
		 *
		 * Unlike other traversal methods, iterators need to handle both:
		 * - dequeued nodes (p.next == p)
		 * - (possibly multiple) interior removed nodes (p.item == null)
		 */
		sp<Node> nextNode(sp<Node> p) {
			for (;;) {
				sp<Node> s = p->next;
				if (s == null && p != self->last) //!! if (s == p)
					return self->head->next;
				if (s == null || s->item != null)
					return s;
				p = s;
			}
			//not reach here!
			return null;
		}

	public:
		Itr(ELinkedBlockingQueue<E>* s) : self(s) {
			self->fullyLock();
			current = self->head->next;
			if (current != null)
				currentElement = current->item;
			self->fullyUnlock();
		}

		virtual boolean hasNext() {
			return current != null;
		}

		virtual sp<E> next() {
			sp<E> x;
			self->fullyLock();
			try {
				if (current == null)
					throw ENoSuchElementException(__FILE__, __LINE__);
				x = currentElement;
				lastRet = current;
				current = nextNode(current);
				currentElement = (current == null) ? null : current->item;
			} catch(...) {
				self->fullyUnlock();
				throw; //!
			} finally {
				self->fullyUnlock();
			}
			return x;
		}

		virtual void remove() {
			if (lastRet == null)
				throw EIllegalStateException(__FILE__, __LINE__);
			self->fullyLock();
			try {
				sp<Node> node = lastRet;
				lastRet = null;
				for (sp<Node> trail = self->head, p = trail->next;
					 p != null;
					 trail = p, p = p->next) {
					if (p == node) {
						self->unlink(p, trail);
						break;
					}
				}
			} catch(...) {
				self->fullyUnlock();
				throw; //!
			} finally {
				self->fullyUnlock();
			}
		}
        virtual sp<E> moveOut() {
            throw EUnsupportedOperationException(__FILE__, __LINE__);
        }
	};
};

} /* namespace efc */
#endif /* ELINKEDBLOCKINGQUEUE_HH_ */
