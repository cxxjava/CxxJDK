/*
 * EConcurrentLinkedQueue.hh
 *
 *  Created on: 2014-6-7
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTLINKEDQUEUE_HH_
#define ECONCURRENTLINKEDQUEUE_HH_

#include "../EInteger.hh"
#include "../EQueue.hh"
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
 * A {@code ConcurrentLinkedQueue} is an appropriate choice when
 * many threads will share access to a common collection.
 * Like most other concurrent collection implementations, this class
 * does not permit the use of {@code null} elements.
 *
 * <p>This implementation employs an efficient <em>non-blocking</em>
 * algorithm based on one described in <a
 * href="http://www.cs.rochester.edu/u/michael/PODC96.html"> Simple,
 * Fast, and Practical Non-Blocking and Blocking Concurrent Queue
 * Algorithms</a> by Maged M. Michael and Michael L. Scott.
 *
 * <p>Iterators are <i>weakly consistent</i>, returning elements
 * reflecting the state of the queue at some point at or since the
 * creation of the iterator.  They do <em>not</em> throw {@link
 * java.util.ConcurrentModificationException}, and may proceed concurrently
 * with other operations.  Elements contained in the queue since the creation
 * of the iterator will be returned exactly once.
 *
 * <p>Beware that, unlike in most collections, the {@code size} method
 * is <em>NOT</em> a constant-time operation. Because of the
 * asynchronous nature of these queues, determining the current number
 * of elements requires a traversal of the elements, and so may report
 * inaccurate results if this collection is modified during traversal.
 * Additionally, the bulk operations {@code addAll},
 * {@code removeAll}, {@code retainAll}, {@code containsAll},
 * {@code equals}, and {@code toArray} are <em>not</em> guaranteed
 * to be performed atomically. For example, an iterator operating
 * concurrently with an {@code addAll} operation might view only some
 * of the added elements.
 *
 * <p>This class and its iterator implement all of the <em>optional</em>
 * methods of the {@link Queue} and {@link Iterator} interfaces.
 *
 * <p>Memory consistency effects: As with other concurrent
 * collections, actions in a thread prior to placing an object into a
 * {@code ConcurrentLinkedQueue}
 * <a href="package-summary.html#MemoryVisibility"><i>happen-before</i></a>
 * actions subsequent to the access or removal of that element from
 * the {@code ConcurrentLinkedQueue} in another thread.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since 1.5
 * @param <E> the type of elements held in this collection
 */

