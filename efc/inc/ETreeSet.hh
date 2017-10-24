/*
 * ETreeSet.hh
 *
 *  Created on: 2013-12-18
 *      Author: cxxjava@163.com
 */

#ifndef ETREESET_HH_
#define ETREESET_HH_

#include "EAbstractSet.hh"
#include "ETreeMap.hh"
#include "EMath.hh"

namespace efc {

/**
 * A {@link NavigableSet} implementation based on a {@link TreeMap}.
 * The elements are ordered using their {@linkplain Comparable natural
 * ordering}, or by a {@link Comparator} provided at set creation
 * time, depending on which constructor is used.
 *
 * <p>This implementation provides guaranteed log(n) time cost for the basic
 * operations ({@code add}, {@code remove} and {@code contains}).
 *
 * <p>Note that the ordering maintained by a set (whether or not an explicit
 * comparator is provided) must be <i>consistent with equals</i> if it is to
 * correctly implement the {@code Set} interface.  (See {@code Comparable}
 * or {@code Comparator} for a precise definition of <i>consistent with
 * equals</i>.)  This is so because the {@code Set} interface is defined in
 * terms of the {@code equals} operation, but a {@code TreeSet} instance
 * performs all element comparisons using its {@code compareTo} (or
 * {@code compare}) method, so two elements that are deemed equal by this method
 * are, from the standpoint of the set, equal.  The behavior of a set
 * <i>is</i> well-defined even if its ordering is inconsistent with equals; it
 * just fails to obey the general contract of the {@code Set} interface.
 *
 * <p><strong>Note that this implementation is not synchronized.</strong>
 * If multiple threads access a tree set concurrently, and at least one
 * of the threads modifies the set, it <i>must</i> be synchronized
 * externally.  This is typically accomplished by synchronizing on some
 * object that naturally encapsulates the set.
 * If no such object exists, the set should be "wrapped" using the
 * {@link Collections#synchronizedSortedSet Collections.synchronizedSortedSet}
 * method.  This is best done at creation time, to prevent accidental
 * unsynchronized access to the set: <pre>
 *   SortedSet s = Collections.synchronizedSortedSet(new TreeSet(...));</pre>
 *
 * <p>The iterators returned by this class's {@code iterator} method are
 * <i>fail-fast</i>: if the set is modified at any time after the iterator is
 * created, in any way except through the iterator's own {@code remove}
 * method, the iterator will throw a {@link ConcurrentModificationException}.
 * Thus, in the face of concurrent modification, the iterator fails quickly
 * and cleanly, rather than risking arbitrary, non-deterministic behavior at
 * an undetermined time in the future.
 *
 * <p>Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw {@code ConcurrentModificationException} on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness:   <i>the fail-fast behavior of iterators
 * should be used only to detect bugs.</i>
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @param <E> the type of elements maintained by this set
 *
 * @see     Collection
 * @see     Set
 * @see     HashSet
 * @see     Comparable
 * @see     Comparator
 * @see     TreeMap
 * @since   1.2
 */

template<typename E>
class ETreeSet: public EAbstractSet<E>, virtual public ENavigableSet<E> {
public:
	virtual ~ETreeSet() {
		delete map_;
	}

	/**
	 * Constructs a new, empty tree set, sorted according to the
	 * natural ordering of its elements.  All elements inserted into
	 * the set must implement the {@link Comparable} interface.
	 * Furthermore, all such elements must be <i>mutually
	 * comparable</i>: {@code e1.compareTo(e2)} must not throw a
	 * {@code ClassCastException} for any elements {@code e1} and
	 * {@code e2} in the set.  If the user attempts to add an element
	 * to the set that violates this constraint (for example, the user
	 * attempts to add a string element to a set whose elements are
	 * integers), the {@code add} call will throw a
	 * {@code ClassCastException}.
	 */
	ETreeSet(boolean autoFree = true) : needFreeMap(true) {
		map_ = new ETreeMap<E, EObject*>(autoFree, true);
	}

