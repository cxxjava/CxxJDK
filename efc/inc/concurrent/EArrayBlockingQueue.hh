/*
 * EArrayBlockingQueue.hh
 *
 *  Created on: 2018-1-1
 *      Author: cxxjava@163.com
 */

#ifndef EArrayBlockingQueue_HH_
#define EArrayBlockingQueue_HH_

#include "../EAbstractQueue.hh"
#include "../EA.hh"
#include "../EIllegalArgumentException.hh"
#include "../EArrayIndexOutOfBoundsException.hh"

namespace efc {

/**
 * A bounded {@linkplain BlockingQueue blocking queue} backed by an
 * array.  This queue orders elements FIFO (first-in-first-out).  The
 * <em>head</em> of the queue is that element that has been on the
 * queue the longest time.  The <em>tail</em> of the queue is that
 * element that has been on the queue the shortest time. New elements
 * are inserted at the tail of the queue, and the queue retrieval
 * operations obtain elements at the head of the queue.
 *
 * <p>This is a classic &quot;bounded buffer&quot;, in which a
 * fixed-sized array holds elements inserted by producers and
 * extracted by consumers.  Once created, the capacity cannot be
 * changed.  Attempts to {@code put} an element into a full queue
 * will result in the operation blocking; attempts to {@code take} an
 * element from an empty queue will similarly block.
 *
 * <p>This class supports an optional fairness policy for ordering
 * waiting producer and consumer threads.  By default, this ordering
 * is not guaranteed. However, a queue constructed with fairness set
 * to {@code true} grants threads access in FIFO order. Fairness
 * generally decreases throughput but reduces variability and avoids
 * starvation.
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
 */

template<typename E>
class EArrayBlockingQueue: virtual public EAbstractQueue<sp<E> >,
		virtual public EBlockingQueue<E> {
public:
	virtual ~EArrayBlockingQueue() {
		delete notEmpty;
		delete notFull;
	}

	/**
	 * Creates an {@code ArrayBlockingQueue} with the given (fixed)
	 * capacity and default access policy.
	 *
	 * @param capacity the capacity of this queue
	 * @throws IllegalArgumentException if {@code capacity < 1}
	 */
	EArrayBlockingQueue(int capacity) : items(capacity),
			takeIndex(0), putIndex(0), count(0), lock(false) {
		if (capacity <= 0)
			throw EIllegalArgumentException(__FILE__, __LINE__);
		notEmpty = lock.newCondition();
		notFull =  lock.newCondition();
	}

	/**
	 * Creates an {@code ArrayBlockingQueue} with the given (fixed)
	 * capacity and the specified access policy.
	 *
	 * @param capacity the capacity of this queue
	 * @param fair if {@code true} then queue accesses for threads blocked
	 *        on insertion or removal, are processed in FIFO order;
	 *        if {@code false} the access order is unspecified.
	 * @throws IllegalArgumentException if {@code capacity < 1}
	 */
	EArrayBlockingQueue(int capacity, boolean fair) : items(capacity),
			takeIndex(0), putIndex(0), count(0), lock(fair) {
		if (capacity <= 0)
			throw EIllegalArgumentException(__FILE__, __LINE__);
		notEmpty = lock.newCondition();
		notFull =  lock.newCondition();
	}

	/**
	 * Creates an {@code ArrayBlockingQueue} with the given (fixed)
	 * capacity, the specified access policy and initially containing the
	 * elements of the given collection,
	 * added in traversal order of the collection's iterator.
	 *
	 * @param capacity the capacity of this queue
	 * @param fair if {@code true} then queue accesses for threads blocked
	 *        on insertion or removal, are processed in FIFO order;
	 *        if {@code false} the access order is unspecified.
	 * @param c the collection of elements to initially contain
	 * @throws IllegalArgumentException if {@code capacity} is less than
	 *         {@code c.size()}, or less than 1.
	 * @throws NullPointerException if the specified collection or any
	 *         of its elements are null
	 */
	EArrayBlockingQueue(int capacity, boolean fair,
						  ECollection<sp<E> >* c) : items(capacity),
			takeIndex(0), putIndex(0), count(0), lock(fair) {
		if (capacity <= 0)
			throw EIllegalArgumentException(__FILE__, __LINE__);
		notEmpty = lock.newCondition();
		notFull =  lock.newCondition();

		SYNCBLOCK(&lock) { // Lock only for visibility, not mutual exclusion
			int i = 0;
			try {
				sp<EIterator<sp<E> > > iter = c->iterator();
				while (iter->hasNext()) {
					sp<E> e = iter->next();
					checkNotNull(e);
					items[i++] = e;
				}
			} catch (EArrayIndexOutOfBoundsException& ex) {
				throw EIllegalArgumentException(__FILE__, __LINE__);
			}
			count = i;
			putIndex = (i == capacity) ? 0 : i;
		}}
	}

