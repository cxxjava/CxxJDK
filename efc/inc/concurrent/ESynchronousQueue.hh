/*
 * ESynchronousQueue.hh
 *
 *  Created on: 2015-6-13
 *      Author: cxxjava@163.com
 */

#ifndef ESYNCHRONOUSQUEUE_HH_
#define ESYNCHRONOUSQUEUE_HH_

#include "./EBlockingQueue.hh"
#include "./EAbstractConcurrentQueue.hh"
#include "../EInterruptedException.hh"
#include "../ENullPointerException.hh"

namespace efc {

//@see: jdk5.0/j2se/src/share/classes/java/util/concurrent/SynchronousQueue.java
//FIXME: update to jdk7, see: commited on 20150625

/**
 * A {@linkplain BlockingQueue blocking queue} in which each
 * <tt>put</tt> must wait for a <tt>take</tt>, and vice versa.  A
 * synchronous queue does not have any internal capacity, not even a
 * capacity of one. You cannot <tt>peek</tt> at a synchronous queue
 * because an element is only present when you try to take it; you
 * cannot add an element (using any method) unless another thread is
 * trying to remove it; you cannot iterate as there is nothing to
 * iterate.  The <em>head</em> of the queue is the element that the
 * first queued thread is trying to add to the queue; if there are no
 * queued threads then no element is being added and the head is
 * <tt>null</tt>.  For purposes of other <tt>Collection</tt> methods
 * (for example <tt>contains</tt>), a <tt>SynchronousQueue</tt> acts
 * as an empty collection.  This queue does not permit <tt>null</tt>
 * elements.
 *
 * <p>Synchronous queues are similar to rendezvous channels used in
 * CSP and Ada. They are well suited for handoff designs, in which an
 * object running in one thread must sync up with an object running
 * in another thread in order to hand it some information, event, or
 * task.
 *
 * <p> This class supports an optional fairness policy for ordering
 * waiting producer and consumer threads.  By default, this ordering
 * is not guaranteed. However, a queue constructed with fairness set
 * to <tt>true</tt> grants threads access in FIFO order. Fairness
 * generally decreases throughput but reduces variability and avoids
 * starvation.
 *
 * <p>This class and its iterator implement all of the
 * <em>optional</em> methods of the {@link Collection} and {@link
 * Iterator} interfaces.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../guide/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since 1.5
 * @param <E> the type of elements held in this collection
 */

template<typename E>
class ESynchronousQueue: virtual public EAbstractConcurrentQueue<E>,
		virtual public EBlockingQueue<E> {
private:
	/**
	 * Nodes each maintain an item and handle waits and signals for
	 * getting and setting it. The class extends
	 * AbstractQueuedSynchronizer to manage blocking, using AQS state
	 *  0 for waiting, 1 for ack, -1 for cancelled.
	 */
	class Node : public EAbstractQueuedSynchronizer {
	private:
		/** Synchronization state value representing that node acked */
		static const int ACK    =  1;
		/** Synchronization state value representing that node cancelled */
		static const int CANCEL = -1;
	public:
		/** The item being transferred */
		sp<E> item;
		/** Next node in wait queue */
		sp<Node> next;

		/** Creates a node with initial item */
		Node(sp<E> x) : next(null) { item = x; }

		/** Creates a node with initial item and next */
		Node(sp<E> x, sp<Node> n) { item = x; next = n; }

		/**
		 * Implements AQS base acquire to succeed if not in WAITING state
		 */
		boolean tryAcquire(int ignore) {
			return getState() != 0;
		}

		/**
		 * Implements AQS base release to signal if state changed
		 */
		boolean tryRelease(int newState) {
			return compareAndSetState(0, newState);
		}

		/**
		 * Takes item and nulls out field (for sake of GC)
		 */
		sp<E> extract() {
			sp<E> x = item;
			item = null;
			return x;
		}

		/**
		 * Tries to cancel on interrupt; if so rethrowing,
		 * else setting interrupt state
		 */
		void checkCancellationOnInterrupt(EInterruptedException& ie)
			THROWS(EInterruptedException) {
			if (release(CANCEL))
				throw ie;
			EThread::currentThread()->interrupt();
		}

		/**
		 * Fills in the slot created by the consumer and signal consumer to
		 * continue.
		 */
		boolean setItem(sp<E> x) {
			item = x; // can place in slot even if cancelled
			return release(ACK);
		}

		/**
		 * Removes item from slot created by producer and signal producer
		 * to continue.
		 */
		sp<E> getItem() {
			return (release(ACK))? extract() : null;
		}

		/**
		 * Waits for a consumer to take item placed by producer.
		 */
		void waitForTake() THROWS(EInterruptedException) {
			try {
				acquireInterruptibly(0);
			} catch (EInterruptedException& ie) {
				checkCancellationOnInterrupt(ie);
			}
		}

		/**
		 * Waits for a producer to put item placed by consumer.
		 */
		sp<E> waitForPut() THROWS(EInterruptedException) {
			try {
				acquireInterruptibly(0);
			} catch (EInterruptedException& ie) {
				checkCancellationOnInterrupt(ie);
			}
			return extract();
		}

		/**
		 * Waits for a consumer to take item placed by producer or time out.
		 */
		boolean waitForTake(llong nanos) THROWS(EInterruptedException) {
			try {
				if (!tryAcquireNanos(0, nanos) &&
					release(CANCEL))
					return false;
			} catch (EInterruptedException& ie) {
				checkCancellationOnInterrupt(ie);
			}
			return true;
		}

		/**
		 * Waits for a producer to put item placed by consumer, or time out.
		 */
		sp<E> waitForPut(llong nanos) THROWS(EInterruptedException) {
			try {
				if (!tryAcquireNanos(0, nanos) &&
					release(CANCEL))
					return null;
			} catch (EInterruptedException& ie) {
				checkCancellationOnInterrupt(ie);
			}
			return extract();
		}
	};

	/**
	 * Queue to hold waiting puts/takes; specialized to Fifo/Lifo below.
	 * These queues have all transient fields, but are serializable
	 * in order to recover fairness settings when deserialized.
	 */
	interface WaitQueue : public EObject {
		/** Create, add, and return node for x */
		virtual sp<Node> enq(sp<E> x) = 0;
		/** Remove and return node, or null if empty */
		virtual sp<Node> deq() = 0;
	};


    /**
     * FIFO queue to hold waiting puts/takes.
     */
    class FifoWaitQueue : public WaitQueue {
    public:
		sp<Node> head;
		sp<Node> last;

        FifoWaitQueue() : head(null), last(null) {
        }

        sp<Node> enq(sp<E> x) {
        	sp<Node> p = new Node(x);
            if (last == null)
                last = head = p;
            else
                last = last->next = p;
            return p;
        }

        sp<Node> deq() {
        	sp<Node> p = head;
            if (p != null) {
                if ((head = p->next) == null)
                    last = null;
                p->next = null;
            }
            return p;
        }
    };

    /**
     * LIFO queue to hold waiting puts/takes.
     */
    class LifoWaitQueue : public WaitQueue {
    public:
    	sp<Node> head;

        LifoWaitQueue() : head(null) {
        }

        sp<Node> enq(sp<E> x) {
            return head = new Node(x, head);
        }

        sp<Node> deq() {
        	sp<Node> p = head;
            if (p != null) {
                head = p->next;
                p->next = null;
            }
            return p;
        }
    };

public:

	/*
	  This implementation divides actions into two cases for puts:

	  * An arriving producer that does not already have a waiting consumer
	  creates a node holding item, and then waits for a consumer to take it.
	  * An arriving producer that does already have a waiting consumer fills
	  the slot node created by the consumer, and notifies it to continue.

	  And symmetrically, two for takes:

	  * An arriving consumer that does not already have a waiting producer
	  creates an empty slot node, and then waits for a producer to fill it.
	  * An arriving consumer that does already have a waiting producer takes
	  item from the node created by the producer, and notifies it to continue.

	  When a put or take waiting for the actions of its counterpart
	  aborts due to interruption or timeout, it marks the node
	  it created as "CANCELLED", which causes its counterpart to retry
	  the entire put or take sequence.

	  This requires keeping two simple queues, waitingProducers and
	  waitingConsumers. Each of these can be FIFO (preserves fairness)
	  or LIFO (improves throughput).
	*/

	virtual ~ESynchronousQueue() {
		delete waitingProducers;
		delete waitingConsumers;
		delete qlock;
	}

	/**
	 * Creates a <tt>SynchronousQueue</tt> with specified fairness policy.
	 * @param fair if true, threads contend in FIFO order for access;
	 * otherwise the order is unspecified.
	 */
	ESynchronousQueue(boolean fair=false) {
		if (fair) {
			qlock = new EReentrantLock(true);
			waitingProducers = new FifoWaitQueue();
			waitingConsumers = new FifoWaitQueue();
		}
		else {
			qlock = new EReentrantLock();
			waitingProducers = new LifoWaitQueue();
			waitingConsumers = new LifoWaitQueue();
		}
	}

	/**
	 * Adds the specified element to this queue, waiting if necessary for
	 * another thread to receive it.
	 *
	 * @throws InterruptedException {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual void put(E* e) THROWS(EInterruptedException) {
		sp<E> x(e);
		put(x);
	}
	virtual void put(sp<E> o) THROWS(EInterruptedException) {
		if (o == null) throw ENullPointerException(__FILE__, __LINE__);
		for (;;) {
			sp<Node> node;
			boolean mustWait;
			if (EThread::interrupted()) throw EInterruptedException(__FILE__, __LINE__);
			SYNCBLOCK(qlock) {
				node = waitingConsumers->deq();
				if ( (mustWait = (node == null)) )
					node = waitingProducers->enq(o);
            }}

			if (mustWait) {
				node->waitForTake();
				return;
			}

			else if (node->setItem(o)) {
				return;
			}

			// else consumer cancelled, so retry
		}
	}

	/**
	 * Inserts the specified element into this queue, waiting if necessary
	 * up to the specified wait time for another thread to receive it.
	 *
	 * @return <tt>true</tt> if successful, or <tt>false</tt> if the
	 *         specified waiting time elapses before a consumer appears.
	 * @throws InterruptedException {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean offer(E* e, llong timeout, ETimeUnit* unit)
			THROWS(EInterruptedException) {
		sp<E> x(e);
		boolean r = offer(x, timeout, unit);
		if (!r) {
			x.dismiss();
		}
		return r;
	}
	virtual boolean offer(sp<E> o, llong timeout, ETimeUnit* unit)
			THROWS(EInterruptedException) {
		if (o == null) throw ENullPointerException(__FILE__, __LINE__);
		llong nanos = unit->toNanos(timeout);
		for (;;) {
			sp<Node> node;
			boolean mustWait;
			if (EThread::interrupted()) throw EInterruptedException(__FILE__, __LINE__);
			SYNCBLOCK(qlock) {
				node = waitingConsumers->deq();
				if ( (mustWait = (node == null)) )
					node = waitingProducers->enq(o);
            }}

			if (mustWait) {
				return node->waitForTake(nanos);
			}

			else if (node->setItem(o)) {
				return true;
			}

			// else consumer cancelled, so retry
		}
	}

	/**
	 * Inserts the specified element into this queue, if another thread is
	 * waiting to receive it.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> if the element was added to this queue, else
	 *         <tt>false</tt>
	 * @throws NullPointerException if the specified element is null
	 */
	virtual boolean offer(E* e) {
		sp<E> x(e);
		boolean r = offer(x);
		if (!r) {
			x.dismiss();
		}
		return r;
	}
	virtual boolean offer(sp<E> e) {
		if (e == null) throw ENullPointerException(__FILE__, __LINE__);
		for (;;) {
			sp<Node> node;
			SYNCBLOCK(qlock) {
				node = waitingConsumers->deq();
            }}
			if (node == null)
				return false;

			else if (node->setItem(e))
				return true;
			// else retry
		}
	}

