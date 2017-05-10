/*
 * EIdentityHashMap.hh
 *
 *  Created on: 2014-9-18
 *      Author: cxxjava@163.com
 */

#ifndef EIDENTITYHASHMAP_HH_
#define EIDENTITYHASHMAP_HH_

#include "EA.hh"
#include "ESystem.hh"
#include "EAbstractMap.hh"
#include "EToDoException.hh"
#include "EIllegalStateException.hh"
#include "EIllegalArgumentException.hh"
#include "EConcurrentModificationException.hh"

namespace efc {

namespace idhm {
	extern EObject* NULL_KEY;
}
using namespace idhm;

/**
 * This class implements the <tt>Map</tt> interface with a hash table, using
 * reference-equality in place of object-equality when comparing keys (and
 * values).  In other words, in an <tt>IdentityHashMap</tt>, two keys
 * <tt>k1</tt> and <tt>k2</tt> are considered equal if and only if
 * <tt>(k1==k2)</tt>.  (In normal <tt>Map</tt> implementations (like
 * <tt>HashMap</tt>) two keys <tt>k1</tt> and <tt>k2</tt> are considered equal
 * if and only if <tt>(k1==null ? k2==null : k1.equals(k2))</tt>.)
 *
 * <p><b>This class is <i>not</i> a general-purpose <tt>Map</tt>
 * implementation!  While this class implements the <tt>Map</tt> interface, it
 * intentionally violates <tt>Map's</tt> general contract, which mandates the
 * use of the <tt>equals</tt> method when comparing objects.  This class is
 * designed for use only in the rare cases wherein reference-equality
 * semantics are required.</b>
 *
 * <p>A typical use of this class is <i>topology-preserving object graph
 * transformations</i>, such as serialization or deep-copying.  To perform such
 * a transformation, a program must maintain a "node table" that keeps track
 * of all the object references that have already been processed.  The node
 * table must not equate distinct objects even if they happen to be equal.
 * Another typical use of this class is to maintain <i>proxy objects</i>.  For
 * example, a debugging facility might wish to maintain a proxy object for
 * each object in the program being debugged.
 *
 * <p>This class provides all of the optional map operations, and permits
 * <tt>null</tt> values and the <tt>null</tt> key.  This class makes no
 * guarantees as to the order of the map; in particular, it does not guarantee
 * that the order will remain constant over time.
 *
 * <p>This class provides constant-time performance for the basic
 * operations (<tt>get</tt> and <tt>put</tt>), assuming the system
 * identity hash function ({@link System#identityHashCode(Object)})
 * disperses elements properly among the buckets.
 *
 * <p>This class has one tuning parameter (which affects performance but not
 * semantics): <i>expected maximum size</i>.  This parameter is the maximum
 * number of key-value mappings that the map is expected to hold.  Internally,
 * this parameter is used to determine the number of buckets initially
 * comprising the hash table.  The precise relationship between the expected
 * maximum size and the number of buckets is unspecified.
 *
 * <p>If the size of the map (the number of key-value mappings) sufficiently
 * exceeds the expected maximum size, the number of buckets is increased
 * Increasing the number of buckets ("rehashing") may be fairly expensive, so
 * it pays to create identity hash maps with a sufficiently large expected
 * maximum size.  On the other hand, iteration over collection views requires
 * time proportional to the number of buckets in the hash table, so it
 * pays not to set the expected maximum size too high if you are especially
 * concerned with iteration performance or memory usage.
 *
 * <p><strong>Note that this implementation is not synchronized.</strong>
 * If multiple threads access an identity hash map concurrently, and at
 * least one of the threads modifies the map structurally, it <i>must</i>
 * be synchronized externally.  (A structural modification is any operation
 * that adds or deletes one or more mappings; merely changing the value
 * associated with a key that an instance already contains is not a
 * structural modification.)  This is typically accomplished by
 * synchronizing on some object that naturally encapsulates the map.
 *
 * If no such object exists, the map should be "wrapped" using the
 * {@link Collections#synchronizedMap Collections.synchronizedMap}
 * method.  This is best done at creation time, to prevent accidental
 * unsynchronized access to the map:<pre>
 *   Map m = Collections.synchronizedMap(new IdentityHashMap(...));</pre>
 *
 * <p>The iterators returned by the <tt>iterator</tt> method of the
 * collections returned by all of this class's "collection view
 * methods" are <i>fail-fast</i>: if the map is structurally modified
 * at any time after the iterator is created, in any way except
 * through the iterator's own <tt>remove</tt> method, the iterator
 * will throw a {@link ConcurrentModificationException}.  Thus, in the
 * face of concurrent modification, the iterator fails quickly and
 * cleanly, rather than risking arbitrary, non-deterministic behavior
 * at an undetermined time in the future.
 *
 * <p>Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw <tt>ConcurrentModificationException</tt> on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness: <i>fail-fast iterators should be used only
 * to detect bugs.</i>
 *
 * <p>Implementation note: This is a simple <i>linear-probe</i> hash table,
 * as described for example in texts by Sedgewick and Knuth.  The array
 * alternates holding keys and values.  (This has better locality for large
 * tables than does using separate arrays.)  For many JRE implementations
 * and operation mixes, this class will yield better performance than
 * {@link HashMap} (which uses <i>chaining</i> rather than linear-probing).
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @see     System#identityHashCode(Object)
 * @see     Object#hashCode()
 * @see     Collection
 * @see     Map
 * @see     HashMap
 * @see     TreeMap
 * @since   1.4
 */

/**
 * The initial capacity used by the no-args constructor.
 * MUST be a power of two.  The value 32 corresponds to the
 * (specified) expected maximum size of 21, given a load factor
 * of 2/3.
 */
#define  IHM_DEFAULT_CAPACITY   32

/**
 * The minimum capacity, used if a lower value is implicitly specified
 * by either of the constructors with arguments.  The value 4 corresponds
 * to an expected maximum size of 2, given a load factor of 2/3.
 * MUST be a power of two.
 */
#define  IHM_MINIMUM_CAPACITY   4

/**
 * The maximum capacity, used if a higher value is implicitly specified
 * by either of the constructors with arguments.
 * MUST be a power of two <= 1<<29.
 */
#define  IHM_MAXIMUM_CAPACITY   (1 << 29)

template<typename K, typename V>
class EIdentityHashMap: public EAbstractMap<K, V>, virtual EMap<K, V> {
public:
	~EIdentityHashMap() {
		delete table;
		delete _entrySet;
	}