template<typename E>
class EConcurrentLinkedQueue: public EQueue<sp<E> > {
private:
	/*
	 * This is a modification of the Michael & Scott algorithm,
	 * adapted for a garbage-collected environment, with support for
	 * interior node deletion (to support remove(Object)).  For
	 * explanation, read the paper.
	 *
	 * Note that like most non-blocking algorithms in this package,
	 * this implementation relies on the fact that in garbage
	 * collected systems, there is no possibility of ABA problems due
	 * to recycled nodes, so there is no need to use "counted
	 * pointers" or related techniques seen in versions used in
	 * non-GC'ed settings.
	 *
	 * The fundamental invariants are:
	 * - There is exactly one (last) Node with a null next reference,
	 *   which is CASed when enqueueing.  This last Node can be
	 *   reached in O(1) time from tail, but tail is merely an
	 *   optimization - it can always be reached in O(N) time from
	 *   head as well.
	 * - The elements contained in the queue are the non-null items in
	 *   Nodes that are reachable from head.  CASing the item
	 *   reference of a Node to null atomically removes it from the
	 *   queue.  Reachability of all elements from head must remain
	 *   true even in the case of concurrent modifications that cause
	 *   head to advance.  A dequeued Node may remain in use
	 *   indefinitely due to creation of an Iterator or simply a
	 *   poll() that has lost its time slice.
	 *
	 * The above might appear to imply that all Nodes are GC-reachable
	 * from a predecessor dequeued Node.  That would cause two problems:
	 * - allow a rogue Iterator to cause unbounded memory retention
	 * - cause cross-generational linking of old Nodes to new Nodes if
	 *   a Node was tenured while live, which generational GCs have a
	 *   hard time dealing with, causing repeated major collections.
	 * However, only non-deleted Nodes need to be reachable from
	 * dequeued Nodes, and reachability does not necessarily have to
	 * be of the kind understood by the GC.  We use the trick of
	 * linking a Node that has just been dequeued to itself.  Such a
	 * self-link implicitly means to advance to head.
	 *
	 * Both head and tail are permitted to lag.  In fact, failing to
	 * update them every time one could is a significant optimization
	 * (fewer CASes). As with LinkedTransferQueue (see the internal
	 * documentation for that class), we use a slack threshold of two;
	 * that is, we update head/tail when the current pointer appears
	 * to be two or more steps away from the first/last node.
	 *
	 * Since head and tail are updated concurrently and independently,
	 * it is possible for tail to lag behind head (why not)?
	 *
	 * CASing a Node's item reference to null atomically removes the
	 * element from the queue.  Iterators skip over Nodes with null
	 * items.  Prior implementations of this class had a race between
	 * poll() and remove(Object) where the same element would appear
	 * to be successfully removed by two concurrent operations.  The
	 * method remove(Object) also lazily unlinks deleted Nodes, but
	 * this is merely an optimization.
	 *
	 * When constructing a Node (before enqueuing it) we avoid paying
	 * for a volatile write to item by using Unsafe.putObject instead
	 * of a normal write.  This allows the cost of enqueue to be
	 * "one-and-a-half" CASes.
	 *
	 * Both head and tail may or may not point to a Node with a
	 * non-null item.  If the queue is empty, all items must of course
	 * be null.  Upon creation, both head and tail refer to a dummy
	 * Node with null item.  Both head and tail are only updated using
	 * CAS, so they never regress, although again this is merely an
	 * optimization.
	 */

    class Node {
    public:
    	sp<E> item;
        sp<Node> next;

    public:
        Node(sp<E> x) { item = x; }

        ALWAYS_INLINE boolean casItem(sp<E>& cmp, sp<E>& val) {
        	//@see: return UNSAFE.compareAndSwapObject(this, itemOffset, cmp, val);
        	return atomic_compare_exchange(&item, &cmp, val);
        }
        ALWAYS_INLINE boolean casItem(sp<E>& cmp, es_nullptr_t) {
			//@see: return UNSAFE.compareAndSwapObject(this, itemOffset, cmp, val);
        	sp<E> val;
			return atomic_compare_exchange(&item, &cmp, val);
		}

        ALWAYS_INLINE void lazySetNext(sp<Node>& val) {
        	//@see: UNSAFE.putOrderedObject(this, nextOffset, val);
        	atomic_store(&next, val);
        }

        ALWAYS_INLINE boolean casNext(sp<Node>& cmp, sp<Node>& val) {
        	//@see: return UNSAFE.compareAndSwapObject(this, nextOffset, cmp, val);
        	return atomic_compare_exchange(&next, &cmp, val);
        }
        ALWAYS_INLINE boolean casNext(es_nullptr_t, sp<Node>& val) {
			//@see: return UNSAFE.compareAndSwapObject(this, nextOffset, cmp, val);
        	sp<Node> cmp;
			return atomic_compare_exchange(&next, &cmp, val);
		}

        ALWAYS_INLINE sp<E> getItem() {
            return atomic_load(&item);
        }

        ALWAYS_INLINE void setItem(es_nullptr_t) {
			sp<E> t;
			atomic_store(&item, t);
		}
        ALWAYS_INLINE void setItem(sp<E> val) {
        	atomic_store(&item, val);
        }

        ALWAYS_INLINE sp<Node> getNext() {
        	return atomic_load(&next);
        }