	/**
	 * Constructs a new, empty tree set, sorted according to the specified
	 * comparator.  All elements inserted into the set must be <i>mutually
	 * comparable</i> by the specified comparator: {@code comparator.compare(e1,
	 * e2)} must not throw a {@code ClassCastException} for any elements
	 * {@code e1} and {@code e2} in the set.  If the user attempts to add
	 * an element to the set that violates this constraint, the
	 * {@code add} call will throw a {@code ClassCastException}.
	 *
	 * @param comparator the comparator that will be used to order this set.
	 *        If {@code null}, the {@linkplain Comparable natural
	 *        ordering} of the elements will be used.
	 */
	ETreeSet(EComparator<E>* comparator, boolean autoFree = true) : needFreeMap(true) {
		map_ = new ETreeMap<E, EObject*>(comparator, autoFree, true);
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
	ETreeSet(ENavigableMap<E, EObject*> *map) : needFreeMap(false) {
		map_ = map;
	}

	/**
	 * Constructs a new tree set containing the elements in the specified
	 * collection, sorted according to the <i>natural ordering</i> of its
	 * elements.  All elements inserted into the set must implement the
	 * {@link Comparable} interface.  Furthermore, all such elements must be
	 * <i>mutually comparable</i>: {@code e1.compareTo(e2)} must not throw a
	 * {@code ClassCastException} for any elements {@code e1} and
	 * {@code e2} in the set.
	 *
	 * @param c collection whose elements will comprise the new set
	 * @throws ClassCastException if the elements in {@code c} are
	 *         not {@link Comparable}, or are not mutually comparable
	 * @throws NullPointerException if the specified collection is null
	 */
	ETreeSet(ECollection<E> *c) : needFreeMap(true) {
		map_ = new ETreeMap<E, EObject*>(false, false);
		addAll(c);
	}

	/**
	 * Constructs a new tree set containing the same elements and
	 * using the same ordering as the specified sorted set.
	 *
	 * @param s sorted set whose elements will comprise the new set
	 * @throws NullPointerException if the specified sorted set is null
	 */
	ETreeSet(ESortedSet<E>* s): needFreeMap(true) {
		map_ = new ETreeMap<E, EObject*>(s->comparator, false, false);
		addAll(s);
	}

	//TODO:
	ETreeSet(const ETreeSet<E>& that);
	ETreeSet<E>& operator= (const ETreeSet<E>& that);

	/**
	 * Returns an iterator over the elements in this set.  The elements
	 * are returned in no particular order.
	 *
	 * @return an Iterator over the elements in this set
	 * @see ConcurrentModificationException
	 */
	sp<EIterator<E> > iterator(int index=0) {
		return map_->navigableKeySet()->iterator();
	}

	/**
	 * Returns an iterator over the elements in this set in descending order.
	 *
	 * @return an iterator over the elements in this set in descending order
	 * @since 1.6
	 */
	sp<EIterator<E> > descendingIterator() {
		return map_->descendingKeySet()->iterator();
	}

	/**
	 * @since 1.6
	 */
	ENavigableSet<E>* descendingSet() {
		throw EToDoException(__FILE__, __LINE__);
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

	/**
	 * Adds all of the elements in the specified collection to this set.
	 *
	 * @param c collection containing elements to be added to this set
	 * @return {@code true} if this set changed as a result of the call
	 * @throws ClassCastException if the elements provided cannot be compared
	 *         with the elements currently in the set
	 * @throws NullPointerException if the specified collection is null or
	 *         if any element is null and this set uses natural ordering, or
	 *         its comparator does not permit null elements
	 */
	boolean addAll(ECollection<E>* c) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromElement} or {@code toElement}
	 *         is null and this set uses natural ordering, or its comparator
	 *         does not permit null elements
	 * @throws IllegalArgumentException {@inheritDoc}
	 * @since 1.6
	 */
	ENavigableSet<E>* subSet(E fromElement, boolean fromInclusive,
								  E toElement,   boolean toInclusive) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code toElement} is null and
	 *         this set uses natural ordering, or its comparator does
	 *         not permit null elements
	 * @throws IllegalArgumentException {@inheritDoc}
	 * @since 1.6
	 */
	ENavigableSet<E>* headSet(E toElement, boolean inclusive) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromElement} is null and
	 *         this set uses natural ordering, or its comparator does
	 *         not permit null elements
	 * @throws IllegalArgumentException {@inheritDoc}
	 * @since 1.6
	 */
	ENavigableSet<E>* tailSet(E fromElement, boolean inclusive) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromElement} or
	 *         {@code toElement} is null and this set uses natural ordering,
	 *         or its comparator does not permit null elements
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	ESortedSet<E>* subSet(E fromElement, E toElement) {
		return subSet(fromElement, true, toElement, false);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code toElement} is null
	 *         and this set uses natural ordering, or its comparator does
	 *         not permit null elements
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	ESortedSet<E>* headSet(E toElement) {
		return headSet(toElement, false);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromElement} is null
	 *         and this set uses natural ordering, or its comparator does
	 *         not permit null elements
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	ESortedSet<E>* tailSet(E fromElement) {
		return tailSet(fromElement, true);
	}

	EComparator<E>* comparator() {
		return map_->comparator();
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E first() {
		return map_->firstKey();
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	E last() {
		return map_->lastKey();
	}

	// NavigableSet API methods

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified element is null
	 *         and this set uses natural ordering, or its comparator
	 *         does not permit null elements
	 * @since 1.6
	 */
	E lower(E e) {
		return map_->lowerKey(e);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified element is null
	 *         and this set uses natural ordering, or its comparator
	 *         does not permit null elements
	 * @since 1.6
	 */
	E floor(E e) {
		return map_->floorKey(e);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified element is null
	 *         and this set uses natural ordering, or its comparator
	 *         does not permit null elements
	 * @since 1.6
	 */
	E ceiling(E e) {
		return map_->ceilingKey(e);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified element is null
	 *         and this set uses natural ordering, or its comparator
	 *         does not permit null elements
	 * @since 1.6
	 */
	E higher(E e) {
		return map_->higherKey(e);
	}

	/**
	 * @since 1.6
	 */
	E pollFirst() {
		EMapEntry<E,EObject*>* e = map_->pollFirstEntry();
		return (e == null) ? null : e->getKey();
	}

	/**
	 * @since 1.6
	 */
	E pollLast() {
		EMapEntry<E,EObject*>* e = map_->pollLastEntry();
		return (e == null) ? null : e->getKey();
	}

private:
	ENavigableMap<E, EObject*> *map_;
	boolean needFreeMap;
};

} /* namespace efc */
#endif /* ETREESET_HH_ */
