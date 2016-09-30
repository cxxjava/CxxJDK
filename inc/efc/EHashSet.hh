/*
 * EHashSet.hh
 *
 *  Created on: 2013-12-18
 *      Author: cxxjava@163.com
 */

#ifndef EHASHSET_HH_
#define EHASHSET_HH_

#include "EAbstractSet.hh"
#include "EHashMap.hh"
#include "EMath.hh"

namespace efc {

/**
 * This class implements the <tt>Set</tt> interface, backed by a hash table
 * (actually a <tt>HashMap</tt> instance).  It makes no guarantees as to the
 * iteration order of the set; in particular, it does not guarantee that the
 * order will remain constant over time.  This class permits the <tt>null</tt>
 * element.
 *
 * <p>This class offers constant time performance for the basic operations
 * (<tt>add</tt>, <tt>remove</tt>, <tt>contains</tt> and <tt>size</tt>),
 * assuming the hash function disperses the elements properly among the
 * buckets.  Iterating over this set requires time proportional to the sum of
 * the <tt>HashSet</tt> instance's size (the number of elements) plus the
 * "capacity" of the backing <tt>HashMap</tt> instance (the number of
 * buckets).  Thus, it's very important not to set the initial capacity too
 * high (or the load factor too low) if iteration performance is important.
 *
 * <p><strong>Note that this implementation is not synchronized.</strong>
 * If multiple threads access a hash set concurrently, and at least one of
 * the threads modifies the set, it <i>must</i> be synchronized externally.
 * This is typically accomplished by synchronizing on some object that
 * naturally encapsulates the set.
 *
 * If no such object exists, the set should be "wrapped" using the
 * {@link Collections#synchronizedSet Collections.synchronizedSet}
 * method.  This is best done at creation time, to prevent accidental
 * unsynchronized access to the set:<pre>
 *   Set s = Collections.synchronizedSet(new HashSet(...));</pre>
 *
 * <p>The iterators returned by this class's <tt>iterator</tt> method are
 * <i>fail-fast</i>: if the set is modified at any time after the iterator is
 * created, in any way except through the iterator's own <tt>remove</tt>
 * method, the Iterator throws a {@link ConcurrentModificationException}.
 * Thus, in the face of concurrent modification, the iterator fails quickly
 * and cleanly, rather than risking arbitrary, non-deterministic behavior at
 * an undetermined time in the future.
 *
 * <p>Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw <tt>ConcurrentModificationException</tt> on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness: <i>the fail-fast behavior of iterators
 * should be used only to detect bugs.</i>
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @param <E> the type of elements maintained by this set
 *
 * @version 1.37, 04/21/06
 * @see	    Collection
 * @see	    Set
 * @see	    TreeSet
 * @see	    HashMap
 * @since   1.2
 */

template<typename E>
class EHashSet: public EAbstractSet<E> {
public:
	virtual ~EHashSet() {
		delete map_;
	}

	/**
	 * Constructs a new, empty set; the backing <tt>HashMap</tt> instance has
	 * default initial capacity (16) and load factor (0.75).
	 */
	EHashSet() {
		map_ = new EHashMap<E, EObject*>(16, true, true);
	}
	explicit
	EHashSet(boolean autoFree) {
		map_ = new EHashMap<E, EObject*>(autoFree, true);
	}

	/**
	 * Constructs a new, empty set; the backing <tt>HashMap</tt> instance has
	 * the specified initial capacity and the specified load factor.
	 *
	 * @param      initialCapacity   the initial capacity of the hash map
	 * @param      loadFactor        the load factor of the hash map
	 * @throws     IllegalArgumentException if the initial capacity is less
	 *             than zero, or if the load factor is nonpositive
	 */
	explicit
	EHashSet(int initialCapacity, float loadFactor/*0.75*/, boolean autoFree=true) {
		map_ = new EHashMap<E, EObject*>(initialCapacity, loadFactor, autoFree, true);
	}

	/**
	 * Constructs a new set containing the elements in the specified
	 * collection.  The <tt>HashMap</tt> is created with default load factor
	 * (0.75) and an initial capacity sufficient to contain the elements in
	 * the specified collection.
	 *
	 * @param c the collection whose elements are to be placed into this set
	 * @throws NullPointerException if the specified collection is null
	 */
	EHashSet(ECollection<E> *c) {
		map_ = new EHashMap<E, EObject*>(
				EMath::max((int) (c->size() / .75f) + 1, 16), false, false);
		addAll(c);
	}

	/**
	 *
	 */
	EHashSet(const EHashSet<E>& that) {
		EHashSet<E>* t = (EHashSet<E>*)&that;
		map_ = new EHashMap<E, EObject*>(*t->map_);
	}

	/**
	 *
	 */
	EHashSet<E>& operator= (const EHashSet<E>& that) {
		if (this == &that) return *this;

		EHashSet<E>* t = (EHashSet<E>*)&that;

		//1.
		delete map_;

		//2.
		map_ = new EHashMap<E, EObject*>(*t->map_);

		return *this;
	}

	/**
	 * Returns an iterator over the elements in this set.  The elements
	 * are returned in no particular order.
	 *
	 * @return an Iterator over the elements in this set
	 * @see ConcurrentModificationException
	 */
	sp<EIterator<E> > iterator(int index=0) {
		return map_->keySet()->iterator();
	}

	/**
	 * Returns the number of elements in this set (its cardinality).
	 *
	 * @return the number of elements in this set (its cardinality)
	 */
	int size() {
		return map_->size();
	}

	/**
	 * Returns <tt>true</tt> if this set contains no elements.
	 *
	 * @return <tt>true</tt> if this set contains no elements
	 */
	boolean isEmpty() {
		return map_->isEmpty();
	}

	/**
	 * Returns <tt>true</tt> if this set contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this set
	 * contains an element <tt>e</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>.
	 *
	 * @param o element whose presence in this set is to be tested
	 * @return <tt>true</tt> if this set contains the specified element
	 */
	boolean contains(E o) {
		return map_->containsKey(o);
	}

	/**
	 * Adds the specified element to this set if it is not already present.
	 * More formally, adds the specified element <tt>e</tt> to this set if
	 * this set contains no element <tt>e2</tt> such that
	 * <tt>(e==null&nbsp;?&nbsp;e2==null&nbsp;:&nbsp;e.equals(e2))</tt>.
	 * If this set already contains the element, the call leaves the set
	 * unchanged and returns <tt>false</tt>.
	 *
	 * @param e element to be added to this set
	 * @return <tt>true</tt> if this set did not already contain the specified
	 * element
	 */
	boolean add(E e) {
		boolean absent;
		map_->put(e, null, &absent);
		return absent;
	}

	/**
	 * Removes the specified element from this set if it is present.
	 * More formally, removes an element <tt>e</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>,
	 * if this set contains such an element.  Returns <tt>true</tt> if
	 * this set contained the element (or equivalently, if this set
	 * changed as a result of the call).  (This set will not contain the
	 * element once the call returns.)
	 *
	 * @param o object to be removed from this set, if present
	 * @return <tt>true</tt> if the set contained the specified element
	 */
	boolean remove(E o) {
		return map_->remove(o) == null;
	}

	/**
	 * Removes all of the elements from this set.
	 * The set will be empty after this call returns.
	 */
	void clear() {
		map_->clear();
	}

	void setAutoFree(boolean autoFree) {
		map_->setAutoFree(autoFree, true);
	}

	boolean getAutoFree() {
		return map_->getAutoFreeKey();
	}

private:
	EHashMap<E, EObject*> *map_;
};

} /* namespace efc */
#endif /* EHASHSET_HH_ */