	/**
	 * Constructs a new, empty identity hash map with a default expected
	 * maximum size (21).
	 */
	EIdentityHashMap() {
		init(IHM_DEFAULT_CAPACITY, true);
	}
	explicit
	EIdentityHashMap(boolean autoFree) {
		init(IHM_DEFAULT_CAPACITY, autoFree);
	}

	/**
	 * Constructs a new, empty map with the specified expected maximum size.
	 * Putting more than the expected number of key-value mappings into
	 * the map may cause the internal data structure to grow, which may be
	 * somewhat time-consuming.
	 *
	 * @param expectedMaxSize the expected maximum size of the map
	 * @throws IllegalArgumentException if <tt>expectedMaxSize</tt> is negative
	 */
	explicit
	EIdentityHashMap(int expectedMaxSize, boolean autoFree) {
		if (expectedMaxSize < 0)
			throw EIllegalArgumentException(__FILE__, __LINE__,
					EString::formatOf("expectedMaxSize is negative: %d",
							expectedMaxSize).c_str());
		init(capacity(expectedMaxSize), autoFree);
	}

	// TODO...
	EIdentityHashMap(const EIdentityHashMap<K, V>& that);
	EIdentityHashMap<K, V>& operator= (const EIdentityHashMap<K, V>& that);

	/**
	 * Returns the number of key-value mappings in this identity hash map.
	 *
	 * @return the number of key-value mappings in this map
	 */
	int size() {
		return size_;
	}

	/**
	 * Returns <tt>true</tt> if this identity hash map contains no key-value
	 * mappings.
	 *
	 * @return <tt>true</tt> if this identity hash map contains no key-value
	 *         mappings
	 */
	boolean isEmpty() {
		return size_ == 0;
	}

