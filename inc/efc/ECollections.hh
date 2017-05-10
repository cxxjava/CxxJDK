/*
 * ECollections.hh
 *
 *  Created on: 2013-8-21
 *      Author: cxxjava@163.com
 */

#ifndef ECOLLECTIONS_HH_
#define ECOLLECTIONS_HH_

#include "EComparable.hh"
#include "EComparator.hh"
#include "ERandom.hh"
#include "EIterator.hh"
#include "ECollection.hh"
#include "ETraits.hh"
#include "EList.hh"
#include "ESortedMap.hh"
#include "ENullPointerException.hh"

namespace efc {

/**
 * This class consists exclusively of static methods that operate on or return
 * collections.  It contains polymorphic algorithms that operate on
 * collections, "wrappers", which return a new collection backed by a
 * specified collection, and a few other odds and ends.
 *
 * <p>The methods of this class all throw a <tt>NullPointerException</tt>
 * if the collections or class objects provided to them are null.
 *
 * <p>The documentation for the polymorphic algorithms contained in this class
 * generally includes a brief description of the <i>implementation</i>.  Such
 * descriptions should be regarded as <i>implementation notes</i>, rather than
 * parts of the <i>specification</i>.  Implementors should feel free to
 * substitute other algorithms, so long as the specification itself is adhered
 * to.  (For example, the algorithm used by <tt>sort</tt> does not have to be
 * a mergesort, but it does have to be <i>stable</i>.)
 *
 * <p>The "destructive" algorithms contained in this class, that is, the
 * algorithms that modify the collection on which they operate, are specified
 * to throw <tt>UnsupportedOperationException</tt> if the collection does not
 * support the appropriate mutation primitive(s), such as the <tt>set</tt>
 * method.  These algorithms may, but are not required to, throw this
 * exception if an invocation would have no effect on the collection.  For
 * example, invoking the <tt>sort</tt> method on an unmodifiable list that is
 * already sorted may or may not throw <tt>UnsupportedOperationException</tt>.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @version 1.106, 04/21/06
 * @see	    Collection
 * @see	    Set
 * @see	    List
 * @see	    Map
 * @since   1.2
 */

/**
 * Tuning parameter: list size at or below which insertion sort will be
 * used in preference to mergesort or quicksort.
 */
#define INSERTIONSORT_THRESHOLD 7

class ECollections {
public:

	//List

