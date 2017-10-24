#ifndef __EHashMap_H__
#define __EHashMap_H__

#include "EAbstractMap.hh"
#include "EInteger.hh"
#include "EIllegalStateException.hh"
#include "ENoSuchElementException.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

/**
 * Hash table based implementation of the <tt>Map</tt> interface.  This
 * implementation provides all of the optional map operations, and permits
 * <tt>null</tt> values and the <tt>null</tt> key.  (The <tt>HashMap</tt>
 * class is roughly equivalent to <tt>Hashtable</tt>, except that it is
 * unsynchronized and permits nulls.)  This class makes no guarantees as to
 * the order of the map; in particular, it does not guarantee that the order
 * will remain constant over time.
 *
 * <p>This implementation provides constant-time performance for the basic
 * operations (<tt>get</tt> and <tt>put</tt>), assuming the hash function
 * disperses the elements properly among the buckets.  Iteration over
 * collection views requires time proportional to the "capacity" of the
 * <tt>HashMap</tt> instance (the number of buckets) plus its size (the number
 * of key-value mappings).  Thus, it's very important not to set the initial
 * capacity too high (or the load factor too low) if iteration performance is
 * important.
 *
 * <p>An instance of <tt>HashMap</tt> has two parameters that affect its
 * performance: <i>initial capacity</i> and <i>load factor</i>.  The
 * <i>capacity</i> is the number of buckets in the hash table, and the initial
 * capacity is simply the capacity at the time the hash table is created.  The
 * <i>load factor</i> is a measure of how full the hash table is allowed to
 * get before its capacity is automatically increased.  When the number of
 * entries in the hash table exceeds the product of the load factor and the
 * current capacity, the hash table is <i>rehashed</i> (that is, internal data
 * structures are rebuilt) so that the hash table has approximately twice the
 * number of buckets.
 *
 * <p>As a general rule, the default load factor (.75) offers a good tradeoff
 * between time and space costs.  Higher values decrease the space overhead
 * but increase the lookup cost (reflected in most of the operations of the
 * <tt>HashMap</tt> class, including <tt>get</tt> and <tt>put</tt>).  The
 * expected number of entries in the map and its load factor should be taken
 * into account when setting its initial capacity, so as to minimize the
 * number of rehash operations.  If the initial capacity is greater
 * than the maximum number of entries divided by the load factor, no
 * rehash operations will ever occur.
 *
 * <p>If many mappings are to be stored in a <tt>HashMap</tt> instance,
 * creating it with a sufficiently large capacity will allow the mappings to
 * be stored more efficiently than letting it perform automatic rehashing as
 * needed to grow the table.
 *
 * <p><strong>Note that this implementation is not synchronized.</strong>
 * If multiple threads access a hash map concurrently, and at least one of
 * the threads modifies the map structurally, it <i>must</i> be
 * synchronized externally.  (A structural modification is any operation
 * that adds or deletes one or more mappings; merely changing the value
 * associated with a key that an instance already contains is not a
 * structural modification.)  This is typically accomplished by
 * synchronizing on some object that naturally encapsulates the map.
 *
 * If no such object exists, the map should be "wrapped" using the
 * {@link Collections#synchronizedMap Collections.synchronizedMap}
 * method.  This is best done at creation time, to prevent accidental
 * unsynchronized access to the map:<pre>
 *   Map m = Collections.synchronizedMap(new HashMap(...));</pre>
 *
 * <p>The iterators returned by all of this class's "collection view methods"
 * are <i>fail-fast</i>: if the map is structurally modified at any time after
 * the iterator is created, in any way except through the iterator's own
 * <tt>remove</tt> method, the iterator will throw a
 * {@link ConcurrentModificationException}.  Thus, in the face of concurrent
 * modification, the iterator fails quickly and cleanly, rather than risking
 * arbitrary, non-deterministic behavior at an undetermined time in the
 * future.
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
 * @param <K> the type of keys maintained by this map
 * @param <V> the type of mapped values
 *
 * @version 1.73, 03/13/07
 * @see     Object#hashCode()
 * @see     Collection
 * @see	    Map
 * @see	    TreeMap
 * @see	    Hashtable
 * @since   1.2
 */

/**
 * The default initial capacity - MUST be a power of two.
 */
#define HM_DEFAULT_INITIAL_CAPACITY   16

/**
 * The maximum capacity, used if a higher value is implicitly specified
 * by either of the constructors with arguments.
 * MUST be a power of two <= 1<<30.
 */
#define  HM_MAXIMUM_CAPACITY   (1 << 30)

/**
 * The load factor used when none specified in constructor.
 */
#define  HM_DEFAULT_LOAD_FACTOR 0.75f

//=============================================================================
//Primitive Key && Native pointer Value.