	/**
	 * Retrieves and removes the head of this queue, waiting if necessary
	 * for another thread to insert it.
	 *
	 * @return the head of this queue
	 * @throws InterruptedException {@inheritDoc}
	 */
	virtual sp<E> take() THROWS(EInterruptedException) {
		for (;;) {
			sp<Node> node;
			boolean mustWait;

			if (EThread::interrupted()) throw EInterruptedException(__FILE__, __LINE__);
			SYNCBLOCK(qlock) {
				node = waitingProducers->deq();
				if ( (mustWait = (node == null)) )
					node = waitingConsumers->enq(null);
            }}

			if (mustWait) {
				return node->waitForPut();
			}
			else {
				sp<E> x = node->getItem();
				if (x != null) {
					return x;
				}
				// else cancelled, so retry
			}
		}
	}

	/**
	 * Retrieves and removes the head of this queue, waiting
	 * if necessary up to the specified wait time, for another thread
	 * to insert it.
	 *
	 * @return the head of this queue, or <tt>null</tt> if the
	 *         specified waiting time elapses before an element is present.
	 * @throws InterruptedException {@inheritDoc}
	 */
	virtual sp<E> poll(llong timeout, ETimeUnit* unit) THROWS(EInterruptedException) {
		llong nanos = unit->toNanos(timeout);

		for (;;) {
			sp<Node> node;
			boolean mustWait;

			if (EThread::interrupted()) throw EInterruptedException(__FILE__, __LINE__);
			SYNCBLOCK(qlock) {
				node = waitingProducers->deq();
				if ( (mustWait = (node == null)) )
					node = waitingConsumers->enq(null);
            }}

			if (mustWait) {
				sp<E> x = node->waitForPut(nanos);
				return x;
			}
			else {
				sp<E> x = node->getItem();
				if (x != null) {
					return x;
				}
				// else cancelled, so retry
			}
		}
	}

	/**
	 * Retrieves and removes the head of this queue, if another thread
	 * is currently making an element available.
	 *
	 * @return the head of this queue, or <tt>null</tt> if no
	 *         element is available.
	 */
	virtual sp<E> poll() {
		for (;;) {
			sp<Node> node;
			SYNCBLOCK(qlock) {
				node = waitingProducers->deq();
            }}
			if (node == null)
				return null;

			else {
				sp<E> x = node->getItem();
				if (x != null) {
					return x;
				}
				// else retry
			}
		}
	}

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
	virtual boolean add(E* e) {
		return EAbstractConcurrentQueue<E>::add(e);
	}
	virtual boolean add(sp<E> e) {
		return EAbstractConcurrentQueue<E>::add(e);
	}

	/**
	 * Always returns <tt>true</tt>.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @return <tt>true</tt>
	 */
	virtual boolean isEmpty() {
		return true;
	}

	/**
	 * Always returns zero.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @return zero.
	 */
	virtual int size() {
		return 0;
	}

	/**
	 * Always returns zero.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @return zero.
	 */
	virtual int remainingCapacity() {
		return 0;
	}

	/**
	 * Does nothing.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 */
	virtual void clear() {
	}

	/**
	 * Always returns <tt>false</tt>.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @param o the element
	 * @return <tt>false</tt>
	 */
	virtual boolean contains(E* o) {
		return false;
	}

	/**
	 * Always returns <tt>false</tt>.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @param o the element to remove
	 * @return <tt>false</tt>
	 */
	virtual boolean remove(E* o) {
		return false;
	}

	/**
	 * Returns <tt>false</tt> unless the given collection is empty.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @param c the collection
	 * @return <tt>false</tt> unless given collection is empty
	 */
	virtual boolean containsAll(EConcurrentCollection<E>* c) {
		return c->isEmpty();
	}

	/**
	 * Always returns <tt>false</tt>.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @param c the collection
	 * @return <tt>false</tt>
	 */
	virtual boolean removeAll(EConcurrentCollection<E> *c) {
		return false;
	}

	/**
	 * Always returns <tt>false</tt>.
	 * A <tt>SynchronousQueue</tt> has no internal capacity.
	 *
	 * @param c the collection
	 * @return <tt>false</tt>
	 */
	virtual boolean retainAll(EConcurrentCollection<E> *c) {
		return false;
	}

	/**
	 * Always returns <tt>null</tt>.
	 * A <tt>SynchronousQueue</tt> does not return elements
	 * unless actively waited on.
	 *
	 * @return <tt>null</tt>
	 */
	virtual sp<E> peek() {
		return null;
	}

	/**
	 * Returns an empty iterator in which <tt>hasNext</tt> always returns
	 * <tt>false</tt>.
	 *
	 * @return an empty iterator
	 */
	virtual sp<EConcurrentIterator<E> > iterator() {
		return null;//Collections.emptyIterator();
	}

	/**
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	virtual int drainTo(EConcurrentCollection<E>* c) {
		if (c == null)
			throw ENullPointerException(__FILE__, __LINE__);
		if (c == this)
			throw EIllegalArgumentException(__FILE__, __LINE__);
		int n = 0;
		sp<E> e;
		while ( (e = poll()) != null) {
			c->add(e);
			++n;
		}
		return n;
	}
	virtual int drainTo(ECollection<sp<E> >* c) {
		if (c == null)
			throw ENullPointerException(__FILE__, __LINE__);
		int n = 0;
		sp<E> e;
		while ( (e = poll()) != null) {
			c->add(e);
			++n;
		}
		return n;
	}

	/**
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	virtual int drainTo(EConcurrentCollection<E>* c, int maxElements) {
		if (c == null)
			throw ENullPointerException(__FILE__, __LINE__);
		if (c == this)
			throw EIllegalArgumentException(__FILE__, __LINE__);
		int n = 0;
		sp<E> e;
		while (n < maxElements && (e = poll()) != null) {
			c->add(e);
			++n;
		}
		return n;
	}
	virtual int drainTo(ECollection<sp<E> >* c, int maxElements) {
		if (c == null)
			throw ENullPointerException(__FILE__, __LINE__);
		int n = 0;
		sp<E> e;
		while (n < maxElements && (e = poll()) != null) {
			c->add(e);
			++n;
		}
		return n;
	}

private:
	/** Lock protecting both wait queues */
	EReentrantLock* qlock;
	/** Queue holding waiting puts */
	WaitQueue* waitingProducers;
	/** Queue holding waiting takes */
	WaitQueue* waitingConsumers;
};

} /* namespace efc */
#endif /* ESYNCHRONOUSQUEUE_HH_ */