	/**
	 * Returns the value to which the specified key is mapped,
	 * or {@code null} if this map contains no mapping for the key.
	 *
	 * <p>More formally, if this map contains a mapping from a key
	 * {@code k} to a value {@code v} such that {@code (key == k)},
	 * then this method returns {@code v}; otherwise it returns
	 * {@code null}.  (There can be at most one such mapping.)
	 *
	 * <p>A return value of {@code null} does not <i>necessarily</i>
	 * indicate that the map contains no mapping for the key; it's also
	 * possible that the map explicitly maps the key to {@code null}.
	 * The {@link #containsKey containsKey} operation may be used to
	 * distinguish these two cases.
	 *
	 * @see #put(Object, Object)
	 */
	V get(K key) {
		EObject* k = maskNull(key);
		EA<EObject*>& tab = *table;
		int len = tab.length();
		int i = hash(k, len);
		while (true ) {
			EObject* item = tab[i];
			if (item == k)
				return dynamic_cast<V>(tab[i + 1]);
			if (item == null)
				return null;
			i = nextKeyIndex(i, len);
		}
		//always not reach here.
		return null;
	}

	/**
	 * Tests whether the specified object reference is a key in this identity
	 * hash map.
	 *
	 * @param   key   possible key
	 * @return  <code>true</code> if the specified object reference is a key
	 *          in this map
	 * @see     #containsValue(Object)
	 */
	boolean containsKey(K key) {
		EObject* k = maskNull(key);
		EA<EObject*>& tab = *table;
		int len = tab.length();
		int i = hash(k, len);
		while (true ) {
			EObject* item = tab[i];
			if (item == k)
				return true ;
			if (item == null)
				return false ;
			i = nextKeyIndex(i, len);
		}
		//always not reach here.
		return false ;
	}

	/**
	 * Tests whether the specified object reference is a value in this identity
	 * hash map.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map maps one or more keys to the
	 *         specified object reference
	 * @see     #containsKey(Object)
	 */
	boolean containsValue(V value) {
		EA<EObject*>& tab = *table;
		for (int i = 1; i < tab.length(); i += 2)
			if (tab[i] == value && tab[i - 1] != null)
				return true ;

		return false ;
	}

	/**
	 * Associates the specified value with the specified key in this identity
	 * hash map.  If the map previously contained a mapping for the key, the
	 * old value is replaced.
	 *
	 * @param key the key with which the specified value is to be associated
	 * @param value the value to be associated with the specified key
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 * @see     Object#equals(Object)
	 * @see     #get(Object)
	 * @see     #containsKey(Object)
	 */
	V put(K key, V value, boolean *absent=null) {
		EObject* k = maskNull(key);
		EA<EObject*>& tab = *table;
		int len = tab.length();
		int i = hash(k, len);

		EObject* item;
		while ((item = tab[i]) != null) {
			if (item == k) {
				V oldValue = dynamic_cast<V>(tab[i + 1]);
				tab[i + 1] = value;
				return oldValue;
			}
			i = nextKeyIndex(i, len);
		}

		modCount++;
		tab[i] = k;
		tab[i + 1] = value;
		if (++size_ >= threshold)
			resize(len); // len == 2 * current capacity.
		return null;
	}

	/**
	 * Removes the mapping for this key from this map if present.
	 *
	 * @param key key whose mapping is to be removed from the map
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V remove(K key) {
		EObject* k = maskNull(key);
		EA<EObject*>& tab = *table;
		int len = tab.length();
		int i = hash(k, len);

		while (true ) {
			EObject* item = tab[i];
			if (item == k) {
				modCount++;
				size_--;
				V oldValue = dynamic_cast<V>(tab[i + 1]);
				tab[i + 1] = null;
				tab[i] = null;
				closeDeletion(i);
				return oldValue;
			}
			if (item == null)
				return null;
			i = nextKeyIndex(i, len);
		}

		//always not reach here.
		return null;
	}

	/**
	 * Removes all of the mappings from this map.
	 * The map will be empty after this call returns.
	 */
	void clear() {
		modCount++;
		EA<EObject*>& tab = *table;
		for (int i = 0; i < tab.length(); i++) {
			if (tab[i] != null) {
				delete tab[i];
				tab[i] = null;
			}
		}
		size_ = 0;
	}

