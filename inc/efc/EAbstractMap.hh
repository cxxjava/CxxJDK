#ifndef __EAbstractMap_H__
#define __EAbstractMap_H__

#include "EMap.hh"
#include "EAbstractSet.hh"
#include "EAbstractCollection.hh"
#include "EIterator.hh"
#include "ENoSuchElementException.hh"

namespace efc {

/**
 * This class provides a skeletal implementation of the <tt>Map</tt>
 * interface, to minimize the effort required to implement this interface.
 *
 * <p>To implement an unmodifiable map, the programmer needs only to extend this
 * class and provide an implementation for the <tt>entrySet</tt> method, which
 * returns a set-view of the map's mappings.  Typically, the returned set
 * will, in turn, be implemented atop <tt>AbstractSet</tt>.  This set should
 * not support the <tt>add</tt> or <tt>remove</tt> methods, and its iterator
 * should not support the <tt>remove</tt> method.
 *
 * <p>To implement a modifiable map, the programmer must additionally override
 * this class's <tt>put</tt> method (which otherwise throws an
 * <tt>UnsupportedOperationException</tt>), and the iterator returned by
 * <tt>entrySet().iterator()</tt> must additionally implement its
 * <tt>remove</tt> method.
 *
 * <p>The programmer should generally provide a void (no argument) and map
 * constructor, as per the recommendation in the <tt>Map</tt> interface
 * specification.
 *
 * <p>The documentation for each non-abstract method in this class describes its
 * implementation in detail.  Each of these methods may be overridden if the
 * map being implemented admits a more efficient implementation.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @param <K> the type of keys maintained by this map
 * @param <V> the type of mapped values
 *
 * @author  Josh Bloch
 * @author  Neal Gafter
 * @version 1.50, 06/16/06
 * @see Map
 * @see Collection
 * @since 1.2
 */

//=============================================================================

// Implementation Note: SimpleEntry and SimpleImmutableEntry
// are distinct unrelated classes, even though they share
// some code. Since you can't add or subtract final-ness
// of a field in a subclass, they can't share representations,
// and the amount of duplicated code is too small to warrant
// exposing a common abstract class.

/**
 * An Entry maintaining a key and a value.  The value may be
 * changed using the <tt>setValue</tt> method.  This class
 * facilitates the process of building custom map
 * implementations. For example, it may be convenient to return
 * arrays of <tt>SimpleEntry</tt> instances in method
 * <tt>Map.entrySet().toArray</tt>.
 *
 * @since 1.6
 */

template<typename EK, typename EV>
class ESimpleEntry: virtual public EMapEntry<EK,
		EV> {
private:
	EK key;
	EV value;

	static boolean eq(EObject* o1, EObject* o2) {
		return o1 == null ? o2 == null : o1->equals(o2);
	}
public:
	virtual ~ESimpleEntry(){}

	/**
	 * Creates an entry representing a mapping from the specified
	 * key to the specified value.
	 *
	 * @param key the key represented by this entry
	 * @param value the value represented by this entry
	 */
	ESimpleEntry(EK key, EV value) {
		this->key = key;
		this->value = value;
	}

	/**
	 * Creates an entry representing the same mapping as the
	 * specified entry.
	 *
	 * @param entry the entry to copy
	 */
	ESimpleEntry(
			EMapEntry<EK, EV> *entry) {
		this->key = entry->getKey();
		this->value = entry->getValue();
	}

	/**
	 * Returns the key corresponding to this entry.
	 *
	 * @return the key corresponding to this entry
	 */
	virtual EK getKey() {
		return key;
	}

	/**
	 * Returns the value corresponding to this entry.
	 *
	 * @return the value corresponding to this entry
	 */
	virtual EV getValue() {
		return value;
	}

	/**
	 * Replaces the value corresponding to this entry with the specified
	 * value.
	 *
	 * @param value new value to be stored in this entry
	 * @return the old value corresponding to the entry
	 */
	virtual EV setValue(EV value) {
		EV oldValue = this->value;
		this->value = value;
		return oldValue;
	}

	/**
	 * Compares the specified object with this entry for equality.
	 * Returns {@code true} if the given object is also a map entry and
	 * the two entries represent the same mapping.  More formally, two
	 * entries {@code e1} and {@code e2} represent the same mapping
	 * if<pre>
	 *   (e1.getKey()==null ?
	 *    e2.getKey()==null :
	 *    e1.getKey().equals(e2.getKey()))
	 *   &amp;&amp;
	 *   (e1.getValue()==null ?
	 *    e2.getValue()==null :
	 *    e1.getValue().equals(e2.getValue()))</pre>
	 * This ensures that the {@code equals} method works properly across
	 * different implementations of the {@code Map.Entry} interface.
	 *
	 * @param o object to be compared for equality with this map entry
	 * @return {@code true} if the specified object is equal to this map
	 *         entry
	 * @see    #hashCode
	 */
	virtual boolean equals(EMapEntry<EK, EV>* e) {
		return eq(key, e->getKey()) && eq(value, e->getValue());
	}

	/**
	 * Returns the hash code value for this map entry.  The hash code
	 * of a map entry {@code e} is defined to be: <pre>
	 *   (e.getKey()==null   ? 0 : e.getKey().hashCode()) ^
	 *   (e.getValue()==null ? 0 : e.getValue().hashCode())</pre>
	 * This ensures that {@code e1.equals(e2)} implies that
	 * {@code e1.hashCode()==e2.hashCode()} for any two Entries
	 * {@code e1} and {@code e2}, as required by the general
	 * contract of {@link Object#hashCode}.
	 *
	 * @return the hash code value for this map entry
	 * @see    #equals
	 */
	virtual int hashCode() {
		return (key   == null ? 0 : key->hashCode()) ^
			   (value == null ? 0 : value->hashCode());
	}
};

/**
 * An Entry maintaining an immutable key and value.  This class
 * does not support method <tt>setValue</tt>.  This class may be
 * convenient in methods that return thread-safe snapshots of
 * key-value mappings.
 *
 * @since 1.6
 */
template<typename K, typename V>
class ESimpleImmutableEntry: virtual public EMapEntry<K,
		V> {
private:
	K key;
	V value;

	static boolean eq(EObject* o1, EObject* o2) {
		return o1 == null ? o2 == null : o1->equals(o2);
	}
public:
	virtual ~ESimpleImmutableEntry(){}

	/**
	 * Creates an entry representing a mapping from the specified
	 * key to the specified value.
	 *
	 * @param key the key represented by this entry
	 * @param value the value represented by this entry
	 */
	ESimpleImmutableEntry(K key,
			V value) {
		this->key = key;
		this->value = value;
	}

	/**
	 * Creates an entry representing the same mapping as the
	 * specified entry.
	 *
	 * @param entry the entry to copy
	 */
	ESimpleImmutableEntry(
			EMapEntry<K, V> *entry) {
		this->key = entry->getKey();
		this->value = entry->getValue();
	}

	/**
	 * Returns the key corresponding to this entry.
	 *
	 * @return the key corresponding to this entry
	 */
	virtual K getKey() {
		return key;
	}

	/**
	 * Returns the value corresponding to this entry.
	 *
	 * @return the value corresponding to this entry
	 */
	virtual V getValue() {
		return value;
	}

	/**
	 * Replaces the value corresponding to this entry with the specified
	 * value (optional operation).  This implementation simply throws
	 * <tt>UnsupportedOperationException</tt>, as this class implements
	 * an <i>immutable</i> map entry.
	 *
	 * @param value new value to be stored in this entry
	 * @return (Does not return)
	 * @throws UnsupportedOperationException always
	 */
	virtual V setValue(V value) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	/**
	 * Compares the specified object with this entry for equality.
	 * Returns {@code true} if the given object is also a map entry and
	 * the two entries represent the same mapping.  More formally, two
	 * entries {@code e1} and {@code e2} represent the same mapping
	 * if<pre>
	 *   (e1.getKey()==null ?
	 *    e2.getKey()==null :
	 *    e1.getKey().equals(e2.getKey()))
	 *   &amp;&amp;
	 *   (e1.getValue()==null ?
	 *    e2.getValue()==null :
	 *    e1.getValue().equals(e2.getValue()))</pre>
	 * This ensures that the {@code equals} method works properly across
	 * different implementations of the {@code Map.Entry} interface.
	 *
	 * @param o object to be compared for equality with this map entry
	 * @return {@code true} if the specified object is equal to this map
	 *         entry
	 * @see    #hashCode
	 */
	virtual boolean equals(
			EMapEntry<K, V>* e) {
		return eq(key, e->getKey()) && eq(value, e->getValue());
	}

	/**
	 * Returns the hash code value for this map entry.  The hash code
	 * of a map entry {@code e} is defined to be: <pre>
	 *   (e.getKey()==null   ? 0 : e.getKey().hashCode()) ^
	 *   (e.getValue()==null ? 0 : e.getValue().hashCode())</pre>
	 * This ensures that {@code e1.equals(e2)} implies that
	 * {@code e1.hashCode()==e2.hashCode()} for any two Entries
	 * {@code e1} and {@code e2}, as required by the general
	 * contract of {@link Object#hashCode}.
	 *
	 * @return the hash code value for this map entry
	 * @see    #equals
	 */
	virtual int hashCode() {
		return (key == null ? 0 : key->hashCode())
				^ (value == null ? 0 : value->hashCode());
	}
};

template<typename K, typename V>
abstract class EAbstractMap: virtual public EMap<K, V> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractMap() {
		_keySet = null;
		_values = null;
	}

