/*
 * ETreeMap.hh
 *
 *  Created on: 2014-8-15
 *      Author: cxxjava@163.com
 */

#ifndef ETREEMAP_HH_
#define ETREEMAP_HH_

#include "EAbstractMap.hh"
#include "ENavigableMap.hh"
#include "EComparator.hh"
#include "EToDoException.hh"
#include "ENullPointerException.hh"
#include "ENoSuchElementException.hh"
#include "EIllegalStateException.hh"
#include "EConcurrentModificationException.hh"

namespace efc {

/**
 * A Red-Black tree based {@link NavigableMap} implementation.
 * The map is sorted according to the {@linkplain Comparable natural
 * ordering} of its keys, or by a {@link Comparator} provided at map
 * creation time, depending on which constructor is used.
 *
 * <p>This implementation provides guaranteed log(n) time cost for the
 * <tt>containsKey</tt>, <tt>get</tt>, <tt>put</tt> and <tt>remove</tt>
 * operations.  Algorithms are adaptations of those in Cormen, Leiserson, and
 * Rivest's <I>Introduction to Algorithms</I>.
 *
 * <p>Note that the ordering maintained by a sorted map (whether or not an
 * explicit comparator is provided) must be <i>consistent with equals</i> if
 * this sorted map is to correctly implement the <tt>Map</tt> interface.  (See
 * <tt>Comparable</tt> or <tt>Comparator</tt> for a precise definition of
 * <i>consistent with equals</i>.)  This is so because the <tt>Map</tt>
 * interface is defined in terms of the equals operation, but a map performs
 * all key comparisons using its <tt>compareTo</tt> (or <tt>compare</tt>)
 * method, so two keys that are deemed equal by this method are, from the
 * standpoint of the sorted map, equal.  The behavior of a sorted map
 * <i>is</i> well-defined even if its ordering is inconsistent with equals; it
 * just fails to obey the general contract of the <tt>Map</tt> interface.
 *
 * <p><strong>Note that this implementation is not synchronized.</strong>
 * If multiple threads access a map concurrently, and at least one of the
 * threads modifies the map structurally, it <i>must</i> be synchronized
 * externally.  (A structural modification is any operation that adds or
 * deletes one or more mappings; merely changing the value associated
 * with an existing key is not a structural modification.)  This is
 * typically accomplished by synchronizing on some object that naturally
 * encapsulates the map.
 * If no such object exists, the map should be "wrapped" using the
 * {@link Collections#synchronizedSortedMap Collections.synchronizedSortedMap}
 * method.  This is best done at creation time, to prevent accidental
 * unsynchronized access to the map: <pre>
 *   SortedMap m = Collections.synchronizedSortedMap(new TreeMap(...));</pre>
 *
 * <p>The iterators returned by the <tt>iterator</tt> method of the collections
 * returned by all of this class's "collection view methods" are
 * <i>fail-fast</i>: if the map is structurally modified at any time after the
 * iterator is created, in any way except through the iterator's own
 * <tt>remove</tt> method, the iterator will throw a {@link
 * ConcurrentModificationException}.  Thus, in the face of concurrent
 * modification, the iterator fails quickly and cleanly, rather than risking
 * arbitrary, non-deterministic behavior at an undetermined time in the future.
 *
 * <p>Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw <tt>ConcurrentModificationException</tt> on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness:   <i>the fail-fast behavior of iterators
 * should be used only to detect bugs.</i>
 *
 * <p>All <tt>Map.Entry</tt> pairs returned by methods in this class
 * and its views represent snapshots of mappings at the time they were
 * produced. They do <em>not</em> support the <tt>Entry.setValue</tt>
 * method. (Note however that it is possible to change mappings in the
 * associated map using <tt>put</tt>.)
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @param <K> the type of keys maintained by this map
 * @param <V> the type of mapped values
 *
 * @see Map
 * @see HashMap
 * @see Hashtable
 * @see Comparable
 * @see Comparator
 * @see Collection
 * @since 1.2
 */

template<typename K, typename V>
class ETreeMap : public EAbstractMap<K,V>,
    virtual public ENavigableMap<K,V>
{
private:
	class KeyIterator;
	class ValueIterator;
	class DescendingKeyIterator;

	// Red-black mechanics
	static const boolean RED = false;
	static const boolean BLACK = true;

	/**
	 * Node in the Tree.  Doubles as a means to pass key-value pairs back to
	 * user (see Map.Entry).
	 */
	class Entry: public EMapEntry<K,V> {
	public:
		K key;
		V value;
		Entry* left;// = null;
		Entry* right;// = null;
		Entry* parent;
		boolean color;// = BLACK;
		ETreeMap<K, V> *map;

		~Entry() {
			if (map->getAutoFreeKey()) {
				delete key;
			}
			if (map->getAutoFreeValue()) {
				delete value;
			}
			delete left;
			delete right;
		}

		/**
		 * Make a new cell with given key, value, and parent, and with
		 * <tt>null</tt> child links, and BLACK color.
		 */
		Entry(K key, V value, Entry* parent, ETreeMap<K, V> *map) :
				left(null), right(null), color(BLACK) {
			this->key = key;
			this->value = value;
			this->parent = parent;
			this->map = map;
		}

		/**
		 * Returns the key.
		 *
		 * @return the key
		 */
		K getKey() {
			return key;
		}

		/**
		 * Returns the value associated with the key.
		 *
		 * @return the value associated with the key
		 */
		V getValue() {
			return value;
		}

		/**
		 * Replaces the value currently associated with the key with the given
		 * value.
		 *
		 * @return the value associated with the key before this method was
		 *         called
		 */
		V setValue(V value) {
			V oldValue = this->value;
			this->value = value;
			return oldValue;
		}

		boolean equals(EMapEntry<K,V>* o) {
			return valEquals(key,o->getKey()) && valEquals(value,o->getValue());
		}

		virtual int hashCode() {
			int keyHash = (key==null ? 0 : key->hashCode());
			int valueHash = (value==null ? 0 : value->hashCode());
			return keyHash ^ valueHash;
		}

		virtual EStringBase toString() {
			return EStringBase::formatOf("%s=%s", key->toString().c_str(), value->toString().c_str());
		}
	};

	/**
	 * Base class for TreeMap Iterators
	 */
	template<typename T>
	abstract class PrivateEntryIterator : virtual public EIterator<T> {
	public:
		Entry* next;
		Entry* lastReturned;
		int expectedModCount;
		ETreeMap<K,V>* _map;

		PrivateEntryIterator(Entry* first, ETreeMap<K,V> *map) {
			_map = map;

			expectedModCount = _map->modCount;
			lastReturned = null;
			next = first;
		}

		virtual Entry* nextEntry() {
			Entry* e = next;
			if (e == null)
				throw ENoSuchElementException(__FILE__, __LINE__);
			if (_map->modCount != expectedModCount)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			next = successor(e);
			lastReturned = e;
			return e;
		}

		virtual Entry* prevEntry() {
			Entry* e = next;
			if (e == null)
				throw ENoSuchElementException(__FILE__, __LINE__);
			if (_map->modCount != expectedModCount)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			next = predecessor(e);
			lastReturned = e;
			return e;
		}

		virtual boolean hasNext() {
			return next != null;
		}

		virtual void remove() {
			if (lastReturned == null)
				throw EIllegalStateException(__FILE__, __LINE__);
			if (_map->modCount != expectedModCount)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			// deleted entries are replaced by their successors
			if (lastReturned->left != null && lastReturned->right != null)
				next = lastReturned;
			delete _map->deleteEntry(lastReturned);
			expectedModCount = _map->modCount;
			lastReturned = null;
		}

		virtual Entry* moveOutEntry() {
			if (lastReturned == null)
				throw EIllegalStateException(__FILE__, __LINE__);
			if (_map->modCount != expectedModCount)
				throw EConcurrentModificationException(__FILE__, __LINE__);
			// deleted entries are replaced by their successors
			if (lastReturned->left != null && lastReturned->right != null)
				next = lastReturned;
			Entry* e = _map->deleteEntry(lastReturned);
			expectedModCount = _map->modCount;
			lastReturned = null;
			return e;
		}
	};

	class EntryIterator : public PrivateEntryIterator<EMapEntry<K,V>*> {
	public:
		EntryIterator(Entry* first, ETreeMap<K,V>* map) : PrivateEntryIterator<EMapEntry<K,V>*>(first, map) {
		}
		EMapEntry<K,V>* next() {
			return PrivateEntryIterator<EMapEntry<K,V>*>::nextEntry();
		}
		EMapEntry<K,V>* moveOut() {
			return PrivateEntryIterator<EMapEntry<K,V>*>::moveOutEntry();
		}
	};

	class ValueIterator : public PrivateEntryIterator<V> {
	public:
		ValueIterator(Entry* first, ETreeMap<K,V>* map) : PrivateEntryIterator<V>(first, map) {
		}
		V next() {
			return PrivateEntryIterator<V>::nextEntry()->value;
		}
		V moveOut() {
			Entry* e = PrivateEntryIterator<V>::moveOutEntry();
			V v = e->value;
			delete e;
			return v;
		}
	};

	class KeyIterator : public PrivateEntryIterator<K> {
	public:
		KeyIterator(Entry* first, ETreeMap<K,V> *map) : PrivateEntryIterator<K>(first, map) {
		}
		K next() {
			return PrivateEntryIterator<K>::nextEntry()->key;
		}
		K moveOut() {
			Entry* e = PrivateEntryIterator<K>::moveOutEntry();
			K k = e->key;
			delete e;
			return k;
		}
	};

	class DescendingKeyIterator : public PrivateEntryIterator<K> {
	public:
		DescendingKeyIterator(Entry* first, ETreeMap<K,V> *map) : PrivateEntryIterator<K>(first, map) {
		}
		K next() {
			return PrivateEntryIterator<K>::prevEntry()->key;
		}
		K moveOut() {
			Entry* e = PrivateEntryIterator<K>::moveOutEntry();
			K k = e->key;
			delete e;
			return k;
		}
	};

	class Values : public EAbstractCollection<V> {
	private:
		ETreeMap<K,V> *_map;

	public:
		Values(ETreeMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<V> > iterator(int index=0) {
			ES_ASSERT(index == 0);
			return new ValueIterator(_map->getFirstEntry(), _map);
		}

		int size() {
			return _map->size();
		}

		boolean contains(V o) {
			return _map->containsValue(o);
		}

		boolean remove(V o) {
			for (Entry* e = _map->getFirstEntry(); e != null; e = successor(e)) {
				if (valEquals(e->getValue(), o)) {
					delete _map->deleteEntry(e);
					return true;
				}
			}
			return false;
		}

		void clear() {
			_map->clear();
		}
	};

	class EntrySet : public EAbstractSet<EMapEntry<K,V>*> {
	private:
		ETreeMap<K,V> *_map;

	public:
		EntrySet(ETreeMap<K,V> *map) {
			_map = map;
		}

		sp<EIterator<EMapEntry<K,V>*> > iterator(int index=0) {
			return new EntryIterator(_map->getFirstEntry(), _map);
		}

		boolean contains(EMapEntry<K,V>* o) {
			EMapEntry<K,V>* entry = (EMapEntry<K,V>*) o;
			V value = entry->getValue();
			Entry* p = _map->getEntry(entry->getKey());
			return p != null && valEquals(p->getValue(), value);
		}

		boolean remove(EMapEntry<K,V>* o) {
			EMapEntry<K,V>* entry = (EMapEntry<K,V>*) o;
			V value = entry->getValue();
			Entry* p = _map->getEntry(entry->getKey());
			if (p != null && valEquals(p->getValue(), value)) {
				delete _map->deleteEntry(p);
				return true;
			}
			return false;
		}

		int size() {
			return _map->size();
		}

		void clear() {
			_map->clear();
		}
	};

	class KeySet : public EAbstractSet<K>, virtual public ENavigableSet<K> {
	private:
		ETreeMap<K,V>* m;

	public:
		KeySet(ETreeMap<K,V>* map) { m = map; }

		sp<EIterator<K> > iterator(int index=0) {
			return m->keyIterator();
		}

		sp<EIterator<K> > descendingIterator() {
			return m->descendingKeyIterator();
		}

		int size() { return m->size(); }
		boolean isEmpty() { return m->isEmpty(); }
		boolean contains(K o) { return m->containsKey(o); }
		void clear() { m->clear(); }
		K lower(K e) { return m->lowerKey(e); }
		K floor(K e) { return m->floorKey(e); }
		K ceiling(K e) { return m->ceilingKey(e); }
		K higher(K e) { return m->higherKey(e); }
		K first() { return m->firstKey(); }
		K last() { return m->lastKey(); }
		EComparator<K>* comparator() { return m->comparator(); }
		K pollFirst() {
			EMapEntry<K,V>* e = m->pollFirstEntry();
			return e == null? null : e->getKey();
		}
		K pollLast() {
			EMapEntry<K,V>* e = m->pollLastEntry();
			return e == null? null : e->getKey();
		}
		boolean remove(K o) {
			int oldSize = size();
			V v = m->remove(o);
			if (m->_autoFreeValue) delete v; //!
			return size() != oldSize;
		}
		ENavigableSet<K>* subSet(K fromElement, boolean fromInclusive,
									  K toElement,   boolean toInclusive) {
			throw EToDoException(__FILE__, __LINE__);
		}
		ENavigableSet<K>* headSet(K toElement, boolean inclusive) {
			throw EToDoException(__FILE__, __LINE__);
		}
		ENavigableSet<K>* tailSet(K fromElement, boolean inclusive) {
			throw EToDoException(__FILE__, __LINE__);
		}
		ESortedSet<K>* subSet(K fromElement, K toElement) {
			throw EToDoException(__FILE__, __LINE__);
		}
		ESortedSet<K>* headSet(K toElement) {
			throw EToDoException(__FILE__, __LINE__);
		}
		ESortedSet<K>* tailSet(K fromElement) {
			throw EToDoException(__FILE__, __LINE__);
		}
		ENavigableSet<K>* descendingSet() {
			throw EToDoException(__FILE__, __LINE__);
		}
	};

private:
	/**
	 * The comparator used to maintain order in this tree map, or
	 * null if it uses the natural ordering of its keys.
	 *
	 * @serial
	 */
	EComparator<K>* comparator_;

	Entry* root;// = null;

	/**
	 * The number of entries in the tree
	 */
	int size_;// = 0;

	/**
	 * The number of structural modifications to the tree.
	 */
	int modCount;// = 0;

	/**
	 * Fields initialized to contain an instance of the entry set view
	 * the first time this view is requested.  Views are stateless, so
	 * there's no reason to create more than one.
	 */
	sp<EntrySet> entrySet_;// = null;
	sp<KeySet> navigableKeySet_;// = null;
	sp<ENavigableMap<K,V> > descendingMap_;// = null;

	/**
	 * Auto free object flag
	 */
	boolean _autoFreeKey;
	boolean _autoFreeValue;

private:
	/**
	 * Balancing operations.
	 *
	 * Implementations of rebalancings during insertion and deletion are
	 * slightly different than the CLR version.  Rather than using dummy
	 * nilnodes, we use a set of accessors that deal properly with null.  They
	 * are used to avoid messiness surrounding nullness checks in the main
	 * algorithms.
	 */

	static boolean colorOf(Entry* p) {
		return (p == null ? BLACK : p->color);
	}

	static Entry* parentOf(Entry* p) {
		return (p == null ? null: p->parent);
	}

	static void setColor(Entry* p, boolean c) {
		if (p != null)
			p->color = c;
	}

	static Entry* leftOf(Entry* p) {
		return (p == null) ? null: p->left;
	}

	static Entry* rightOf(Entry* p) {
		return (p == null) ? null: p->right;
	}

	/** From CLR */
	void rotateLeft(Entry* p) {
		if (p != null) {
			Entry* r = p->right;
			p->right = r->left;
			if (r->left != null)
				r->left->parent = p;
			r->parent = p->parent;
			if (p->parent == null)
				root = r;
			else if (p->parent->left == p)
				p->parent->left = r;
			else
				p->parent->right = r;
			r->left = p;
			p->parent = r;
		}
	}

	/** From CLR */
	void rotateRight(Entry* p) {
		if (p != null) {
			Entry* l = p->left;
			p->left = l->right;
			if (l->right != null) l->right->parent = p;
			l->parent = p->parent;
			if (p->parent == null)
				root = l;
			else if (p->parent->right == p)
				p->parent->right = l;
			else p->parent->left = l;
			l->right = p;
			p->parent = l;
		}
	}

	/** From CLR */
	void fixAfterInsertion(Entry* x) {
		x->color = RED;

		while (x != null && x != root && x->parent->color == RED) {
			if (parentOf(x) == leftOf(parentOf(parentOf(x)))) {
				Entry* y = rightOf(parentOf(parentOf(x)));
				if (colorOf(y) == RED) {
					setColor(parentOf(x), BLACK);
					setColor(y, BLACK);
					setColor(parentOf(parentOf(x)), RED);
					x = parentOf(parentOf(x));
				} else {
					if (x == rightOf(parentOf(x))) {
						x = parentOf(x);
						rotateLeft(x);
					}
					setColor(parentOf(x), BLACK);
					setColor(parentOf(parentOf(x)), RED);
					rotateRight(parentOf(parentOf(x)));
				}
			} else {
				Entry* y = leftOf(parentOf(parentOf(x)));
				if (colorOf(y) == RED) {
					setColor(parentOf(x), BLACK);
					setColor(y, BLACK);
					setColor(parentOf(parentOf(x)), RED);
					x = parentOf(parentOf(x));
				} else {
					if (x == leftOf(parentOf(x))) {
						x = parentOf(x);
						rotateRight(x);
					}
					setColor(parentOf(x), BLACK);
					setColor(parentOf(parentOf(x)), RED);
					rotateLeft(parentOf(parentOf(x)));
				}
			}
		}
		root->color = BLACK;
	}

	/**
	 * Delete node p, and then rebalance the tree.
	 */
	Entry* deleteEntry(Entry* p) {
		modCount++;
		size_--;

		Entry* d = null;

		// If strictly internal, copy successor's element to p and then make p
		// point to successor.
		if (p->left != null && p->right != null) {
			Entry* s = successor(p);
			p->key = s->key;
			p->value = s->value;
			p = s;
			d = s;
		} // p has 2 children
		else {
			d = p;
		}

		// Start fixup at replacement node, if it exists.
		Entry* replacement = (p->left != null ? p->left : p->right);

		if (replacement != null) {
			// Link replacement to parent
			replacement->parent = p->parent;
			if (p->parent == null)
				root = replacement;
			else if (p == p->parent->left)
				p->parent->left  = replacement;
			else
				p->parent->right = replacement;

			// Null out links so they are OK to use by fixAfterDeletion.
			p->left = p->right = p->parent = null;

			// Fix replacement
			if (p->color == BLACK)
				fixAfterDeletion(replacement);
		} else if (p->parent == null) { // return if we are the only node.
			root = null;
		} else { //  No children. Use self as phantom replacement and unlink.
			if (p->color == BLACK)
				fixAfterDeletion(p);

			if (p->parent != null) {
				if (p == p->parent->left)
					p->parent->left = null;
				else if (p == p->parent->right)
					p->parent->right = null;
				p->parent = null;
			}
		}

		return d;
	}

	/** From CLR */
	void fixAfterDeletion(Entry* x) {
		while (x != root && colorOf(x) == BLACK) {
			if (x == leftOf(parentOf(x))) {
				Entry* sib = rightOf(parentOf(x));

				if (colorOf(sib) == RED) {
					setColor(sib, BLACK);
					setColor(parentOf(x), RED);
					rotateLeft(parentOf(x));
					sib = rightOf(parentOf(x));
				}

				if (colorOf(leftOf(sib))  == BLACK &&
					colorOf(rightOf(sib)) == BLACK) {
					setColor(sib, RED);
					x = parentOf(x);
				} else {
					if (colorOf(rightOf(sib)) == BLACK) {
						setColor(leftOf(sib), BLACK);
						setColor(sib, RED);
						rotateRight(sib);
						sib = rightOf(parentOf(x));
					}
					setColor(sib, colorOf(parentOf(x)));
					setColor(parentOf(x), BLACK);
					setColor(rightOf(sib), BLACK);
					rotateLeft(parentOf(x));
					x = root;
				}
			} else { // symmetric
				Entry* sib = leftOf(parentOf(x));

				if (colorOf(sib) == RED) {
					setColor(sib, BLACK);
					setColor(parentOf(x), RED);
					rotateRight(parentOf(x));
					sib = leftOf(parentOf(x));
				}

				if (colorOf(rightOf(sib)) == BLACK &&
					colorOf(leftOf(sib)) == BLACK) {
					setColor(sib, RED);
					x = parentOf(x);
				} else {
					if (colorOf(leftOf(sib)) == BLACK) {
						setColor(rightOf(sib), BLACK);
						setColor(sib, RED);
						rotateLeft(sib);
						sib = leftOf(parentOf(x));
					}
					setColor(sib, colorOf(parentOf(x)));
					setColor(parentOf(x), BLACK);
					setColor(leftOf(sib), BLACK);
					rotateRight(parentOf(x));
					x = root;
				}
			}
		}

		setColor(x, BLACK);
	}

public:
	virtual ~ETreeMap() {
		clear();
	}

	/**
	 * Constructs a new, empty tree map, using the natural ordering of its
	 * keys.  All keys inserted into the map must implement the {@link
	 * Comparable} interface.  Furthermore, all such keys must be
	 * <i>mutually comparable</i>: <tt>k1.compareTo(k2)</tt> must not throw
	 * a <tt>ClassCastException</tt> for any keys <tt>k1</tt> and
	 * <tt>k2</tt> in the map.  If the user attempts to put a key into the
	 * map that violates this constraint (for example, the user attempts to
	 * put a string key into a map whose keys are integers), the
	 * <tt>put(Object key, Object value)</tt> call will throw a
	 * <tt>ClassCastException</tt>.
	 */
	ETreeMap(boolean autoFreeKey = true, boolean autoFreeValue = true) :
			comparator_(null), root(null), size_(0), modCount(0), entrySet_(
					null), navigableKeySet_(null), descendingMap_(null) {
		_autoFreeKey = autoFreeKey;
		_autoFreeValue = autoFreeValue;
	}

	/**
	 * Constructs a new, empty tree map, ordered according to the given
	 * comparator.  All keys inserted into the map must be <i>mutually
	 * comparable</i> by the given comparator: <tt>comparator.compare(k1,
	 * k2)</tt> must not throw a <tt>ClassCastException</tt> for any keys
	 * <tt>k1</tt> and <tt>k2</tt> in the map.  If the user attempts to put
	 * a key into the map that violates this constraint, the <tt>put(Object
	 * key, Object value)</tt> call will throw a
	 * <tt>ClassCastException</tt>.
	 *
	 * @param comparator the comparator that will be used to order this map.
	 *        If <tt>null</tt>, the {@linkplain Comparable natural
	 *        ordering} of the keys will be used.
	 */
	ETreeMap(EComparator<K>* comparator, boolean autoFreeKey = true, boolean autoFreeValue = true) :
			comparator_(comparator), root(null), size_(0), modCount(0), entrySet_(
					null), navigableKeySet_(null), descendingMap_(null) {
		_autoFreeKey = autoFreeKey;
		_autoFreeValue = autoFreeValue;
	}

	//TODO:
	ETreeMap(const ETreeMap<K, V>& that);
	ETreeMap<K, V>& operator= (const ETreeMap<K, V>& that);

	// Query Operations

	/**
	 * Returns the number of key-value mappings in this map.
	 *
	 * @return the number of key-value mappings in this map
	 */
	int size() {
		return size_;
	}

	/**
	 * Returns <tt>true</tt> if this map contains a mapping for the specified
	 * key.
	 *
	 * @param key key whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map contains a mapping for the
	 *         specified key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 */
	boolean containsKey(K key) {
		return getEntry(key) != null;
	}

	/**
	 * Returns <tt>true</tt> if this map maps one or more keys to the
	 * specified value.  More formally, returns <tt>true</tt> if and only if
	 * this map contains at least one mapping to a value <tt>v</tt> such
	 * that <tt>(value==null ? v==null : value.equals(v))</tt>.  This
	 * operation will probably require time linear in the map size for
	 * most implementations.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if a mapping to <tt>value</tt> exists;
	 *         <tt>false</tt> otherwise
	 * @since 1.2
	 */
	boolean containsValue(V value) {
		for (Entry* e = getFirstEntry(); e != null; e = successor(e))
			if (valEquals(value, e->value))
				return true;
		return false;
	}

	/**
	 * Returns the value to which the specified key is mapped,
	 * or {@code null} if this map contains no mapping for the key.
	 *
	 * <p>More formally, if this map contains a mapping from a key
	 * {@code k} to a value {@code v} such that {@code key} compares
	 * equal to {@code k} according to the map's ordering, then this
	 * method returns {@code v}; otherwise it returns {@code null}.
	 * (There can be at most one such mapping.)
	 *
	 * <p>A return value of {@code null} does not <i>necessarily</i>
	 * indicate that the map contains no mapping for the key; it's also
	 * possible that the map explicitly maps the key to {@code null}.
	 * The {@link #containsKey containsKey} operation may be used to
	 * distinguish these two cases.
	 *
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 */
	V get(K key) {
		Entry* p = getEntry(key);
		return (p==null ? null : p->value);
	}

	EComparator<K>* comparator() {
		return comparator_;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	K firstKey() {
		return key(getFirstEntry());
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	K lastKey() {
		return key(getLastEntry());
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old
	 * value is replaced.
	 *
	 * @param key key with which the specified value is to be associated
	 * @param value value to be associated with the specified key
	 *
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 */
	V put(K key, V value, boolean *absent=null) {
		if (absent) {
			*absent = true;
		}
		Entry* t = root;
		if (t == null) {
			// TBD:
			// 5045147: (coll) Adding null to an empty TreeSet should
			// throw NullPointerException
			//
			// compare(key, key); // type check
			root = new Entry(key, value, null, this);
			size_ = 1;
			modCount++;
			return null;
		}
		int cmp;
		Entry* parent;
		// split comparator and comparable paths
		EComparator<K>* cpr = comparator_;
		if (cpr != null) {
			do {
				parent = t;
				cmp = cpr->compare(key, t->key);
				if (cmp < 0)
					t = t->left;
				else if (cmp > 0)
					t = t->right;
				else {
					if (absent) {
						*absent = false;
					}
					if (_autoFreeKey && key != t->key) {
						delete key; //!
					}

					return t->setValue(value);
				}
			} while (t != null);
		}
		else {
			if (key == null)
				throw ENullPointerException(__FILE__, __LINE__);
			EComparable<K>* k = (EComparable<K>*) key;
			do {
				parent = t;
				cmp = k->compareTo(t->key);
				if (cmp < 0)
					t = t->left;
				else if (cmp > 0)
					t = t->right;
				else {
					if (absent) {
						*absent = false;
					}
					if (_autoFreeKey && key != t->key) {
						delete key; //!
					}

					return t->setValue(value);
				}
			} while (t != null);
		}
		Entry* e = new Entry(key, value, parent, this);
		if (cmp < 0)
			parent->left = e;
		else
			parent->right = e;
		fixAfterInsertion(e);
		size_++;
		modCount++;
		return null;
	}

	/**
	 * Removes the mapping for this key from this TreeMap if present.
	 *
	 * @param  key key for which mapping should be removed
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>.
	 *         (A <tt>null</tt> return can also indicate that the map
	 *         previously associated <tt>null</tt> with <tt>key</tt>.)
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 */
	V remove(K key) {
		Entry* p = getEntry(key);
		if (p == null)
			return null;

		if (_autoFreeKey)
			delete p->key; //!
		V oldValue = p->value;
		Entry* d = deleteEntry(p);
		if (d) {
			d->key = null;
			d->value = null;
			d->left = d->right = null;
			delete d; //!
		}
		return oldValue;
	}

	/**
	 * Removes all of the mappings from this map.
	 * The map will be empty after this call returns.
	 */
	void clear() {
		modCount++;
		size_ = 0;
		delete root; //!
		root = null;
	}

	// NavigableMap API methods

	/**
	 * @since 1.6
	 */
	EMapEntry<K,V>* firstEntry() {
		return exportEntry(getFirstEntry());
	}

	/**
	 * @since 1.6
	 */
	EMapEntry<K,V>* lastEntry() {
		return exportEntry(getLastEntry());
	}

	/**
	 * @since 1.6
	 */
	EMapEntry<K,V>* pollFirstEntry() {
		Entry* p = getFirstEntry();
		EMapEntry<K,V>* result = exportEntry(p);
		if (p != null)
			deleteEntry(p);
		return result;
	}

	/**
	 * @since 1.6
	 */
	EMapEntry<K,V>* pollLastEntry() {
		Entry* p = getLastEntry();
		EMapEntry<K,V>* result = exportEntry(p);
		if (p != null)
			deleteEntry(p);
		return result;
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	EMapEntry<K,V>* lowerEntry(K key) {
		return exportEntry(getLowerEntry(key));
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	K lowerKey(K key) {
		return keyOrNull(getLowerEntry(key));
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	EMapEntry<K,V>* floorEntry(K key) {
		return exportEntry(getFloorEntry(key));
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	K floorKey(K key) {
		return keyOrNull(getFloorEntry(key));
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	EMapEntry<K,V>* ceilingEntry(K key) {
		return exportEntry(getCeilingEntry(key));
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	K ceilingKey(K key) {
		return keyOrNull(getCeilingEntry(key));
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	EMapEntry<K,V>* higherEntry(K key) {
		return exportEntry(getHigherEntry(key));
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @since 1.6
	 */
	K higherKey(K key) {
		return keyOrNull(getHigherEntry(key));
	}

	// Views

	sp<ESet<K> > keySet() {
		return navigableKeySet();
	}

	sp<ENavigableSet<K> > navigableKeySet() {
		sp<KeySet> nks = navigableKeySet_;
		return (nks != null) ? nks : (navigableKeySet_ = new KeySet(this));
	}

	sp<ENavigableSet<K> > descendingKeySet() {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection's iterator returns the values in ascending order
	 * of the corresponding keys.
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
		sp<ECollection<V> > vs = EAbstractMap<K,V>::_values;
		return (vs != null) ? vs : (EAbstractMap<K,V>::_values = new Values(this));
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * The set's iterator returns the entries in ascending key order.
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
	 */
	sp<ESet<EMapEntry<K,V>*> > entrySet() {
		sp<EntrySet> es = entrySet_;
		return (es != null) ? es : (entrySet_ = new EntrySet(this));
	}

	/**
	 * @since 1.6
	 */
	ENavigableMap<K, V>* descendingMap() {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException       {@inheritDoc}
	 * @throws NullPointerException if <tt>fromKey</tt> or <tt>toKey</tt> is
	 *         null and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @throws IllegalArgumentException {@inheritDoc}
	 * @since 1.6
	 */
	ENavigableMap<K,V>* subMap(K fromKey, boolean fromInclusive,
									K toKey,   boolean toInclusive) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException       {@inheritDoc}
	 * @throws NullPointerException if <tt>toKey</tt> is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @throws IllegalArgumentException {@inheritDoc}
	 * @since 1.6
	 */
	ENavigableMap<K,V>* headMap(K toKey, boolean inclusive) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException       {@inheritDoc}
	 * @throws NullPointerException if <tt>fromKey</tt> is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @throws IllegalArgumentException {@inheritDoc}
	 * @since 1.6
	 */
	ENavigableMap<K,V>* tailMap(K fromKey, boolean inclusive) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException       {@inheritDoc}
	 * @throws NullPointerException if <tt>fromKey</tt> or <tt>toKey</tt> is
	 *         null and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	ESortedMap<K,V>* subMap(K fromKey, K toKey) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException       {@inheritDoc}
	 * @throws NullPointerException if <tt>toKey</tt> is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	ESortedMap<K,V>* headMap(K toKey) {
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException       {@inheritDoc}
	 * @throws NullPointerException if <tt>fromKey</tt> is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	ESortedMap<K,V>* tailMap(K fromKey) {
		throw EToDoException(__FILE__, __LINE__);
	}

	sp<EIterator<K> > keyIterator() {
		return new KeyIterator(getFirstEntry(), this);
	}

	sp<EIterator<K> > descendingKeyIterator() {
		return new DescendingKeyIterator(getLastEntry(), this);
	}

	// Autofree

	void setAutoFree(boolean autoFreeKey = true, boolean autoFreeValue = true) {
		_autoFreeKey = autoFreeKey;
		_autoFreeValue = autoFreeValue;
	}

	boolean getAutoFreeKey() {
		return _autoFreeKey;
	}

	boolean getAutoFreeValue() {
		return _autoFreeValue;
	}

protected:
	/**
	 * Returns this map's entry for the given key, or <tt>null</tt> if the map
	 * does not contain an entry for the key.
	 *
	 * @return this map's entry for the given key, or <tt>null</tt> if the map
	 *         does not contain an entry for the key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 *         and this map uses natural ordering, or its comparator
	 *         does not permit null keys
	 */
	Entry* getEntry(K key) {
		// Offload comparator-based version for sake of performance
		if (comparator_ != null)
			return getEntryUsingComparator(key);
		if (key == null)
			throw ENullPointerException(__FILE__, __LINE__);
		EComparable<K>* k = (EComparable<K>*) key;
		Entry* p = root;
		while (p != null) {
			int cmp = k->compareTo(p->key);
			if (cmp < 0)
				p = p->left;
			else if (cmp > 0)
				p = p->right;
			else
				return p;
		}
		return null;
	}

	/**
	 * Version of getEntry using comparator. Split off from getEntry
	 * for performance. (This is not worth doing for most methods,
	 * that are less dependent on comparator performance, but is
	 * worthwhile here.)
	 */
	Entry* getEntryUsingComparator(K key) {
		K k = (K) key;
		EComparator<K>* cpr = comparator_;
		if (cpr != null) {
			Entry* p = root;
			while (p != null) {
				int cmp = cpr->compare(k, p->key);
				if (cmp < 0)
					p = p->left;
				else if (cmp > 0)
					p = p->right;
				else
					return p;
			}
		}
		return null;
	}

	/**
	 * Gets the entry corresponding to the specified key; if no such entry
	 * exists, returns the entry for the least key greater than the specified
	 * key; if no such entry exists (i.e., the greatest key in the Tree is less
	 * than the specified key), returns <tt>null</tt>.
	 */
	Entry* getCeilingEntry(K key) {
		Entry* p = root;
		while (p != null) {
			int cmp = compare(key, p->key);
			if (cmp < 0) {
				if (p->left != null)
					p = p->left;
				else
					return p;
			} else if (cmp > 0) {
				if (p->right != null) {
					p = p->right;
				} else {
					Entry* parent = p->parent;
					Entry* ch = p;
					while (parent != null && ch == parent->right) {
						ch = parent;
						parent = parent->parent;
					}
					return parent;
				}
			} else
				return p;
		}
		return null;
	}

	/**
	 * Gets the entry corresponding to the specified key; if no such entry
	 * exists, returns the entry for the greatest key less than the specified
	 * key; if no such entry exists, returns <tt>null</tt>.
	 */
	Entry* getFloorEntry(K key) {
		Entry* p = root;
		while (p != null) {
			int cmp = compare(key, p->key);
			if (cmp > 0) {
				if (p->right != null)
					p = p->right;
				else
					return p;
			} else if (cmp < 0) {
				if (p->left != null) {
					p = p->left;
				} else {
					Entry* parent = p->parent;
					Entry* ch = p;
					while (parent != null && ch == parent->left) {
						ch = parent;
						parent = parent->parent;
					}
					return parent;
				}
			} else
				return p;

		}
		return null;
	}

	/**
	 * Gets the entry for the least key greater than the specified
	 * key; if no such entry exists, returns the entry for the least
	 * key greater than the specified key; if no such entry exists
	 * returns <tt>null</tt>.
	 */
	Entry* getHigherEntry(K key) {
		Entry* p = root;
		while (p != null) {
			int cmp = compare(key, p->key);
			if (cmp < 0) {
				if (p->left != null)
					p = p->left;
				else
					return p;
			} else {
				if (p->right != null) {
					p = p->right;
				} else {
					Entry* parent = p->parent;
					Entry* ch = p;
					while (parent != null && ch == parent->right) {
						ch = parent;
						parent = parent->parent;
					}
					return parent;
				}
			}
		}
		return null;
	}

	/**
	 * Returns the entry for the greatest key less than the specified key; if
	 * no such entry exists (i.e., the least key in the Tree is greater than
	 * the specified key), returns <tt>null</tt>.
	 */
	Entry* getLowerEntry(K key) {
		Entry* p = root;
		while (p != null) {
			int cmp = compare(key, p->key);
			if (cmp > 0) {
				if (p->right != null)
					p = p->right;
				else
					return p;
			} else {
				if (p->left != null) {
					p = p->left;
				} else {
					Entry* parent = p->parent;
					Entry* ch = p;
					while (parent != null && ch == parent->left) {
						ch = parent;
						parent = parent->parent;
					}
					return parent;
				}
			}
		}
		return null;
	}

	/**
	 * Returns the first Entry in the TreeMap (according to the TreeMap's
	 * key-sort function).  Returns null if the TreeMap is empty.
	 */
	Entry* getFirstEntry() {
		Entry* p = root;
		if (p != null)
			while (p->left != null)
				p = p->left;
		return p;
	}

	/**
	 * Returns the last Entry in the TreeMap (according to the TreeMap's
	 * key-sort function).  Returns null if the TreeMap is empty.
	 */
	Entry* getLastEntry() {
		Entry* p = root;
		if (p != null)
			while (p->right != null)
				p = p->right;
		return p;
	}

	/**
	 * Returns the successor of the specified Entry, or null if no such.
	 */
	static Entry* successor(Entry* t) {
		if (t == null)
			return null;
		else if (t->right != null) {
			Entry* p = t->right;
			while (p->left != null)
				p = p->left;
			return p;
		} else {
			Entry* p = t->parent;
			Entry* ch = t;
			while (p != null && ch == p->right) {
				ch = p;
				p = p->parent;
			}
			return p;
		}
	}

	/**
	 * Returns the predecessor of the specified Entry, or null if no such.
	 */
	static Entry* predecessor(Entry* t) {
		if (t == null)
			return null;
		else if (t->left != null) {
			Entry* p = t->left;
			while (p->right != null)
				p = p->right;
			return p;
		} else {
			Entry* p = t->parent;
			Entry* ch = t;
			while (p != null && ch == p->left) {
				ch = p;
				p = p->parent;
			}
			return p;
		}
	}

	// Little utilities

	/**
	 * Compares two keys using the correct comparison method for this TreeMap.
	 */
	int compare(K k1, K k2) {
		return comparator_ == null ?
				(((EComparable<K>*) k1)->compareTo(k2)) :
				comparator_->compare(k1, k2);
	}

	/**
	 * Test two values for equality.  Differs from o1.equals(o2) only in
	 * that it copes with <tt>null</tt> o1 properly.
	 */
	static boolean valEquals(EObject* o1, EObject* o2) {
		return (o1==null ? o2==null : o1->equals(o2));
	}

	/**
	 * Return SimpleImmutableEntry for entry, or null if null
	 */
	static EMapEntry<K,V>* exportEntry(Entry* e) {
		//@see:
		//return e == null? null :
		//	new ESimpleImmutableEntry<K,V>(e);
		return e;
	}

	/**
	 * Return key for entry, or null if null
	 */
	static K keyOrNull(Entry* e) {
		return e == null? null : e->key;
	}

	/**
	 * Returns the key corresponding to the specified Entry.
	 * @throws NoSuchElementException if the Entry is null
	 */
	static K key(Entry* e) {
		if (e==null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return e->key;
	}
};

} /* namespace efc */
#endif /* ETREEMAP_HH_ */