    /**
     * Returns an identity-based set view of the keys contained in this map.
     * The set is backed by the map, so changes to the map are reflected in
     * the set, and vice-versa.  If the map is modified while an iteration
     * over the set is in progress, the results of the iteration are
     * undefined.  The set supports element removal, which removes the
     * corresponding mapping from the map, via the <tt>Iterator.remove</tt>,
     * <tt>Set.remove</tt>, <tt>removeAll</tt>, <tt>retainAll</tt>, and
     * <tt>clear</tt> methods.  It does not support the <tt>add</tt> or
     * <tt>addAll</tt> methods.
     *
     * <p><b>While the object returned by this method implements the
     * <tt>Set</tt> interface, it does <i>not</i> obey <tt>Set's</tt> general
     * contract.  Like its backing map, the set returned by this method
     * defines element equality as reference-equality rather than
     * object-equality.  This affects the behavior of its <tt>contains</tt>,
     * <tt>remove</tt>, <tt>containsAll</tt>, <tt>equals</tt>, and
     * <tt>hashCode</tt> methods.</b>
     *
     * <p><b>The <tt>equals</tt> method of the returned set returns <tt>true</tt>
     * only if the specified object is a set containing exactly the same
     * object references as the returned set.  The symmetry and transitivity
     * requirements of the <tt>Object.equals</tt> contract may be violated if
     * the set returned by this method is compared to a normal set.  However,
     * the <tt>Object.equals</tt> contract is guaranteed to hold among sets
     * returned by this method.</b>
     *
     * <p>The <tt>hashCode</tt> method of the returned set returns the sum of
     * the <i>identity hashcodes</i> of the elements in the set, rather than
     * the sum of their hashcodes.  This is mandated by the change in the
     * semantics of the <tt>equals</tt> method, in order to enforce the
     * general contract of the <tt>Object.hashCode</tt> method among sets
     * returned by this method.
     *
     * @return an identity-based set view of the keys contained in this map
     * @see Object#equals(Object)
     * @see System#identityHashCode(Object)
     */
	sp<ESet<K> > keySet() {
		if (!EAbstractMap<K,V>::_keySet) {
			EAbstractMap<K,V>::_keySet = new KeySet(this);
		}
		return EAbstractMap<K,V>::_keySet;
    }

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection is backed by the map, so changes to the map are
	 * reflected in the collection, and vice-versa.  If the map is
	 * modified while an iteration over the collection is in progress,
	 * the results of the iteration are undefined.  The collection
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Collection.remove</tt>, <tt>removeAll</tt>,
	 * <tt>retainAll</tt> and <tt>clear</tt> methods.  It does not
	 * support the <tt>add</tt> or <tt>addAll</tt> methods.
	 *
	 * <p><b>While the object returned by this method implements the
	 * <tt>Collection</tt> interface, it does <i>not</i> obey
	 * <tt>Collection's</tt> general contract.  Like its backing map,
	 * the collection returned by this method defines element equality as
	 * reference-equality rather than object-equality.  This affects the
	 * behavior of its <tt>contains</tt>, <tt>remove</tt> and
	 * <tt>containsAll</tt> methods.</b>
	 */
	sp<ECollection<V> > values() {
		if (!EAbstractMap<K,V>::_values) {
			EAbstractMap<K,V>::_values = new Values(this);
		}
		return EAbstractMap<K,V>::_values;
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * Each element in the returned set is a reference-equality-based
	 * <tt>Map.Entry</tt>.  The set is backed by the map, so changes
	 * to the map are reflected in the set, and vice-versa.  If the
	 * map is modified while an iteration over the set is in progress,
	 * the results of the iteration are undefined.  The set supports
	 * element removal, which removes the corresponding mapping from
	 * the map, via the <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt> and <tt>clear</tt>
	 * methods.  It does not support the <tt>add</tt> or
	 * <tt>addAll</tt> methods.
	 *
	 * <p>Like the backing map, the <tt>Map.Entry</tt> objects in the set
	 * returned by this method define key and value equality as
	 * reference-equality rather than object-equality.  This affects the
	 * behavior of the <tt>equals</tt> and <tt>hashCode</tt> methods of these
	 * <tt>Map.Entry</tt> objects.  A reference-equality based <tt>Map.Entry
	 * e</tt> is equal to an object <tt>o</tt> if and only if <tt>o</tt> is a
	 * <tt>Map.Entry</tt> and <tt>e.getKey()==o.getKey() &amp;&amp;
	 * e.getValue()==o.getValue()</tt>.  To accommodate these equals
	 * semantics, the <tt>hashCode</tt> method returns
	 * <tt>System.identityHashCode(e.getKey()) ^
	 * System.identityHashCode(e.getValue())</tt>.
	 *
	 * <p><b>Owing to the reference-equality-based semantics of the
	 * <tt>Map.Entry</tt> instances in the set returned by this method,
	 * it is possible that the symmetry and transitivity requirements of
	 * the {@link Object#equals(Object)} contract may be violated if any of
	 * the entries in the set is compared to a normal map entry, or if
	 * the set returned by this method is compared to a set of normal map
	 * entries (such as would be returned by a call to this method on a normal
	 * map).  However, the <tt>Object.equals</tt> contract is guaranteed to
	 * hold among identity-based map entries, and among sets of such entries.
	 * </b>
	 *
	 * @return a set view of the identity-mappings contained in this map
	 */
	sp<ESet<EMapEntry<K, V>*> > entrySet() {
		if (!_entrySet) {
			_entrySet = new EntrySet(this);
		}
		return _entrySet;
	}

	void setAutoFree(boolean autoFreeKey = true, boolean autoFreeValue = true) {
		if (autoFreeKey != autoFreeValue) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "Only support autoFreeKey==autoFreeValue.");
		}
		_autoFree = autoFreeKey;
	}