	/**
	 * Sorts the specified list according to the order induced by the
	 * specified comparator.  All elements in the list must be <i>mutually
	 * comparable</i> using the specified comparator (that is,
	 * <tt>c.compare(e1, e2)</tt> must not throw a <tt>ClassCastException</tt>
	 * for any elements <tt>e1</tt> and <tt>e2</tt> in the list).<p>
	 *
	 * This sort is guaranteed to be <i>stable</i>:  equal elements will
	 * not be reordered as a result of the sort.<p>
	 *
	 * The sorting algorithm is a modified mergesort (in which the merge is
	 * omitted if the highest element in the low sublist is less than the
	 * lowest element in the high sublist).  This algorithm offers guaranteed
	 * n log(n) performance.
	 *
	 * The specified list must be modifiable, but need not be resizable.
	 * This implementation dumps the specified list into an array, sorts
	 * the array, and iterates over the list resetting each element
	 * from the corresponding position in the array.  This avoids the
	 * n<sup>2</sup> log(n) performance that would result from attempting
	 * to sort a linked list in place.
	 *
	 * @param  list the list to be sorted.
	 * @param  c the comparator to determine the order of the list.  A
	 *        <tt>null</tt> value indicates that the elements' <i>natural
	 *        ordering</i> should be used.
	 * @throws ClassCastException if the list contains elements that are not
	 *	       <i>mutually comparable</i> using the specified comparator.
	 * @throws UnsupportedOperationException if the specified list's
	 *	       list-iterator does not support the <tt>set</tt> operation.
	 * @see Comparator
	 */
	template<typename E>
	static void sort(EList<E> *list,
			EComparator<E> *comp=null, int off = 0, int len = -1) {
		if (len < 0) {
			len = list->size();
		}

		if (comp) {
			// Insertion sort on smallest arrays
			if (len < INSERTIONSORT_THRESHOLD) {
				for (int i = off; i < len + off; i++)
					for (int j = i;
							j > off
									&& comp->compare(list->getAt(j - 1),
											list->getAt(j)) > 0; j--)
						swap(list, j, j - 1);
				return;
			}

			// Choose a partition element, v
			int m = off + (len >> 1); // Small arrays, middle element
			if (len > INSERTIONSORT_THRESHOLD) {
				int l = off;
				int n = off + len - 1;
				if (len > 40) { // Big arrays, pseudomedian of 9
					int s = len / 8;
					l = med3(list, comp, l, l + s, l + 2 * s);
					m = med3(list, comp, m - s, m, m + s);
					n = med3(list, comp, n - 2 * s, n - s, n);
				}
				m = med3(list, comp, l, m, n); // Mid-size, med of 3
			}
			E v = list->getAt(m);

			// Establish Invariant: v* (<v)* (>v)* v*
			int a = off, b = a, c = off + len - 1, d = c;
			int rbv, rcv;
			while (true ) {
				while (b <= c && (rbv = comp->compare(list->getAt(b), v)) <= 0) {
					if (rbv == 0)
						swap(list, a++, b);
					b++;
				}

				while (c >= b && (rcv = comp->compare(list->getAt(c), v)) >= 0) {
					if (rcv == 0)
						swap(list, c, d--);
					c--;
				}
				if (b > c)
					break;
				swap(list, b++, c--);
			}

			// Swap partition elements back to middle
			int s, n = off + len;
			s = ES_MIN(a - off, b-a );
			vecswap(list, off, b - s, s);
			s = ES_MIN(d-c, n-d-1);
			vecswap(list, b, n - s, s);

			// Recursively sort non-partition-elements
			if ((s = b - a) > 1)
				sort(list, comp, off, s);
			if ((s = d - c) > 1)
				sort(list, comp, n - s, s);
		}
		else { //!comp
			// Insertion sort on smallest arrays
			if (len < INSERTIONSORT_THRESHOLD) {
				for (int i = off; i < len + off; i++) {
					for (int j = i;
							j > off
//							&& (list->getAt(j - 1))->compareTo(list->getAt(j)) > 0;
							&& compare((list->getAt(j - 1)), list->getAt(j)) > 0;
							j--)
						swap(list, j, j - 1);
                }
				return;
			}

			// Choose a partition element, v
			int m = off + (len >> 1); // Small arrays, middle element
			if (len > INSERTIONSORT_THRESHOLD) {
				int l = off;
				int n = off + len - 1;
				if (len > 40) { // Big arrays, pseudomedian of 9
					int s = len / 8;
					l = med3(list, l, l + s, l + 2 * s);
					m = med3(list, m - s, m, m + s);
					n = med3(list, n - 2 * s, n - s, n);
				}
				m = med3(list, l, m, n); // Mid-size, med of 3
			}
			E v = list->getAt(m);

			// Establish Invariant: v* (<v)* (>v)* v*
			int a = off, b = a, c = off + len - 1, d = c;
			int rbv, rcv;
			while (true ) {
//				while (b <= c && (rbv = (list->getAt(b))->compareTo(v)) <= 0) {
				while (b <= c && (rbv = compare((list->getAt(b)), v)) <= 0) {
					if (rbv == 0)
						swap(list, a++, b);
					b++;
				}

//				while (c >= b && (rcv = (list->getAt(c))->compareTo(v)) >= 0) {
				while (c >= b && (rcv = compare((list->getAt(c)), v)) >= 0) {
					if (rcv == 0)
						swap(list, c, d--);
					c--;
				}
				if (b > c)
					break;
				swap(list, b++, c--);
			}

			// Swap partition elements back to middle
			int s, n = off + len;
			s = ES_MIN(a - off, b-a );
			vecswap(list, off, b - s, s);
			s = ES_MIN(d-c, n-d-1);
			vecswap(list, b, n - s, s);

			// Recursively sort non-partition-elements
			if ((s = b - a) > 1)
				sort(list, comp, off, s);
			if ((s = d - c) > 1)
				sort(list, comp, n - s, s);
		}
	}

	/**
	 * Randomly permutes the specified list using a default source of
	 * randomness.  All permutations occur with approximately equal
	 * likelihood.<p>
	 *
	 * The hedge "approximately" is used in the foregoing description because
	 * default source of randomness is only approximately an unbiased source
	 * of independently chosen bits. If it were a perfect source of randomly
	 * chosen bits, then the algorithm would choose permutations with perfect
	 * uniformity.<p>
	 *
	 * This implementation traverses the list backwards, from the last element
	 * up to the second, repeatedly swapping a randomly selected element into
	 * the "current position".  Elements are randomly selected from the
	 * portion of the list that runs from the first element to the current
	 * position, inclusive.<p>
	 *
	 * This method runs in linear time.  If the specified list does not
	 * implement the {@link RandomAccess} interface and is large, this
	 * implementation dumps the specified list into an array before shuffling
	 * it, and dumps the shuffled array back into the list.  This avoids the
	 * quadratic behavior that would result from shuffling a "sequential
	 * access" list in place.
	 *
	 * @param  list the list to be shuffled.
	 * @throws UnsupportedOperationException if the specified list or
	 *         its list-iterator does not support the <tt>set</tt> operation.
	 */
	template<typename E>
	static void shuffle(EList<E> *list) {
		ERandom rnd;
		int size = list->size();
		for (int i=size; i>1; i--)
			swap(list, i-1, rnd.nextInt(i));
	}

	/**
	 * Reverses the order of the elements in the specified list.<p>
	 *
	 * This method runs in linear time.
	 *
	 * @param  list the list whose elements are to be reversed.
	 * @throws UnsupportedOperationException if the specified list or
	 *         its list-iterator does not support the <tt>set</tt> operation.
	 */
	template<typename E>
	static void reverse(EList<E> *list) {
		int size = list->size();
		for (int i = 0, mid = size >> 1, j = size - 1; i < mid; i++, j--)
			swap(list, i, j);
	}