	/**
	 * Inserts the specified element at the tail of this queue if it is
	 * possible to do so immediately without exceeding the queue's capacity,
	 * returning {@code true} upon success and throwing an
	 * {@code IllegalStateException} if this queue is full.
	 *
	 * @param e the element to add
	 * @return {@code true} (as specified by {@link Collection#add})
	 * @throws IllegalStateException if this queue is full
	 * @throws NullPointerException if the specified element is null
	 */
	virtual boolean add(sp<E> e) {
		return EAbstractQueue<sp<E> >::add(e);
	}

	/**
	 * Inserts the specified element at the tail of this queue if it is
	 * possible to do so immediately without exceeding the queue's capacity,
	 * returning {@code true} upon success and {@code false} if this queue
	 * is full.  This method is generally preferable to method {@link #add},
	 * which can fail to insert an element only by throwing an exception.
	 *
	 * @throws NullPointerException if the specified element is null
	 */
	virtual boolean offer(sp<E> e) {
		checkNotNull(e);
		SYNCBLOCK(&lock) {
			if (count == items.length())
				return false;
			else {
				enqueue(e);
				return true;
			}
		}}
	}

	/**
	 * Inserts the specified element at the tail of this queue, waiting
	 * for space to become available if the queue is full.
	 *
	 * @throws InterruptedException {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual void put(sp<E> e) THROWS(EInterruptedException) {
		checkNotNull(e);
		lock.lockInterruptibly();
		try {
			while (count == items.length())
				notFull->await();
			enqueue(e);
		} catch (...) {
			lock.unlock();
			throw; //!
		} finally {
			lock.unlock();
		}
	}

	/**
	 * Inserts the specified element at the tail of this queue, waiting
	 * up to the specified wait time for space to become available if
	 * the queue is full.
	 *
	 * @throws InterruptedException {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean offer(sp<E> e, llong timeout, ETimeUnit* unit)
			THROWS(EInterruptedException) {
		checkNotNull(e);
		llong nanos = unit->toNanos(timeout);
		boolean rv = true;
		lock.lockInterruptibly();
		try {
			while (count == items.length()) {
				if (nanos <= 0) {
					rv = false;
					goto FINALLY;
				}
				nanos = notFull->awaitNanos(nanos);
			}
			enqueue(e);
		} catch (...) {
			lock.unlock();
			throw; //!
		}
		FINALLY:
		finally {
			lock.unlock();
		}
		return rv;
	}

	virtual sp<E> poll() {
		SYNCBLOCK(&lock) {
			return (count == 0) ? null : dequeue();
		}}
	}

	virtual sp<E> take() THROWS(EInterruptedException) {
		sp<E> e;
		lock.lockInterruptibly();
		try {
			while (count == 0)
				notEmpty->await();
			e = dequeue();
		} catch (...) {
			lock.unlock();
			throw; //!
		} finally {
			lock.unlock();
		}
		return e;
	}

	virtual sp<E> poll(llong timeout, ETimeUnit* unit) THROWS(EInterruptedException) {
		llong nanos = unit->toNanos(timeout);
		sp<E> e;
		lock.lockInterruptibly();
		try {
			while (count == 0) {
				if (nanos <= 0)
					goto FAINALLY;
				nanos = notEmpty->awaitNanos(nanos);
			}
			e = dequeue();
		} catch (...) {
			lock.unlock();
			throw; //!
		}
		FAINALLY:
		finally {
			lock.unlock();
		}
		return e;
	}

	virtual sp<E> peek() {
		SYNCBLOCK(&lock) {
			return itemAt(takeIndex); // null when queue is empty
		}}
	}

	// this doc comment is overridden to remove the reference to collections
	// greater in size than Integer.MAX_VALUE
	/**
	 * Returns the number of elements in this queue.
	 *
	 * @return the number of elements in this queue
	 */
	virtual int size() {
		SYNCBLOCK(&lock) {
			return count;
		}}
	}

	// this doc comment is a modified copy of the inherited doc comment,
	// without the reference to unlimited queues.
	/**
	 * Returns the number of additional elements that this queue can ideally
	 * (in the absence of memory or resource constraints) accept without
	 * blocking. This is always equal to the initial capacity of this queue
	 * less the current {@code size} of this queue.
	 *
	 * <p>Note that you <em>cannot</em> always tell if an attempt to insert
	 * an element will succeed by inspecting {@code remainingCapacity}
	 * because it may be the case that another thread is about to
	 * insert or remove an element.
	 */
	virtual int remainingCapacity() {
		SYNCBLOCK(&lock) {
			return items.length() - count;
		}}
	}

	/**
	 * Removes a single instance of the specified element from this queue,
	 * if it is present.  More formally, removes an element {@code e} such
	 * that {@code o.equals(e)}, if this queue contains one or more such
	 * elements.
	 * Returns {@code true} if this queue contained the specified element
	 * (or equivalently, if this queue changed as a result of the call).
	 *
	 * <p>Removal of interior elements in circular array based queues
	 * is an intrinsically slow and disruptive operation, so should
	 * be undertaken only in exceptional circumstances, ideally
	 * only when the queue is known not to be accessible by other
	 * threads.
	 *
	 * @param o element to be removed from this queue, if present
	 * @return {@code true} if this queue changed as a result of the call
	 */
	virtual boolean remove(E* o) {
		if (o == null) return false;
		EA<sp<E> >& items = this->items;
		EReentrantLock& lock = this->lock;
		SYNCBLOCK(&lock) {
			if (count > 0) {
				int putIndex = this->putIndex;
				int i = takeIndex;
				do {
					if (o->equals(items[i].get())) {
						removeAt(i);
						return true;
					}
					if (++i == items.length())
						i = 0;
				} while (i != putIndex);
			}
			return false;
		}}
	}

	/**
	 * {@inherit from super for c++ hides overloaded virtual function}
	 */
	using EAbstractQueue<sp<E> >::remove;

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
		EA<sp<E> >& items = this->items;
		EReentrantLock& lock = this->lock;
		SYNCBLOCK(&lock) {
			if (count > 0) {
				int putIndex = this->putIndex;
				int i = takeIndex;
				do {
					if (o->equals(items[i].get()))
						return true;
					if (++i == items.length())
						i = 0;
				} while (i != putIndex);
			}
			return false;
		}}
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
		EReentrantLock& lock = this->lock;
		SYNCBLOCK(&lock) {
			int count = this->count;
			EA<sp<E> > a(count);
			int n = items.length() - takeIndex;
			if (count <= n)
				ESystem::arraycopy(items, takeIndex, a, 0, count);
			else {
				ESystem::arraycopy(items, takeIndex, a, 0, n);
				ESystem::arraycopy(items, 0, a, n, count - n);
			}
		}}
		return EA<sp<E> >(0);
	}

	virtual EString toString() {
		EReentrantLock& lock = this->lock;
		SYNCBLOCK(&lock) {
			int k = count;
			if (k == 0)
				return "[]";

			EA<sp<E> >& items = this->items;
			EString sb;
			sb.append('[');
			for (int i = takeIndex; ; ) {
				sp<E> e = items[i];
				sb.append(e.get() == dynamic_cast<E*>(this) ? "(this Collection)" : e->toString());
				if (--k == 0)
					return sb.append(']');
				sb.append(',').append(' ');
				if (++i == items.length())
					i = 0;
			}
		}}
		//not reach here.
		return "?";
	}

	/**
	 * Atomically removes all of the elements from this queue.
	 * The queue will be empty after this call returns.
	 */
	virtual void clear() {
		EA<sp<E> >& items = this->items;
		EReentrantLock& lock = this->lock;
		SYNCBLOCK(&lock) {
			int k = count;
			if (k > 0) {
				int putIndex = this->putIndex;
				int i = takeIndex;
				do {
					items[i] = null;
					if (++i == items.length())
						i = 0;
				} while (i != putIndex);
				takeIndex = putIndex;
				count = 0;
				if (itrs != null)
					itrs->queueIsEmpty();
				for (; k > 0 && lock.hasWaiters(notFull); k--)
					notFull->signal();
			}
		}}
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
		checkNotNull(c);
		if (c == dynamic_cast<ECollection<sp<E> >*>(this))
			throw EIllegalArgumentException(__FILE__, __LINE__);
		if (maxElements <= 0)
			return 0;
		EA<sp<E> >& items = this->items;
		EReentrantLock& lock = this->lock;
		SYNCBLOCK(&lock) {
			int n = EMath::min(maxElements, count);
			int take = takeIndex;
			int i = 0;
			try {
				while (i < n) {
					sp<E> x = items[take];
					c->add(x);
					items[take] = null;
					if (++take == items.length())
						take = 0;
					i++;
				}
				return n;
			} catch (...) {
				finally {
					// Restore invariants even if c.add() threw
					if (i > 0) {
						count -= i;
						takeIndex = take;
						if (itrs != null) {
							if (count == 0)
								itrs->queueIsEmpty();
							else if (i > take)
								itrs->takeIndexWrapped();
						}
						for (; i > 0 && lock.hasWaiters(notFull); i--)
							notFull->signal();
					}
				}
				throw; //!
			} finally {
				// Restore invariants even if c.add() threw
				if (i > 0) {
					count -= i;
					takeIndex = take;
					if (itrs != null) {
						if (count == 0)
							itrs->queueIsEmpty();
						else if (i > take)
							itrs->takeIndexWrapped();
					}
					for (; i > 0 && lock.hasWaiters(notFull); i--)
						notFull->signal();
				}
			}
		}}
	}

	/**
	 * Returns an iterator over the elements in this queue in proper sequence.
	 * The elements will be returned in order from first (head) to last (tail).
	 *
	 * <p>The returned iterator is
	 * <a href="package-summary.html#Weakly"><i>weakly consistent</i></a>.
	 *
	 * @return an iterator over the elements in this queue in proper sequence
	 */
	sp<EIterator<sp<E> > > iterator(int index=0) {
		sp<Itr> it = new Itr(this);
		it->init(); //!!!
		return it;
	}