	boolean getAutoFreeKey() {
		return _autoFree;
	}

	boolean getAutoFreeValue() {
		return _autoFree;
	}

private:
	/**
	 * Auto free object flag
	 */
	boolean _autoFree;

	/**
	 * The table, resized as necessary. Length MUST always be a power of two.
	 */
	EA<EObject*>* table;

	/**
	 * The number of key-value mappings contained in this identity hash map.
	 *
	 * @serial
	 */
	int size_;

	/**
	 * The number of modifications, to support fast-fail iterators
	 */
	volatile int modCount;

	/**
	 * The next size value at which to resize (capacity * load factor).
	 */
	int threshold;

	/**
	 * Value representing null keys inside tables.
	 */
	//static EObject* NULL_KEY;

	// Views

	/**
	 * This field is initialized to contain an instance of the entry set
	 * view the first time this view is requested.  The view is stateless,
	 * so there's no reason to create more than one.
	 */
	ESet<EMapEntry<K, V>*> *_entrySet;

	/**
	 * Use NULL_KEY for key if it is null.
	 */
	static EObject* maskNull(EObject* key) {
		return (key == null ? NULL_KEY : key);
	}

	/**
	 * Returns internal representation of null key back to caller as null.
	 */
	static EObject* unmaskNull(EObject* key) {
		return (key == NULL_KEY ? null : key);
	}

	/**
	 * Returns the appropriate capacity for the specified expected maximum
	 * size.  Returns the smallest power of two between MINIMUM_CAPACITY
	 * and MAXIMUM_CAPACITY, inclusive, that is greater than
	 * (3 * expectedMaxSize)/2, if such a number exists.  Otherwise
	 * returns MAXIMUM_CAPACITY.  If (3 * expectedMaxSize)/2 is negative, it
	 * is assumed that overflow has occurred, and MAXIMUM_CAPACITY is returned.
	 */
	int capacity(int expectedMaxSize) {
		// Compute min capacity for expectedMaxSize given a load factor of 2/3
		int minCapacity = (3 * expectedMaxSize) / 2;

		// Compute the appropriate capacity
		int result;
		if (minCapacity > IHM_MAXIMUM_CAPACITY || minCapacity < 0) {
			result = IHM_MAXIMUM_CAPACITY;
		} else {
			result = IHM_MINIMUM_CAPACITY;
			while (result < minCapacity)
				result <<= 1;
		}
		return result;
	}