        ALWAYS_INLINE void setNext(sp<Node> val) {
        	atomic_store(&next, val);
        }
        ALWAYS_INLINE void setNext(es_nullptr_t) {
        	sp<Node> t;
			atomic_store(&next, t);
		}
    };

public:
    virtual ~EConcurrentLinkedQueue() {
    	// node unlink for recursion
    	sp<Node> node = head;
    	while (node != null) {
    		sp<Node> next = node->next;
    		node->next = null;
    		node = next;
    	}
    }

    /**
	 * Creates a {@code ConcurrentLinkedQueue} that is initially empty.
	 */
    EConcurrentLinkedQueue() {
    	head = new Node(null);
    	tail = head;

    	xxxx = new Node(null);
    }

    // Have to override just to update the javadoc

	/**
	 * Inserts the specified element at the tail of this queue.
	 * As the queue is unbounded, this method will never throw
	 * {@link IllegalStateException} or return {@code false}.
	 *
	 * @return {@code true} (as specified by {@link Collection#add})
	 * @throws NullPointerException if the specified element is null
	 */
    boolean add(sp<E> e) {
		return offer(e);
	}

    /**
	 * Inserts the specified element at the tail of this queue.
	 * As the queue is unbounded, this method will never return {@code false}.
	 *
	 * @return {@code true} (as specified by {@link Queue#offer})
	 * @throws NullPointerException if the specified element is null
	 */
    boolean offer(sp<E> e) {
    	if (e == null) throw ENullPointerException(__FILE__, __LINE__);

    	sp<Node> newNode(new Node(e));

    	sp<Node> t = atomic_load(&tail);
    	sp<Node> p = t;
    	for (;;) {
			sp<Node> q = p->getNext();
			if (q == null) {
				// p is last node
				if (p->casNext(null, newNode)) {
					// Successful CAS is the linearization point
					// for e to become an element of this queue,
					// and for newNode to become "live".
					if (p != t) // hop two nodes at a time
						casTail(t, newNode);  // Failure is OK.
					return true;
				}
				// Lost CAS race to another thread; re-read next
			}
			else if (q == xxxx)
				// We have fallen off list.  If tail is unchanged, it
				// will also be off-list, in which case we need to
				// jump to head, from which all live nodes are always
				// reachable.  Else the new tail is a better bet.
				p = (t != (t = atomic_load(&tail))) ? t : atomic_load(&head);
			else
				// Check for tail updates after two hops.
				p = (p != t && t != (t = atomic_load(&tail))) ? t : q;
		}
		//always not reach here.
		return true;
    }

    sp<E> poll() {
    	restartFromHead:
		for (;;) {
			sp<Node> h = atomic_load(&head);
			sp<Node> p = h, q;
			for (;;) {
				sp<E> item = p->getItem();

				if (item != null && p->casItem(item, null)) {
					// Successful CAS is the linearization point
					// for item to be removed from this queue.
					if (p != h) // hop two nodes at a time
						updateHead(h, ((q = p->getNext()) != null) ? q : p);
					return item;
				}
				else if ((q = p->getNext()) == null) {
					updateHead(h, p);
					return null;
				}
				else if (q == xxxx)
					goto restartFromHead;
				else
					p = q;
			}
		}
        //always not reach here.
        return null;
    }

    sp<E> element() {
    	sp<E> x = peek();
		if (x != null)
			return x;
		else
			throw ENoSuchElementException(__FILE__, __LINE__);
    }

    sp<E> peek() {
    	restartFromHead:
		for (;;) {
			sp<Node> h = atomic_load(&head);
			sp<Node>& p = h, q;
			for (;;) {
				sp<E> item = p->getItem();
				if (item != null || (q = p->getNext()) == null) {
					updateHead(h, p);
					return item;
				}
				else if (q == xxxx)
					goto restartFromHead;
				else
					p = q;
			}
		}
        //always not reach here.
        return null;
    }