private:
	/** Condition for waiting takes */
	ECondition* notEmpty;

	/** Condition for waiting puts */
	ECondition* notFull;

	/**
	 * Throws NullPointerException if argument is null.
	 *
	 * @param v the element
	 */
	static void checkNotNull(EObject* v) {
		if (v == null)
			throw ENullPointerException(__FILE__, __LINE__);
	}
	static void checkNotNull(sp<EObject> v) {
		if (v == null)
			throw ENullPointerException(__FILE__, __LINE__);
	}

	/**
	 * Inserts element at current put position, advances, and signals.
	 * Call only when holding lock.
	 */
	void enqueue(sp<E> x) {
		// assert lock.getHoldCount() == 1;
		// assert items[putIndex] == null;
		EA<sp<E> >& items = this->items;
		items[putIndex] = x;
		if (++putIndex == items.length())
			putIndex = 0;
		count++;
		notEmpty->signal();
	}

	/**
	 * Extracts element at current take position, advances, and signals.
	 * Call only when holding lock.
	 */
	sp<E> dequeue() {
		// assert lock.getHoldCount() == 1;
		// assert items[takeIndex] != null;
		EA<sp<E> >& items = this->items;
		sp<E> x = items[takeIndex];
		items[takeIndex] = null;
		if (++takeIndex == items.length())
			takeIndex = 0;
		count--;
		if (itrs != null)
			itrs->elementDequeued();
		notFull->signal();
		return x;
	}

	/**
	 * Iterator for ArrayBlockingQueue.
	 *
	 * To maintain weak consistency with respect to puts and takes, we
	 * read ahead one slot, so as to not report hasNext true but then
	 * not have an element to return.
	 *
	 * We switch into "detached" mode (allowing prompt unlinking from
	 * itrs without help from the GC) when all indices are negative, or
	 * when hasNext returns false for the first time.  This allows the
	 * iterator to track concurrent updates completely accurately,
	 * except for the corner case of the user calling Iterator.remove()
	 * after hasNext() returned false.  Even in this case, we ensure
	 * that we don't remove the wrong element by keeping track of the
	 * expected element to remove, in lastItem.  Yes, we may fail to
	 * remove lastItem from the queue if it moved due to an interleaved
	 * interior remove while in detached mode.
	 */
	class Itr : public EIterator<sp<E> >, public enable_shared_from_this<Itr> {
	private:
		friend class Itrs;
		EArrayBlockingQueue<E>* self;

		/** Index to look for new nextItem; NONE at end */
		int cursor;

		/** Element to be returned by next call to next(); null if none */
		sp<E> nextItem;

		/** Index of nextItem; NONE if none, REMOVED if removed elsewhere */
		int nextIndex;

		/** Last element returned; null if none or not detached. */
		sp<E> lastItem;

		/** Index of lastItem, NONE if none, REMOVED if removed elsewhere */
		int lastRet;

		/** Previous value of takeIndex, or DETACHED when detached */
		int prevTakeIndex;

		/** Previous value of iters.cycles */
		int prevCycles;

		/** Special index value indicating "not available" or "undefined" */
		static const int NONE = -1;

		/**
		 * Special index value indicating "removed elsewhere", that is,
		 * removed by some operation other than a call to this.remove().
		 */
		static const int REMOVED = -2;

		/** Special value for prevTakeIndex indicating "detached mode" */
		static const int DETACHED = -3;

	private:
		int incCursor(int index) {
			// assert lock.getHoldCount() == 1;
			if (++index == self->items.length())
				index = 0;
			if (index == self->putIndex)
				index = NONE;
			return index;
		}

		/**
		 * Returns true if index is invalidated by the given number of
		 * dequeues, starting from prevTakeIndex.
		 */
		boolean invalidated(int index, int prevTakeIndex,
									long dequeues, int length) {
			if (index < 0)
				return false;
			int distance = index - prevTakeIndex;
			if (distance < 0)
				distance += length;
			return dequeues > distance;
		}

		/**
		 * Adjusts indices to incorporate all dequeues since the last
		 * operation on this iterator.  Call only from iterating thread.
		 */
		void incorporateDequeues() {
			// assert lock.getHoldCount() == 1;
			// assert itrs != null;
			// assert !isDetached();
			// assert count > 0;

			int cycles = self->itrs->cycles;
			int takeIndex = self->takeIndex;
			int prevCycles = this->prevCycles;
			int prevTakeIndex = this->prevTakeIndex;

			if (cycles != prevCycles || takeIndex != prevTakeIndex) {
				int len = self->items.length();
				// how far takeIndex has advanced since the previous
				// operation of this iterator
				long dequeues = (cycles - prevCycles) * len
					+ (takeIndex - prevTakeIndex);

				// Check indices for invalidation
				if (invalidated(lastRet, prevTakeIndex, dequeues, len))
					lastRet = REMOVED;
				if (invalidated(nextIndex, prevTakeIndex, dequeues, len))
					nextIndex = REMOVED;
				if (invalidated(cursor, prevTakeIndex, dequeues, len))
					cursor = takeIndex;

				if (cursor < 0 && nextIndex < 0 && lastRet < 0)
					detach();
				else {
					this->prevCycles = cycles;
					this->prevTakeIndex = takeIndex;
				}
			}
		}

		/**
		 * Called when itrs should stop tracking this iterator, either
		 * because there are no more indices to update (cursor < 0 &&
		 * nextIndex < 0 && lastRet < 0) or as a special exception, when
		 * lastRet >= 0, because hasNext() is about to return false for the
		 * first time.  Call only from iterating thread.
		 */
		void detach() {
			// Switch to detached mode
			// assert lock.getHoldCount() == 1;
			// assert cursor == NONE;
			// assert nextIndex < 0;
			// assert lastRet < 0 || nextItem == null;
			// assert lastRet < 0 ^ lastItem != null;
			if (prevTakeIndex >= 0) {
				// assert itrs != null;
				prevTakeIndex = DETACHED;
				// try to unlink from itrs (but not too hard)
				self->itrs->doSomeSweeping(true);
			}
		}

		void noNext() {
			EReentrantLock& lock = self->lock;
			SYNCBLOCK(&lock) {
				// assert cursor == NONE;
				// assert nextIndex == NONE;
				if (!isDetached()) {
					// assert lastRet >= 0;
					incorporateDequeues(); // might update lastRet
					if (lastRet >= 0) {
						lastItem = self->itemAt(lastRet);
						// assert lastItem != null;
						detach();
					}
				}
				// assert isDetached();
				// assert lastRet < 0 ^ lastItem != null;
			}}
		}

		int distance(int index, int prevTakeIndex, int length) {
			int distance = index - prevTakeIndex;
			if (distance < 0)
				distance += length;
			return distance;
		}

	public:
		Itr(EArrayBlockingQueue<E>* s) : self(s) {
			//
		}

		void init() {
			// assert lock.getHoldCount() == 0;
			lastRet = NONE;
			EReentrantLock& lock = self->lock;
			SYNCBLOCK(&lock) {
				if (self->count == 0) {
					// assert itrs == null;
					cursor = NONE;
					nextIndex = NONE;
					prevTakeIndex = DETACHED;
				} else {
					int takeIndex = self->takeIndex;
					prevTakeIndex = takeIndex;
					nextItem = self->itemAt(nextIndex = takeIndex);
					cursor = incCursor(takeIndex);
					if (self->itrs == null) {
						self->itrs = new Itrs(self, Itr::shared_from_this());
					} else {
						self->itrs->register_(Itr::shared_from_this()); // in this order
						self->itrs->doSomeSweeping(false);
					}
					prevCycles = self->itrs->cycles;
					// assert takeIndex >= 0;
					// assert prevTakeIndex == takeIndex;
					// assert nextIndex >= 0;
					// assert nextItem != null;
				}
			}}
		}

		boolean isDetached() {
			// assert lock.getHoldCount() == 1;
			return prevTakeIndex < 0;
		}

		/**
		 * For performance reasons, we would like not to acquire a lock in
		 * hasNext in the common case.  To allow for this, we only access
		 * fields (i.e. nextItem) that are not modified by update operations
		 * triggered by queue modifications.
		 */
		virtual boolean hasNext() {
			// assert lock.getHoldCount() == 0;
			if (nextItem != null)
				return true;
			noNext();
			return false;
		}

		virtual sp<E> next() {
			// assert lock.getHoldCount() == 0;
			sp<E> x = nextItem;
			if (x == null)
				throw ENoSuchElementException(__FILE__, __LINE__);
			EReentrantLock& lock = self->lock;
			SYNCBLOCK(&lock) {
				if (!isDetached())
					incorporateDequeues();
				// assert nextIndex != NONE;
				// assert lastItem == null;
				lastRet = nextIndex;
				int cursor = this->cursor;
				if (cursor >= 0) {
					nextItem = self->itemAt(nextIndex = cursor);
					// assert nextItem != null;
					this->cursor = incCursor(cursor);
				} else {
					nextIndex = NONE;
					nextItem = null;
				}
			}}
			return x;
		}

		virtual void remove() {
			// assert lock.getHoldCount() == 0;
			EReentrantLock& lock = self->lock;
			SYNCBLOCK(&lock) {
				if (!isDetached())
					incorporateDequeues(); // might update lastRet or detach
				int lastRet = this->lastRet;
				this->lastRet = NONE;
				if (lastRet >= 0) {
					if (!isDetached())
						self->removeAt(lastRet);
					else {
						sp<E> lastItem = this->lastItem;
						// assert lastItem != null;
						this->lastItem = null;
						if (self->itemAt(lastRet) == lastItem)
							self->removeAt(lastRet);
					}
				} else if (lastRet == NONE)
					throw EIllegalStateException(__FILE__, __LINE__);
				// else lastRet == REMOVED and the last returned element was
				// previously asynchronously removed via an operation other
				// than this.remove(), so nothing to do.

				if (cursor < 0 && nextIndex < 0)
					detach();
			}}
		}

		virtual sp<E> moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

		/**
		 * Called to notify the iterator that the queue is empty, or that it
		 * has fallen hopelessly behind, so that it should abandon any
		 * further iteration, except possibly to return one more element
		 * from next(), as promised by returning true from hasNext().
		 */
		void shutdown() {
			// assert lock.getHoldCount() == 1;
			cursor = NONE;
			if (nextIndex >= 0)
				nextIndex = REMOVED;
			if (lastRet >= 0) {
				lastRet = REMOVED;
				lastItem = null;
			}
			prevTakeIndex = DETACHED;
			// Don't set nextItem to null because we must continue to be
			// able to return it on next().
			//
			// Caller will unlink from itrs when convenient.
		}

		/**
		 * Called whenever an interior remove (not at takeIndex) occurred.
		 *
		 * @return true if this iterator should be unlinked from itrs
		 */
		boolean removedAt(int removedIndex) {
			// assert lock.getHoldCount() == 1;
			if (isDetached())
				return true;

			int cycles = self->itrs->cycles;
			int takeIndex = self->takeIndex;
			int prevCycles = this->prevCycles;
			int prevTakeIndex = this->prevTakeIndex;
			int len = self->items.length();
			int cycleDiff = cycles - prevCycles;
			if (removedIndex < takeIndex)
				cycleDiff++;
			int removedDistance =
				(cycleDiff * len) + (removedIndex - prevTakeIndex);
			// assert removedDistance >= 0;
			int cursor = this->cursor;
			if (cursor >= 0) {
				int x = distance(cursor, prevTakeIndex, len);
				if (x == removedDistance) {
					if (cursor == self->putIndex)
						this->cursor = cursor = NONE;
				}
				else if (x > removedDistance) {
					// assert cursor != prevTakeIndex;
					this->cursor = cursor = self->dec(cursor);
				}
			}
			int lastRet = this->lastRet;
			if (lastRet >= 0) {
				int x = distance(lastRet, prevTakeIndex, len);
				if (x == removedDistance)
					this->lastRet = lastRet = REMOVED;
				else if (x > removedDistance)
					this->lastRet = lastRet = self->dec(lastRet);
			}
			int nextIndex = this->nextIndex;
			if (nextIndex >= 0) {
				int x = distance(nextIndex, prevTakeIndex, len);
				if (x == removedDistance)
					this->nextIndex = nextIndex = REMOVED;
				else if (x > removedDistance)
					this->nextIndex = nextIndex = self->dec(nextIndex);
			}
			else if (cursor < 0 && nextIndex < 0 && lastRet < 0) {
				this->prevTakeIndex = DETACHED;
				return true;
			}
			return false;
		}

		/**
		 * Called whenever takeIndex wraps around to zero.
		 *
		 * @return true if this iterator should be unlinked from itrs
		 */
		boolean takeIndexWrapped() {
			// assert lock.getHoldCount() == 1;
			if (isDetached())
				return true;
			if (self->itrs->cycles - prevCycles > 1) {
				// All the elements that existed at the time of the last
				// operation are gone, so abandon further iteration.
				shutdown();
				return true;
			}
			return false;
		}