	template<typename _EKeySet>
	class KeySet: public EAbstractSet<_EKeySet> {
	private:
		template<typename _EKeySetI>
		class KSListIterator: public EIterator<_EKeySetI> {
		private:
			EIterator<EMapEntry<K,V>*> *i;

		public:
			KSListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			_EKeySetI next() {
				return i->next()->getKey();
			}

			void remove() {
				i->remove();
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		KeySet(EAbstractMap<K,V> *map) {
			_map = map;
		}

		EIterator<_EKeySet>* iterator(int index=0) {
			return new KSListIterator<_EKeySet>(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(_EKeySet k) {
			return _map->containsKey(k);
		}
	};

	template<typename _EValueCollection>
	class ValueCollection: public EAbstractCollection<_EValueCollection> {
	private:
		template<typename _EValueCollectionI>
		class VCListIterator: public EIterator<_EValueCollectionI> {
		private:
			EIterator<EMapEntry<K,V>*> *i;

		public:
			VCListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			_EValueCollectionI next() {
				return i->next()->getValue();
			}

			void remove() {
				i->remove();
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		ValueCollection(EAbstractMap<K,V> *map) {
			_map = map;
		}

		EIterator<_EValueCollection>* iterator(int index=0) {
			return new VCListIterator<_EValueCollection>(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(K v) {
			return _map->containsKey(v);
		}
	};

public:
	virtual ~EAbstractMap() {
		if (_keySet) {
			delete _keySet;
		}
		if (_values) {
			delete _values;
		}
	}

	// Query Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns <tt>entrySet().size()</tt>.
	 */
	virtual int size() {
		return entrySet()->size();
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns <tt>size() == 0</tt>.
	 */
	virtual boolean isEmpty() {
		return size() == 0;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified value.  If such an entry is found,
	 * <tt>true</tt> is returned.  If the iteration terminates without
	 * finding such an entry, <tt>false</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean containsValue(V value) {
		EIterator<EMapEntry<K, V>*> *i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			if (value->equals(e->getValue())) {
				delete i;
				return true;
			}
		}
		delete i;
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified key.  If such an entry is found,
	 * <tt>true</tt> is returned.  If the iteration terminates without
	 * finding such an entry, <tt>false</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map; many
	 * implementations will override this method.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean containsKey(K key) {
		EIterator<EMapEntry<K, V>*> *i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			if (key->equals(e->getKey())) {
				delete i;
				return true;
			}
		}
		delete i;
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified key.  If such an entry is found,
	 * the entry's value is returned.  If the iteration terminates without
	 * finding such an entry, <tt>null</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map; many
	 * implementations will override this method.
	 *
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 */
	virtual V get(K key) THROWS(ENoSuchElementException) {
		EIterator<EMapEntry<K, V>*> *i =
						entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			if (key->equals(e->getKey())) {
				delete i;
				return e->getValue();
			}
		}
		delete i;
		throw ENOSUCHELEMENTEXCEPTION;
	}

	// Modification Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * <tt>UnsupportedOperationException</tt>.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	virtual V put(K key, V value, boolean *absent=null) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching for an
	 * entry with the specified key.  If such an entry is found, its value is
	 * obtained with its <tt>getValue</tt> operation, the entry is removed
	 * from the collection (and the backing map) with the iterator's
	 * <tt>remove</tt> operation, and the saved value is returned.  If the
	 * iteration terminates without finding such an entry, <tt>null</tt> is
	 * returned.  Note that this implementation requires linear time in the
	 * size of the map; many implementations will override this method.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if the <tt>entrySet</tt>
	 * iterator does not support the <tt>remove</tt> method and this map
	 * contains a mapping for the specified key.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 */
	virtual V remove(K key) THROWS(ENoSuchElementException) {
		EIterator<EMapEntry<K, V>*> *i =
								entrySet()->iterator();
		EMapEntry<K, V> *correctEntry = null;
		while (correctEntry == null && i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			if (key->equals(e->getKey()))
				correctEntry = e;
		}

		V oldValue;
		if (correctEntry != null) {
			oldValue = correctEntry->getValue();
			i->remove();
			delete i;
			return oldValue;
		}
		delete i;
		throw ENOSUCHELEMENTEXCEPTION;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation calls <tt>entrySet().clear()</tt>.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if the <tt>entrySet</tt>
	 * does not support the <tt>clear</tt> operation.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 */
	virtual void clear() {
		entrySet()->clear();
	}

	// Views

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a set that subclasses {@link AbstractSet}.
	 * The subclass's iterator method returns a "wrapper object" over this
	 * map's <tt>entrySet()</tt> iterator.  The <tt>size</tt> method
	 * delegates to this map's <tt>size</tt> method and the
	 * <tt>contains</tt> method delegates to this map's
	 * <tt>containsKey</tt> method.
	 *
	 * <p>The set is created the first time this method is called,
	 * and returned in response to all subsequent calls.  No synchronization
	 * is performed, so there is a slight chance that multiple calls to this
	 * method will not all return the same set.
	 */
	virtual ESet<K>* keySet() {
		if (_keySet == null) {
			_keySet = new KeySet<K>(this);
		}
		return _keySet;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a collection that subclasses {@link
	 * AbstractCollection}.  The subclass's iterator method returns a
	 * "wrapper object" over this map's <tt>entrySet()</tt> iterator.
	 * The <tt>size</tt> method delegates to this map's <tt>size</tt>
	 * method and the <tt>contains</tt> method delegates to this map's
	 * <tt>containsValue</tt> method.
	 *
	 * <p>The collection is created the first time this method is called, and
	 * returned in response to all subsequent calls.  No synchronization is
	 * performed, so there is a slight chance that multiple calls to this
	 * method will not all return the same collection.
	 */
	virtual ECollection<V>* values() {
		if (_values == null) {
			_values = new ValueCollection<V>(this);
		}
		return _values;
	}

	virtual ESet<EMapEntry<K, V>*>* entrySet() = 0;

protected:
	/**
	 * Each of these fields are initialized to contain an instance of the
	 * appropriate view the first time this view is requested.  The views are
	 * stateless, so there's no reason to create more than one of each.
	 */
	ESet<K> *_keySet;
	ECollection<V> *_values;
};

//=============================================================================

#if !(defined(_MSC_VER) && (_MSC_VER<=1200))


// Implementation Note: SimpleEntry and SimpleImmutableEntry
// are distinct unrelated classes, even though they share
// some code. Since you can't add or subtract final-ness
// of a field in a subclass, they can't share representations,
// and the amount of duplicated code is too small to warrant
// exposing a common abstract class.

/**
 * An Entry maintaining a key and a value.  The value may be
 * changed using the <tt>setValue</tt> method.  This class
 * facilitates the process of building custom map
 * implementations. For example, it may be convenient to return
 * arrays of <tt>SimpleEntry</tt> instances in method
 * <tt>Map.entrySet().toArray</tt>.
 *
 * @since 1.6
 */
template<typename EV>
class ESimpleEntry<int, EV> : virtual public EMapEntry<int,
		EV> {
private:
	int key;
	EV value;

	static boolean eq(EObject* o1, EObject* o2) {
		return o1 == null ? o2 == null : o1->equals(o2);
	}
public:
	virtual ~ESimpleEntry(){}

	/**
	 * Creates an entry representing a mapping from the specified
	 * key to the specified value.
	 *
	 * @param key the key represented by this entry
	 * @param value the value represented by this entry
	 */
	ESimpleEntry(int key, EV value) {
		this->key = key;
		this->value = value;
	}

	/**
	 * Creates an entry representing the same mapping as the
	 * specified entry.
	 *
	 * @param entry the entry to copy
	 */
	ESimpleEntry(
			EMapEntry<int, EV> *entry) {
		this->key = entry->getKey();
		this->value = entry->getValue();
	}

	/**
	 * Returns the key corresponding to this entry.
	 *
	 * @return the key corresponding to this entry
	 */
	virtual int getKey() {
		return key;
	}

	/**
	 * Returns the value corresponding to this entry.
	 *
	 * @return the value corresponding to this entry
	 */
	virtual EV getValue() {
		return value;
	}

	/**
	 * Replaces the value corresponding to this entry with the specified
	 * value.
	 *
	 * @param value new value to be stored in this entry
	 * @return the old value corresponding to the entry
	 */
	virtual EV setValue(EV value) {
		EV oldValue = this->value;
		this->value = value;
		return oldValue;
	}

	/**
	 * Compares the specified object with this entry for equality.
	 * Returns {@code true} if the given object is also a map entry and
	 * the two entries represent the same mapping.  More formally, two
	 * entries {@code e1} and {@code e2} represent the same mapping
	 * if<pre>
	 *   (e1.getKey()==null ?
	 *    e2.getKey()==null :
	 *    e1.getKey().equals(e2.getKey()))
	 *   &amp;&amp;
	 *   (e1.getValue()==null ?
	 *    e2.getValue()==null :
	 *    e1.getValue().equals(e2.getValue()))</pre>
	 * This ensures that the {@code equals} method works properly across
	 * different implementations of the {@code Map.Entry} interface.
	 *
	 * @param o object to be compared for equality with this map entry
	 * @return {@code true} if the specified object is equal to this map
	 *         entry
	 * @see    #hashCode
	 */
	virtual boolean equals(EMapEntry<int, EV>* e) {
		return (key == e->getKey()) && eq(value, e->getValue());
	}

	/**
	 * Returns the hash code value for this map entry.  The hash code
	 * of a map entry {@code e} is defined to be: <pre>
	 *   (e.getKey()==null   ? 0 : e.getKey().hashCode()) ^
	 *   (e.getValue()==null ? 0 : e.getValue().hashCode())</pre>
	 * This ensures that {@code e1.equals(e2)} implies that
	 * {@code e1.hashCode()==e2.hashCode()} for any two Entries
	 * {@code e1} and {@code e2}, as required by the general
	 * contract of {@link Object#hashCode}.
	 *
	 * @return the hash code value for this map entry
	 * @see    #equals
	 */
	virtual int hashCode() {
		return (key) ^
			   (value == null ? 0 : value->hashCode());
	}
};

/**
 * An Entry maintaining an immutable key and value.  This class
 * does not support method <tt>setValue</tt>.  This class may be
 * convenient in methods that return thread-safe snapshots of
 * key-value mappings.
 *
 * @since 1.6
 */
template<typename V>
class ESimpleImmutableEntry<int, V>: virtual public EMapEntry<int,
		V> {
private:
	int key;
	V value;

	static boolean eq(EObject* o1, EObject* o2) {
		return o1 == null ? o2 == null : o1->equals(o2);
	}
public:
	virtual ~ESimpleImmutableEntry(){}

	/**
	 * Creates an entry representing a mapping from the specified
	 * key to the specified value.
	 *
	 * @param key the key represented by this entry
	 * @param value the value represented by this entry
	 */
	ESimpleImmutableEntry(int key,
			V value) {
		this->key = key;
		this->value = value;
	}

	/**
	 * Creates an entry representing the same mapping as the
	 * specified entry.
	 *
	 * @param entry the entry to copy
	 */
	ESimpleImmutableEntry(
			EMapEntry<int, V> *entry) {
		this->key = entry->getKey();
		this->value = entry->getValue();
	}

	/**
	 * Returns the key corresponding to this entry.
	 *
	 * @return the key corresponding to this entry
	 */
	virtual int getKey() {
		return key;
	}

	/**
	 * Returns the value corresponding to this entry.
	 *
	 * @return the value corresponding to this entry
	 */
	virtual V getValue() {
		return value;
	}

	/**
	 * Replaces the value corresponding to this entry with the specified
	 * value (optional operation).  This implementation simply throws
	 * <tt>UnsupportedOperationException</tt>, as this class implements
	 * an <i>immutable</i> map entry.
	 *
	 * @param value new value to be stored in this entry
	 * @return (Does not return)
	 * @throws UnsupportedOperationException always
	 */
	virtual V setValue(V value) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	/**
	 * Compares the specified object with this entry for equality.
	 * Returns {@code true} if the given object is also a map entry and
	 * the two entries represent the same mapping.  More formally, two
	 * entries {@code e1} and {@code e2} represent the same mapping
	 * if<pre>
	 *   (e1.getKey()==null ?
	 *    e2.getKey()==null :
	 *    e1.getKey().equals(e2.getKey()))
	 *   &amp;&amp;
	 *   (e1.getValue()==null ?
	 *    e2.getValue()==null :
	 *    e1.getValue().equals(e2.getValue()))</pre>
	 * This ensures that the {@code equals} method works properly across
	 * different implementations of the {@code Map.Entry} interface.
	 *
	 * @param o object to be compared for equality with this map entry
	 * @return {@code true} if the specified object is equal to this map
	 *         entry
	 * @see    #hashCode
	 */
	virtual boolean equals(
			EMapEntry<int, V>* e) {
		return (key == e->getKey()) && eq(value, e->getValue());
	}

	/**
	 * Returns the hash code value for this map entry.  The hash code
	 * of a map entry {@code e} is defined to be: <pre>
	 *   (e.getKey()==null   ? 0 : e.getKey().hashCode()) ^
	 *   (e.getValue()==null ? 0 : e.getValue().hashCode())</pre>
	 * This ensures that {@code e1.equals(e2)} implies that
	 * {@code e1.hashCode()==e2.hashCode()} for any two Entries
	 * {@code e1} and {@code e2}, as required by the general
	 * contract of {@link Object#hashCode}.
	 *
	 * @return the hash code value for this map entry
	 * @see    #equals
	 */
	virtual int hashCode() {
		return (key)
				^ (value == null ? 0 : value->hashCode());
	}
};

template<typename V>
abstract class EAbstractMap<int, V>: virtual public EMap<int, V> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractMap() {
		_keySet = null;
		_values = null;
	}

	template<typename _EKeySet>
	class KeySet: public EAbstractSet<_EKeySet> {
	private:
		template<typename _EKeySetI>
		class KSListIterator: public EIterator<_EKeySetI> {
		private:
			EIterator<EMapEntry<int,V>*> *i;

		public:
			KSListIterator(EAbstractMap<int,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			_EKeySetI next() {
				return i->next()->getKey();
			}

			void remove() {
				i->remove();
			}
		};

		EAbstractMap<int,V> *_map;

	public:
		KeySet(EAbstractMap<int,V> *map) {
			_map = map;
		}

		EIterator<_EKeySet>* iterator(int index=0) {
			return new KSListIterator<_EKeySet>(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(_EKeySet k) {
			return _map->containsKey(k);
		}
	};

	template<typename _EValueCollection>
		class ValueCollection: public EAbstractCollection<_EValueCollection> {
		private:
			template<typename _EValueCollectionI>
			class VCListIterator: public EIterator<_EValueCollectionI> {
			private:
				EIterator<EMapEntry<int,V>*> *i;

			public:
				VCListIterator(EAbstractMap<int,V> *map) {
					 i = map->entrySet()->iterator();
				}

				boolean hasNext() {
					return i->hasNext();
				}

				_EValueCollectionI next() {
					return i->next()->getValue();
				}

				void remove() {
					i->remove();
				}
			};

			EAbstractMap<int,V> *_map;

		public:
			ValueCollection(EAbstractMap<int,V> *map) {
				_map = map;
			}

			EIterator<_EValueCollection>* iterator(int index=0) {
				return new VCListIterator<_EValueCollection>(_map);
			}

			int size() {
				return _map->size();
			}

			boolean contains(int v) {
				return _map->containsKey(v);
			}
		};

public:
	virtual ~EAbstractMap() {
		if (_keySet) {
			delete _keySet;
		}
		if (_values) {
			delete _values;
		}
	}

	// Query Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns <tt>entrySet().size()</tt>.
	 */
	virtual int size() {
		return entrySet()->size();
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns <tt>size() == 0</tt>.
	 */
	virtual boolean isEmpty() {
		return size() == 0;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified value.  If such an entry is found,
	 * <tt>true</tt> is returned.  If the iteration terminates without
	 * finding such an entry, <tt>false</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean containsValue(V value) {
		EIterator<EMapEntry<int, V>*> *i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<int, V> *e = i->next();
			if (value->equals(e->getValue())) {
				delete i;
				return true;
			}
		}
		delete i;
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified key.  If such an entry is found,
	 * <tt>true</tt> is returned.  If the iteration terminates without
	 * finding such an entry, <tt>false</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map; many
	 * implementations will override this method.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean containsKey(int key) {
		EIterator<EMapEntry<int, V>*> *i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<int, V> *e = i->next();
			if (key == (e->getKey())) {
				delete i;
				return true;
			}
		}
		delete i;
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified key.  If such an entry is found,
	 * the entry's value is returned.  If the iteration terminates without
	 * finding such an entry, <tt>null</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map; many
	 * implementations will override this method.
	 *
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 */
	virtual V get(int key) THROWS(ENoSuchElementException) {
		EIterator<EMapEntry<int, V>*> *i =
						entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<int, V> *e = i->next();
			if (key == (e->getKey())) {
				delete i;
				return e->getValue();
			}
		}
		delete i;
		throw ENOSUCHELEMENTEXCEPTION;
	}

	// Modification Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * <tt>UnsupportedOperationException</tt>.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	virtual V put(int key, V value) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching for an
	 * entry with the specified key.  If such an entry is found, its value is
	 * obtained with its <tt>getValue</tt> operation, the entry is removed
	 * from the collection (and the backing map) with the iterator's
	 * <tt>remove</tt> operation, and the saved value is returned.  If the
	 * iteration terminates without finding such an entry, <tt>null</tt> is
	 * returned.  Note that this implementation requires linear time in the
	 * size of the map; many implementations will override this method.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if the <tt>entrySet</tt>
	 * iterator does not support the <tt>remove</tt> method and this map
	 * contains a mapping for the specified key.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 */
	virtual V remove(int key) THROWS(ENoSuchElementException) {
		EIterator<EMapEntry<int, V>*> *i =
								entrySet()->iterator();
		EMapEntry<int, V> *correctEntry = null;
		while (correctEntry == null && i->hasNext()) {
			EMapEntry<int, V> *e = i->next();
			if (key == (e->getKey()))
				correctEntry = e;
		}

		V oldValue;
		if (correctEntry != null) {
			oldValue = correctEntry->getValue();
			i->remove();
			delete i;
			return oldValue;
		}
		delete i;
		throw ENOSUCHELEMENTEXCEPTION;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation calls <tt>entrySet().clear()</tt>.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if the <tt>entrySet</tt>
	 * does not support the <tt>clear</tt> operation.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 */
	virtual void clear() {
		entrySet()->clear();
	}

	// Views

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a set that subclasses {@link AbstractSet}.
	 * The subclass's iterator method returns a "wrapper object" over this
	 * map's <tt>entrySet()</tt> iterator.  The <tt>size</tt> method
	 * delegates to this map's <tt>size</tt> method and the
	 * <tt>contains</tt> method delegates to this map's
	 * <tt>containsKey</tt> method.
	 *
	 * <p>The set is created the first time this method is called,
	 * and returned in response to all subsequent calls.  No synchronization
	 * is performed, so there is a slight chance that multiple calls to this
	 * method will not all return the same set.
	 */
	virtual ESet<int>* keySet() {
		if (_keySet == null) {
			_keySet = new KeySet<int>(this);
		}
		return _keySet;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a collection that subclasses {@link
	 * AbstractCollection}.  The subclass's iterator method returns a
	 * "wrapper object" over this map's <tt>entrySet()</tt> iterator.
	 * The <tt>size</tt> method delegates to this map's <tt>size</tt>
	 * method and the <tt>contains</tt> method delegates to this map's
	 * <tt>containsValue</tt> method.
	 *
	 * <p>The collection is created the first time this method is called, and
	 * returned in response to all subsequent calls.  No synchronization is
	 * performed, so there is a slight chance that multiple calls to this
	 * method will not all return the same collection.
	 */
	virtual ECollection<V>* values() {
		if (_values == null) {
			_values = new ValueCollection<V>(this);
		}
		return _values;
	}

	virtual ESet<EMapEntry<int, V>*>* entrySet() = 0;

protected:
	/**
	 * Each of these fields are initialized to contain an instance of the
	 * appropriate view the first time this view is requested.  The views are
	 * stateless, so there's no reason to create more than one of each.
	 */
	ESet<int> *_keySet;
	ECollection<V> *_values;
};

//=============================================================================


// Implementation Note: SimpleEntry and SimpleImmutableEntry
// are distinct unrelated classes, even though they share
// some code. Since you can't add or subtract final-ness
// of a field in a subclass, they can't share representations,
// and the amount of duplicated code is too small to warrant
// exposing a common abstract class.

/**
 * An Entry maintaining a key and a value.  The value may be
 * changed using the <tt>setValue</tt> method.  This class
 * facilitates the process of building custom map
 * implementations. For example, it may be convenient to return
 * arrays of <tt>SimpleEntry</tt> instances in method
 * <tt>Map.entrySet().toArray</tt>.
 *
 * @since 1.6
 */
template<typename EV>
class ESimpleEntry<llong, EV>: virtual public EMapEntry<llong,
		EV> {
private:
	llong key;
	EV value;

	static boolean eq(EObject* o1, EObject* o2) {
		return o1 == null ? o2 == null : o1->equals(o2);
	}
public:
	virtual ~ESimpleEntry(){}

	/**
	 * Creates an entry representing a mapping from the specified
	 * key to the specified value.
	 *
	 * @param key the key represented by this entry
	 * @param value the value represented by this entry
	 */
	ESimpleEntry(llong key, EV value) {
		this->key = key;
		this->value = value;
	}

	/**
	 * Creates an entry representing the same mapping as the
	 * specified entry.
	 *
	 * @param entry the entry to copy
	 */
	ESimpleEntry(
			EMapEntry<llong, EV> *entry) {
		this->key = entry->getKey();
		this->value = entry->getValue();
	}

	/**
	 * Returns the key corresponding to this entry.
	 *
	 * @return the key corresponding to this entry
	 */
	virtual llong getKey() {
		return key;
	}

	/**
	 * Returns the value corresponding to this entry.
	 *
	 * @return the value corresponding to this entry
	 */
	virtual EV getValue() {
		return value;
	}

	/**
	 * Replaces the value corresponding to this entry with the specified
	 * value.
	 *
	 * @param value new value to be stored in this entry
	 * @return the old value corresponding to the entry
	 */
	virtual EV setValue(EV value) {
		EV oldValue = this->value;
		this->value = value;
		return oldValue;
	}

	/**
	 * Compares the specified object with this entry for equality.
	 * Returns {@code true} if the given object is also a map entry and
	 * the two entries represent the same mapping.  More formally, two
	 * entries {@code e1} and {@code e2} represent the same mapping
	 * if<pre>
	 *   (e1.getKey()==null ?
	 *    e2.getKey()==null :
	 *    e1.getKey().equals(e2.getKey()))
	 *   &amp;&amp;
	 *   (e1.getValue()==null ?
	 *    e2.getValue()==null :
	 *    e1.getValue().equals(e2.getValue()))</pre>
	 * This ensures that the {@code equals} method works properly across
	 * different implementations of the {@code Map.Entry} interface.
	 *
	 * @param o object to be compared for equality with this map entry
	 * @return {@code true} if the specified object is equal to this map
	 *         entry
	 * @see    #hashCode
	 */
	virtual boolean equals(EMapEntry<llong, EV>* e) {
		return (key == e->getKey()) && eq(value, e->getValue());
	}

	/**
	 * Returns the hash code value for this map entry.  The hash code
	 * of a map entry {@code e} is defined to be: <pre>
	 *   (e.getKey()==null   ? 0 : e.getKey().hashCode()) ^
	 *   (e.getValue()==null ? 0 : e.getValue().hashCode())</pre>
	 * This ensures that {@code e1.equals(e2)} implies that
	 * {@code e1.hashCode()==e2.hashCode()} for any two Entries
	 * {@code e1} and {@code e2}, as required by the general
	 * contract of {@link Object#hashCode}.
	 *
	 * @return the hash code value for this map entry
	 * @see    #equals
	 */
	virtual int hashCode() {
		return (key) ^
			   (value == null ? 0 : value->hashCode());
	}
};

/**
 * An Entry maintaining an immutable key and value.  This class
 * does not support method <tt>setValue</tt>.  This class may be
 * convenient in methods that return thread-safe snapshots of
 * key-value mappings.
 *
 * @since 1.6
 */
template<typename V>
class ESimpleImmutableEntry<llong, V>: virtual public EMapEntry<llong,
		V> {
private:
	llong key;
	V value;

	static boolean eq(EObject* o1, EObject* o2) {
		return o1 == null ? o2 == null : o1->equals(o2);
	}
public:
	virtual ~ESimpleImmutableEntry(){}

	/**
	 * Creates an entry representing a mapping from the specified
	 * key to the specified value.
	 *
	 * @param key the key represented by this entry
	 * @param value the value represented by this entry
	 */
	ESimpleImmutableEntry(llong key,
			V value) {
		this->key = key;
		this->value = value;
	}

	/**
	 * Creates an entry representing the same mapping as the
	 * specified entry.
	 *
	 * @param entry the entry to copy
	 */
	ESimpleImmutableEntry(
			EMapEntry<llong, V> *entry) {
		this->key = entry->getKey();
		this->value = entry->getValue();
	}

	/**
	 * Returns the key corresponding to this entry.
	 *
	 * @return the key corresponding to this entry
	 */
	virtual llong getKey() {
		return key;
	}

	/**
	 * Returns the value corresponding to this entry.
	 *
	 * @return the value corresponding to this entry
	 */
	virtual V getValue() {
		return value;
	}

	/**
	 * Replaces the value corresponding to this entry with the specified
	 * value (optional operation).  This implementation simply throws
	 * <tt>UnsupportedOperationException</tt>, as this class implements
	 * an <i>immutable</i> map entry.
	 *
	 * @param value new value to be stored in this entry
	 * @return (Does not return)
	 * @throws UnsupportedOperationException always
	 */
	virtual V setValue(V value) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	/**
	 * Compares the specified object with this entry for equality.
	 * Returns {@code true} if the given object is also a map entry and
	 * the two entries represent the same mapping.  More formally, two
	 * entries {@code e1} and {@code e2} represent the same mapping
	 * if<pre>
	 *   (e1.getKey()==null ?
	 *    e2.getKey()==null :
	 *    e1.getKey().equals(e2.getKey()))
	 *   &amp;&amp;
	 *   (e1.getValue()==null ?
	 *    e2.getValue()==null :
	 *    e1.getValue().equals(e2.getValue()))</pre>
	 * This ensures that the {@code equals} method works properly across
	 * different implementations of the {@code Map.Entry} interface.
	 *
	 * @param o object to be compared for equality with this map entry
	 * @return {@code true} if the specified object is equal to this map
	 *         entry
	 * @see    #hashCode
	 */
	virtual boolean equals(
			EMapEntry<llong, V>* e) {
		return (key == e->getKey()) && eq(value, e->getValue());
	}

	/**
	 * Returns the hash code value for this map entry.  The hash code
	 * of a map entry {@code e} is defined to be: <pre>
	 *   (e.getKey()==null   ? 0 : e.getKey().hashCode()) ^
	 *   (e.getValue()==null ? 0 : e.getValue().hashCode())</pre>
	 * This ensures that {@code e1.equals(e2)} implies that
	 * {@code e1.hashCode()==e2.hashCode()} for any two Entries
	 * {@code e1} and {@code e2}, as required by the general
	 * contract of {@link Object#hashCode}.
	 *
	 * @return the hash code value for this map entry
	 * @see    #equals
	 */
	virtual int hashCode() {
		return (key)
				^ (value == null ? 0 : value->hashCode());
	}
};

template<typename V>
abstract class EAbstractMap<llong, V>: virtual public EMap<llong, V> {
protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractMap() {
		_keySet = null;
		_values = null;
	}

	template<typename _EKeySet>
	class KeySet: public EAbstractSet<_EKeySet> {
	private:
		template<typename _EKeySetI>
		class KSListIterator: public EIterator<_EKeySetI> {
		private:
			EIterator<EMapEntry<llong,V>*> *i;

		public:
			KSListIterator(EAbstractMap<llong,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			_EKeySetI next() {
				return i->next()->getKey();
			}

			void remove() {
				i->remove();
			}
		};

		EAbstractMap<llong,V> *_map;

	public:
		KeySet(EAbstractMap<llong,V> *map) {
			_map = map;
		}

		EIterator<_EKeySet>* iterator(int index=0) {
			return new KSListIterator<_EKeySet>(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(_EKeySet k) {
			return _map->containsKey(k);
		}
	};

	template<typename _EValueCollection>
		class ValueCollection: public EAbstractCollection<_EValueCollection> {
		private:
			template<typename _EValueCollectionI>
			class VCListIterator: public EIterator<_EValueCollectionI> {
			private:
				EIterator<EMapEntry<llong,V>*> *i;

			public:
				VCListIterator(EAbstractMap<llong,V> *map) {
					 i = map->entrySet()->iterator();
				}

				boolean hasNext() {
					return i->hasNext();
				}

				_EValueCollectionI next() {
					return i->next()->getValue();
				}

				void remove() {
					i->remove();
				}
			};

			EAbstractMap<llong,V> *_map;

		public:
			ValueCollection(EAbstractMap<llong,V> *map) {
				_map = map;
			}

			EIterator<_EValueCollection>* iterator(int index=0) {
				return new VCListIterator<_EValueCollection>(_map);
			}

			int size() {
				return _map->size();
			}

			boolean contains(llong v) {
				return _map->containsKey(v);
			}
		};

public:
	virtual ~EAbstractMap() {
		if (_keySet) {
			delete _keySet;
		}
		if (_values) {
			delete _values;
		}
	}

	// Query Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns <tt>entrySet().size()</tt>.
	 */
	virtual int size() {
		return entrySet()->size();
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns <tt>size() == 0</tt>.
	 */
	virtual boolean isEmpty() {
		return size() == 0;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified value.  If such an entry is found,
	 * <tt>true</tt> is returned.  If the iteration terminates without
	 * finding such an entry, <tt>false</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean containsValue(V value) {
		EIterator<EMapEntry<llong, V>*> *i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<llong, V> *e = i->next();
			if (value->equals(e->getValue())) {
				delete i;
				return true;
			}
		}
		delete i;
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified key.  If such an entry is found,
	 * <tt>true</tt> is returned.  If the iteration terminates without
	 * finding such an entry, <tt>false</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map; many
	 * implementations will override this method.
	 *
	 * @throws ClassCastException   {@inheritDoc}
	 * @throws NullPointerException {@inheritDoc}
	 */
	virtual boolean containsKey(llong key) {
		EIterator<EMapEntry<llong, V>*> *i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<llong, V> *e = i->next();
			if (key == (e->getKey())) {
				delete i;
				return true;
			}
		}
		delete i;
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching
	 * for an entry with the specified key.  If such an entry is found,
	 * the entry's value is returned.  If the iteration terminates without
	 * finding such an entry, <tt>null</tt> is returned.  Note that this
	 * implementation requires linear time in the size of the map; many
	 * implementations will override this method.
	 *
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 */
	virtual V get(llong key) THROWS(ENoSuchElementException) {
		EIterator<EMapEntry<llong, V>*> *i =
						entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<llong, V> *e = i->next();
			if (key == (e->getKey())) {
				delete i;
				return e->getValue();
			}
		}
		delete i;
		throw ENOSUCHELEMENTEXCEPTION;
	}

	// Modification Operations

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation always throws an
	 * <tt>UnsupportedOperationException</tt>.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	virtual V put(llong key, V value) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt> searching for an
	 * entry with the specified key.  If such an entry is found, its value is
	 * obtained with its <tt>getValue</tt> operation, the entry is removed
	 * from the collection (and the backing map) with the iterator's
	 * <tt>remove</tt> operation, and the saved value is returned.  If the
	 * iteration terminates without finding such an entry, <tt>null</tt> is
	 * returned.  Note that this implementation requires linear time in the
	 * size of the map; many implementations will override this method.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if the <tt>entrySet</tt>
	 * iterator does not support the <tt>remove</tt> method and this map
	 * contains a mapping for the specified key.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 */
	virtual V remove(llong key) THROWS(ENoSuchElementException) {
		EIterator<EMapEntry<llong, V>*> *i =
								entrySet()->iterator();
		EMapEntry<llong, V> *correctEntry = null;
		while (correctEntry == null && i->hasNext()) {
			EMapEntry<llong, V> *e = i->next();
			if (key == (e->getKey()))
				correctEntry = e;
		}

		V oldValue;
		if (correctEntry != null) {
			oldValue = correctEntry->getValue();
			i->remove();
			delete i;
			return oldValue;
		}
		delete i;
		throw ENOSUCHELEMENTEXCEPTION;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation calls <tt>entrySet().clear()</tt>.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if the <tt>entrySet</tt>
	 * does not support the <tt>clear</tt> operation.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 */
	virtual void clear() {
		entrySet()->clear();
	}

	// Views

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a set that subclasses {@link AbstractSet}.
	 * The subclass's iterator method returns a "wrapper object" over this
	 * map's <tt>entrySet()</tt> iterator.  The <tt>size</tt> method
	 * delegates to this map's <tt>size</tt> method and the
	 * <tt>contains</tt> method delegates to this map's
	 * <tt>containsKey</tt> method.
	 *
	 * <p>The set is created the first time this method is called,
	 * and returned in response to all subsequent calls.  No synchronization
	 * is performed, so there is a slight chance that multiple calls to this
	 * method will not all return the same set.
	 */
	virtual ESet<llong>* keySet() {
		if (_keySet == null) {
			_keySet = new KeySet<llong>(this);
		}
		return _keySet;
	}

	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation returns a collection that subclasses {@link
	 * AbstractCollection}.  The subclass's iterator method returns a
	 * "wrapper object" over this map's <tt>entrySet()</tt> iterator.
	 * The <tt>size</tt> method delegates to this map's <tt>size</tt>
	 * method and the <tt>contains</tt> method delegates to this map's
	 * <tt>containsValue</tt> method.
	 *
	 * <p>The collection is created the first time this method is called, and
	 * returned in response to all subsequent calls.  No synchronization is
	 * performed, so there is a slight chance that multiple calls to this
	 * method will not all return the same collection.
	 */
	virtual ECollection<V>* values() {
		if (_values == null) {
			_values = new ValueCollection<V>(this);
		}
		return _values;
	}

	virtual ESet<EMapEntry<llong, V>*>* entrySet() = 0;

protected:
	/**
	 * Each of these fields are initialized to contain an instance of the
	 * appropriate view the first time this view is requested.  The views are
	 * stateless, so there's no reason to create more than one of each.
	 */
	ESet<llong> *_keySet;
	ECollection<V> *_values;
};

#endif //!(defined(_MSC_VER) && (_MSC_VER<=1200))

} /* namespace efc */
#endif //!__EAbstractMap_H__
