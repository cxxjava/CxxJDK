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
 * @version 1.50, 06/16/06
 * @see Map
 * @see Collection
 * @since 1.2
 */

//=============================================================================
//Primitive Key && (Native pointer Value | Shared pointer Value).

template<typename K, typename V>
abstract class EAbstractMap : virtual public EMap<K, V> {
public:
	typedef typename ETraits<V>::indexType idxV;

protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractMap() {
		_keySet = null;
		_values = null;
	}

	class KeySet: public EAbstractSet<K> {
	private:
		class KSListIterator: public EIterator<K> {
		private:
			sp<EIterator<EMapEntry<K,V>*> > i;

		public:
			KSListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			K next() {
				return i->next()->getKey();
			}

			void remove() {
				i->remove();
			}

			K moveOut() {
				EMapEntry<K,V>* e = i->moveOut();
				K o = e->getKey();
				delete e;
				return o;
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		KeySet(EAbstractMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<K> > iterator(int index=0) {
			return new KSListIterator(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(K k) {
			return _map->containsKey(k);
		}
	};

	class ValueCollection: public EAbstractCollection<V> {
	private:
		class VCListIterator: public EIterator<V> {
		private:
			sp<EIterator<EMapEntry<K,V>*> > i;

		public:
			VCListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			V next() {
				return i->next()->getValue();
			}

			void remove() {
				i->remove();
			}

			V moveOut() {
				EMapEntry<K,V>* e = i->moveOut();
				V o = e->getValue();
				delete e;
				return o;
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		ValueCollection(EAbstractMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<V> > iterator(int index=0) {
			return new VCListIterator(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(idxV v) {
			return _map->containsValue(v);
		}
	};

public:
	virtual ~EAbstractMap() {
		//
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
	virtual boolean containsValue(idxV value) {
		sp<EIterator<EMapEntry<K, V>*> > i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			V v = e->getValue();
			if ((v != null && v->equals(value)) || (v == null && value == null)) {
				return true;
			}
		}
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
		sp<EIterator<EMapEntry<K, V>*> > i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			if (key == (e->getKey())) {
				return true;
			}
		}
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
		sp<EIterator<EMapEntry<K, V>*> > i =
						entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			if (key == (e->getKey())) {
				return e->getValue();
			}
		}
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
		sp<EIterator<EMapEntry<K, V>*> > i =
								entrySet()->iterator();
		EMapEntry<K, V> *correctEntry = null;
		while (correctEntry == null && i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			if (key == (e->getKey()))
				correctEntry = e;
		}

		V oldValue;
		if (correctEntry != null) {
			oldValue = correctEntry->getValue();
			i->remove();
			return oldValue;
		}
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
	virtual sp<ESet<K> > keySet() {
		if (_keySet == null) {
			_keySet = new KeySet(this);
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
	virtual sp<ECollection<V> > values() {
		if (_values == null) {
			_values = new ValueCollection(this);
		}
		return _values;
	}

	/**
	 * Returns the hash code value for this map.  The hash code of a map is
	 * defined to be the sum of the hash codes of each entry in the map's
	 * <tt>entrySet()</tt> view.  This ensures that <tt>m1.equals(m2)</tt>
	 * implies that <tt>m1.hashCode()==m2.hashCode()</tt> for any two maps
	 * <tt>m1</tt> and <tt>m2</tt>, as required by the general contract of
	 * {@link Object#hashCode}.
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt>, calling
	 * {@link Map.Entry#hashCode hashCode()} on each element (entry) in the
	 * set, and adding up the results.
	 *
	 * @return the hash code value for this map
	 * @see Map.Entry#hashCode()
	 * @see Object#equals(Object)
	 * @see Set#equals(Object)
	 */
	virtual int hashCode() {
		int h = 0;
		sp<EIterator<EMapEntry<K,V>*> > i = entrySet()->iterator();
		while (i->hasNext())
			h += i->next()->hashCode();
		return h;
	}

	/**
	 * Returns a string representation of this map.  The string representation
	 * consists of a list of key-value mappings in the order returned by the
	 * map's <tt>entrySet</tt> view's iterator, enclosed in braces
	 * (<tt>"{}"</tt>).  Adjacent mappings are separated by the characters
	 * <tt>", "</tt> (comma and space).  Each key-value mapping is rendered as
	 * the key followed by an equals sign (<tt>"="</tt>) followed by the
	 * associated value.  Keys and values are converted to strings as by
	 * {@link String#valueOf(Object)}.
	 *
	 * @return a string representation of this map
	 */
	virtual EStringBase toString() {
		sp<EIterator<EMapEntry<K,V>*> > i = entrySet()->iterator();
		if (! i->hasNext()) {
			return "{}";
		}

		EStringBase sb;
		sb.append('{');
		for (;;) {
			EMapEntry<K,V>* e = i->next();
			K key = e->getKey();
			V value = e->getValue();
			sb.append(key);
			sb.append('=');
			sb.append(value->toString().c_str());
			if (! i->hasNext()) {
				return sb.append('}');
			}
			sb.append(',').append(' ');
		}
		return sb;
	}

	virtual sp<ESet<EMapEntry<K, V>*> > entrySet() = 0;

protected:
	/**
	 * Each of these fields are initialized to contain an instance of the
	 * appropriate view the first time this view is requested.  The views are
	 * stateless, so there's no reason to create more than one of each.
	 */
	sp<ESet<K> > _keySet;
	sp<ECollection<V> > _values;
};

//=============================================================================
//Native poiner Types.

template<typename _K, typename _V>
abstract class EAbstractMap<_K*, _V*>: virtual public EMap<_K*, _V*> {
public:
	typedef _K* K;
	typedef _V* V;

protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractMap() {
		_keySet = null;
		_values = null;
	}

	class KeySet: public EAbstractSet<K> {
	private:
		class KSListIterator: public EIterator<K> {
		private:
			sp<EIterator<EMapEntry<K,V>*> > i;

		public:
			KSListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			K next() {
				return i->next()->getKey();
			}

			void remove() {
				i->remove();
			}

			K moveOut() {
				EMapEntry<K,V>* e = i->moveOut();
				K o = e->getKey();
				delete e;
				return o;
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		KeySet(EAbstractMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<K> > iterator(int index=0) {
			return new KSListIterator(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(_K* k) {
			return _map->containsKey(k);
		}
	};

	class ValueCollection: public EAbstractCollection<V> {
	private:
		class VCListIterator: public EIterator<V> {
		private:
			sp<EIterator<EMapEntry<K,V>*> > i;

		public:
			VCListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			V next() {
				return i->next()->getValue();
			}

			void remove() {
				i->remove();
			}

			V moveOut() {
				EMapEntry<K,V>* e = i->moveOut();
				V o = e->getValue();
				delete e;
				return o;
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		ValueCollection(EAbstractMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<V> > iterator(int index=0) {
			return new VCListIterator(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(_V* v) {
			return _map->containsValue(v);
		}
	};

public:
	virtual ~EAbstractMap() {
		//
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
	virtual boolean containsValue(_V* value) {
		sp<EIterator<EMapEntry<K, V>*> > i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			V v = e->getValue();
			if ((v != null && v->equals(value)) || (v == null && value == null)) {
				return true;
			}
		}
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
	virtual boolean containsKey(_K* key) {
		sp<EIterator<EMapEntry<K, V>*> > i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			K k = e->getKey();
			if ((k != null && k->equals(key)) || (k == null && key == null)) {
				return true;
			}
		}
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
	virtual V get(_K* key) THROWS(ENoSuchElementException) {
		sp<EIterator<EMapEntry<K, V>*> > i =
						entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			K k = e->getKey();
			if ((k != null && k->equals(key)) || (k == null && key == null)) {
				return e->getValue();
			}
		}
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
	virtual V remove(_K* key) THROWS(ENoSuchElementException) {
		sp<EIterator<EMapEntry<K, V>*> > i =
								entrySet()->iterator();
		EMapEntry<K, V> *correctEntry = null;
		while (correctEntry == null && i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			K k = e->getKey();
			if ((k != null && k->equals(key)) || (k == null && key == null)) {
				correctEntry = e;
			}
		}

		V oldValue;
		if (correctEntry != null) {
			oldValue = correctEntry->getValue();
			i->remove();
			return oldValue;
		}
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
	virtual sp<ESet<K> > keySet() {
		if (_keySet == null) {
			_keySet = new KeySet(this);
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
	virtual sp<ECollection<V> > values() {
		if (_values == null) {
			_values = new ValueCollection(this);
		}
		return _values;
	}

	/**
	 * Returns the hash code value for this map.  The hash code of a map is
	 * defined to be the sum of the hash codes of each entry in the map's
	 * <tt>entrySet()</tt> view.  This ensures that <tt>m1.equals(m2)</tt>
	 * implies that <tt>m1.hashCode()==m2.hashCode()</tt> for any two maps
	 * <tt>m1</tt> and <tt>m2</tt>, as required by the general contract of
	 * {@link Object#hashCode}.
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt>, calling
	 * {@link Map.Entry#hashCode hashCode()} on each element (entry) in the
	 * set, and adding up the results.
	 *
	 * @return the hash code value for this map
	 * @see Map.Entry#hashCode()
	 * @see Object#equals(Object)
	 * @see Set#equals(Object)
	 */
	virtual int hashCode() {
		int h = 0;
		sp<EIterator<EMapEntry<K,V>*> > i = entrySet()->iterator();
		while (i->hasNext())
			h += i->next()->hashCode();
		return h;
	}

	/**
	 * Returns a string representation of this map.  The string representation
	 * consists of a list of key-value mappings in the order returned by the
	 * map's <tt>entrySet</tt> view's iterator, enclosed in braces
	 * (<tt>"{}"</tt>).  Adjacent mappings are separated by the characters
	 * <tt>", "</tt> (comma and space).  Each key-value mapping is rendered as
	 * the key followed by an equals sign (<tt>"="</tt>) followed by the
	 * associated value.  Keys and values are converted to strings as by
	 * {@link String#valueOf(Object)}.
	 *
	 * @return a string representation of this map
	 */
	virtual EStringBase toString() {
		sp<EIterator<EMapEntry<K,V>*> > i = entrySet()->iterator();
		if (! i->hasNext()) {
			return "{}";
		}

		EStringBase sb;
		sb.append('{');
		for (;;) {
			EMapEntry<K,V>* e = i->next();
			K key = e->getKey();
			V value = e->getValue();
			sb.append((void*)key   == (void*)this ? "(this Map)" : key->toString().c_str());
			sb.append('=');
			sb.append((void*)value   == (void*)this ? "(this Map)" : value->toString().c_str());
			if (! i->hasNext()) {
				return sb.append('}');
			}
			sb.append(',').append(' ');
		}
		return sb;
	}

	virtual sp<ESet<EMapEntry<K, V>*> > entrySet() = 0;

protected:
	/**
	 * Each of these fields are initialized to contain an instance of the
	 * appropriate view the first time this view is requested.  The views are
	 * stateless, so there's no reason to create more than one of each.
	 */
	sp<ESet<K> > _keySet;
	sp<ECollection<V> > _values;
};

//=============================================================================
//Shared poiner Types.

template<typename _K, typename _V>
abstract class EAbstractMap<sp<_K>, sp<_V> >: virtual public EMap<sp<_K>, sp<_V> > {
public:
	typedef sp<_K> K;
	typedef sp<_V> V;

protected:
	/**
	 * Sole constructor.  (For invocation by subclass constructors, typically
	 * implicit.)
	 */
	EAbstractMap() {
		_keySet = null;
		_values = null;
	}

	class KeySet: public EAbstractSet<K> {
	private:
		class KSListIterator: public EIterator<K> {
		private:
			sp<EIterator<EMapEntry<K,V>*> > i;

		public:
			KSListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			K next() {
				return i->next()->getKey();
			}

			void remove() {
				i->remove();
			}

			K moveOut() {
				EMapEntry<K,V>* e = i->moveOut();
				K o = e->getKey();
				delete e;
				return o;
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		KeySet(EAbstractMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<K> > iterator(int index=0) {
			return new KSListIterator(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(_K* k) {
			return _map->containsKey(k);
		}
	};

	class ValueCollection: public EAbstractCollection<V> {
	private:
		class VCListIterator: public EIterator<V> {
		private:
			sp<EIterator<EMapEntry<K,V>*> > i;

		public:
			VCListIterator(EAbstractMap<K,V> *map) {
				 i = map->entrySet()->iterator();
			}

			boolean hasNext() {
				return i->hasNext();
			}

			V next() {
				return i->next()->getValue();
			}

			void remove() {
				i->remove();
			}

			V moveOut() {
				EMapEntry<K,V>* e = i->moveOut();
				V o = e->getValue();
				delete e;
				return o;
			}
		};

		EAbstractMap<K,V> *_map;

	public:
		ValueCollection(EAbstractMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<V> > iterator(int index=0) {
			return new VCListIterator(_map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(_V* v) {
			return _map->containsValue(v);
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
	virtual boolean containsValue(_V* value) {
		sp<EIterator<EMapEntry<K, V>*> > i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			V v = e->getValue();
			if ((v != null && v->equals(value)) || (v == null && value == null)) {
				return true;
			}
		}
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
	virtual boolean containsKey(_K* key) {
		sp<EIterator<EMapEntry<K, V>*> > i =
				entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			K k = e->getKey();
			if ((k != null && k->equals(key)) || (k == null && key == null)) {
				return true;
			}
		}
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
	virtual V get(_K* key) THROWS(ENoSuchElementException) {
		sp<EIterator<EMapEntry<K, V>*> > i =
						entrySet()->iterator();
		while (i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			K k = e->getKey();
			if ((k != null && k->equals(key)) || (k == null && key == null)) {
				return e->getValue();
			}
		}
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
	virtual V remove(_K* key) THROWS(ENoSuchElementException) {
		sp<EIterator<EMapEntry<K, V>*> > i =
								entrySet()->iterator();
		EMapEntry<K, V> *correctEntry = null;
		while (correctEntry == null && i->hasNext()) {
			EMapEntry<K, V> *e = i->next();
			K k = e->getKey();
			if ((k != null && k->equals(key)) || (k == null && key == null)) {
				correctEntry = e;
			}
		}

		V oldValue;
		if (correctEntry != null) {
			oldValue = correctEntry->getValue();
			i->remove();
			return oldValue;
		}
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
	virtual sp<ESet<K> > keySet() {
		if (_keySet == null) {
			_keySet = new KeySet(this);
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
	virtual sp<ECollection<V> > values() {
		if (_values == null) {
			_values = new ValueCollection(this);
		}
		return _values;
	}

	/**
	 * Returns the hash code value for this map.  The hash code of a map is
	 * defined to be the sum of the hash codes of each entry in the map's
	 * <tt>entrySet()</tt> view.  This ensures that <tt>m1.equals(m2)</tt>
	 * implies that <tt>m1.hashCode()==m2.hashCode()</tt> for any two maps
	 * <tt>m1</tt> and <tt>m2</tt>, as required by the general contract of
	 * {@link Object#hashCode}.
	 *
	 * <p>This implementation iterates over <tt>entrySet()</tt>, calling
	 * {@link Map.Entry#hashCode hashCode()} on each element (entry) in the
	 * set, and adding up the results.
	 *
	 * @return the hash code value for this map
	 * @see Map.Entry#hashCode()
	 * @see Object#equals(Object)
	 * @see Set#equals(Object)
	 */
	virtual int hashCode() {
		int h = 0;
		sp<EIterator<EMapEntry<K,V>*> > i = entrySet()->iterator();
		while (i->hasNext())
			h += i->next()->hashCode();
		return h;
	}

	/**
	 * Returns a string representation of this map.  The string representation
	 * consists of a list of key-value mappings in the order returned by the
	 * map's <tt>entrySet</tt> view's iterator, enclosed in braces
	 * (<tt>"{}"</tt>).  Adjacent mappings are separated by the characters
	 * <tt>", "</tt> (comma and space).  Each key-value mapping is rendered as
	 * the key followed by an equals sign (<tt>"="</tt>) followed by the
	 * associated value.  Keys and values are converted to strings as by
	 * {@link String#valueOf(Object)}.
	 *
	 * @return a string representation of this map
	 */
	virtual EStringBase toString() {
		sp<EIterator<EMapEntry<K,V>*> > i = entrySet()->iterator();
		if (! i->hasNext()) {
			return "{}";
		}

		EStringBase sb;
		sb.append('{');
		for (;;) {
			EMapEntry<K,V>* e = i->next();
			K key = e->getKey();
			V value = e->getValue();
			sb.append(key->toString().c_str());
			sb.append('=');
			sb.append(value->toString().c_str());
			if (! i->hasNext()) {
				return sb.append('}');
			}
			sb.append(',').append(' ');
		}
		return sb;
	}

	virtual sp<ESet<EMapEntry<K, V>*> > entrySet() = 0;

protected:
	/**
	 * Each of these fields are initialized to contain an instance of the
	 * appropriate view the first time this view is requested.  The views are
	 * stateless, so there's no reason to create more than one of each.
	 */
	sp<ESet<K> > _keySet;
	sp<ECollection<V> > _values;
};

} /* namespace efc */
#endif //!__EAbstractMap_H__