//         /** Uncomment for debugging. */
//         public String toString() {
//             return ("cursor=" + cursor + " " +
//                     "nextIndex=" + nextIndex + " " +
//                     "lastRet=" + lastRet + " " +
//                     "nextItem=" + nextItem + " " +
//                     "lastItem=" + lastItem + " " +
//                     "prevCycles=" + prevCycles + " " +
//                     "prevTakeIndex=" + prevTakeIndex + " " +
//                     "size()=" + size() + " " +
//                     "remainingCapacity()=" + remainingCapacity());
//         }
	};

	/**
	 * Shared data between iterators and their queue, allowing queue
	 * modifications to update iterators when elements are removed.
	 *
	 * This adds a lot of complexity for the sake of correctly
	 * handling some uncommon operations, but the combination of
	 * circular-arrays and supporting interior removes (i.e., those
	 * not at head) would cause iterators to sometimes lose their
	 * places and/or (re)report elements they shouldn't.  To avoid
	 * this, when a queue has one or more iterators, it keeps iterator
	 * state consistent by:
	 *
	 * (1) keeping track of the number of "cycles", that is, the
	 *     number of times takeIndex has wrapped around to 0.
	 * (2) notifying all iterators via the callback removedAt whenever
	 *     an interior element is removed (and thus other elements may
	 *     be shifted).
	 *
	 * These suffice to eliminate iterator inconsistencies, but
	 * unfortunately add the secondary responsibility of maintaining
	 * the list of iterators.  We track all active iterators in a
	 * simple linked list (accessed only when the queue's lock is
	 * held) of weak references to Itr.  The list is cleaned up using
	 * 3 different mechanisms:
	 *
	 * (1) Whenever a new iterator is created, do some O(1) checking for
	 *     stale list elements.
	 *
	 * (2) Whenever takeIndex wraps around to 0, check for iterators
	 *     that have been unused for more than one wrap-around cycle.
	 *
	 * (3) Whenever the queue becomes empty, all iterators are notified
	 *     and this entire data structure is discarded.
	 *
	 * So in addition to the removedAt callback that is necessary for
	 * correctness, iterators have the shutdown and takeIndexWrapped
	 * callbacks that help remove stale iterators from the list.
	 *
	 * Whenever a list element is examined, it is expunged if either
	 * the GC has determined that the iterator is discarded, or if the
	 * iterator reports that it is "detached" (does not need any
	 * further state updates).  Overhead is maximal when takeIndex
	 * never advances, iterators are discarded before they are
	 * exhausted, and all removals are interior removes, in which case
	 * all stale iterators are discovered by the GC.  But even in this
	 * case we don't increase the amortized complexity.
	 *
	 * Care must be taken to keep list sweeping methods from
	 * reentrantly invoking another such method, causing subtle
	 * corruption bugs.
	 */
	class Itrs : public EObject {
	private:
		friend class Itr;

		EArrayBlockingQueue<E>* self;

		/**
		 * Node in a linked list of weak iterator references.
		 */
		class Node : public EObject {
		public:
			wp<Itr> iterator;
			sp<Node> next;

			Node(sp<Itr>& iterator, sp<Node>& next) {
				this->iterator = iterator;
				this->next = next;
			}
		};

		/** Incremented whenever takeIndex wraps around to 0 */
		int cycles;// = 0;

		/** Linked list of weak iterator references */
		sp<Node> head;

		/** Used to expunge stale iterators */
		sp<Node> sweeper;// = null;

		static const int SHORT_SWEEP_PROBES = 4;
		static const int LONG_SWEEP_PROBES = 16;

	public:
		Itrs(EArrayBlockingQueue<E>* s, sp<Itr> initial) : self(s), cycles(0) {
			register_(initial);
		}

		/**
		 * Sweeps itrs, looking for and expunging stale iterators.
		 * If at least one was found, tries harder to find more.
		 * Called only from iterating thread.
		 *
		 * @param tryHarder whether to start in try-harder mode, because
		 * there is known to be at least one iterator to collect
		 */
		void doSomeSweeping(boolean tryHarder) {
			// assert lock.getHoldCount() == 1;
			// assert head != null;
			int probes = tryHarder ? LONG_SWEEP_PROBES : SHORT_SWEEP_PROBES;
			sp<Node> o, p;
			sp<Node> sweeper = this->sweeper;
			boolean passedGo;   // to limit search to one full sweep

			if (sweeper == null) {
				o = null;
				p = head;
				passedGo = true;
			} else {
				o = sweeper;
				p = o->next;
				passedGo = false;
			}

			for (; probes > 0; probes--) {
				if (p == null) {
					if (passedGo)
						break;
					o = null;
					p = head;
					passedGo = true;
				}
				sp<Itr> it = p->iterator.lock(); //p.get();
				sp<Node> next = p->next;
				if (it == null || it->isDetached()) {
					// found a discarded/exhausted iterator
					probes = LONG_SWEEP_PROBES; // "try harder"
					// unlink p
                    p->iterator = null;// p->clear();
					p->next = null;
					if (o == null) {
						head = next;
						if (next == null) {
							// We've run out of iterators to track; retire
							self->itrs = null;
							return;
						}
					}
					else
						o->next = next;
				} else {
					o = p;
				}
				p = next;
			}

			this->sweeper = (p == null) ? null : o;
		}

		/**
		 * Adds a new iterator to the linked list of tracked iterators.
		 */
		void register_(sp<Itr> itr) {
			// assert lock.getHoldCount() == 1;
			head = new Node(itr, head);
		}

		/**
		 * Called whenever takeIndex wraps around to 0.
		 *
		 * Notifies all iterators, and expunges any that are now stale.
		 */
		void takeIndexWrapped() {
			// assert lock.getHoldCount() == 1;
			cycles++;
			for (sp<Node> o = null, p = head; p != null;) {
				sp<Itr> it = p->iterator.lock(); //p.get();
				sp<Node> next = p->next;
				if (it == null || it->takeIndexWrapped()) {
					// unlink p
					// assert it == null || it.isDetached();
					p->iterator = null;// p->clear();
					p->next = null;
					if (o == null)
						head = next;
					else
						o->next = next;
				} else {
					o = p;
				}
				p = next;
			}
			if (head == null)   // no more iterators to track
				self->itrs = null;
		}

		/**
		 * Called whenever an interior remove (not at takeIndex) occurred.
		 *
		 * Notifies all iterators, and expunges any that are now stale.
		 */
		void removedAt(int removedIndex) {
			for (sp<Node> o = null, p = head; p != null;) {
				sp<Itr> it = p->iterator.lock(); //p.get();
				sp<Node> next = p->next;
				if (it == null || it->removedAt(removedIndex)) {
					// unlink p
					// assert it == null || it.isDetached();
					p->iterator = null;// p->clear();
					p->next = null;
					if (o == null)
						head = next;
					else
						o->next = next;
				} else {
					o = p;
				}
				p = next;
			}
			if (head == null)   // no more iterators to track
				self->itrs = null;
		}

		/**
		 * Called whenever the queue becomes empty.
		 *
		 * Notifies all active iterators that the queue is empty,
		 * clears all weak refs, and unlinks the itrs datastructure.
		 */
		void queueIsEmpty() {
			// assert lock.getHoldCount() == 1;
			for (sp<Node> p = head; p != null; p = p->next) {
				sp<Itr> it = p->iterator.lock(); //p.get();
				if (it != null) {
					p->iterator = null;// p->clear();
					it->shutdown();
				}
			}
			head = null;
			self->itrs = null;
		}

		/**
		 * Called whenever an element has been dequeued (at takeIndex).
		 */
		void elementDequeued() {
			// assert lock.getHoldCount() == 1;
			if (self->count == 0)
				queueIsEmpty();
			else if (self->takeIndex == 0)
				takeIndexWrapped();
		}
	};