	/**
	 * Initializes object to be an empty map with the specified initial
	 * capacity, which is assumed to be a power of two between
	 * MINIMUM_CAPACITY and MAXIMUM_CAPACITY inclusive.
	 */
	void init(int initCapacity, boolean autoFree) {
		// assert (initCapacity & -initCapacity) == initCapacity; // power of 2
		// assert initCapacity >= MINIMUM_CAPACITY;
		// assert initCapacity <= MAXIMUM_CAPACITY;

		size_ = 0;
		modCount = 0;
		threshold = 0;

		threshold = (initCapacity * 2) / 3;
		table = new EA<EObject*>(2 * initCapacity, autoFree);

		_entrySet = null;
	}

	/**
	 * Returns index for Object x.
	 */
	static int hash(EObject* x, int length) {
		int h = ESystem::identityHashCode(x);
		// Multiply by -127, and left-shift to use least bit as part of hash
		return ((h << 1) - (h << 8)) & (length - 1);
	}

	/**
	 * Circularly traverses table of size len.
	 */
	static int nextKeyIndex(int i, int len) {
		return (i + 2 < len ? i + 2 : 0);
	}

	/**
	 * Removes the specified key-value mapping from the map if it is present.
	 *
	 * @param   key   possible key
	 * @param   value possible value
	 * @return  <code>true</code> if and only if the specified key-value
	 *          mapping was in the map
	 */
	boolean removeMapping(K key, V value) {
		EObject* k = maskNull(key);
		EA<EObject*>& tab = (*table);
		int len = tab.length();
		int i = hash(k, len);

		while (true) {
			EObject* item = tab[i];
			if (item == k) {
				if (tab[i + 1] != value)
					return false;
				modCount++;
				size_--;
				tab[i] = null;
				tab[i + 1] = null;
				closeDeletion(i);
				return true;
			}
			if (item == null)
				return false;
			i = nextKeyIndex(i, len);
		}
		//always not reach here.
		return false ;
	}

	/**
	 * Resize the table to hold given capacity.
	 *
	 * @param newCapacity the new capacity, must be a power of two.
	 */
	void resize(int newCapacity) {
		// assert (newCapacity & -newCapacity) == newCapacity; // power of 2
		int newLength = newCapacity * 2;

		EA<EObject*>& oldTable = *table;
		int oldLength = oldTable.length();
		if (oldLength == 2 * IHM_MAXIMUM_CAPACITY) { // can't expand any further
			if (threshold == IHM_MAXIMUM_CAPACITY - 1)
				throw EIllegalStateException(__FILE__, __LINE__, "Capacity exhausted.");
			threshold = IHM_MAXIMUM_CAPACITY - 1;  // Gigantic map!
			return;
		}
		if (oldLength >= newLength)
			return;

		EA<EObject*>* newTable = new EA<EObject*>(newLength);
		threshold = newLength / 3;

		for (int j = 0; j < oldLength; j += 2) {
			EObject* key = oldTable[j];
			if (key != null) {
				EObject* value = oldTable[j + 1];
				oldTable[j] = null;
				oldTable[j + 1] = null;
				int i = hash(key, newLength);
				while ((*newTable)[i] != null)
					i = nextKeyIndex(i, newLength);
				(*newTable)[i] = key;
				(*newTable)[i + 1] = value;
			}
		}
		table = newTable;
	}

	/**
	 * Rehash all possibly-colliding entries following a
	 * deletion. This preserves the linear-probe
	 * collision properties required by get, put, etc.
	 *
	 * @param d the index of a newly empty deleted slot
	 */
	void closeDeletion(int d) {
		// Adapted from Knuth Section 6.4 Algorithm R
		EA<EObject*>& tab = *table;
		int len = tab.length();

		// Look for items to swap into newly vacated slot
		// starting at index immediately following deletion,
		// and continuing until a null slot is seen, indicating
		// the end of a run of possibly-colliding keys.
		EObject* item;
		for (int i = nextKeyIndex(d, len); (item = tab[i]) != null; i =
				nextKeyIndex(i, len)) {
			// The following test triggers if the item at slot i (which
			// hashes to be at slot r) should take the spot vacated by d.
			// If so, we swap it in, and then continue with d now at the
			// newly vacated i.  This process will terminate when we hit
			// the null slot at the end of this run.
			// The test is messy because we are using a circular table.
			int r = hash(item, len);
			if ((i < r && (r <= d || d <= i)) || (r <= d && d <= i)) {
				tab[d] = item;
				tab[d + 1] = tab[i + 1];
				tab[i] = null;
				tab[i + 1] = null;
				d = i;
			}
		}
	}