    /**
	 * Returns the first live (non-deleted) node on list, or null if none.
	 * This is yet another variant of poll/peek; here returning the
	 * first node, not element.  We could make peek() a wrapper around
	 * first(), but that would cost an extra volatile read of item,
	 * and the need to add a retry loop to deal with the possibility
	 * of losing a race to a concurrent poll().
	 */
    sp<Node> first() {
    	restartFromHead:
		for (;;) {
			sp<Node> h = atomic_load(&head);
			sp<Node>& p = h, q;
			for (;;) {
				boolean hasItem = (p->getItem() != null);
				if (hasItem || (q = p->getNext()) == null) {
					updateHead(h, p);
					return hasItem ? p : null;
				}
				else if (q == xxxx)
					goto restartFromHead;
				else
					p = q;
			}
		}
        //always not reach here.
        return null;
    }


    /**
     * Returns <tt>true</tt> if this queue contains no elements.
     *
     * @return <tt>true</tt> if this queue contains no elements
     */
    boolean isEmpty() {
        return first() == null;
    }

    /**
	 * Returns the number of elements in this queue.  If this queue
	 * contains more than {@code Integer.MAX_VALUE} elements, returns
	 * {@code Integer.MAX_VALUE}.
	 *
	 * <p>Beware that, unlike in most collections, this method is
	 * <em>NOT</em> a constant-time operation. Because of the
	 * asynchronous nature of these queues, determining the current
	 * number of elements requires an O(n) traversal.
	 * Additionally, if elements are added or removed during execution
	 * of this method, the returned result may be inaccurate.  Thus,
	 * this method is typically not very useful in concurrent
	 * applications.
	 *
	 * @return the number of elements in this queue
	 */
    int size() {
        int count = 0;
        for (sp<Node> p = first(); p != null; p = succ(p)) {
            if (p->getItem() != null) {
                // Collections.size() spec says to max out
                if (++count == EInteger::MAX_VALUE)
                    break;
            }
        }
        return count;
    }

    /**
	 * Returns {@code true} if this queue contains the specified element.
	 * More formally, returns {@code true} if and only if this queue contains
	 * at least one element {@code e} such that {@code o.equals(e)}.
	 *
	 * @param o object to be checked for containment in this queue
	 * @return {@code true} if this queue contains the specified element
	 */
    boolean contains(E* o) {
        if (o == null) return false;
        for (sp<Node> p = first(); p != null; p = succ(p)) {
            sp<E> item = p->getItem();
            if (item != null && o->equals(item.get()))
                return true;
        }
        return false;
    }