protected:
	friend class Itr;

	/** The queued items */
	EA<sp<E> > items;

	/** items index for next take, poll, peek or remove */
	int takeIndex;

	/** items index for next put, offer, or add */
	int putIndex;

	/** Number of elements in the queue */
	int count;

	/*
	 * Concurrency control uses the classic two-condition algorithm
	 * found in any textbook.
	 */

	/** Main lock guarding all access */
	EReentrantLock lock;

	/**
	 * Shared state for currently active iterators, or null if there
	 * are known not to be any.  Allows queue operations to update
	 * iterator state.
	 */
	sp<Itrs> itrs;// = null;

	// Internal helper methods

	/**
	 * Circularly decrement i.
	 */
	int dec(int i) {
		return ((i == 0) ? items.length() : i) - 1;
	}

	/**
	 * Returns item at index i.
	 */
	sp<E> itemAt(int i) {
		return items[i];
	}

	/**
	 * Deletes item at array index removeIndex.
	 * Utility for remove(Object) and iterator.remove.
	 * Call only when holding lock.
	 */
	void removeAt(int removeIndex) {
		// assert lock.getHoldCount() == 1;
		// assert items[removeIndex] != null;
		// assert removeIndex >= 0 && removeIndex < items.length();
		EA<sp<E> >& items = this->items;
		if (removeIndex == takeIndex) {
			// removing front item; just advance
			items[takeIndex] = null;
			if (++takeIndex == items.length())
				takeIndex = 0;
			count--;
			if (itrs != null)
				itrs->elementDequeued();
		} else {
			// an "interior" remove

			// slide over all others up through putIndex.
			int putIndex = this->putIndex;
			for (int i = removeIndex;;) {
				int next = i + 1;
				if (next == items.length())
					next = 0;
				if (next != putIndex) {
					items[i] = items[next];
					i = next;
				} else {
					items[i] = null;
					this->putIndex = i;
					break;
				}
			}
			count--;
			if (itrs != null)
				itrs->removedAt(removeIndex);
		}
		notFull->signal();
	}
};

} /* namespace efc */
#endif /* EArrayBlockingQueue_HH_ */