	/**
	 * Swaps the elements at the specified positions in the specified list.
	 * (If the specified positions are equal, invoking this method leaves
	 * the list unchanged.)
	 *
	 * @param list The list in which to swap elements.
	 * @param i the index of one element to be swapped.
	 * @param j the index of the other element to be swapped.
	 * @throws IndexOutOfBoundsException if either <tt>i</tt> or <tt>j</tt>
	 *         is out of range (i &lt; 0 || i &gt;= list.size()
	 *         || j &lt; 0 || j &gt;= list.size()).
	 * @since 1.4
	 */
	template<typename E>
	static void swap(EList<E> *list, int i, int j) {
		if (i == j) {
			return;
		}
		list->setAt(i, list->setAt(j, list->getAt(i)));
	}

	//Collection

	/**
	 * Returns the maximum element of the given collection, according to the
	 * order induced by the specified comparator.  All elements in the
	 * collection must be <i>mutually comparable</i> by the specified
	 * comparator (that is, <tt>comp.compare(e1, e2)</tt> must not throw a
	 * <tt>ClassCastException</tt> for any elements <tt>e1</tt> and
	 * <tt>e2</tt> in the collection).<p>
	 *
	 * This method iterates over the entire collection, hence it requires
	 * time proportional to the size of the collection.
	 *
	 * @param  coll the collection whose maximum element is to be determined.
	 * @param  comp the comparator with which to determine the maximum element.
	 *         A <tt>null</tt> value indicates that the elements' <i>natural
	 *        ordering</i> should be used.
	 * @return the maximum element of the given collection, according
	 *         to the specified comparator.
	 * @throws ClassCastException if the collection contains elements that are
	 *	       not <i>mutually comparable</i> using the specified comparator.
	 * @throws NoSuchElementException if the collection is empty.
	 * @see Comparable
	 */
	template<typename E>
	static E max(ECollection<E> *coll,
			EComparator<E> *comp) {
		sp<EIterator<E> > i = coll->iterator();
		E candidate = i->next();

		while (i->hasNext()) {
			E next = i->next();
			if (comp->compare(next, candidate) > 0)
				candidate = next;
		}
		return candidate;
	}

	/**
	 * Returns the minimum element of the given collection, according to the
	 * order induced by the specified comparator.  All elements in the
	 * collection must be <i>mutually comparable</i> by the specified
	 * comparator (that is, <tt>comp.compare(e1, e2)</tt> must not throw a
	 * <tt>ClassCastException</tt> for any elements <tt>e1</tt> and
	 * <tt>e2</tt> in the collection).<p>
	 *
	 * This method iterates over the entire collection, hence it requires
	 * time proportional to the size of the collection.
	 *
	 * @param  coll the collection whose minimum element is to be determined.
	 * @param  comp the comparator with which to determine the minimum element.
	 *         A <tt>null</tt> value indicates that the elements' <i>natural
	 *         ordering</i> should be used.
	 * @return the minimum element of the given collection, according
	 *         to the specified comparator.
	 * @throws ClassCastException if the collection contains elements that are
	 *	       not <i>mutually comparable</i> using the specified comparator.
	 * @throws NoSuchElementException if the collection is empty.
	 * @see Comparable
	 */
	template<typename E>
	static E min(ECollection<E> *coll,
			EComparator<E> *comp) {
		sp<EIterator<E> > i = coll->iterator();
		E candidate = i->next();

		while (i->hasNext()) {
			E next = i->next();
			if (comp->compare(next, candidate) < 0)
				candidate = next;
		}
		return candidate;
	}

	// Synch Wrappers

