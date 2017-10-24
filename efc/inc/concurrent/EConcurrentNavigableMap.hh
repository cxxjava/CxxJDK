/*
 * EConcurrentNavigableMap.hh
 *
 *  Created on: 2014-2-24
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTNAVIGABLEMAP_HH_
#define ECONCURRENTNAVIGABLEMAP_HH_

#include "./EConcurrentMap.hh"
#include "../ENavigableMap.hh"
#include "../ENavigableSet.hh"
#include "./EConcurrentSortedMap.hh"
#include "./EConcurrentNavigableSet.hh"

namespace efc {

/**
 * A {@link ConcurrentMap} supporting {@link NavigableMap} operations,
 * and recursively so for its navigable sub-maps.
 *
 * <p>This interface is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @param <K> the type of keys maintained by this map
 * @param <V> the type of mapped values
 * @since 1.6
 */

template<typename K, typename V>
interface EConcurrentNavigableMap: virtual public EConcurrentSortedMap<K, V>
{
	virtual ~EConcurrentNavigableMap() {}

    /**
     * @throws ClassCastException       {@inheritDoc}
     * @throws NullPointerException     {@inheritDoc}
     * @throws IllegalArgumentException {@inheritDoc}
     */
	virtual EConcurrentNavigableMap<K,V>* subMap(K* fromKey, boolean fromInclusive,
                                       K* toKey,   boolean toInclusive) = 0;

    /**
     * @throws ClassCastException       {@inheritDoc}
     * @throws NullPointerException     {@inheritDoc}
     * @throws IllegalArgumentException {@inheritDoc}
     */
	virtual EConcurrentNavigableMap<K,V>* headMap(K* toKey, boolean inclusive) = 0;


    /**
     * @throws ClassCastException       {@inheritDoc}
     * @throws NullPointerException     {@inheritDoc}
     * @throws IllegalArgumentException {@inheritDoc}
     */
	virtual EConcurrentNavigableMap<K,V>* tailMap(K* fromKey, boolean inclusive) = 0;

    /**
     * @throws ClassCastException       {@inheritDoc}
     * @throws NullPointerException     {@inheritDoc}
     * @throws IllegalArgumentException {@inheritDoc}
     */
	virtual EConcurrentNavigableMap<K,V>* subMap(K* fromKey, K* toKey) = 0;

    /**
     * @throws ClassCastException       {@inheritDoc}
     * @throws NullPointerException     {@inheritDoc}
     * @throws IllegalArgumentException {@inheritDoc}
     */
	virtual EConcurrentNavigableMap<K,V>* headMap(K* toKey) = 0;

    /**
     * @throws ClassCastException       {@inheritDoc}
     * @throws NullPointerException     {@inheritDoc}
     * @throws IllegalArgumentException {@inheritDoc}
     */
	virtual EConcurrentNavigableMap<K,V>* tailMap(K* fromKey) = 0;

    /**
     * Returns a reverse order view of the mappings contained in this map.
     * The descending map is backed by this map, so changes to the map are
     * reflected in the descending map, and vice-versa.
     *
     * <p>The returned map has an ordering equivalent to
     * <tt>{@link Collections#reverseOrder(Comparator) Collections.reverseOrder}(comparator())</tt>.
     * The expression {@code m.descendingMap().descendingMap()} returns a
     * view of {@code m} essentially equivalent to {@code m}.
     *
     * @return a reverse order view of this map
     */
	virtual sp<EConcurrentNavigableMap<K,V> > descendingMap() = 0;

    /**
     * Returns a {@link NavigableSet} view of the keys contained in this map.
     * The set's iterator returns the keys in ascending order.
     * The set is backed by the map, so changes to the map are
     * reflected in the set, and vice-versa.  The set supports element
     * removal, which removes the corresponding mapping from the map,
     * via the {@code Iterator.remove}, {@code Set.remove},
     * {@code removeAll}, {@code retainAll}, and {@code clear}
     * operations.  It does not support the {@code add} or {@code addAll}
     * operations.
     *
     * <p>The view's {@code iterator} is a "weakly consistent" iterator
     * that will never throw {@link ConcurrentModificationException},
     * and guarantees to traverse elements as they existed upon
     * construction of the iterator, and may (but is not guaranteed to)
     * reflect any modifications subsequent to construction.
     *
     * @return a navigable set view of the keys in this map
     */
	virtual sp<EConcurrentNavigableSet<K> > navigableKeySet() = 0;