template<typename K, typename V>
class EHashMap : public EAbstractMap<K, V>,
		virtual public EMap<K, V> {
public:
	class Entry: public EMapEntry<K, V> {
	private:
		friend class EHashMap;
		K key;
		V value;
		Entry *next;
		EHashMap<K, V> *map;
		int hash;

	public:
		~Entry() {
			if (map->getAutoFreeValue()) {
				delete value;
			}
			delete next; //!
		}

		/**
		 * Creates new entry.
		 */
		Entry(int h, K k, V v,
				Entry *n,
				EHashMap<K, V> *m) {
			value = v;
			next = n;
			key = k;
			hash = h;
			map = m;
		}

		K getKey() {
			return key;
		}

		V getValue() {
			return value;
		}

		V setValue(V newValue) {
			V oldValue = value;
			value = newValue;
			return oldValue;
		}

		boolean equals(EMapEntry<K, V> *e) {
			K k1 = getKey();
			K k2 = e->getKey();
			if (k1 == k2) {
				V v1 = getValue();
				V v2 = e->getValue();
				if (v1 == v2 || (v1 != null && v1->equals(v2)))
					return true;
			}
			return false;
		}

		virtual int hashCode() {
			return key ^ (value == null ? 0 : value->hashCode());
		}

		/**
		 * This method is invoked whenever the value in an entry is
		 * overwritten by an invocation of put(k,v) for a key k that's already
		 * in the HashMap.
		 */
		void recordAccess(EHashMap<K, V> *m) {
		}

		/**
		 * This method is invoked whenever the entry is
		 * removed from the table.
		 */
		void recordRemoval(EHashMap<K, V> *m) {
		}
	};

private:
	template<typename I>
	class HashIterator: public EIterator<I> {
	private:
		EHashMap<K,V> *_map;

		Entry *next; // next entry to return
		int index; // current slot
		Entry *current; // current entry
	public:
		HashIterator(EHashMap<K, V> *map) : next(null), index(0), current(null) {
			_map = map;

			if (_map->size() > 0) { // advance to first entry
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
		}

		boolean hasNext() {
			return next != null;
		}

		Entry* nextEntry() {
			Entry *e = next;
			if (e == null)
				throw ENOSUCHELEMENTEXCEPTION;

			if ((next = e->next) == null) {
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
			current = e;
			return e;
		}

		void remove() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			delete _map->removeEntryForKey(k);
		}

		Entry* moveOutEntry() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			return _map->removeEntryForKey(k);
		}
	};

	template<typename EI>
	class EntryIterator: public HashIterator<EI> {
	public:
		EntryIterator(EHashMap<K, V> *map) :
				HashIterator<EI>(map) {
		}

		EI next() {
			return HashIterator<EI>::nextEntry();
		}

		EI moveOut() {
			return HashIterator<EI>::moveOutEntry();
		}
	};

	class ValueIterator: public HashIterator<V> {
	public:
		ValueIterator(EHashMap<K, V> *map) :
				HashIterator<V>(map) {
		}

		V next() {
			return HashIterator<V>::nextEntry()->getValue();
		}

		V moveOut() {
			Entry* e = HashIterator<V>::moveOutEntry();
			V v = e->getValue();
			delete e;
			return v;
		}
	};

	class KeyIterator: public HashIterator<K> {
	public:
		KeyIterator(EHashMap<K, V> *map) :
				HashIterator<K>(map) {
		}

		K next() {
			return HashIterator<K>::nextEntry()->getKey();
		}

		K moveOut() {
			Entry* e = HashIterator<K>::moveOutEntry();
			K k = e->getKey();
			delete e;
			return k;
		}
	};

	class EntrySet: public EAbstractSet<EMapEntry<K,V>*> {
	private:
		EHashMap<K,V> *_map;

	public:
		EntrySet(EHashMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<EMapEntry<K,V>*> > iterator(int index=0) {
			return new EntryIterator<EMapEntry<K,V>*>(_map);
		}
		boolean contains(EMapEntry<K,V> *e) {
			EMapEntry<K,V> *candidate = _map->getEntry(e->getKey());
			return candidate != null && candidate->equals(e);
		}
		boolean remove(EMapEntry<K,V> *o) {
			return _map->removeMapping(o) != null;
		}
		int size() {
			return _map->size();
		}
		void clear() {
			_map->clear();
		}
	};

	class Values: public EAbstractCollection<V> {
	private:
		EHashMap<K, V> *_map;
	public:
		Values(EHashMap<K, V> *map) {
			_map = map;
		}
		~Values() {
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

	class Keys: public EAbstractSet<K> {
	private:
		EHashMap<K, V> *_map;
	public:
		Keys(EHashMap<K, V> *map) {
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
			return _map->removeEntryForKey(o) != null;
		}
		void clear() {
			_map->clear();
		}
	};

private:
	/**
	 * The table, resized as necessary. Length MUST Always be a power of two.
	 */
	Entry **_table;

	/**
	 * The size of _table.
	 */
	uint _capacity;

private:

	/**
	 * Auto free object flag
	 */
	boolean _autoFreeValue;

	/**
	 * The number of key-value mappings contained in this map.
	 */
	int _size;

	/**
	 * The next size value at which to resize (capacity * load factor).
	 * @serial
	 */
	int _threshold;

	/**
	 * The load factor for the hash table.
	 *
	 * @serial
	 */
	float _loadFactor;

	// Views
	sp<ESet<EMapEntry<K, V>*> > _entrySet;

	/**
	 * Initialization hook for subclasses. This method is called
	 * in all constructors and pseudo-constructors (clone, readObject)
	 * after HashMap has been initialized but before any entries have
	 * been inserted.  (In the absence of this method, readObject would
	 * require explicit knowledge of subclasses.)
	 */
	void init(uint initialCapacity, float loadFactor, boolean autoFreeValue) {
		if (initialCapacity < HM_DEFAULT_INITIAL_CAPACITY)
			initialCapacity = HM_DEFAULT_INITIAL_CAPACITY;
		if (initialCapacity > HM_MAXIMUM_CAPACITY)
			initialCapacity = HM_MAXIMUM_CAPACITY;

		_autoFreeValue = autoFreeValue;

		_size = 0;

		// Find a power of 2 >= initialCapacity
		_capacity = 1;
		while (_capacity < initialCapacity)
			_capacity <<= 1;

		_loadFactor = loadFactor;
		_threshold = (int) (_capacity * _loadFactor);
		_table = new Entry*[_capacity]();
		_entrySet = null;
	}

	/**
	 * Special-case code for containsValue with null argument
	 */
	boolean containsNullValue() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (e->value == null)
					return true;
		return false;
	}

public:
	/**
	 * Constructs an empty <tt>HashMap</tt> with the default initial capacity
	 * (16) and the default load factor (0.75).
	 */
	EHashMap() {
		init(HM_DEFAULT_INITIAL_CAPACITY, HM_DEFAULT_LOAD_FACTOR, true);
	}
	explicit
	EHashMap(boolean autoFreeValue) {
		init(HM_DEFAULT_INITIAL_CAPACITY, HM_DEFAULT_LOAD_FACTOR, autoFreeValue);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and the default load factor (0.75).
	 *
	 * @param  initialCapacity the initial capacity.
	 * @throws IllegalArgumentException if the initial capacity is negative.
	 */
	explicit
	EHashMap(uint initialCapacity, boolean autoFreeValue) {
		init(initialCapacity, HM_DEFAULT_LOAD_FACTOR, autoFreeValue);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and load factor.
	 *
	 * @param  initialCapacity the initial capacity
	 * @param  loadFactor      the load factor
	 * @throws IllegalArgumentException if the initial capacity is negative
	 *         or the load factor is nonpositive
	 */
	EHashMap(uint initialCapacity, float loadFactor, boolean autoFreeValue) {
		init(initialCapacity, loadFactor, autoFreeValue);
	}

	virtual ~EHashMap() {
		clear();

		delete[] _table;
	}

	EHashMap(const EHashMap<K, V>& that) {
		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_autoFreeValue = t->_autoFreeValue;
		t->setAutoFree(false);
		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;
	}

	EHashMap<K, V>& operator= (const EHashMap<K, V>& that) {
		if (this == &that) return *this;

		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		//1.
		clear();

		delete[] _table;

		//2.
		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_autoFreeValue = t->_autoFreeValue;
		t->setAutoFree(false);
		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;

		return *this;
	}

	// internal utilities

	/**
	 * Applies a supplemental hash function to a given hashCode, which
	 * defends against poor quality hash functions.  This is critical
	 * because HashMap uses power-of-two length hash tables, that
	 * otherwise encounter collisions for hashCodes that do not differ
	 * in lower bits. Note: Null keys always map to hash 0, thus index 0.
	 */
	static int hashIt(int h) {
		// This function ensures that hashCodes that differ only by
		// constant multiples at each bit position have a bounded
		// number of collisions (approximately 8 at default load factor).
		unsigned int uh = (unsigned int) h;
		uh ^= (uh >> 20) ^ (uh >> 12);
		return uh ^ (uh >> 7) ^ (uh >> 4);
	}

	/**
	 * Returns index for hash code h.
	 */
	static int indexFor(int h, int length) {
		return h & (length - 1);
	}

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of key-value mappings in this map
	 */
	int size() {
		return _size;
	}

	/**
	 * Returns <tt>true</tt> if this map contains no key-value mappings.
	 *
	 * @return <tt>true</tt> if this map contains no key-value mappings
	 */
	boolean isEmpty() {
		return _size == 0;
	}

	/**
	 * Returns the value to which the specified key is mapped,
	 * or {@code null} if this map contains no mapping for the key.
	 *
	 * <p>More formally, if this map contains a mapping from a key
	 * {@code k} to a value {@code v} such that {@code (key==null ? k==null :
	 * key.equals(k))}, then this method returns {@code v}; otherwise
	 * it returns {@code null}.  (There can be at most one such mapping.)
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
		int hash = hashIt(key);
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			if (e->hash == hash && (e->key == key))
				return e->value;
		}
		return null;
	}

	/**
	 * Returns <tt>true</tt> if this map contains a mapping for the
	 * specified key.
	 *
	 * @param   key   The key whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map contains a mapping for the specified
	 * key.
	 */
	boolean containsKey(K key) {
		return getEntry(key) != null;
	}

	/**
	 * Returns the entry associated with the specified key in the
	 * HashMap.  Returns null if the HashMap contains no mapping
	 * for the key.
	 */
	Entry* getEntry(K key) {
		int hash = hashIt(key);
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			if (e->hash == hash
					&& (e->key == key))
				return e;
		}
		return null;
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old
	 * value is replaced.
	 *
	 * @param key key with which the specified value is to be associated
	 * @param value value to be associated with the specified key
	 * @param absent test key is not exist
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V put(K key, V value, boolean *absent=null) {
		int hash = hashIt(key);
		int i = indexFor(hash, _capacity);
		for (Entry *e = _table[i]; e != null;
				e = e->next) {
			if (e->hash == hash && (e->key == key)) {
				if (absent) {
					*absent = false;
				}

				V oldValue = e->value;
				e->value = value;
				e->recordAccess(this);
				return oldValue;
			}
		}

		if (absent) {
			*absent = true;
		}
		addEntry(hash, key, value, i);
		return null;
	}

	/**
	 * Rehashes the contents of this map into a new array with a
	 * larger capacity.  This method is called automatically when the
	 * number of keys in this map reaches its _threshold.
	 *
	 * If current capacity is MAXIMUM_CAPACITY, this method does not
	 * resize the map, but sets _threshold to Integer.MAX_VALUE.
	 * This has the effect of preventing future calls.
	 *
	 * @param newCapacity the new capacity, MUST be a power of two;
	 *        must be greater than current capacity unless current
	 *        capacity is MAXIMUM_CAPACITY (in which case value
	 *        is irrelevant).
	 */
	void resize(int newCapacity) {
		int oldCapacity = _capacity;
		if (oldCapacity == HM_MAXIMUM_CAPACITY) {
			_threshold = EInteger::MAX_VALUE;
			return;
		}

		Entry **newTable = new Entry*[newCapacity]();
		transfer(newTable, newCapacity);
		delete[] _table; //!
		_table = newTable;
		_capacity = newCapacity;
		_threshold = (int) (newCapacity * _loadFactor);
	}

	/**
	 * Transfers all entries from current table to newTable.
	 */
	void transfer(Entry *newTable[],
			int newCapacity) {
		Entry **src = _table;
		for (int j = 0; j < (int)_capacity; j++) {
			Entry *e = src[j];
			if (e != null) {
				do {
					Entry *next = e->next;
					int i = indexFor(e->hash, newCapacity);
					e->next = newTable[i];
					newTable[i] = e;
					e = next;
				} while (e != null);
			}
		}
	}

	/**
	 * Removes the mapping for the specified key from this map if present.
	 *
	 * @param  key key whose mapping is to be removed from the map
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V remove(K key) {
		Entry *e = removeEntryForKey(key);
		if (e) {
			V v = e->value;
			e->value = null;
			delete e;
			return v;
		}
		return null;
	}

	/**
	 * Removes and returns the entry associated with the specified key
	 * in the HashMap.  Returns null if the HashMap contains no mapping
	 * for this key.
	 */
	Entry* removeEntryForKey(K key) {
		int hash = hashIt(key);
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			if (e->hash == hash
					&& (e->key == key)) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				e->next = null;
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Special version of remove for EntrySet.
	 */
	Entry* removeMapping(EMapEntry<K,V> *entry) {
		K key = entry->getKey();
		int hash = hashIt(key);
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			if (e->hash == hash && e->equals(entry)) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Removes all of the mappings from this map.
	 * The map will be empty after this call returns.
	 */
	void clear() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			if (tab[i] != null) {
				delete tab[i];
				tab[i] = null;
			}
		_size = 0;
	}

	/**
	 * Returns <tt>true</tt> if this map maps one or more keys to the
	 * specified value.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map maps one or more keys to the
	 *         specified value
	 */
	boolean containsValue(V value) {
		if (value == null)
			return containsNullValue();

		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (value->equals(e->value))
					return true;
		return false;
	}

	/**
	 * Adds a new entry with the specified key, value and hash code to
	 * the specified bucket.  It is the responsibility of this
	 * method to resize the table if appropriate.
	 *
	 * Subclass overrides this to alter the behavior of put method.
	 */
	void addEntry(int hash, K key, V value,
			int bucketIndex) {
		Entry *e = _table[bucketIndex];
		_table[bucketIndex] = new Entry(hash, key,
				value, e, this);
		if (_size++ >= _threshold)
			resize(2 * _capacity);
	}

	// Views

	/**
	 * Returns a {@link Set} view of the keys contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation), the results of
	 * the iteration are undefined.  The set supports element removal,
	 * which removes the corresponding mapping from the map, via the
	 * <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt>, and <tt>clear</tt>
	 * operations.  It does not support the <tt>add</tt> or <tt>addAll</tt>
	 * operations.
	 */
	sp<ESet<K> > keySet() {
		if (!EAbstractMap<K,V>::_keySet) {
			EAbstractMap<K,V>::_keySet = new Keys(this);
		}
		return EAbstractMap<K,V>::_keySet;
	}

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection is backed by the map, so changes to the map are
	 * reflected in the collection, and vice-versa.  If the map is
	 * modified while an iteration over the collection is in progress
	 * (except through the iterator's own <tt>remove</tt> operation),
	 * the results of the iteration are undefined.  The collection
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Collection.remove</tt>, <tt>removeAll</tt>,
	 * <tt>retainAll</tt> and <tt>clear</tt> operations.  It does not
	 * support the <tt>add</tt> or <tt>addAll</tt> operations.
	 */
	sp<ECollection<V> > values() {
		if (!EAbstractMap<K,V>::_values) {
			EAbstractMap<K,V>::_values = new Values(this);
		}
		return EAbstractMap<K,V>::_values;
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation, or through the
	 * <tt>setValue</tt> operation on a map entry returned by the
	 * iterator) the results of the iteration are undefined.  The set
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Set.remove</tt>, <tt>removeAll</tt>, <tt>retainAll</tt> and
	 * <tt>clear</tt> operations.  It does not support the
	 * <tt>add</tt> or <tt>addAll</tt> operations.
	 *
	 * @return a set view of the mappings contained in this map
	 */
	sp<ESet<EMapEntry<K, V>*> > entrySet() {
		if (_entrySet == null) {
			_entrySet = new EntrySet(this);
		}
		return _entrySet;
	}

	// These methods are used when serializing HashSets
	int capacity() {
		return _capacity;
	}
	float loadFactor() {
		return _loadFactor;
	}

	void setAutoFree(boolean autoFreeValue = true) {
		_autoFreeValue = autoFreeValue;
	}

	boolean getAutoFreeValue() {
		return _autoFreeValue;
	}
};

//=============================================================================
//Primitive Key && Shared pointer Value.

template<typename K, typename _V>
class EHashMap<K, sp<_V> > : public EAbstractMap<K, sp<_V> >,
		virtual public EMap<K, sp<_V> > {
public:
	typedef sp<_V> V;

	class Entry: public EMapEntry<K, V> {
	private:
		friend class EHashMap;
		K key;
		V value;
		Entry *next;
		EHashMap<K, V> *map;
		int hash;

	public:
		~Entry() {
			delete next; //!
		}

		/**
		 * Creates new entry.
		 */
		Entry(int h, K k, V v,
				Entry *n,
				EHashMap<K, V> *m) {
			value = v;
			next = n;
			key = k;
			hash = h;
			map = m;
		}

		K getKey() {
			return key;
		}

		V getValue() {
			return value;
		}

		V setValue(V newValue) {
			V oldValue = value;
			value = newValue;
			return oldValue;
		}

		boolean equals(EMapEntry<K, V> *e) {
			K k1 = getKey();
			K k2 = e->getKey();
			if (k1 == k2) {
				V v1 = getValue();
				V v2 = e->getValue();
				if (v1 == v2 || (v1 != null && v1->equals(v2.get())))
					return true;
			}
			return false;
		}

		virtual int hashCode() {
			return key ^ (value == null ? 0 : value->hashCode());
		}

		/**
		 * This method is invoked whenever the value in an entry is
		 * overwritten by an invocation of put(k,v) for a key k that's already
		 * in the HashMap.
		 */
		void recordAccess(EHashMap<K, V> *m) {
		}

		/**
		 * This method is invoked whenever the entry is
		 * removed from the table.
		 */
		void recordRemoval(EHashMap<K, V> *m) {
		}
	};

private:
	template<typename I>
	class HashIterator: public EIterator<I> {
	private:
		EHashMap<K,V> *_map;

		Entry *next; // next entry to return
		int index; // current slot
		Entry *current; // current entry
	public:
		HashIterator(EHashMap<K, V> *map) : next(null), index(0), current(null) {
			_map = map;

			if (_map->size() > 0) { // advance to first entry
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
		}

		boolean hasNext() {
			return next != null;
		}

		Entry* nextEntry() {
			Entry *e = next;
			if (e == null)
				throw ENOSUCHELEMENTEXCEPTION;

			if ((next = e->next) == null) {
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
			current = e;
			return e;
		}

		void remove() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			delete _map->removeEntryForKey(k);
		}

		Entry* moveOutEntry() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			return _map->removeEntryForKey(k);
		}
	};

	template<typename EI>
	class EntryIterator: public HashIterator<EI> {
	public:
		EntryIterator(EHashMap<K, V> *map) :
				HashIterator<EI>(map) {
		}

		EI next() {
			return HashIterator<EI>::nextEntry();
		}

		EI moveOut() {
			return HashIterator<EI>::moveOutEntry();
		}
	};

	class ValueIterator: public HashIterator<V> {
	public:
		ValueIterator(EHashMap<K, V> *map) :
				HashIterator<V>(map) {
		}

		V next() {
			return HashIterator<V>::nextEntry()->getValue();
		}

		V moveOut() {
			Entry* e = HashIterator<V>::moveOutEntry();
			V v = e->getValue();
			delete e;
			return v;
		}
	};

	class KeyIterator: public HashIterator<K> {
	public:
		KeyIterator(EHashMap<K, V> *map) :
				HashIterator<K>(map) {
		}

		K next() {
			return HashIterator<K>::nextEntry()->getKey();
		}

		K moveOut() {
			Entry* e = HashIterator<K>::moveOutEntry();
			K k = e->getKey();
			delete e;
			return k;
		}
	};

	class EntrySet: public EAbstractSet<EMapEntry<K,V>*> {
	private:
		EHashMap<K,V> *_map;

	public:
		EntrySet(EHashMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<EMapEntry<K,V>*> > iterator(int index=0) {
			return new EntryIterator<EMapEntry<K,V>*>(_map);
		}
		boolean contains(EMapEntry<K,V> *e) {
			EMapEntry<K,V> *candidate = _map->getEntry(e->getKey());
			return candidate != null && candidate->equals(e);
		}
		boolean remove(EMapEntry<K,V> *o) {
			return _map->removeMapping(o) != null;
		}
		int size() {
			return _map->size();
		}
		void clear() {
			_map->clear();
		}
	};

	class Values: public EAbstractCollection<V> {
	private:
		EHashMap<K, V> *_map;
	public:
		Values(EHashMap<K, V> *map) {
			_map = map;
		}
		~Values() {
		}
		sp<EIterator<V> > iterator(int index = 0) {
			return new ValueIterator(_map);
		}
		int size() {
			return _map->size();
		}
		boolean contains(_V* o) {
			return _map->containsValue(o);
		}
		void clear() {
			_map->clear();
		}
	};

	class Keys: public EAbstractSet<K> {
	private:
		EHashMap<K, V> *_map;
	public:
		Keys(EHashMap<K, V> *map) {
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
			return _map->removeEntryForKey(o) != null;
		}
		void clear() {
			_map->clear();
		}
	};

private:
	/**
	 * The table, resized as necessary. Length MUST Always be a power of two.
	 */
	Entry **_table;

	/**
	 * The size of _table.
	 */
	uint _capacity;

private:
	/**
	 * The number of key-value mappings contained in this map.
	 */
	int _size;

	/**
	 * The next size value at which to resize (capacity * load factor).
	 * @serial
	 */
	int _threshold;

	/**
	 * The load factor for the hash table.
	 *
	 * @serial
	 */
	float _loadFactor;

	// Views
	sp<ESet<EMapEntry<K, V>*> > _entrySet;

	/**
	 * Initialization hook for subclasses. This method is called
	 * in all constructors and pseudo-constructors (clone, readObject)
	 * after HashMap has been initialized but before any entries have
	 * been inserted.  (In the absence of this method, readObject would
	 * require explicit knowledge of subclasses.)
	 */
	void init(uint initialCapacity, float loadFactor) {
		if (initialCapacity < HM_DEFAULT_INITIAL_CAPACITY)
			initialCapacity = HM_DEFAULT_INITIAL_CAPACITY;
		if (initialCapacity > HM_MAXIMUM_CAPACITY)
			initialCapacity = HM_MAXIMUM_CAPACITY;
		_size = 0;

		// Find a power of 2 >= initialCapacity
		_capacity = 1;
		while (_capacity < initialCapacity)
			_capacity <<= 1;

		_loadFactor = loadFactor;
		_threshold = (int) (_capacity * _loadFactor);
		_table = new Entry*[_capacity]();
		_entrySet = null;
	}

	/**
	 * Special-case code for containsValue with null argument
	 */
	boolean containsNullValue() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (e->value == null)
					return true;
		return false;
	}

public:
	/**
	 * Constructs an empty <tt>HashMap</tt> with the default initial capacity
	 * (16) and the default load factor (0.75).
	 */
	EHashMap() {
		init(HM_DEFAULT_INITIAL_CAPACITY, HM_DEFAULT_LOAD_FACTOR);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and the default load factor (0.75).
	 *
	 * @param  initialCapacity the initial capacity.
	 * @throws IllegalArgumentException if the initial capacity is negative.
	 */
	explicit
	EHashMap(uint initialCapacity) {
		init(initialCapacity, HM_DEFAULT_LOAD_FACTOR);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and load factor.
	 *
	 * @param  initialCapacity the initial capacity
	 * @param  loadFactor      the load factor
	 * @throws IllegalArgumentException if the initial capacity is negative
	 *         or the load factor is nonpositive
	 */
	EHashMap(uint initialCapacity, float loadFactor) {
		init(initialCapacity, loadFactor);
	}

	virtual ~EHashMap() {
		clear();

		delete[] _table;
	}

	EHashMap(const EHashMap<K, V>& that) {
		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;
	}

	EHashMap<K, V>& operator= (const EHashMap<K, V>& that) {
		if (this == &that) return *this;

		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		//1.
		clear();

		delete[] _table;

		//2.
		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;

		return *this;
	}

	// internal utilities

	/**
	 * Applies a supplemental hash function to a given hashCode, which
	 * defends against poor quality hash functions.  This is critical
	 * because HashMap uses power-of-two length hash tables, that
	 * otherwise encounter collisions for hashCodes that do not differ
	 * in lower bits. Note: Null keys always map to hash 0, thus index 0.
	 */
	static int hashIt(int h) {
		// This function ensures that hashCodes that differ only by
		// constant multiples at each bit position have a bounded
		// number of collisions (approximately 8 at default load factor).
		unsigned int uh = (unsigned int) h;
		uh ^= (uh >> 20) ^ (uh >> 12);
		return uh ^ (uh >> 7) ^ (uh >> 4);
	}

	/**
	 * Returns index for hash code h.
	 */
	static int indexFor(int h, int length) {
		return h & (length - 1);
	}

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of key-value mappings in this map
	 */
	int size() {
		return _size;
	}

	/**
	 * Returns <tt>true</tt> if this map contains no key-value mappings.
	 *
	 * @return <tt>true</tt> if this map contains no key-value mappings
	 */
	boolean isEmpty() {
		return _size == 0;
	}

	/**
	 * Returns the value to which the specified key is mapped,
	 * or {@code null} if this map contains no mapping for the key.
	 *
	 * <p>More formally, if this map contains a mapping from a key
	 * {@code k} to a value {@code v} such that {@code (key==null ? k==null :
	 * key.equals(k))}, then this method returns {@code v}; otherwise
	 * it returns {@code null}.  (There can be at most one such mapping.)
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
		int hash = hashIt(key);
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			if (e->hash == hash && (e->key == key))
				return e->value;
		}
		return null;
	}

	/**
	 * Returns <tt>true</tt> if this map contains a mapping for the
	 * specified key.
	 *
	 * @param   key   The key whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map contains a mapping for the specified
	 * key.
	 */
	boolean containsKey(K key) {
		return getEntry(key) != null;
	}

	/**
	 * Returns the entry associated with the specified key in the
	 * HashMap.  Returns null if the HashMap contains no mapping
	 * for the key.
	 */
	Entry* getEntry(K key) {
		int hash = hashIt(key);
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			if (e->hash == hash
					&& (e->key == key))
				return e;
		}
		return null;
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old
	 * value is replaced.
	 *
	 * @param key key with which the specified value is to be associated
	 * @param value value to be associated with the specified key
	 * @param absent test key is not exist
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V put(K key, V value, boolean *absent=null) {
		int hash = hashIt(key);
		int i = indexFor(hash, _capacity);
		for (Entry *e = _table[i]; e != null;
				e = e->next) {
			if (e->hash == hash && (e->key == key)) {
				if (absent) {
					*absent = false;
				}

				V oldValue = e->value;
				e->value = value;
				e->recordAccess(this);
				return oldValue;
			}
		}

		if (absent) {
			*absent = true;
		}
		addEntry(hash, key, value, i);
		return null;
	}

	/**
	 * Rehashes the contents of this map into a new array with a
	 * larger capacity.  This method is called automatically when the
	 * number of keys in this map reaches its _threshold.
	 *
	 * If current capacity is MAXIMUM_CAPACITY, this method does not
	 * resize the map, but sets _threshold to Integer.MAX_VALUE.
	 * This has the effect of preventing future calls.
	 *
	 * @param newCapacity the new capacity, MUST be a power of two;
	 *        must be greater than current capacity unless current
	 *        capacity is MAXIMUM_CAPACITY (in which case value
	 *        is irrelevant).
	 */
	void resize(int newCapacity) {
		int oldCapacity = _capacity;
		if (oldCapacity == HM_MAXIMUM_CAPACITY) {
			_threshold = EInteger::MAX_VALUE;
			return;
		}

		Entry **newTable = new Entry*[newCapacity]();
		transfer(newTable, newCapacity);
		delete[] _table; //!
		_table = newTable;
		_capacity = newCapacity;
		_threshold = (int) (newCapacity * _loadFactor);
	}

	/**
	 * Transfers all entries from current table to newTable.
	 */
	void transfer(Entry *newTable[],
			int newCapacity) {
		Entry **src = _table;
		for (int j = 0; j < (int)_capacity; j++) {
			Entry *e = src[j];
			if (e != null) {
				do {
					Entry *next = e->next;
					int i = indexFor(e->hash, newCapacity);
					e->next = newTable[i];
					newTable[i] = e;
					e = next;
				} while (e != null);
			}
		}
	}

	/**
	 * Removes the mapping for the specified key from this map if present.
	 *
	 * @param  key key whose mapping is to be removed from the map
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V remove(K key) {
		Entry *e = removeEntryForKey(key);
		if (e) {
			V v = e->value;
			e->value = null;
			delete e;
			return v;
		}
		return null;
	}

	/**
	 * Removes and returns the entry associated with the specified key
	 * in the HashMap.  Returns null if the HashMap contains no mapping
	 * for this key.
	 */
	Entry* removeEntryForKey(K key) {
		int hash = hashIt(key);
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			if (e->hash == hash
					&& (e->key == key)) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				e->next = null;
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Special version of remove for EntrySet.
	 */
	Entry* removeMapping(EMapEntry<K,V> *entry) {
		K key = entry->getKey();
		int hash = hashIt(key);
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			if (e->hash == hash && e->equals(entry)) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Removes all of the mappings from this map.
	 * The map will be empty after this call returns.
	 */
	void clear() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			if (tab[i] != null) {
				delete tab[i];
				tab[i] = null;
			}
		_size = 0;
	}

	/**
	 * Returns <tt>true</tt> if this map maps one or more keys to the
	 * specified value.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map maps one or more keys to the
	 *         specified value
	 */
	boolean containsValue(_V* value) {
		if (value == null)
			return containsNullValue();

		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (value->equals(e->value.get()))
					return true;
		return false;
	}

	/**
	 * Adds a new entry with the specified key, value and hash code to
	 * the specified bucket.  It is the responsibility of this
	 * method to resize the table if appropriate.
	 *
	 * Subclass overrides this to alter the behavior of put method.
	 */
	void addEntry(int hash, K key, V value,
			int bucketIndex) {
		Entry *e = _table[bucketIndex];
		_table[bucketIndex] = new Entry(hash, key,
				value, e, this);
		if (_size++ >= _threshold)
			resize(2 * _capacity);
	}

	// Views

	/**
	 * Returns a {@link Set} view of the keys contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation), the results of
	 * the iteration are undefined.  The set supports element removal,
	 * which removes the corresponding mapping from the map, via the
	 * <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt>, and <tt>clear</tt>
	 * operations.  It does not support the <tt>add</tt> or <tt>addAll</tt>
	 * operations.
	 */
	sp<ESet<K> > keySet() {
		if (!EAbstractMap<K,V>::_keySet) {
			EAbstractMap<K,V>::_keySet = new Keys(this);
		}
		return EAbstractMap<K,V>::_keySet;
	}

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection is backed by the map, so changes to the map are
	 * reflected in the collection, and vice-versa.  If the map is
	 * modified while an iteration over the collection is in progress
	 * (except through the iterator's own <tt>remove</tt> operation),
	 * the results of the iteration are undefined.  The collection
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Collection.remove</tt>, <tt>removeAll</tt>,
	 * <tt>retainAll</tt> and <tt>clear</tt> operations.  It does not
	 * support the <tt>add</tt> or <tt>addAll</tt> operations.
	 */
	sp<ECollection<V> > values() {
		if (!EAbstractMap<K,V>::_values) {
			EAbstractMap<K,V>::_values = new Values(this);
		}
		return EAbstractMap<K,V>::_values;
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation, or through the
	 * <tt>setValue</tt> operation on a map entry returned by the
	 * iterator) the results of the iteration are undefined.  The set
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Set.remove</tt>, <tt>removeAll</tt>, <tt>retainAll</tt> and
	 * <tt>clear</tt> operations.  It does not support the
	 * <tt>add</tt> or <tt>addAll</tt> operations.
	 *
	 * @return a set view of the mappings contained in this map
	 */
	sp<ESet<EMapEntry<K, V>*> > entrySet() {
		if (_entrySet == null) {
			_entrySet = new EntrySet(this);
		}
		return _entrySet;
	}

	// These methods are used when serializing HashSets
	int capacity() {
		return _capacity;
	}
	float loadFactor() {
		return _loadFactor;
	}
};

//=============================================================================
//Native poiner Types.

template<typename _K, typename _V>
class EHashMap<_K*, _V*>: public EAbstractMap<_K*, _V*>,
		virtual public EMap<_K*, _V*> {
public:
	typedef _K* K;
	typedef _V* V;

	class Entry: public EMapEntry<K, V> {
	private:
		friend class EHashMap;
		K key;
		V value;
		Entry *next;
		EHashMap<K, V> *map;
		int hash;

	public:
		~Entry() {
			if (map->getAutoFreeKey()) {
				delete key;
			}
			if (map->getAutoFreeValue()) {
				delete value;
			}
			delete next; //!
		}

		/**
		 * Creates new entry.
		 */
		Entry(int h, K k, V v,
				Entry *n,
				EHashMap<K, V> *m) {
			value = v;
			next = n;
			key = k;
			hash = h;
			map = m;
		}

		K getKey() {
			return key;
		}

		V getValue() {
			return value;
		}

		V setValue(V newValue) {
			V oldValue = value;
			value = newValue;
			return oldValue;
		}

		boolean equals(EMapEntry<K, V> *e) {
			K k1 = getKey();
			K k2 = e->getKey();
			if (k1 == k2 || (k1 != null && k1->equals(k2))) {
				V v1 = getValue();
				V v2 = e->getValue();
				if (v1 == v2 || (v1 != null && v1->equals(v2)))
					return true;
			}
			return false;
		}

		virtual int hashCode() {
			return (key == null ? 0 : key->hashCode())
					^ (value == null ? 0 : value->hashCode());
		}

		/**
		 * This method is invoked whenever the value in an entry is
		 * overwritten by an invocation of put(k,v) for a key k that's already
		 * in the HashMap.
		 */
		void recordAccess(EHashMap<K, V> *m) {
		}

		/**
		 * This method is invoked whenever the entry is
		 * removed from the table.
		 */
		void recordRemoval(EHashMap<K, V> *m) {
		}
	};

private:
	template<typename I>
	class HashIterator: public EIterator<I> {
	private:
		EHashMap<K,V> *_map;

		Entry *next; // next entry to return
		int index; // current slot
		Entry *current; // current entry
	public:
		HashIterator(EHashMap<K, V> *map) : next(null), index(0), current(null) {
			_map = map;

			if (_map->size() > 0) { // advance to first entry
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
		}

		boolean hasNext() {
			return next != null;
		}

		Entry* nextEntry() {
			Entry *e = next;
			if (e == null)
				throw ENOSUCHELEMENTEXCEPTION;

			if ((next = e->next) == null) {
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
			current = e;
			return e;
		}

		void remove() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			delete _map->removeEntryForKey(k);
		}

		Entry* moveOutEntry() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			return _map->removeEntryForKey(k);
		}
	};

	template<typename EI>
	class EntryIterator: public HashIterator<EI> {
	public:
		EntryIterator(EHashMap<K, V> *map) :
				HashIterator<EI>(map) {
		}

		EI next() {
			return HashIterator<EI>::nextEntry();
		}

		EI moveOut() {
			return HashIterator<EI>::moveOutEntry();
		}
	};

	class ValueIterator: public HashIterator<V> {
	public:
		ValueIterator(EHashMap<K, V> *map) :
				HashIterator<V>(map) {
		}

		V next() {
			return HashIterator<V>::nextEntry()->getValue();
		}

		V moveOut() {
			Entry* e = HashIterator<V>::moveOutEntry();
			V v = e->getValue();
			delete e;
			return v;
		}
	};

	class KeyIterator: public HashIterator<K> {
	public:
		KeyIterator(EHashMap<K, V> *map) :
				HashIterator<K>(map) {
		}

		K next() {
			return HashIterator<K>::nextEntry()->getKey();
		}

		K moveOut() {
			Entry* e = HashIterator<K>::moveOutEntry();
			K k = e->getKey();
			delete e;
			return k;
		}
	};

	class EntrySet: public EAbstractSet<EMapEntry<K,V>*> {
	private:
		EHashMap<K,V> *_map;

	public:
		EntrySet(EHashMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<EMapEntry<K,V>*> > iterator(int index=0) {
			return new EntryIterator<EMapEntry<K,V>*>(_map);
		}
		boolean contains(EMapEntry<K,V> *e) {
			EMapEntry<K,V> *candidate = _map->getEntry(e->getKey());
			return candidate != null && candidate->equals(e);
		}
		boolean remove(EMapEntry<K,V> *o) {
			return _map->removeMapping(o) != null;
		}
		int size() {
			return _map->size();
		}
		void clear() {
			_map->clear();
		}
	};

	class Values: public EAbstractCollection<V> {
	private:
		EHashMap<K, V> *_map;
	public:
		Values(EHashMap<K, V> *map) {
			_map = map;
		}
		~Values() {
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

	class Keys: public EAbstractSet<K> {
	private:
		EHashMap<K, V> *_map;
	public:
		Keys(EHashMap<K, V> *map) {
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
			return _map->removeEntryForKey(o) != null;
		}
		void clear() {
			_map->clear();
		}
	};

private:
	/**
	 * The table, resized as necessary. Length MUST Always be a power of two.
	 */
	Entry **_table;

	/**
	 * The size of _table.
	 */
	uint _capacity;

private:

	/**
	 * Auto free object flag
	 */
	boolean _autoFreeKey;
	boolean _autoFreeValue;

	/**
	 * The number of key-value mappings contained in this map.
	 */
	int _size;

	/**
	 * The next size value at which to resize (capacity * load factor).
	 * @serial
	 */
	int _threshold;

	/**
	 * The load factor for the hash table.
	 *
	 * @serial
	 */
	float _loadFactor;

	// Views
	sp<ESet<EMapEntry<K, V>*> > _entrySet;

	/**
	 * Initialization hook for subclasses. This method is called
	 * in all constructors and pseudo-constructors (clone, readObject)
	 * after HashMap has been initialized but before any entries have
	 * been inserted.  (In the absence of this method, readObject would
	 * require explicit knowledge of subclasses.)
	 */
	void init(uint initialCapacity, float loadFactor, boolean autoFreeKey, boolean autoFreeValue) {
		if (initialCapacity < HM_DEFAULT_INITIAL_CAPACITY)
			initialCapacity = HM_DEFAULT_INITIAL_CAPACITY;
		if (initialCapacity > HM_MAXIMUM_CAPACITY)
			initialCapacity = HM_MAXIMUM_CAPACITY;

		_autoFreeKey = autoFreeKey;
		_autoFreeValue = autoFreeValue;

		_size = 0;

		// Find a power of 2 >= initialCapacity
		_capacity = 1;
		while (_capacity < initialCapacity)
			_capacity <<= 1;

		_loadFactor = loadFactor;
		_threshold = (int) (_capacity * _loadFactor);
		_table = new Entry*[_capacity]();
		_entrySet = null;
	}

	/**
	 * Offloaded version of get() to look up null keys.  Null keys map
	 * to index 0.  This null case is split out into separate methods
	 * for the sake of performance in the two most commonly used
	 * operations (get and put), but incorporated with conditionals in
	 * others.
	 */
	V getForNullKey() {
		for (Entry *e = _table[0]; e != null;
				e = e->next) {
			if (e->key == null)
				return e->value;
		}
		return null;
	}

	/**
	 * Offloaded version of put for null keys
	 */
	V putForNullKey(V value) {
		for (Entry *e = _table[0]; e != null;
				e = e->next) {
			if (e->key == null) {
				V oldValue = e->value;
				e->value = value;
				e->recordAccess(this);
				return oldValue;
			}
		}
		addEntry(0, null, value, 0);
		return null;
	}

	/**
	 * Special-case code for containsValue with null argument
	 */
	boolean containsNullValue() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (e->value == null)
					return true;
		return false;
	}

public:
	/**
	 * Constructs an empty <tt>HashMap</tt> with the default initial capacity
	 * (16) and the default load factor (0.75).
	 */
	EHashMap() {
		init(HM_DEFAULT_INITIAL_CAPACITY, HM_DEFAULT_LOAD_FACTOR, true, true);
	}
	explicit
	EHashMap(boolean autoFreeKey, boolean autoFreeValue) {
		init(HM_DEFAULT_INITIAL_CAPACITY, HM_DEFAULT_LOAD_FACTOR, autoFreeKey, autoFreeValue);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and the default load factor (0.75).
	 *
	 * @param  initialCapacity the initial capacity.
	 * @throws IllegalArgumentException if the initial capacity is negative.
	 */
	explicit
	EHashMap(uint initialCapacity) {
		init(initialCapacity, HM_DEFAULT_LOAD_FACTOR, true, true);
	}
	explicit
	EHashMap(uint initialCapacity, boolean autoFreeKey, boolean autoFreeValue) {
		init(initialCapacity, HM_DEFAULT_LOAD_FACTOR, autoFreeKey, autoFreeValue);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and load factor.
	 *
	 * @param  initialCapacity the initial capacity
	 * @param  loadFactor      the load factor
	 * @throws IllegalArgumentException if the initial capacity is negative
	 *         or the load factor is nonpositive
	 */
	explicit
	EHashMap(uint initialCapacity, float loadFactor, boolean autoFreeKey, boolean autoFreeValue) {
		init(initialCapacity, loadFactor, autoFreeKey, autoFreeValue);
	}

	virtual ~EHashMap() {
		clear();

		delete[] _table;
	}

	EHashMap(const EHashMap<K, V>& that) {
		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_autoFreeKey = t->_autoFreeKey;
		_autoFreeValue = t->_autoFreeValue;
		t->setAutoFree(false, false);
		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;
	}

	EHashMap<K, V>& operator= (const EHashMap<K, V>& that) {
		if (this == &that) return *this;

		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		//1.
		clear();

		delete[] _table;

		//2.
		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_autoFreeKey = t->_autoFreeKey;
		_autoFreeValue = t->_autoFreeValue;
		t->setAutoFree(false, false);
		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;

		return *this;
	}

	// internal utilities

	/**
	 * Applies a supplemental hash function to a given hashCode, which
	 * defends against poor quality hash functions.  This is critical
	 * because HashMap uses power-of-two length hash tables, that
	 * otherwise encounter collisions for hashCodes that do not differ
	 * in lower bits. Note: Null keys always map to hash 0, thus index 0.
	 */
	static int hashIt(int h) {
		// This function ensures that hashCodes that differ only by
		// constant multiples at each bit position have a bounded
		// number of collisions (approximately 8 at default load factor).
		unsigned int uh = (unsigned int) h;
		uh ^= (uh >> 20) ^ (uh >> 12);
		return uh ^ (uh >> 7) ^ (uh >> 4);
	}

	/**
	 * Returns index for hash code h.
	 */
	static int indexFor(int h, int length) {
		return h & (length - 1);
	}

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of key-value mappings in this map
	 */
	int size() {
		return _size;
	}

	/**
	 * Returns <tt>true</tt> if this map contains no key-value mappings.
	 *
	 * @return <tt>true</tt> if this map contains no key-value mappings
	 */
	boolean isEmpty() {
		return _size == 0;
	}

	/**
	 * Returns the value to which the specified key is mapped,
	 * or {@code null} if this map contains no mapping for the key.
	 *
	 * <p>More formally, if this map contains a mapping from a key
	 * {@code k} to a value {@code v} such that {@code (key==null ? k==null :
	 * key.equals(k))}, then this method returns {@code v}; otherwise
	 * it returns {@code null}.  (There can be at most one such mapping.)
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
		if (key == null)
			return getForNullKey();
		int hash = hashIt(key->hashCode());
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			K k;
			if (e->hash == hash && ((k = e->key) == key || key->equals(k)))
				return e->value;
		}
		return null;
	}

	/**
	 * Returns <tt>true</tt> if this map contains a mapping for the
	 * specified key.
	 *
	 * @param   key   The key whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map contains a mapping for the specified
	 * key.
	 */
	boolean containsKey(K key) {
		return getEntry(key) != null;
	}

	/**
	 * Returns the entry associated with the specified key in the
	 * HashMap.  Returns null if the HashMap contains no mapping
	 * for the key.
	 */
	Entry* getEntry(K key) {
		int hash = (key == null) ? 0 : hashIt(key->hashCode());
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			K k;
			if (e->hash == hash
					&& ((k = e->key) == key || (key != null && key->equals(k))))
				return e;
		}
		return null;
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old
	 * value is replaced.
	 *
	 * @param key key with which the specified value is to be associated
	 * @param value value to be associated with the specified key
	 * @param absent test key is not exist
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V put(K key, V value, boolean *absent=null) {
		if (key == null)
			return putForNullKey(value);
		int hash = hashIt(key->hashCode());
		int i = indexFor(hash, _capacity);
		for (Entry *e = _table[i]; e != null;
				e = e->next) {
			K k;
			if (e->hash == hash && ((k = e->key) == key || key->equals(k))) {
				if (absent) {
					*absent = false;
				}
				if (_autoFreeKey && key != e->key) {
					delete key; //!
				}

				V oldValue = e->value;
				e->value = value;
				e->recordAccess(this);
				return oldValue;
			}
		}

		if (absent) {
			*absent = true;
		}
		addEntry(hash, key, value, i);
		return null;
	}

	/**
	 * Rehashes the contents of this map into a new array with a
	 * larger capacity.  This method is called automatically when the
	 * number of keys in this map reaches its _threshold.
	 *
	 * If current capacity is MAXIMUM_CAPACITY, this method does not
	 * resize the map, but sets _threshold to Integer.MAX_VALUE.
	 * This has the effect of preventing future calls.
	 *
	 * @param newCapacity the new capacity, MUST be a power of two;
	 *        must be greater than current capacity unless current
	 *        capacity is MAXIMUM_CAPACITY (in which case value
	 *        is irrelevant).
	 */
	void resize(int newCapacity) {
		int oldCapacity = _capacity;
		if (oldCapacity == HM_MAXIMUM_CAPACITY) {
			_threshold = EInteger::MAX_VALUE;
			return;
		}

		Entry **newTable = new Entry*[newCapacity]();
		transfer(newTable, newCapacity);
		delete[] _table; //!
		_table = newTable;
		_capacity = newCapacity;
		_threshold = (int) (newCapacity * _loadFactor);
	}

	/**
	 * Transfers all entries from current table to newTable.
	 */
	void transfer(Entry *newTable[],
			int newCapacity) {
		Entry **src = _table;
		for (int j = 0; j < (int)_capacity; j++) {
			Entry *e = src[j];
			if (e != null) {
				do {
					Entry *next = e->next;
					int i = indexFor(e->hash, newCapacity);
					e->next = newTable[i];
					newTable[i] = e;
					e = next;
				} while (e != null);
			}
		}
	}

	/**
	 * Removes the mapping for the specified key from this map if present.
	 *
	 * @param  key key whose mapping is to be removed from the map
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V remove(K key) {
		Entry *e = removeEntryForKey(key);
		if (e) {
			V v = e->value;
			e->value = null;
			delete e;
			return v;
		}
		return null;
	}

	/**
	 * Removes and returns the entry associated with the specified key
	 * in the HashMap.  Returns null if the HashMap contains no mapping
	 * for this key.
	 */
	Entry* removeEntryForKey(K key) {
		int hash = (key == null) ? 0 : hashIt(key->hashCode());
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			K k;
			if (e->hash == hash
					&& ((k = e->key) == key || (key != null && key->equals(k)))) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				e->next = null;
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Special version of remove for EntrySet.
	 */
	Entry* removeMapping(EMapEntry<K,V> *entry) {
		K key = entry->getKey();
		int hash = (key == null) ? 0 : hashIt(key->hashCode());
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			if (e->hash == hash && e->equals(entry)) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Removes all of the mappings from this map.
	 * The map will be empty after this call returns.
	 */
	void clear() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			if (tab[i] != null) {
				delete tab[i];
				tab[i] = null;
			}
		_size = 0;
	}

	/**
	 * Returns <tt>true</tt> if this map maps one or more keys to the
	 * specified value.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map maps one or more keys to the
	 *         specified value
	 */
	boolean containsValue(V value) {
		if (value == null)
			return containsNullValue();

		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (value->equals(e->value))
					return true;
		return false;
	}

	/**
	 * Adds a new entry with the specified key, value and hash code to
	 * the specified bucket.  It is the responsibility of this
	 * method to resize the table if appropriate.
	 *
	 * Subclass overrides this to alter the behavior of put method.
	 */
	void addEntry(int hash, K key, V value,
			int bucketIndex) {
		Entry *e = _table[bucketIndex];
		_table[bucketIndex] = new Entry(hash, key,
				value, e, this);
		if (_size++ >= _threshold)
			resize(2 * _capacity);
	}

	// Views

	/**
	 * Returns a {@link Set} view of the keys contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation), the results of
	 * the iteration are undefined.  The set supports element removal,
	 * which removes the corresponding mapping from the map, via the
	 * <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt>, and <tt>clear</tt>
	 * operations.  It does not support the <tt>add</tt> or <tt>addAll</tt>
	 * operations.
	 */
	sp<ESet<K> > keySet() {
		if (!EAbstractMap<K,V>::_keySet) {
			EAbstractMap<K,V>::_keySet = new Keys(this);
		}
		return EAbstractMap<K,V>::_keySet;
	}

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection is backed by the map, so changes to the map are
	 * reflected in the collection, and vice-versa.  If the map is
	 * modified while an iteration over the collection is in progress
	 * (except through the iterator's own <tt>remove</tt> operation),
	 * the results of the iteration are undefined.  The collection
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Collection.remove</tt>, <tt>removeAll</tt>,
	 * <tt>retainAll</tt> and <tt>clear</tt> operations.  It does not
	 * support the <tt>add</tt> or <tt>addAll</tt> operations.
	 */
	sp<ECollection<V> > values() {
		if (!EAbstractMap<K,V>::_values) {
			EAbstractMap<K,V>::_values = new Values(this);
		}
		return EAbstractMap<K,V>::_values;
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation, or through the
	 * <tt>setValue</tt> operation on a map entry returned by the
	 * iterator) the results of the iteration are undefined.  The set
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Set.remove</tt>, <tt>removeAll</tt>, <tt>retainAll</tt> and
	 * <tt>clear</tt> operations.  It does not support the
	 * <tt>add</tt> or <tt>addAll</tt> operations.
	 *
	 * @return a set view of the mappings contained in this map
	 */
	sp<ESet<EMapEntry<K, V>*> > entrySet() {
		if (_entrySet == null) {
			_entrySet = new EntrySet(this);
		}
		return _entrySet;
	}

	// These methods are used when serializing HashSets
	int capacity() {
		return _capacity;
	}
	float loadFactor() {
		return _loadFactor;
	}

	void setAutoFree(boolean autoFreeKey, boolean autoFreeValue) {
		_autoFreeKey = autoFreeKey;
		_autoFreeValue = autoFreeValue;
	}

	boolean getAutoFreeKey() {
		return _autoFreeKey;
	}

	boolean getAutoFreeValue() {
		return _autoFreeValue;
	}
};

//=============================================================================
//Shared poiner Types.

template<typename _K, typename _V>
class EHashMap<sp<_K>, sp<_V> >: public EAbstractMap<sp<_K>, sp<_V> >,
		virtual public EMap<sp<_K>, sp<_V> > {
public:
	typedef sp<_K> K;
	typedef sp<_V> V;

	class Entry: public EMapEntry<K, V> {
	private:
		friend class EHashMap;
		K key;
		V value;
		Entry *next;
		EHashMap<K, V> *map;
		int hash;

	public:
		~Entry() {
			delete next; //!
		}

		/**
		 * Creates new entry.
		 */
		Entry(int h, K k, V v,
				Entry *n,
				EHashMap<K, V> *m) {
			value = v;
			next = n;
			key = k;
			hash = h;
			map = m;
		}

		K getKey() {
			return key;
		}

		V getValue() {
			return value;
		}

		V setValue(V newValue) {
			V oldValue = value;
			value = newValue;
			return oldValue;
		}

		boolean equals(EMapEntry<K, V> *e) {
			K k1 = getKey();
			K k2 = e->getKey();
			if (k1 == k2 || (k1 != null && k1->equals(k2.get()))) {
				V v1 = getValue();
				V v2 = e->getValue();
				if (v1 == v2 || (v1 != null && v1->equals(v2.get())))
					return true;
			}
			return false;
		}

		virtual int hashCode() {
			return (key == null ? 0 : key->hashCode())
					^ (value == null ? 0 : value->hashCode());
		}

		/**
		 * This method is invoked whenever the value in an entry is
		 * overwritten by an invocation of put(k,v) for a key k that's already
		 * in the HashMap.
		 */
		void recordAccess(EHashMap<K, V> *m) {
		}

		/**
		 * This method is invoked whenever the entry is
		 * removed from the table.
		 */
		void recordRemoval(EHashMap<K, V> *m) {
		}
	};

private:
	template<typename I>
	class HashIterator: public EIterator<I> {
	private:
		EHashMap<K,V> *_map;

		Entry *next; // next entry to return
		int index; // current slot
		Entry *current; // current entry
	public:
		HashIterator(EHashMap<K, V> *map) : next(null), index(0), current(null) {
			_map = map;

			if (_map->size() > 0) { // advance to first entry
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
		}

		boolean hasNext() {
			return next != null;
		}

		Entry* nextEntry() {
			Entry *e = next;
			if (e == null)
				throw ENOSUCHELEMENTEXCEPTION;

			if ((next = e->next) == null) {
				Entry **t = _map->_table;
				while (index < (int)_map->_capacity && (next = t[index++]) == null)
					;
			}
			current = e;
			return e;
		}

		void remove() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			delete _map->removeEntryForKey(k);
		}

		Entry* moveOutEntry() {
			if (current == null)
				throw EILLEGALSTATEEXCEPTION;
			K k = current->key;
			current = null;
			return _map->removeEntryForKey(k);
		}
	};

	template<typename EI>
	class EntryIterator: public HashIterator<EI> {
	public:
		EntryIterator(EHashMap<K, V> *map) :
				HashIterator<EI>(map) {
		}

		EI next() {
			return HashIterator<EI>::nextEntry();
		}

		EI moveOut() {
			return HashIterator<EI>::moveOutEntry();
		}
	};

	class ValueIterator: public HashIterator<V> {
	public:
		ValueIterator(EHashMap<K, V> *map) :
				HashIterator<V>(map) {
		}

		V next() {
			return HashIterator<V>::nextEntry()->getValue();
		}

		V moveOut() {
			Entry* e = HashIterator<V>::moveOutEntry();
			V v = e->getValue();
			delete e;
			return v;
		}
	};

	class KeyIterator: public HashIterator<K> {
	public:
		KeyIterator(EHashMap<K, V> *map) :
				HashIterator<K>(map) {
		}

		K next() {
			return HashIterator<K>::nextEntry()->getKey();
		}

		K moveOut() {
			Entry* e = HashIterator<K>::moveOutEntry();
			K k = e->getKey();
			delete e;
			return k;
		}
	};

	class EntrySet: public EAbstractSet<EMapEntry<K,V>*> {
	private:
		EHashMap<K,V> *_map;

	public:
		EntrySet(EHashMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<EMapEntry<K,V>*> > iterator(int index=0) {
			return new EntryIterator<EMapEntry<K,V>*>(_map);
		}
		boolean contains(EMapEntry<K,V> *e) {
			EMapEntry<K,V> *candidate = _map->getEntry(e->getKey());
			return candidate != null && candidate->equals(e);
		}
		boolean remove(EMapEntry<K,V> *o) {
			return _map->removeMapping(o) != null;
		}
		int size() {
			return _map->size();
		}
		void clear() {
			_map->clear();
		}
	};

	class Values: public EAbstractCollection<V> {
	private:
		EHashMap<K, V> *_map;
	public:
		Values(EHashMap<K, V> *map) {
			_map = map;
		}
		~Values() {
		}
		sp<EIterator<V> > iterator(int index = 0) {
			return new ValueIterator(_map);
		}
		int size() {
			return _map->size();
		}
		boolean contains(_V* o) {
			return _map->containsValue(o);
		}
		void clear() {
			_map->clear();
		}
	};

	class Keys: public EAbstractSet<K> {
	private:
		EHashMap<K, V> *_map;
	public:
		Keys(EHashMap<K, V> *map) {
			_map = map;
		}
		sp<EIterator<K> > iterator(int index = 0) {
			return new KeyIterator(_map);
		}
		int size() {
			return _map->size();
		}
		boolean contains(_K* o) {
			return _map->containsKey(o);
		}
		boolean remove(_K* o) {
			return _map->removeEntryForKey(o) != null;
		}
		void clear() {
			_map->clear();
		}
	};

private:
	/**
	 * The table, resized as necessary. Length MUST Always be a power of two.
	 */
	Entry **_table;

	/**
	 * The size of _table.
	 */
	uint _capacity;

private:
	/**
	 * The number of key-value mappings contained in this map.
	 */
	int _size;

	/**
	 * The next size value at which to resize (capacity * load factor).
	 * @serial
	 */
	int _threshold;

	/**
	 * The load factor for the hash table.
	 *
	 * @serial
	 */
	float _loadFactor;

	// Views
	sp<ESet<EMapEntry<K, V>*> > _entrySet;

	/**
	 * Initialization hook for subclasses. This method is called
	 * in all constructors and pseudo-constructors (clone, readObject)
	 * after HashMap has been initialized but before any entries have
	 * been inserted.  (In the absence of this method, readObject would
	 * require explicit knowledge of subclasses.)
	 */
	void init(uint initialCapacity, float loadFactor) {
		if (initialCapacity < HM_DEFAULT_INITIAL_CAPACITY)
			initialCapacity = HM_DEFAULT_INITIAL_CAPACITY;
		if (initialCapacity > HM_MAXIMUM_CAPACITY)
			initialCapacity = HM_MAXIMUM_CAPACITY;
		_size = 0;

		// Find a power of 2 >= initialCapacity
		_capacity = 1;
		while (_capacity < initialCapacity)
			_capacity <<= 1;

		_loadFactor = loadFactor;
		_threshold = (int) (_capacity * _loadFactor);
		_table = new Entry*[_capacity]();
		_entrySet = null;
	}

	/**
	 * Offloaded version of get() to look up null keys.  Null keys map
	 * to index 0.  This null case is split out into separate methods
	 * for the sake of performance in the two most commonly used
	 * operations (get and put), but incorporated with conditionals in
	 * others.
	 */
	V getForNullKey() {
		for (Entry *e = _table[0]; e != null;
				e = e->next) {
			if (e->key == null)
				return e->value;
		}
		return null;
	}

	/**
	 * Offloaded version of put for null keys
	 */
	V putForNullKey(V value) {
		for (Entry *e = _table[0]; e != null;
				e = e->next) {
			if (e->key == null) {
				V oldValue = e->value;
				e->value = value;
				e->recordAccess(this);
				return oldValue;
			}
		}
		addEntry(0, null, value, 0);
		return null;
	}

	/**
	 * Special-case code for containsValue with null argument
	 */
	boolean containsNullValue() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (e->value == null)
					return true;
		return false;
	}

public:
	/**
	 * Constructs an empty <tt>HashMap</tt> with the default initial capacity
	 * (16) and the default load factor (0.75).
	 */
	EHashMap() {
		init(HM_DEFAULT_INITIAL_CAPACITY, HM_DEFAULT_LOAD_FACTOR);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and the default load factor (0.75).
	 *
	 * @param  initialCapacity the initial capacity.
	 * @throws IllegalArgumentException if the initial capacity is negative.
	 */
	explicit
	EHashMap(uint initialCapacity) {
		init(initialCapacity, HM_DEFAULT_LOAD_FACTOR);
	}

	/**
	 * Constructs an empty <tt>HashMap</tt> with the specified initial
	 * capacity and load factor.
	 *
	 * @param  initialCapacity the initial capacity
	 * @param  loadFactor      the load factor
	 * @throws IllegalArgumentException if the initial capacity is negative
	 *         or the load factor is nonpositive
	 */
	explicit
	EHashMap(uint initialCapacity, float loadFactor) {
		init(initialCapacity, loadFactor);
	}

	virtual ~EHashMap() {
		clear();

		delete[] _table;
	}

	EHashMap(const EHashMap<K, V>& that) {
		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;
	}

	EHashMap<K, V>& operator= (const EHashMap<K, V>& that) {
		if (this == &that) return *this;

		EHashMap<K, V>* t = (EHashMap<K, V>*)&that;

		//1.
		clear();

		delete[] _table;

		//2.
		_table = new Entry*[t->_capacity]();
		Entry **tab = t->_table;
		Entry* last = null;
		for (int i = 0; i < (int)t->_capacity; i++) {
			for (Entry *e = tab[i]; e != null; e = e->next) {
				Entry* e2 = new Entry(*e);
				e2->next = null;
				e2->map = this;

				if (last == null) {
					_table[i] = e2;
				}
				else {
					last->next = e2;
				}

				last = e2;
			}
			last = null;
		}

		_size = t->_size;
		_capacity = t->_capacity;
		_loadFactor = t->_loadFactor;
		_threshold = t->_threshold;
		_entrySet = null;

		return *this;
	}

	// internal utilities

	/**
	 * Applies a supplemental hash function to a given hashCode, which
	 * defends against poor quality hash functions.  This is critical
	 * because HashMap uses power-of-two length hash tables, that
	 * otherwise encounter collisions for hashCodes that do not differ
	 * in lower bits. Note: Null keys always map to hash 0, thus index 0.
	 */
	static int hashIt(int h) {
		// This function ensures that hashCodes that differ only by
		// constant multiples at each bit position have a bounded
		// number of collisions (approximately 8 at default load factor).
		unsigned int uh = (unsigned int) h;
		uh ^= (uh >> 20) ^ (uh >> 12);
		return uh ^ (uh >> 7) ^ (uh >> 4);
	}

	/**
	 * Returns index for hash code h.
	 */
	static int indexFor(int h, int length) {
		return h & (length - 1);
	}

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of key-value mappings in this map
	 */
	int size() {
		return _size;
	}

	/**
	 * Returns <tt>true</tt> if this map contains no key-value mappings.
	 *
	 * @return <tt>true</tt> if this map contains no key-value mappings
	 */
	boolean isEmpty() {
		return _size == 0;
	}

	/**
	 * Returns the value to which the specified key is mapped,
	 * or {@code null} if this map contains no mapping for the key.
	 *
	 * <p>More formally, if this map contains a mapping from a key
	 * {@code k} to a value {@code v} such that {@code (key==null ? k==null :
	 * key.equals(k))}, then this method returns {@code v}; otherwise
	 * it returns {@code null}.  (There can be at most one such mapping.)
	 *
	 * <p>A return value of {@code null} does not <i>necessarily</i>
	 * indicate that the map contains no mapping for the key; it's also
	 * possible that the map explicitly maps the key to {@code null}.
	 * The {@link #containsKey containsKey} operation may be used to
	 * distinguish these two cases.
	 *
	 * @see #put(Object, Object)
	 */
	V get(_K* key) {
		if (key == null)
			return getForNullKey();
		int hash = hashIt(key->hashCode());
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			K k;
			if (e->hash == hash && ((k = e->key) == key || key->equals(k.get())))
				return e->value;
		}
		return null;
	}

	/**
	 * Returns <tt>true</tt> if this map contains a mapping for the
	 * specified key.
	 *
	 * @param   key   The key whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map contains a mapping for the specified
	 * key.
	 */
	boolean containsKey(_K* key) {
		return getEntry(key) != null;
	}

	/**
	 * Returns the entry associated with the specified key in the
	 * HashMap.  Returns null if the HashMap contains no mapping
	 * for the key.
	 */
	Entry* getEntry(_K* key) {
		int hash = (key == null) ? 0 : hashIt(key->hashCode());
		for (Entry *e = _table[indexFor(hash,
				_capacity)]; e != null; e = e->next) {
			K k;
			if (e->hash == hash
					&& ((k = e->key) == key || (key != null && key->equals(k.get()))))
				return e;
		}
		return null;
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old
	 * value is replaced.
	 *
	 * @param key key with which the specified value is to be associated
	 * @param value value to be associated with the specified key
	 * @param absent test key is not exist
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V put(K key, V value, boolean *absent=null) {
		if (key == null)
			return putForNullKey(value);
		int hash = hashIt(key->hashCode());
		int i = indexFor(hash, _capacity);
		for (Entry *e = _table[i]; e != null;
				e = e->next) {
			K k;
			if (e->hash == hash && ((k = e->key) == key || key->equals(k.get()))) {
				if (absent) {
					*absent = false;
				}

				V oldValue = e->value;
				e->value = value;
				e->recordAccess(this);
				return oldValue;
			}
		}

		if (absent) {
			*absent = true;
		}
		addEntry(hash, key, value, i);
		return null;
	}

	/**
	 * Rehashes the contents of this map into a new array with a
	 * larger capacity.  This method is called automatically when the
	 * number of keys in this map reaches its _threshold.
	 *
	 * If current capacity is MAXIMUM_CAPACITY, this method does not
	 * resize the map, but sets _threshold to Integer.MAX_VALUE.
	 * This has the effect of preventing future calls.
	 *
	 * @param newCapacity the new capacity, MUST be a power of two;
	 *        must be greater than current capacity unless current
	 *        capacity is MAXIMUM_CAPACITY (in which case value
	 *        is irrelevant).
	 */
	void resize(int newCapacity) {
		int oldCapacity = _capacity;
		if (oldCapacity == HM_MAXIMUM_CAPACITY) {
			_threshold = EInteger::MAX_VALUE;
			return;
		}

		Entry **newTable = new Entry*[newCapacity]();
		transfer(newTable, newCapacity);
		delete[] _table; //!
		_table = newTable;
		_capacity = newCapacity;
		_threshold = (int) (newCapacity * _loadFactor);
	}

	/**
	 * Transfers all entries from current table to newTable.
	 */
	void transfer(Entry *newTable[],
			int newCapacity) {
		Entry **src = _table;
		for (int j = 0; j < (int)_capacity; j++) {
			Entry *e = src[j];
			if (e != null) {
				do {
					Entry *next = e->next;
					int i = indexFor(e->hash, newCapacity);
					e->next = newTable[i];
					newTable[i] = e;
					e = next;
				} while (e != null);
			}
		}
	}

	/**
	 * Removes the mapping for the specified key from this map if present.
	 *
	 * @param  key key whose mapping is to be removed from the map
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 */
	V remove(_K* key) {
		Entry *e = removeEntryForKey(key);
		if (e) {
			V v = e->value;
			e->value = null;
			delete e;
			return v;
		}
		return null;
	}

	/**
	 * Removes and returns the entry associated with the specified key
	 * in the HashMap.  Returns null if the HashMap contains no mapping
	 * for this key.
	 */
	Entry* removeEntryForKey(_K* key) {
		int hash = (key == null) ? 0 : hashIt(key->hashCode());
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			K k;
			if (e->hash == hash
					&& ((k = e->key) == key || (key != null && key->equals(k.get())))) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				e->next = null;
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Special version of remove for EntrySet.
	 */
	Entry* removeMapping(EMapEntry<K,V> *entry) {
		K key = entry->getKey();
		int hash = (key == null) ? 0 : hashIt(key->hashCode());
		int i = indexFor(hash, _capacity);
		Entry *prev = _table[i];
		Entry *e = prev;

		while (e != null) {
			Entry *next = e->next;
			if (e->hash == hash && e->equals(entry)) {
				_size--;
				if (prev == e)
					_table[i] = next;
				else
					prev->next = next;
				e->recordRemoval(this);
				return e;
			}
			prev = e;
			e = next;
		}

		return e;
	}

	/**
	 * Removes all of the mappings from this map.
	 * The map will be empty after this call returns.
	 */
	void clear() {
		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			if (tab[i] != null) {
				delete tab[i];
				tab[i] = null;
			}
		_size = 0;
	}

	/**
	 * Returns <tt>true</tt> if this map maps one or more keys to the
	 * specified value.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map maps one or more keys to the
	 *         specified value
	 */
	boolean containsValue(_V* value) {
		if (value == null)
			return containsNullValue();

		Entry **tab = _table;
		for (int i = 0; i < (int)_capacity; i++)
			for (Entry *e = tab[i]; e != null;
					e = e->next)
				if (value->equals(e->value.get()))
					return true;
		return false;
	}

	/**
	 * Adds a new entry with the specified key, value and hash code to
	 * the specified bucket.  It is the responsibility of this
	 * method to resize the table if appropriate.
	 *
	 * Subclass overrides this to alter the behavior of put method.
	 */
	void addEntry(int hash, K key, V value,
			int bucketIndex) {
		Entry *e = _table[bucketIndex];
		_table[bucketIndex] = new Entry(hash, key,
				value, e, this);
		if (_size++ >= _threshold)
			resize(2 * _capacity);
	}

	// Views

	/**
	 * Returns a {@link Set} view of the keys contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation), the results of
	 * the iteration are undefined.  The set supports element removal,
	 * which removes the corresponding mapping from the map, via the
	 * <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt>, and <tt>clear</tt>
	 * operations.  It does not support the <tt>add</tt> or <tt>addAll</tt>
	 * operations.
	 */
	sp<ESet<K> > keySet() {
		if (!EAbstractMap<K,V>::_keySet) {
			EAbstractMap<K,V>::_keySet = new Keys(this);
		}
		return EAbstractMap<K,V>::_keySet;
	}

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection is backed by the map, so changes to the map are
	 * reflected in the collection, and vice-versa.  If the map is
	 * modified while an iteration over the collection is in progress
	 * (except through the iterator's own <tt>remove</tt> operation),
	 * the results of the iteration are undefined.  The collection
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Collection.remove</tt>, <tt>removeAll</tt>,
	 * <tt>retainAll</tt> and <tt>clear</tt> operations.  It does not
	 * support the <tt>add</tt> or <tt>addAll</tt> operations.
	 */
	sp<ECollection<V> > values() {
		if (!EAbstractMap<K,V>::_values) {
			EAbstractMap<K,V>::_values = new Values(this);
		}
		return EAbstractMap<K,V>::_values;
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  If the map is modified
	 * while an iteration over the set is in progress (except through
	 * the iterator's own <tt>remove</tt> operation, or through the
	 * <tt>setValue</tt> operation on a map entry returned by the
	 * iterator) the results of the iteration are undefined.  The set
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Set.remove</tt>, <tt>removeAll</tt>, <tt>retainAll</tt> and
	 * <tt>clear</tt> operations.  It does not support the
	 * <tt>add</tt> or <tt>addAll</tt> operations.
	 *
	 * @return a set view of the mappings contained in this map
	 */
	sp<ESet<EMapEntry<K, V>*> > entrySet() {
		if (_entrySet == null) {
			_entrySet = new EntrySet(this);
		}
		return _entrySet;
	}

	// These methods are used when serializing HashSets
	int capacity() {
		return _capacity;
	}
	float loadFactor() {
		return _loadFactor;
	}
};

} /* namespace efc */
#endif //!__EHashMap_H__