    /**
	 * Removes a single instance of the specified element from this queue,
	 * if it is present.  More formally, removes an element {@code e} such
	 * that {@code o.equals(e)}, if this queue contains one or more such
	 * elements.
	 * Returns {@code true} if this queue contained the specified element
	 * (or equivalently, if this queue changed as a result of the call).
	 *
	 * @param o element to be removed from this queue, if present
	 * @return {@code true} if this queue changed as a result of the call
	 */
    boolean remove(E* o) {
        if (o == null) return false;
        sp<Node> pred = null;
        for (sp<Node> p = first(); p != null; p = succ(p)) {
            sp<E> item = p->getItem();
            if (item != null &&
                o->equals(item.get()) &&
                p->casItem(item, null)) {
            	sp<Node> next = succ(p);
            	if (pred != null && next != null)
					pred->casNext(p, next);
            	return true;
            }
            pred = p;
        }
        return false;
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
	sp<E> remove() {
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
    sp<EIterator<sp<E> > > iterator(int index=0) {
        return new Itr(this);
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
		// Use ArrayList to deal with resizing.
		EArrayList<sp<E> > al;
		for (sp<Node> p = first(); p != null; p = p->getNext()) {
			sp<E> item = p->getItem();
			if (item != null)
				al.add(item);
		}
		return al.toArray();
	}

	/**
	 * {@inheritDoc}
	 */
	virtual boolean containsAll(ECollection<sp<E> > *c) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 */
	virtual boolean removeAll(ECollection<sp<E> > *c) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

	/**
	 * {@inheritDoc}
	 */
	virtual boolean retainAll(ECollection<sp<E> > *c) {
		throw EUnsupportedOperationException(__FILE__, __LINE__);
	}

private:
	/**
	 * A node from which the first live (non-deleted) node (if any)
	 * can be reached in O(1) time.
	 * Invariants:
	 * - all live nodes are reachable from head via succ()
	 * - head != null
	 * - (tmp = head).next != tmp || tmp != head
	 * Non-invariants:
	 * - head.item may or may not be null.
	 * - it is permitted for tail to lag behind head, that is, for tail
	 *   to not be reachable from head!
	 */
	sp<Node> head;

	/**
	 * A node from which the last node on list (that is, the unique
	 * node with node.next == null) can be reached in O(1) time.
	 * Invariants:
	 * - the last node is always reachable from tail via succ()
	 * - tail != null
	 * Non-invariants:
	 * - tail.item may or may not be null.
	 * - it is permitted for tail to lag behind head, that is, for tail
	 *   to not be reachable from head!
	 * - tail.next may or may not be self-pointing to tail.
	 */
	sp<Node> tail;

	sp<Node> xxxx;

    class Itr : public EIterator<sp<E> > {
    private:
    	EConcurrentLinkedQueue* queue;

        /**
         * Next node to return item for.
         */
    	sp<Node> nextNode;

        /**
         * nextItem holds on to item fields because once we claim
         * that an element exists in hasNext(), we must return it in
         * the following next() call even if it was in the process of
         * being removed when hasNext() was called.
         */
        sp<E> nextItem;

        /**
         * Node of the last returned item, to support remove.
         */
        sp<Node> lastRet;

        /**
         * Moves to next valid node and returns item to return for
         * next(), or null if no such.
         */
        sp<E> advance() {
            lastRet = nextNode;
            sp<E> x = nextItem;

            sp<Node> pred, p;
			if (nextNode == null) {
				p = queue->first();
				pred = null;
			} else {
				pred = nextNode;
				p = queue->succ(nextNode);
			}

            for (;;) {
                if (p == null) {
                    nextNode = null;
                    nextItem = null;
                    return x;
                }
                sp<E> item = p->getItem();
                if (item != null) {
                    nextNode = p;
                    nextItem = item;
                    return x;
                } else {
                	// skip over nulls
                	sp<Node> next = queue->succ(p);
					if (pred != null && next != null)
						pred->casNext(p, next);
					p = next;
                }
            }
            //always not reach here.
            return null;
        }
    public:
        Itr(EConcurrentLinkedQueue* queue) {
        	this->queue = queue;
			advance();
		}

        boolean hasNext() {
            return nextNode != null;
        }

        sp<E> next() {
            if (nextNode == null) throw ENoSuchElementException(__FILE__, __LINE__);
            return advance();
        }

        void remove() {
            sp<Node> l = lastRet;
            if (l == null) throw EIllegalStateException(__FILE__, __LINE__);
            // rely on a future traversal to relink.
            l->setItem(null);
            lastRet = null;
        }

        sp<E> moveOut() {
        	throw EUnsupportedOperationException(__FILE__, __LINE__);
        }
    };

    ALWAYS_INLINE boolean casTail(sp<Node>& cmp, sp<Node>& val) {
    	return atomic_compare_exchange(&tail, &cmp, val);
    }

    ALWAYS_INLINE boolean casHead(sp<Node>& cmp, sp<Node>& val) {
    	return atomic_compare_exchange(&head, &cmp, val);
    }

    /**
	 * Tries to CAS head to p. If successful, repoint old head to itself
	 * as sentinel for succ(), below.
	 */
    ALWAYS_INLINE void updateHead(sp<Node>& h, sp<Node>& p) {
		if (h != p && casHead(h, p))
			h->lazySetNext(xxxx);
	}

    /**
	 * Returns the successor of p, or the head node if p.next has been
	 * linked to self, which will only be true if traversing with a
	 * stale pointer that is now off the list.
	 */
    ALWAYS_INLINE sp<Node> succ(sp<Node>& p) {
		sp<Node> next = p->getNext();
		return (next == xxxx) ? atomic_load(&head) : next;
	}
};

} /* namespace efc */
#endif /* ECONCURRENTLINKEDQUEUE_HH_ */