	/**
	 * Tests if the specified key-value mapping is in the map.
	 *
	 * @param   key   possible key
	 * @param   value possible value
	 * @return  <code>true</code> if and only if the specified key-value
	 *          mapping is in the map
	 */
	boolean containsMapping(K key, V value) {
		EObject* k = maskNull(key);
		EA<EObject*>& tab = (*table);
		int len = tab.length();
		int i = hash(k, len);
		while (true) {
			EObject* item = tab[i];
			if (item == k)
				return tab[i + 1] == value;
			if (item == null)
				return false;
			i = nextKeyIndex(i, len);
		}
		//always not reach here.
		return false;
	}

	template<typename T>
	abstract class IdentityHashMapIterator : public EIterator<T> {
	protected:
		EIdentityHashMap<K,V>* m;

	protected:
		int index;// = (size != 0 ? 0 : table.length); // current slot.
		int expectedModCount;// = modCount; // to support fast-fail
		int lastReturnedIndex;// = -1;      // to allow remove()
		boolean indexValid; // To avoid unnecessary next computation
		EA<EObject*>* traversalTable;// = table; // reference to main table or copy

		int nextIndex() {
			if (m->modCount != expectedModCount)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			if (!indexValid && !hasNext())
				throw ENoSuchElementException(__FILE__, __LINE__);

			indexValid = false;
			lastReturnedIndex = index;
			index += 2;
			return lastReturnedIndex;
		}

	public:
		IdentityHashMapIterator(EIdentityHashMap<K,V>* m) : m(m) {
			index = (m->size_ != 0 ? 0 : m->table->length());
			expectedModCount = m->modCount;
			lastReturnedIndex = -1;
			indexValid = false;
			traversalTable = m->table;
		}

		boolean hasNext() {
			EA<EObject*>& tab = (*traversalTable);
			for (int i = index; i < tab.length(); i+=2) {
				EObject* key = tab[i];
				if (key != null) {
					index = i;
					return indexValid = true;
				}
			}
			index = tab.length();
			return false;
		}

		void remove() {
			throw EToDoException(__FILE__, __LINE__);
		}

		T moveOut() {
			throw EToDoException(__FILE__, __LINE__);
		}
	};

	class KeyIterator : public IdentityHashMapIterator<K> {
	public:
		KeyIterator(EIdentityHashMap* m) : IdentityHashMapIterator<K>(m) {
		}

		K next() {
			return dynamic_cast<K>(unmaskNull((*IdentityHashMapIterator<K>::traversalTable)[IdentityHashMapIterator<K>::nextIndex()]));
		}
	};

	class ValueIterator : public IdentityHashMapIterator<V> {
	public:
		ValueIterator(EIdentityHashMap* m) : IdentityHashMapIterator<K>(m) {
		}

		V next() {
			return dynamic_cast<V>((*IdentityHashMapIterator<K>::traversalTable)[IdentityHashMapIterator<K>::nextIndex() + 1]);
		}
	};

	class EntryIterator: public IdentityHashMapIterator<EMapEntry<K,V>*>, public EMapEntry<K,V> {
	public:
		EntryIterator(EIdentityHashMap<K, V> *map) :
			IdentityHashMapIterator<EMapEntry<K,V>*>(map) {
		}

		EMapEntry<K,V>* next() {
			IdentityHashMapIterator<EMapEntry<K,V>*>::nextIndex();
			return this;
		}

		K getKey() {
			EA<EObject*>* traversalTable = IdentityHashMapIterator<EMapEntry<K,V>*>::traversalTable;
			int lastReturnedIndex = IdentityHashMapIterator<EMapEntry<K,V>*>::lastReturnedIndex;

			// Provide a better exception than out of bounds index
			if (lastReturnedIndex < 0)
				throw EIllegalStateException(__FILE__, __LINE__, "Entry was removed");

			return dynamic_cast<K>(unmaskNull((*traversalTable)[lastReturnedIndex]));
		}

