/*
 * EMutexLinkedQueue.hh
 *
 *  Created on: 2018-1-10
 *      Author: cxxjava@163.com
 */

#ifndef MUTEXLINKEDQUEUE_HH_
#define MUTEXLINKEDQUEUE_HH_

#include "../EInteger.hh"
#include "../EAbstractQueue.hh"
#include "../ENoSuchElementException.hh"
#include "../ENullPointerException.hh"
#include "../EIllegalStateException.hh"
#include "../EIndexOutOfBoundsException.hh"

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
class EMutexLinkedQueue : public EAbstractQueue<sp<E> >,
        virtual public EQueue<sp<E> > {
private:
	struct Node {
		sp<E> item;
		sp<Node> next;
		Node() {}
		Node(sp<E> item) : item(item) {}
	};
public:
	virtual ~EMutexLinkedQueue() {
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
    EMutexLinkedQueue() : size_(0) {
        head = tail = new Node();
    }

    /**
     * Creates a {@code ConcurrentLinkedQueue}
     * initially containing the elements of the given collection,
     * added in traversal order of the collection's iterator.
     *
     * @param c the collection of elements to initially contain
     * @throws NullPointerException if the specified collection or any
     *         of its elements are null
     */
    EMutexLinkedQueue(ECollection<sp<E> >* c) {
    	head = tail = new Node();
        sp<EIterator<sp<E> > > it = c->iterator();
        while (it->hasNext()) {
        	sp<E> e = it->next();
        	checkNotNull(e);
			sp<Node> newNode = new Node(e);
			tail->next = newNode;
			tail = newNode;

			size_++;
        }
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
        checkNotNull(e);

        sp<Node> newNode = new Node(e);

        SYNCBLOCK(&lock) {
			tail->next = newNode;
			tail = newNode;

        	size_++;
        }}

    	return true;
    }

    sp<E> poll() {
    	SYNCBLOCK(&lock) {
    		sp<Node> first = head->next;
    		if (first == null) {
    			return null;
    		} else {
				sp<E> item = first->item;
				first->item = null;
                head->next = null;
				head = first;

				size_--;

				return item;
    		}
    	}}
    }

    sp<E> peek() {
    	SYNCBLOCK(&lock) {
    		sp<Node> first = head->next;
			if (first == null) {
				return null;
			} else {
				return first->item;
			}
		}}
    }

    /**
     * Returns {@code true} if this queue contains no elements.
     *
     * @return {@code true} if this queue contains no elements
     */
    boolean isEmpty() {
    	SYNCBLOCK(&lock) {
    		return (size_ == 0);
    	}}
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
    	SYNCBLOCK(&lock) {
    		return size_;
		}}
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
        SYNCBLOCK(&lock) {
        	sp<Node> first = head->next;
			for (sp<Node> p = first; p != null; p = p->next) {
				if (p->item->equals(o))
					return true;
			}
        }}
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
        SYNCBLOCK(&lock) {
        	sp<Node> pred = head;
        	sp<Node> first = pred->next;
			for (sp<Node> p = first; p != null; p = p->next) {
				if (p->item->equals(o)) {
					p->item = null;
					pred->next = p->next;

					size_--;

					return true;
				}
				pred = p;
			}
        }}
    	return false;
    }

    virtual sp<E> remove() {
    	return EAbstractQueue<sp<E> >::remove();
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
    	SYNCBLOCK(&lock) {
    		sp<Node> first = head->next;
			for (sp<Node> p = first; p != null; p = p->next) {
				al.add(p->item);
			}
    	}}
        return al.toArray();
    }

    /**
     * Returns an iterator over the elements in this queue in proper sequence.
     * The elements will be returned in order from first (head) to last (tail).
     *
     * <p>The returned iterator is
     * <a href="package-summary.html#Weakly"><i>weakly consistent</i></a>.
     *
     * @return an iterator over the elements in this queue in proper sequence
     *
     * @example !!!HOW TO USE IT!!!
     *
     *	1:
     *
     *	{ // Add mutex scope !!!
     *		sp<EIterator<sp<E> > > iter = queue.iterator();
     *		while (iter->hasNext()) {
     *			...
     *		}
     *	}
     *
     *	2:
     *
     *	for (sp<EIterator<sp<E> > > iter = queue.iterator(); iter->hasNext();) {
     *		...
     *	}
     *
     */
    sp<EIterator<sp<E> > > iterator(int index=0) {
    	return new Itr(this, index);
    }

private:
    EReentrantLock lock;
    int size_;

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
    /*volatile*/ sp<Node> head;

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
    /*volatile*/ sp<Node> tail;

    /**
     * Throws NullPointerException if argument is null.
     *
     * @param v the element
     */
    static void checkNotNull(sp<E> v) {
        if (v == null)
            throw ENullPointerException(__FILE__, __LINE__);
    }

    class Itr : public EIterator<sp<E> > {
	private:
    	EMutexLinkedQueue* queue;

    	/**
		 * Node of the prev node and the next node to return item for.
		 */
    	sp<Node> prevNode;

	public:
    	virtual ~Itr() {
    		queue->lock.unlock();
    	}
		Itr(EMutexLinkedQueue* queue, int index) : queue(queue) {
			queue->lock.lock();

			if (index >= queue->size_) {
				throw EIndexOutOfBoundsException(__FILE__, __LINE__);
			}

			prevNode = queue->head;
			for (int i=0; i<index; i++) {
				prevNode = prevNode->next;
			}
		}

		boolean hasNext() {
			return prevNode->next != null;
		}

		sp<E> next() {
			if (prevNode->next == null) throw ENoSuchElementException(__FILE__, __LINE__);
			sp<E> e = prevNode->next->item;
			prevNode = prevNode->next;
			return e;
		}

		void remove() {
			if (prevNode->next != null) {
				prevNode->next->item = null;
				prevNode->next = prevNode->next->next;

				queue->size_--;
			}
		}

		sp<E> moveOut() {
			sp<E> e;
			if (prevNode->next != null) {
				e = prevNode->next->item;
				prevNode->next->item = null;
				prevNode->next = prevNode->next->next;

				queue->size_--;
			}
			return e;
		}
	};
};

} /* namespace efc */
#endif /* MUTEXLINKEDQUEUE_HH_ */
