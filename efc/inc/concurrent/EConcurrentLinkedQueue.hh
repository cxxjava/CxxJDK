/*
 * EConcurrentLinkedQueue.hh
 *
 *  Created on: 2014-6-7
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTLINKEDQUEUE_HH_
#define ECONCURRENTLINKEDQUEUE_HH_

#include "../EInteger.hh"
#include "../EArrayList.hh"
#include "./EConcurrentQueue.hh"
#include "../ENoSuchElementException.hh"
#include "../ENullPointerException.hh"
#include "../EIllegalStateException.hh"

namespace efc {

//@see bug: http://bugs.java.com/view_bug.do?bug_id=6785442

/**
 * An unbounded thread-safe {@linkplain Queue queue} based on linked nodes.
 * This queue orders elements FIFO (first-in-first-out).
 * The <em>head</em> of the queue is that element that has been on the
 * queue the longest time.
 * The <em>tail</em> of the queue is that element that has been on the
 * queue the shortest time. New elements
 * are inserted at the tail of the queue, and the queue retrieval
 * operations obtain elements at the head of the queue.
 * A <tt>ConcurrentLinkedQueue</tt> is an appropriate choice when
 * many threads will share access to a common collection.
 * This queue does not permit <tt>null</tt> elements.
 *
 * <p>This implementation employs an efficient &quot;wait-free&quot;
 * algorithm based on one described in <a
 * href="http://www.cs.rochester.edu/u/michael/PODC96.html"> Simple,
 * Fast, and Practical Non-Blocking and Blocking Concurrent Queue
 * Algorithms</a> by Maged M. Michael and Michael L. Scott.
 *
 * <p>Beware that, unlike in most collections, the <tt>size</tt> method
 * is <em>NOT</em> a constant-time operation. Because of the
 * asynchronous nature of these queues, determining the current number
 * of elements requires a traversal of the elements.
 *
 * <p>This class and its iterator implement all of the
 * <em>optional</em> methods of the {@link Collection} and {@link
 * Iterator} interfaces.
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
 *
 */

template<typename E>
class EConcurrentLinkedQueue: public EConcurrentQueue<E> {
private:
    /*
     * This is a straight adaptation of Michael & Scott algorithm.
     * For explanation, read the paper.  The only (minor) algorithmic
     * difference is that this version supports lazy deletion of
     * internal nodes (method remove(Object)) -- remove CAS'es item
     * fields to null. The normal queue operations unlink but then
     * pass over nodes with null item fields. Similarly, iteration
     * methods ignore those with nulls.
     *
     * Also note that like most non-blocking algorithms in this
     * package, this implementation relies on the fact that in garbage
     * collected systems, there is no possibility of ABA problems due
     * to recycled nodes, so there is no need to use "counted
     * pointers" or related techniques seen in versions used in
     * non-GC'ed settings.
     */

    class Node {
    public:
    	sp<E> item; //volatile?
        sp<Node> next; //volatile?

    public:
        Node(E* x) { item = x; }
        Node(sp<E> x) { item = x; }

        Node(E* x, Node* n) { item = x; next = n; }
        Node(sp<E> x, sp<Node> n) { item = x; next = n; }
        Node(es_nullptr_t x, es_nullptr_t n) { }

        sp<E> getItem() {
            return atomic_load(&item);
        }

        boolean casItem(sp<E> cmp, sp<E> val) {
        	return atomic_compare_exchange(&item, &cmp, val);
        }

        void setItem(E* val) {
        	sp<E> t(val);
        	atomic_store(&item, t);
        }
		void setItem(es_nullptr_t) {
			sp<E> t;
			atomic_store(&item, t);
		}
        void setItem(sp<E> val) {
        	atomic_store(&item, val);
        }
        
        sp<Node> getNext() {
        	return atomic_load(&next);
        }

        boolean casNext(sp<Node> cmp, sp<Node> val) {
        	return atomic_compare_exchange(&next, &cmp, val);
        }

        void setNext(Node* val) {
        	sp<Node> t(val);
        	atomic_store(&next, t);
        }
        void setNext(sp<Node> val) {
        	atomic_store(&next, val);
        }
        void setNext(es_nullptr_t) {
        	sp<Node> t;
			atomic_store(&next, t);
		}
    };

public:
    ~EConcurrentLinkedQueue() {
    	// node unlink for recursion
    	sp<Node> node = head;
    	while (node != null) {
    		sp<Node> next = node->getNext();
    		node->next = null;
    		node = next;
    	}
    }