		V getValue() {
			EA<EObject*>* traversalTable = IdentityHashMapIterator<EMapEntry<K,V>*>::traversalTable;
			int lastReturnedIndex = IdentityHashMapIterator<EMapEntry<K,V>*>::lastReturnedIndex;

			// Provide a better exception than out of bounds index
			if (lastReturnedIndex < 0)
				throw EIllegalStateException(__FILE__, __LINE__, "Entry was removed");

			return dynamic_cast<V>((*traversalTable)[lastReturnedIndex+1]);
		}

		V setValue(V value) {
			EA<EObject*>* traversalTable = IdentityHashMapIterator<EMapEntry<K,V>*>::traversalTable;
			int lastReturnedIndex = IdentityHashMapIterator<EMapEntry<K,V>*>::lastReturnedIndex;

			// It would be mean-spirited to proceed here if remove() called
			if (lastReturnedIndex < 0)
				throw EIllegalStateException(__FILE__, __LINE__, "Entry was removed");
			V oldValue = dynamic_cast<V>((*traversalTable)[lastReturnedIndex+1]);
			(*traversalTable)[lastReturnedIndex+1] = value;
			// if shadowing, force into main table
			/** @see:
			if (traversalTable != IdentityHashMap.this.table)
                put((K) traversalTable[lastReturnedIndex], value);
            */
			return oldValue;
		}

		boolean equals(EMapEntry<K,V> *e) {
			if (IdentityHashMapIterator<EMapEntry<K,V>*>::lastReturnedIndex < 0)
				return IdentityHashMapIterator<EMapEntry<K,V>*>::equals(e);

			return e->getKey()   == getKey() &&
				   e->getValue() == getValue();
		}

		virtual int hashCode() {
			if (IdentityHashMapIterator<EMapEntry<K,V>*>::lastReturnedIndex < 0)
				return IdentityHashMapIterator<EMapEntry<K,V>*>::hashCode();

			return ESystem::identityHashCode(getKey()) ^
					ESystem::identityHashCode(getValue());
		}
	};

	class KeySet: public EAbstractSet<K> {
	private:
		EIdentityHashMap<K,V> *_map;
	public:
		KeySet(EIdentityHashMap<K,V> *map) {
			_map = map;
		}
		sp<EIterator<K> > iterator(int index = 0) {
			return new KeyIterator(_map);
		}
		int size() {
			return _map->size();
		}
		boolean contains(K o) {
			return _map->containsKey(o);
		}
		boolean remove(K o) {
			int oldSize = _map->size();
			_map->remove(o);
			return _map->size() != oldSize;
		}
		void clear() {
			_map->clear();
		}
	};

	class Values : public EAbstractCollection<V> {
	private:
		EIdentityHashMap<K,V> *_map;

	public:
		Values(EIdentityHashMap<K,V> *map) {
			_map = map;
		}
		sp<EIterator<V> > iterator(int index = 0) {
			return new ValueIterator(_map);
		}
		int size() {
			return _map->size();
		}
		boolean contains(V o) {
			return _map->containsValue(o);
		}
		void clear() {
			_map->clear();
		}
	};

	class EntrySet: public EAbstractSet<EMapEntry<K,V>*> {
	private:
		EIdentityHashMap<K,V> *_map;

	public:
		EntrySet(EIdentityHashMap *map) {
			_map = map;
		}

		sp<EIterator<EMapEntry<K,V>*> > iterator(int index=0) {
			return new EntryIterator(_map);
		}
		boolean contains(EMapEntry<K,V> *e) {
			return _map->containsMapping(e->getKey(), e->getValue());
		}
		boolean remove(EMapEntry<K,V> *o) {
			return _map->removeMapping(o->getKey(), o->getValue());
		}
		int size() {
			return _map->size();
		}
		void clear() {
			_map->clear();
		}
	};
};

} /* namespace efc */
#endif /* EIDENTITYHASHMAP_HH_ */