    /**
     * Returns a reverse order {@link NavigableSet} view of the keys contained in this map.
     * The set's iterator returns the keys in descending order.
     * The set is backed by the map, so changes to the map are
     * reflected in the set, and vice-versa.  The set supports element
     * removal, which removes the corresponding mapping from the map,
     * via the {@code Iterator.remove}, {@code Set.remove},
     * {@code removeAll}, {@code retainAll}, and {@code clear}
     * operations.  It does not support the {@code add} or {@code addAll}
     * operations.
     *
     * <p>The view's {@code iterator} is a "weakly consistent" iterator
     * that will never throw {@link ConcurrentModificationException},
     * and guarantees to traverse elements as they existed upon
     * construction of the iterator, and may (but is not guaranteed to)
     * reflect any modifications subsequent to construction.
     *
     * @return a reverse order navigable set view of the keys in this map
     */
	virtual sp<EConcurrentNavigableSet<K> > descendingKeySet() = 0;

	/* ---------------- NavigableMap Overrides -------------- */

	/**
	 * Returns a key-value mapping associated with the greatest key
	 * strictly less than the given key, or {@code null} if there is
	 * no such key.
	 *
	 * @param key the key
	 * @return an entry with the greatest key less than {@code key},
	 *         or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<EConcurrentMapEntry<K,V> > lowerEntry(K* key) = 0;

	/**
	 * Returns the greatest key strictly less than the given key, or
	 * {@code null} if there is no such key.
	 *
	 * @param key the key
	 * @return the greatest key less than {@code key},
	 *         or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<K> lowerKey(K* key) = 0;

	/**
	 * Returns a key-value mapping associated with the greatest key
	 * less than or equal to the given key, or {@code null} if there
	 * is no such key.
	 *
	 * @param key the key
	 * @return an entry with the greatest key less than or equal to
	 *         {@code key}, or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<EConcurrentMapEntry<K,V> > floorEntry(K* key) = 0;

	/**
	 * Returns the greatest key less than or equal to the given key,
	 * or {@code null} if there is no such key.
	 *
	 * @param key the key
	 * @return the greatest key less than or equal to {@code key},
	 *         or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<K> floorKey(K* key) = 0;

	/**
	 * Returns a key-value mapping associated with the least key
	 * greater than or equal to the given key, or {@code null} if
	 * there is no such key.
	 *
	 * @param key the key
	 * @return an entry with the least key greater than or equal to
	 *         {@code key}, or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<EConcurrentMapEntry<K,V> > ceilingEntry(K* key) = 0;

	/**
	 * Returns the least key greater than or equal to the given key,
	 * or {@code null} if there is no such key.
	 *
	 * @param key the key
	 * @return the least key greater than or equal to {@code key},
	 *         or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<K> ceilingKey(K* key) = 0;

	/**
	 * Returns a key-value mapping associated with the least key
	 * strictly greater than the given key, or {@code null} if there
	 * is no such key.
	 *
	 * @param key the key
	 * @return an entry with the least key greater than {@code key},
	 *         or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<EConcurrentMapEntry<K,V> > higherEntry(K* key) = 0;

	/**
	 * Returns the least key strictly greater than the given key, or
	 * {@code null} if there is no such key.
	 *
	 * @param key the key
	 * @return the least key greater than {@code key},
	 *         or {@code null} if there is no such key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map does not permit null keys
	 */
	virtual sp<K> higherKey(K* key) = 0;

	/**
	 * Returns a key-value mapping associated with the least
	 * key in this map, or {@code null} if the map is empty.
	 *
	 * @return an entry with the least key,
	 *         or {@code null} if this map is empty
	 */
	virtual sp<EConcurrentMapEntry<K,V> > firstEntry() = 0;

	/**
	 * Returns a key-value mapping associated with the greatest
	 * key in this map, or {@code null} if the map is empty.
	 *
	 * @return an entry with the greatest key,
	 *         or {@code null} if this map is empty
	 */
	virtual sp<EConcurrentMapEntry<K,V> > lastEntry() = 0;

	/**
	 * Removes and returns a key-value mapping associated with
	 * the least key in this map, or {@code null} if the map is empty.
	 *
	 * @return the removed first entry of this map,
	 *         or {@code null} if this map is empty
	 */
	virtual sp<EConcurrentMapEntry<K,V> > pollFirstEntry() = 0;

	/**
	 * Removes and returns a key-value mapping associated with
	 * the greatest key in this map, or {@code null} if the map is empty.
	 *
	 * @return the removed last entry of this map,
	 *         or {@code null} if this map is empty
	 */
	virtual sp<EConcurrentMapEntry<K,V> > pollLastEntry() = 0;
};

} /* namespace efc */
#endif /* ECONCURRENTNAVIGABLEMAP_HH_ */