	/**
	 * Returns a synchronized (thread-safe) collection backed by the specified
	 * collection.  In order to guarantee serial access, it is critical that
	 * <strong>all</strong> access to the backing collection is accomplished
	 * through the returned collection.<p>
	 *
	 * It is imperative that the user manually synchronize on the returned
	 * collection when iterating over it:
	 * <pre>
	 *  Collection c = Collections.synchronizedCollection(myCollection);
	 *     ...
	 *  synchronized(c) {
	 *      Iterator i = c.iterator(); // Must be in the synchronized block
	 *      while (i.hasNext())
	 *         foo(i.next());
	 *  }
	 * </pre>
	 * Failure to follow this advice may result in non-deterministic behavior.
	 *
	 * <p>The returned collection does <i>not</i> pass the <tt>hashCode</tt>
	 * and <tt>equals</tt> operations through to the backing collection, but
	 * relies on <tt>Object</tt>'s equals and hashCode methods.  This is
	 * necessary to preserve the contracts of these operations in the case
	 * that the backing collection is a set or a list.<p>
	 *
	 * The returned collection will be serializable if the specified collection
	 * is serializable.
	 *
	 * @param  c the collection to be "wrapped" in a synchronized collection.
	 * @return a synchronized view of the specified collection.
	 */
	template<typename E>
	class SynchronizedCollection : public ECollection<E> {
	public:
		SynchronizedCollection(ECollection<E>* c, ELock* lock,
				boolean autoFree) :
					autoFree_(autoFree) {
			if (c == null)
				throw ENULLPOINTEREXCEPTION;

			collection_ = c;

			if (lock) {
				lock_ = lock;
				lockNeedFree_ = false;
			}
			else {
				lock_ = new EReentrantLock();
				lockNeedFree_ = true;
			}
		}
		~SynchronizedCollection() {
			if (lockNeedFree_) {
				delete lock_;
			}
			if (autoFree_) {
				delete collection_;
			}
		}
		int size() {
            SYNCBLOCK(lock_) {return collection_->size();}}
		}
		boolean isEmpty() {
            SYNCBLOCK(lock_) {return collection_->isEmpty();}}
		}
		boolean contains(E o) {
            SYNCBLOCK(lock_) {return collection_->contains(o);}}
		}
		sp<EIterator<E> > iterator(int index=0) {
			return collection_->iterator(index); // Must be manually synched and freed by user!
		}
		boolean add(E e) {
            SYNCBLOCK(lock_) {return collection_->add(e);}}
		}
		boolean remove(E o) {
            SYNCBLOCK(lock_) {return collection_->remove(o);}}
		}
		boolean containsAll(ECollection<E> *c) {
            SYNCBLOCK(lock_) {return collection_->containsAll(c);}}
		}
		boolean removeAll(ECollection<E> *c) {
            SYNCBLOCK(lock_) {return collection_->removeAll(c);}}
		}
		boolean retainAll(ECollection<E> *c) {
            SYNCBLOCK(lock_) {return collection_->retainAll(c);}}
		}
		void clear() {
            SYNCBLOCK(lock_) {collection_->clear();}}
		}
		EStringBase toString() {
            SYNCBLOCK(lock_) {return collection_->toString();}}
		}

	protected:
		ECollection<E>* collection_;
		ELock* lock_;
		boolean lockNeedFree_;
		boolean autoFree_;
	};

	template<typename E>
	static ECollection<E>* synchronizedCollection(
			ECollection<E>* c, ELock* lock = null,
			boolean autoFree = true ) {
		return new SynchronizedCollection<E>(c, lock, autoFree);
	}

	/**
	 * Returns a synchronized (thread-safe) set backed by the specified
	 * set.  In order to guarantee serial access, it is critical that
	 * <strong>all</strong> access to the backing set is accomplished
	 * through the returned set.<p>
	 *
	 * It is imperative that the user manually synchronize on the returned
	 * set when iterating over it:
	 * <pre>
	 *  Set s = Collections.synchronizedSet(new HashSet());
	 *      ...
	 *  synchronized(s) {
	 *      Iterator i = s.iterator(); // Must be in the synchronized block
	 *      while (i.hasNext())
	 *          foo(i.next());
	 *  }
	 * </pre>
	 * Failure to follow this advice may result in non-deterministic behavior.
	 *
	 * <p>The returned set will be serializable if the specified set is
	 * serializable.
	 *
	 * @param  s the set to be "wrapped" in a synchronized set.
	 * @return a synchronized view of the specified set.
	 */
	template<typename E>
	class SynchronizedSet: public SynchronizedCollection<E>,
			public ESet<E> {
	public:
		SynchronizedSet(ESet<E>* s, ELock* mutex=null, boolean autoFree=true) :
			SynchronizedCollection<E>(s, mutex, autoFree) {
		}
		int size() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->size();}}
		}
		boolean isEmpty() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->isEmpty();}}
		}
		boolean contains(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->contains(o);}}
		}
		sp<EIterator<E> > iterator(int index=0) {
			return SynchronizedCollection<E>::collection_->iterator(index); // Must be manually synched and freed by user!
		}
		boolean add(E e) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->add(e);}}
		}
		boolean remove(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->remove(o);}}
		}
		boolean containsAll(ECollection<E> *c) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->containsAll(c);}}
		}
		boolean retainAll(ECollection<E> *c) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->retainAll(c);}}
		}
		boolean removeAll(ECollection<E> *c) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->removeAll(c);}}
		}
		void clear() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {SynchronizedCollection<E>::collection_->clear();}}
		}
		boolean equals(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->equals(o);}}
		}
		int hashCode() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->hashCode();}}
		}
	};

	template<typename E>
	static ESet<E>* synchronizedSet(ESet<E>* s, ELock* mutex=null, boolean autoFree=true) {
		return new SynchronizedSet<E>(s, mutex, autoFree);
	}

	/**
	 * Returns a synchronized (thread-safe) list backed by the specified
	 * list.  In order to guarantee serial access, it is critical that
	 * <strong>all</strong> access to the backing list is accomplished
	 * through the returned list.<p>
	 *
	 * It is imperative that the user manually synchronize on the returned
	 * list when iterating over it:
	 * <pre>
	 *  List list = Collections.synchronizedList(new ArrayList());
	 *      ...
	 *  synchronized(list) {
	 *      Iterator i = list.iterator(); // Must be in synchronized block
	 *      while (i.hasNext())
	 *          foo(i.next());
	 *  }
	 * </pre>
	 * Failure to follow this advice may result in non-deterministic behavior.
	 *
	 * <p>The returned list will be serializable if the specified list is
	 * serializable.
	 *
	 * @param  list the list to be "wrapped" in a synchronized list.
	 * @return a synchronized view of the specified list.
	 */
	template<typename E>
	class SynchronizedList : public SynchronizedCollection<E>,
			public EList<E> {
	public:
		EList<E>* list;

		SynchronizedList(EList<E>* list, ELock* mutex = null, boolean autoFree = true ) :
			SynchronizedCollection<E>(list, mutex, autoFree) {
			this->list = list;
		}

		boolean equals(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return list->equals(o);}}
		}
		int hashCode() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return list->hashCode();}}
		}

		E getAt(int index) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return list->getAt(index);}}
		}
		E setAt(int index, E element) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return list->setAt(index, element);}}
		}
		void addAt(int index, E element) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {list->addAt(index, element);}}
		}
		E removeAt(int index) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return list->removeAt(index);}}
		}

		int indexOf(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return list->indexOf(o);}}
		}
		int lastIndexOf(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return list->lastIndexOf(o);}}
		}

		int size() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->size();}}
		}
		boolean isEmpty() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->isEmpty();}}
		}
		boolean contains(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->contains(o);}}
		}
		sp<EIterator<E> > iterator(int index=0) {
			return SynchronizedCollection<E>::collection_->iterator(index); // Must be manually synched and freed by user!
		}
		boolean add(E e) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->add(e);}}
		}
		boolean remove(E o) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->remove(o);}}
		}
		boolean containsAll(ECollection<E> *c) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->containsAll(c);}}
		}
		boolean removeAll(ECollection<E> *c) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->removeAll(c);}}
		}
		boolean retainAll(ECollection<E> *c) {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {return SynchronizedCollection<E>::collection_->retainAll(c);}}
		}
		void clear() {
            SYNCBLOCK(SynchronizedCollection<E>::lock_) {SynchronizedCollection<E>::collection_->clear();}}
		}
		sp<EListIterator<E> > listIterator(int index = 0) {
			return list->listIterator(index); // Must be manually synched by user
		}
	};

	template<typename E>
	static EList<E>* synchronizedList(EList<E>* list, ELock* mutex=null, boolean autoFree=true) {
		return new SynchronizedList<E>(list, mutex, autoFree);
	}

	/**
	 * Returns a synchronized (thread-safe) map backed by the specified
	 * map.  In order to guarantee serial access, it is critical that
	 * <strong>all</strong> access to the backing map is accomplished
	 * through the returned map.<p>
	 *
	 * It is imperative that the user manually synchronize on the returned
	 * map when iterating over any of its collection views:
	 * <pre>
	 *  Map m = Collections.synchronizedMap(new HashMap());
	 *      ...
	 *  Set s = m.keySet();  // Needn't be in synchronized block
	 *      ...
	 *  synchronized(m) {  // Synchronizing on m, not s!
	 *      Iterator i = s.iterator(); // Must be in synchronized block
	 *      while (i.hasNext())
	 *          foo(i.next());
	 *  }
	 * </pre>
	 * Failure to follow this advice may result in non-deterministic behavior.
	 *
	 * <p>The returned map will be serializable if the specified map is
	 * serializable.
	 *
	 * @param  m the map to be "wrapped" in a synchronized map.
	 * @return a synchronized view of the specified map.
	 */
	template<typename K, typename V>
	class SynchronizedMap : public EMap<K,V> {
	private:
		EMap<K,V>* m;     // Backing Map
		ELock*  mutex;        // Object on which to synchronize
		boolean needFreeLock;
		boolean autoFree;

		sp<ESet<K> > keySet_;// = null;
		sp<ESet<EMapEntry<K,V>*> > entrySet_;// = null;
		sp<ECollection<V> > values_;// = null;

	public:
		SynchronizedMap(EMap<K, V>* m, ELock* mutex = null,
				boolean autoFree = true) : keySet_(null), entrySet_(null), values_(null) {
			if (m==null)
				throw ENULLPOINTEREXCEPTION;
			this->m = m;
			if (mutex) {
				this->mutex = mutex;
				this->needFreeLock = false;
			}
			else {
				this->mutex = new EReentrantLock();
				this->needFreeLock = true;
			}
			this->autoFree = autoFree;
		}
		~SynchronizedMap() {
			if (needFreeLock) {
				delete mutex;
			}
			if (autoFree) {
				delete m;
			}
		}

		int size() {
            SYNCBLOCK(mutex) {return m->size();}}
		}
		boolean isEmpty() {
            SYNCBLOCK(mutex) {return m->isEmpty();}}
		}
		boolean containsKey(K key) {
            SYNCBLOCK(mutex) {return m->containsKey(key);}}
		}
		boolean containsValue(V value) {
            SYNCBLOCK(mutex) {return m->containsValue(value);}}
		}
		V get(K key) {
            SYNCBLOCK(mutex) {return m->get(key);}}
		}
		V put(K key, V value, boolean *absent=null) {
            SYNCBLOCK(mutex) {return m->put(key, value, absent);}}
		}
		V remove(K key) {
            SYNCBLOCK(mutex) {return m->remove(key);}}
		}
		void clear() {
            SYNCBLOCK(mutex) {m->clear();}}
		}

		sp<ESet<K> > keySet() {
			SYNCBLOCK(mutex) {
				if (keySet_==null)
					keySet_ = new SynchronizedSet<K>(m->keySet().get(), mutex, false);
				return keySet_;
            }}
		}

		sp<ESet<EMapEntry<K,V>*> > entrySet() {
			SYNCBLOCK(mutex) {
				if (entrySet_==null)
					entrySet_ = new SynchronizedSet<EMapEntry<K,V>*>(m->entrySet().get(), mutex, false);
				return entrySet_;
            }}
		}

		sp<ECollection<V> > values() {
			SYNCBLOCK(mutex) {
				if (values_==null)
					values_ = new SynchronizedCollection<V>(m->values().get(), mutex, false);
				return values_;
            }}
		}
		boolean equals(EMap<K,V>* o) {
            SYNCBLOCK(mutex) {return m->equals(o);}}
		}
		int hashCode() {
            SYNCBLOCK(mutex) {return m->hashCode();}}
		}
		EStringBase toString() {
            SYNCBLOCK(mutex) {return m->toString();}}
		}

		/**
		 * Auto free.
		 */
		void setAutoFree(boolean autoFreeKey, boolean autoFreeValue) {
            SYNCBLOCK(mutex) {m->setAutoFree(autoFreeKey, autoFreeValue);}}
		}

		boolean getAutoFreeKey() {
            SYNCBLOCK(mutex) {return m->getAutoFreeKey();}}
		}

		boolean getAutoFreeValue() {
            SYNCBLOCK(mutex) {return m->getAutoFreeValue();}}
		}
	};

	template<typename K, typename V>
	static EMap<K, V>* synchronizedMap(
			EMap<K, V>* m, ELock* mutex = null,
			boolean autoFree = true ) {
		return new SynchronizedMap<K,V>(m, mutex, autoFree);
	}

	// Unmodifiable Wrappers

	/**
	 * Returns an unmodifiable view of the specified collection.  This method
	 * allows modules to provide users with "read-only" access to internal
	 * collections.  Query operations on the returned collection "read through"
	 * to the specified collection, and attempts to modify the returned
	 * collection, whether direct or via its iterator, result in an
	 * <tt>UnsupportedOperationException</tt>.<p>
	 *
	 * The returned collection does <i>not</i> pass the hashCode and equals
	 * operations through to the backing collection, but relies on
	 * <tt>Object</tt>'s <tt>equals</tt> and <tt>hashCode</tt> methods.  This
	 * is necessary to preserve the contracts of these operations in the case
	 * that the backing collection is a set or a list.<p>
	 *
	 * The returned collection will be serializable if the specified collection
	 * is serializable.
	 *
	 * @param  c the collection for which an unmodifiable view is to be
	 *         returned.
	 * @return an unmodifiable view of the specified collection.
	 */
	/**
	 * @serial include
	 */
	template<typename E>
	class UnmodifiableCollection : public ECollection<E> {
	protected:
		ECollection<E>* c;

	public:
		UnmodifiableCollection(ECollection<E>* c) {
			if (c==null)
				throw ENullPointerException(__FILE__, __LINE__);
			this->c = c;
		}

		int size()                   {return c->size();}
		boolean isEmpty()            {return c->isEmpty();}
		boolean contains(E o) {return c->contains(o);}
		EStringBase toString()           {return c->toString();}

		sp<EIterator<E> > iterator(int index=0) {
			class UnmodifiableIterator : public EIterator<E> {
			private:
				sp<EIterator<E> > i;
			public:
				UnmodifiableIterator(ECollection<E>* c) {
					i = c->iterator();
				}
				~UnmodifiableIterator() {
					//
				}
				boolean hasNext() {return i->hasNext();}
				E next()          {return i->next();}
				void remove() {
					throw EUnsupportedOperationException(__FILE__, __LINE__);
				}
				E moveOut() {
					throw EUnsupportedOperationException(__FILE__, __LINE__);
				}
			};
			return new UnmodifiableIterator(c);
		}

		boolean add(E e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean remove(E o) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

		boolean containsAll(ECollection<E> *coll) {
			return c->containsAll(coll);
		}
		boolean addAll(ECollection<E>* coll) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean removeAll(ECollection<E>* coll) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean retainAll(ECollection<E>* coll) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		void clear() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};
	template<typename T>
	static ECollection<T>* unmodifiableCollection(ECollection<T>* c) {
		return new UnmodifiableCollection<T>(c);
	}

	/**
	 * Returns an unmodifiable view of the specified set.  This method allows
	 * modules to provide users with "read-only" access to internal sets.
	 * Query operations on the returned set "read through" to the specified
	 * set, and attempts to modify the returned set, whether direct or via its
	 * iterator, result in an <tt>UnsupportedOperationException</tt>.<p>
	 *
	 * The returned set will be serializable if the specified set
	 * is serializable.
	 *
	 * @param  s the set for which an unmodifiable view is to be returned.
	 * @return an unmodifiable view of the specified set.
	 */
	/**
	 * @serial include
	 */
	template<typename E>
	class UnmodifiableSet: public UnmodifiableCollection<E>, public ESet<E> {
	public:
		UnmodifiableSet(ESet<E>* s) : UnmodifiableCollection<E>(s) {}

		int size() {
			return UnmodifiableCollection<E>::c->size();
		}
		boolean isEmpty() {
			return UnmodifiableCollection<E>::c->isEmpty();
		}
		boolean contains(E o) {
			return UnmodifiableCollection<E>::c->contains(o);
		}
		sp<EIterator<E> > iterator(int index=0) {
			return UnmodifiableCollection<E>::c->iterator(index);
		}
		boolean add(E e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean remove(E o) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean containsAll(ECollection<E> *c) {
			return UnmodifiableCollection<E>::c->containsAll(c);
		}
		boolean retainAll(ECollection<E> *c) {
			return UnmodifiableCollection<E>::c->retainAll(c);
		}
		boolean removeAll(ECollection<E> *c) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		void clear() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

		boolean equals(E o) {return o == this || UnmodifiableCollection<E>::c->equals(o);}
		int hashCode()           {return UnmodifiableCollection<E>::c->hashCode();}
	};
	template<typename T>
	static ESet<T>* unmodifiableSet(ESet<T>* s) {
		return new UnmodifiableSet<T>(s);
	}

	/**
	 * Returns an unmodifiable view of the specified map.  This method
	 * allows modules to provide users with "read-only" access to internal
	 * maps.  Query operations on the returned map "read through"
	 * to the specified map, and attempts to modify the returned
	 * map, whether direct or via its collection views, result in an
	 * <tt>UnsupportedOperationException</tt>.<p>
	 *
	 * The returned map will be serializable if the specified map
	 * is serializable.
	 *
	 * @param  m the map for which an unmodifiable view is to be returned.
	 * @return an unmodifiable view of the specified map.
	 */
	/**
	 * @serial include
	 */
	template<typename K, typename V>
	class UnmodifiableMap : public EMap<K,V> {
	private:
		EMap<K,V>* m;

		sp<ESet<K> > keySet_;// = null;
		sp<ESet<EMapEntry<K,V>*> > entrySet_;// = null;
		sp<ECollection<V> > values_;// = null;

		/**
		 * We need this class in addition to UnmodifiableSet as
		 * Map.Entries themselves permit modification of the backing Map
		 * via their setValue operation.  This class is subtle: there are
		 * many possible attacks that must be thwarted.
		 *
		 * @serial include
		 */
		class UnmodifiableEntrySet :
			public UnmodifiableSet<EMapEntry<K,V>*> {
		private:
			UnmodifiableEntrySet(ESet<EMapEntry<K,V>*>* s) : UnmodifiableSet<EMapEntry<K,V>*>(s) {
			}
		public:
			sp<EIterator<EMapEntry<K,V>*> > iterator(int index=0) {
				class UnmodifiableEntrySetIterator {
				private:
					sp<EIterator<EMapEntry<K,V>*> > i;

				public:
					UnmodifiableEntrySetIterator() {
						i = UnmodifiableCollection<EMapEntry<K,V>*>::c->iterator();
					}
					~UnmodifiableEntrySetIterator() {
						//
					}

					boolean hasNext() {
						return i->hasNext();
					}
					EMapEntry<K,V>* next() {
						return i->next(); //TODO: return new UnmodifiableEntry<K,V>(i.next());
					}
					void remove() {
						throw EUnsupportedOperationException(__FILE__, __LINE__);
					}
					EMapEntry<K,V>* moveOut() {
						throw EUnsupportedOperationException(__FILE__, __LINE__);
					}
				};
				return new UnmodifiableEntrySetIterator();
			}

			/**
			 * This method is overridden to protect the backing set against
			 * an object with a nefarious equals function that senses
			 * that the equality-candidate is Map.Entry and calls its
			 * setValue method.
			 */
			boolean contains(EMapEntry<K,V>* o) {
				return UnmodifiableCollection<EMapEntry<K,V>*>::c->contains(o);
			}

			/**
			 * The next two methods are overridden to protect against
			 * an unscrupulous List whose contains(Object o) method senses
			 * when o is a Map.Entry, and calls o.setValue.
			 */
			boolean containsAll(ECollection<EMapEntry<K,V>*> coll) {
				sp<EIterator<EMapEntry<K,V>*> > e = coll->iterator();
				while (e->hasNext()) {
					if (!contains(e->next())) // Invokes safe contains() above
					{
						return false;
					}
				}
				return true;
			}
			boolean equals(UnmodifiableEntrySet* o) {
				if (o == this)
					return true;

				/** @see:
				if (!(o instanceof Set))
					return false;
				Set s = (Set) o;
				*/
				if (o->size() != UnmodifiableCollection<EMapEntry<K,V>*>::c->size())
					return false;
				return containsAll(o); // Invokes safe containsAll() above
			}
		};

	public:
		UnmodifiableMap(EMap<K,V>* m) {
			if (m==null)
				throw ENullPointerException(__FILE__, __LINE__);
			this->m = m;

			keySet_ = null;
			entrySet_ = null;
			values_ = null;
		}
		~UnmodifiableMap() {
			//
		}

	public:
		int size()                   {return m->size();}
		boolean isEmpty()            {return m->isEmpty();}
		boolean containsKey(K key)   {return m->containsKey(key);}
		boolean containsValue(V val) {return m->containsValue(val);}
		V get(K key)                 {return m->get(key);}

		V put(K key, V value, boolean *absent=null) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		V remove(K key) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		void putAll(EMap<K,V>* m) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		void clear() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

		sp<ESet<K> > keySet() {
			if (keySet_==null)
				keySet_ = unmodifiableSet(m->keySet().get());
			return keySet_;
		}
		sp<ESet<EMapEntry<K,V>*> > entrySet() {
			/** TODO:
			 if (entrySet_==null)
				entrySet_ = new UnmodifiableEntrySet<K,V>(m.entrySet());
			 return entrySet_;
			 */
			return m->entrySet();
		}
		sp<ECollection<V> > values() {
			if (values_==null)
				values_ = unmodifiableCollection(m->values().get());
			return values_;
		}

		boolean equals(EMapEntry<K,V>* o) {return o == this || m->equals(o);}
		int hashCode()           {return m->hashCode();}
		EStringBase toString()       {return m->toString();}

		/**
		 * Auto free.
		 */
		void setAutoFree(boolean autoFreeKey, boolean autoFreeValue) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

		boolean getAutoFreeKey() {
			return m->getAutoFreeKey();
		}

		boolean getAutoFreeValue() {
			return m->getAutoFreeValue();
		}
	};
	template<typename K, typename V>
	static EMap<K,V>* unmodifiableMap(EMap<K,V>* m) {
		return new UnmodifiableMap<K,V>(m);
	}

	/**
	 * Returns an unmodifiable view of the specified sorted map.  This method
	 * allows modules to provide users with "read-only" access to internal
	 * sorted maps.  Query operations on the returned sorted map "read through"
	 * to the specified sorted map.  Attempts to modify the returned
	 * sorted map, whether direct, via its collection views, or via its
	 * <tt>subMap</tt>, <tt>headMap</tt>, or <tt>tailMap</tt> views, result in
	 * an <tt>UnsupportedOperationException</tt>.<p>
	 *
	 * The returned sorted map will be serializable if the specified sorted map
	 * is serializable.
	 *
	 * @param m the sorted map for which an unmodifiable view is to be
	 *        returned.
	 * @return an unmodifiable view of the specified sorted map.
	 */
	template<typename K, typename V>
	class UnmodifiableSortedMap: public UnmodifiableMap<K,V>, public ESortedMap<K,V> {
	private:
		ESortedMap<K,V>* sm;

	public:
		UnmodifiableSortedMap(ESortedMap<K,V>* m) : UnmodifiableMap<K,V>(m) {sm = m;}

		EComparator<K>* comparator() {return sm->comparator();}

		ESortedMap<K,V>* subMap(K fromKey, K toKey) {
			return new UnmodifiableSortedMap<K,V>(sm->subMap(fromKey, toKey));
		}
		ESortedMap<K,V>* headMap(K toKey) {
			return new UnmodifiableSortedMap<K,V>(sm->headMap(toKey));
		}
		ESortedMap<K,V>* tailMap(K fromKey) {
			return new UnmodifiableSortedMap<K,V>(sm->tailMap(fromKey));
		}

		K firstKey()           {return sm->firstKey();}
		K lastKey()            {return sm->lastKey();}
	};
	template<typename K, typename V>
	static ESortedMap<K,V>* unmodifiableSortedMap(ESortedMap<K,V>* m) {
		return new UnmodifiableSortedMap<K,V>(m);
	}

private:
	// Suppresses default constructor, ensuring non-instantiability.
	ECollections() {
	}

	/**
	 * Swaps x[a .. (a+n-1)] with x[b .. (b+n-1)].
	 */
	template<typename E>
	static void vecswap(EList<E> *list, int a, int b, int n) {
		for (int i = 0; i < n; i++, a++, b++)
			swap(list, a, b);
	}

	/**
	 * Returns the index of the median of the three indexed integers.
	 */
	template<typename E>
	static int med3(EList<E> *list,
			int a, int b, int c) {
		E _a = list->getAt(a);
		E _b = list->getAt(b);
		E _c = list->getAt(c);

		#if 0
		return (_a->compareTo(_b) < 0 ?
				(_b->compareTo(_c) < 0 ? b : _a->compareTo(_c) < 0 ? c : a) :
				(_b->compareTo(_c) > 0 ? b : _a->compareTo(_c) > 0 ? c : a));
		#else
		return (compare(_a, _b) < 0 ?
				(compare(_b, _c) < 0 ? b : compare(_a, _c) < 0 ? c : a) :
				(compare(_b, _c) > 0 ? b : compare(_a, _c) > 0 ? c : a));
		#endif
	}

	template<typename E>
	static int med3(EList<E> *list,
			EComparator<E> *comp, int a, int b, int c) {
		E _a = list->getAt(a);
		E _b = list->getAt(b);
		E _c = list->getAt(c);

		return (comp->compare(_a, _b) < 0 ?
				(comp->compare(_b, _c) < 0 ? b :
					comp->compare(_a, _c) < 0 ? c : a) :
				(comp->compare(_b, _c) > 0 ? b :
					comp->compare(_a, _c) > 0 ? c : a));
	}

	template<typename E>
	static int compare(E o1, E o2) {
		return o1->compareTo(o2);
	}
	template<typename E>
	static int compare(sp<E> o1, sp<E> o2) {
		return o1->compareTo(o2.get());
	}
};

} /* namespace efc */
#endif /* ECOLLECTIONS_HH_ */