    /**
     * Creates a <tt>ConcurrentLinkedQueue</tt> that is initially empty.
     */
    EConcurrentLinkedQueue() {
    	head = new Node(null, null);
    	tail = head;
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
		sp<Node> n(new Node(e));
		for (;;) {
			sp<Node> t = atomic_load(&tail);
			sp<Node> s = t->getNext();
			if (t == atomic_load(&tail)) {
				if (s == null) {
					if (t->casNext(s, n)) {
						casTail(t, n);
						return true;
					}
				} else {
					casTail(t, s);
				}
			}
		}
		//always not reach here.
		return true;
    }

    sp<E> poll() {
        for (;;) {
            sp<Node> h = atomic_load(&head);
            sp<Node> t = atomic_load(&tail);
            sp<Node> first = h->getNext();
            if (h == atomic_load(&head)) {
                if (h == t) {
                    if (first == null)
                        return null;
                    else
                        casTail(t, first);
                } else if (casHead(h, first)) {
                    sp<E> item = first->getItem();
                    if (item != null) {
                        h->setNext(null);
                        first->setItem(null);
                        return item;
                    }
                    // else skip over deleted item, continue loop,
                }
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

    sp<E> peek() { // same as poll except don't remove item
        for (;;) {
            sp<Node> h = atomic_load(&head);
            sp<Node> t = atomic_load(&tail);
            sp<Node> first = h->getNext();
            if (h == atomic_load(&head)) {
                if (h == t) {
                    if (first == null)
                        return null;
                    else
                        casTail(t, first);
                } else {
                    sp<E> item = first->getItem();
                    if (item != null) {
                    	first->setNext(null);
                        return item;
                    }
                    else // remove deleted node and continue
                        casHead(h, first);
                }
            }
        }
        //always not reach here.
        return null;
    }

    /**
     * Returns the first actual (non-header) node on list.  This is yet
     * another variant of poll/peek; here returning out the first
     * node, not element (so we cannot collapse with peek() without
     * introducing race.)
     */
    sp<Node> first() {
        for (;;) {
        	sp<Node> h = atomic_load(&head);
        	sp<Node> t = atomic_load(&tail);
        	sp<Node> first = h->getNext();
            if (h == atomic_load(&head)) {
                if (h == t) {
                    if (first == null)
                        return null;
                    else
                        casTail(t, first);
                } else {
                    if (first->getItem() != null)
                        return first;
                    else // remove deleted node and continue
                        casHead(h, first);
                }
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
        int count = 0;
        for (sp<Node> p = first(); p != null; p = p->getNext()) {
            if (p->getItem() != null) {
                // Collections.size() spec says to max out
                if (++count == EInteger::MAX_VALUE)
                    break;
            }
        }
        return count;
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
        if (o == null) return false;
        for (sp<Node> p = first(); p != null; p = p->getNext()) {
            sp<E> item = p->getItem();
            if (item != null &&
                o->equals(item.get()))
                return true;
        }
        return false;
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
        if (o == null) return false;
        sp<Node> pred = null;
        for (sp<Node> p = first(); p != null; p = p->getNext()) {
            sp<E> item = p->getItem();
            if (item != null &&
                o->equals(item.get()) &&
                p->casItem(item, null)) {
            	//@see: java8
            	sp<Node> next = p->getNext();
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

private:
    /**
	 * Pointer to header node, initialized to a dummy node.  The first
	 * actual node is at head.getNext().
	 */
	sp<Node> head;//(new Node(null, null));

	/** Pointer to last node on list **/
	sp<Node> tail;// = head;

    class Itr : public EConcurrentIterator<E> {
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

            sp<Node> p = (nextNode == null) ? queue->first() : nextNode->getNext();
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
                } else // skip over nulls
                    p = p->getNext();
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
    };

    boolean casTail(sp<Node>& cmp, sp<Node>& val) {
    	return atomic_compare_exchange(&tail, &cmp, val);
    }

    boolean casHead(sp<Node>& cmp, sp<Node>& val) {
    	return atomic_compare_exchange(&head, &cmp, val);
    }
};

} /* namespace efc */
#endif /* ECONCURRENTLINKEDQUEUE_HH_ */
