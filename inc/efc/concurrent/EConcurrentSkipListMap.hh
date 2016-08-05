/*
 * EConcurrentSkipListMap.hh
 *
 *  Created on: 2015-11-12
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTSKIPLISTMAP_HH_
#define ECONCURRENTSKIPLISTMAP_HH_

#include "EInteger.hh"
#include "EThreadLocalRandom.hh"
#include "EConcurrentNavigableMap.hh"
#include "ENullPointerException.hh"
#include "EClassCastException.hh"
#include "EToDoException.hh"

namespace efc {

//@see: openjdk-8/src/share/classes/java/util/concurrent/ConcurrentSkipListMap.java

/**
 * A scalable concurrent {@link ConcurrentNavigableMap} implementation.
 * The map is sorted according to the {@linkplain Comparable natural
 * ordering} of its keys, or by a {@link Comparator} provided at map
 * creation time, depending on which constructor is used.
 *
 * <p>This class implements a concurrent variant of <a
 * href="http://en.wikipedia.org/wiki/Skip_list" target="_top">SkipLists</a>
 * providing expected average <i>log(n)</i> time cost for the
 * {@code containsKey}, {@code get}, {@code put} and
 * {@code remove} operations and their variants.  Insertion, removal,
 * update, and access operations safely execute concurrently by
 * multiple threads.
 *
 * <p>Iterators and spliterators are
 * <a href="package-summary.html#Weakly"><i>weakly consistent</i></a>.
 *
 * <p>Ascending key ordered views and their iterators are faster than
 * descending ones.
 *
 * <p>All {@code Map.Entry} pairs returned by methods in this class
 * and its views represent snapshots of mappings at the time they were
 * produced. They do <em>not</em> support the {@code Entry.setValue}
 * method. (Note however that it is possible to change mappings in the
 * associated map using {@code put}, {@code putIfAbsent}, or
 * {@code replace}, depending on exactly which effect you need.)
 *
 * <p>Beware that, unlike in most collections, the {@code size}
 * method is <em>not</em> a constant-time operation. Because of the
 * asynchronous nature of these maps, determining the current number
 * of elements requires a traversal of the elements, and so may report
 * inaccurate results if this collection is modified during traversal.
 * Additionally, the bulk operations {@code putAll}, {@code equals},
 * {@code toArray}, {@code containsValue}, and {@code clear} are
 * <em>not</em> guaranteed to be performed atomically. For example, an
 * iterator operating concurrently with a {@code putAll} operation
 * might view only some of the added elements.
 *
 * <p>This class and its views and iterators implement all of the
 * <em>optional</em> methods of the {@link Map} and {@link Iterator}
 * interfaces. Like most other concurrent collections, this class does
 * <em>not</em> permit the use of {@code null} keys or values because some
 * null return values cannot be reliably distinguished from the absence of
 * elements.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @param <K> the type of keys maintained by this map
 * @param <V> the type of mapped values
 * @since 1.6
 */

template<typename K, typename V>
class EConcurrentSkipListMap: virtual public EConcurrentNavigableMap<K, V> {
	/*
	 * This class implements a tree-like two-dimensionally linked skip
	 * list in which the index levels are represented in separate
	 * nodes from the base nodes holding data.  There are two reasons
	 * for taking this approach instead of the usual array-based
	 * structure: 1) Array based implementations seem to encounter
	 * more complexity and overhead 2) We can use cheaper algorithms
	 * for the heavily-traversed index lists than can be used for the
	 * base lists.  Here's a picture of some of the basics for a
	 * possible list with 2 levels of index:
	 *
	 * Head nodes          Index nodes
	 * +-+    right        +-+                      +-+
	 * |2|---------------->| |--------------------->| |->null
	 * +-+                 +-+                      +-+
	 *  | down              |                        |
	 *  v                   v                        v
	 * +-+            +-+  +-+       +-+            +-+       +-+
	 * |1|----------->| |->| |------>| |----------->| |------>| |->null
	 * +-+            +-+  +-+       +-+            +-+       +-+
	 *  v              |    |         |              |         |
	 * Nodes  next     v    v         v              v         v
	 * +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+
	 * | |->|A|->|B|->|C|->|D|->|E|->|F|->|G|->|H|->|I|->|J|->|K|->null
	 * +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+  +-+
	 *
	 * The base lists use a variant of the HM linked ordered set
	 * algorithm. See Tim Harris, "A pragmatic implementation of
	 * non-blocking linked lists"
	 * http://www.cl.cam.ac.uk/~tlh20/publications.html and Maged
	 * Michael "High Performance Dynamic Lock-Free Hash Tables and
	 * List-Based Sets"
	 * http://www.research.ibm.com/people/m/michael/pubs.htm.  The
	 * basic idea in these lists is to mark the "next" pointers of
	 * deleted nodes when deleting to avoid conflicts with concurrent
	 * insertions, and when traversing to keep track of triples
	 * (predecessor, node, successor) in order to detect when and how
	 * to unlink these deleted nodes.
	 *
	 * Rather than using mark-bits to mark list deletions (which can
	 * be slow and space-intensive using AtomicMarkedReference), nodes
	 * use direct CAS'able next pointers.  On deletion, instead of
	 * marking a pointer, they splice in another node that can be
	 * thought of as standing for a marked pointer (indicating this by
	 * using otherwise impossible field values).  Using plain nodes
	 * acts roughly like "boxed" implementations of marked pointers,
	 * but uses new nodes only when nodes are deleted, not for every
	 * link.  This requires less space and supports faster
	 * traversal. Even if marked references were better supported by
	 * JVMs, traversal using this technique might still be faster
	 * because any search need only read ahead one more node than
	 * otherwise required (to check for trailing marker) rather than
	 * unmasking mark bits or whatever on each read.
	 *
	 * This approach maintains the essential property needed in the HM
	 * algorithm of changing the next-pointer of a deleted node so
	 * that any other CAS of it will fail, but implements the idea by
	 * changing the pointer to point to a different node, not by
	 * marking it.  While it would be possible to further squeeze
	 * space by defining marker nodes not to have key/value fields, it
	 * isn't worth the extra type-testing overhead.  The deletion
	 * markers are rarely encountered during traversal and are
	 * normally quickly garbage collected. (Note that this technique
	 * would not work well in systems without garbage collection.)
	 *
	 * In addition to using deletion markers, the lists also use
	 * nullness of value fields to indicate deletion, in a style
	 * similar to typical lazy-deletion schemes.  If a node's value is
	 * null, then it is considered logically deleted and ignored even
	 * though it is still reachable. This maintains proper control of
	 * concurrent replace vs delete operations -- an attempted replace
	 * must fail if a delete beat it by nulling field, and a delete
	 * must return the last non-null value held in the field. (Note:
	 * Null, rather than some special marker, is used for value fields
	 * here because it just so happens to mesh with the Map API
	 * requirement that method get returns null if there is no
	 * mapping, which allows nodes to remain concurrently readable
	 * even when deleted. Using any other marker value here would be
	 * messy at best.)
	 *
	 * Here's the sequence of events for a deletion of node n with
	 * predecessor b and successor f, initially:
	 *
	 *        +------+       +------+      +------+
	 *   ...  |   b  |------>|   n  |----->|   f  | ...
	 *        +------+       +------+      +------+
	 *
	 * 1. CAS n's value field from non-null to null.
	 *    From this point on, no public operations encountering
	 *    the node consider this mapping to exist. However, other
	 *    ongoing insertions and deletions might still modify
	 *    n's next pointer.
	 *
	 * 2. CAS n's next pointer to point to a new marker node.
	 *    From this point on, no other nodes can be appended to n.
	 *    which avoids deletion errors in CAS-based linked lists.
	 *
	 *        +------+       +------+      +------+       +------+
	 *   ...  |   b  |------>|   n  |----->|marker|------>|   f  | ...
	 *        +------+       +------+      +------+       +------+
	 *
	 * 3. CAS b's next pointer over both n and its marker.
	 *    From this point on, no new traversals will encounter n,
	 *    and it can eventually be GCed.
	 *        +------+                                    +------+
	 *   ...  |   b  |----------------------------------->|   f  | ...
	 *        +------+                                    +------+
	 *
	 * A failure at step 1 leads to simple retry due to a lost race
	 * with another operation. Steps 2-3 can fail because some other
	 * thread noticed during a traversal a node with null value and
	 * helped out by marking and/or unlinking.  This helping-out
	 * ensures that no thread can become stuck waiting for progress of
	 * the deleting thread.  The use of marker nodes slightly
	 * complicates helping-out code because traversals must track
	 * consistent reads of up to four nodes (b, n, marker, f), not
	 * just (b, n, f), although the next field of a marker is
	 * immutable, and once a next field is CAS'ed to point to a
	 * marker, it never again changes, so this requires less care.
	 *
	 * Skip lists add indexing to this scheme, so that the base-level
	 * traversals start close to the locations being found, inserted
	 * or deleted -- usually base level traversals only traverse a few
	 * nodes. This doesn't change the basic algorithm except for the
	 * need to make sure base traversals start at predecessors (here,
	 * b) that are not (structurally) deleted, otherwise retrying
	 * after processing the deletion.
	 *
	 * Index levels are maintained as lists with volatile next fields,
	 * using CAS to link and unlink.  Races are allowed in index-list
	 * operations that can (rarely) fail to link in a new index node
	 * or delete one. (We can't do this of course for data nodes.)
	 * However, even when this happens, the index lists remain sorted,
	 * so correctly serve as indices.  This can impact performance,
	 * but since skip lists are probabilistic anyway, the net result
	 * is that under contention, the effective "p" value may be lower
	 * than its nominal value. And race windows are kept small enough
	 * that in practice these failures are rare, even under a lot of
	 * contention.
	 *
	 * The fact that retries (for both base and index lists) are
	 * relatively cheap due to indexing allows some minor
	 * simplifications of retry logic. Traversal restarts are
	 * performed after most "helping-out" CASes. This isn't always
	 * strictly necessary, but the implicit backoffs tend to help
	 * reduce other downstream failed CAS's enough to outweigh restart
	 * cost.  This worsens the worst case, but seems to improve even
	 * highly contended cases.
	 *
	 * Unlike most skip-list implementations, index insertion and
	 * deletion here require a separate traversal pass occurring after
	 * the base-level action, to add or remove index nodes.  This adds
	 * to single-threaded overhead, but improves contended
	 * multithreaded performance by narrowing interference windows,
	 * and allows deletion to ensure that all index nodes will be made
	 * unreachable upon return from a public remove operation, thus
	 * avoiding unwanted garbage retention. This is more important
	 * here than in some other data structures because we cannot null
	 * out node fields referencing user keys since they might still be
	 * read by other ongoing traversals.
	 *
	 * Indexing uses skip list parameters that maintain good search
	 * performance while using sparser-than-usual indices: The
	 * hardwired parameters k=1, p=0.5 (see method doPut) mean
	 * that about one-quarter of the nodes have indices. Of those that
	 * do, half have one level, a quarter have two, and so on (see
	 * Pugh's Skip List Cookbook, sec 3.4).  The expected total space
	 * requirement for a map is slightly less than for the current
	 * implementation of java.util.TreeMap.
	 *
	 * Changing the level of the index (i.e, the height of the
	 * tree-like structure) also uses CAS. The head index has initial
	 * level/height of one. Creation of an index with height greater
	 * than the current level adds a level to the head index by
	 * CAS'ing on a new top-most head. To maintain good performance
	 * after a lot of removals, deletion methods heuristically try to
	 * reduce the height if the topmost levels appear to be empty.
	 * This may encounter races in which it possible (but rare) to
	 * reduce and "lose" a level just as it is about to contain an
	 * index (that will then never be encountered). This does no
	 * structural harm, and in practice appears to be a better option
	 * than allowing unrestrained growth of levels.
	 *
	 * The code for all this is more verbose than you'd like. Most
	 * operations entail locating an element (or position to insert an
	 * element). The code to do this can't be nicely factored out
	 * because subsequent uses require a snapshot of predecessor
	 * and/or successor and/or value fields which can't be returned
	 * all at once, at least not without creating yet another object
	 * to hold them -- creating such little objects is an especially
	 * bad idea for basic internal search operations because it adds
	 * to GC overhead.  (This is one of the few times I've wished Java
	 * had macros.) Instead, some traversal code is interleaved within
	 * insertion and removal operations.  The control logic to handle
	 * all the retry conditions is sometimes twisty. Most search is
	 * broken into 2 parts. findPredecessor() searches index nodes
	 * only, returning a base-level predecessor of the key. findNode()
	 * finishes out the base-level search. Even with this factoring,
	 * there is a fair amount of near-duplication of code to handle
	 * variants.
	 *
	 * To produce random values without interference across threads,
     * we use within-JDK thread local random support (via the
     * "secondary seed", to avoid interference with user-level
     * ThreadLocalRandom.)
     *
     * A previous version of this class wrapped non-comparable keys
     * with their comparators to emulate Comparables when using
     * comparators vs Comparables.  However, JVMs now appear to better
     * handle infusing comparator-vs-comparable choice into search
     * loops. Static method cpr(comparator, x, y) is used for all
     * comparisons, which works well as long as the comparator
     * argument is set up outside of loops (thus sometimes passed as
     * an argument to internal methods) to avoid field re-reads.
     *
	 * For explanation of algorithms sharing at least a couple of
	 * features with this one, see Mikhail Fomitchev's thesis
	 * (http://www.cs.yorku.ca/~mikhail/), Keir Fraser's thesis
	 * (http://www.cl.cam.ac.uk/users/kaf24/), and Hakan Sundell's
	 * thesis (http://www.cs.chalmers.se/~phs/).
	 *
	 * Given the use of tree-like index nodes, you might wonder why
	 * this doesn't use some kind of search tree instead, which would
	 * support somewhat faster search operations. The reason is that
	 * there are no known efficient lock-free insertion and deletion
	 * algorithms for search trees. The immutability of the "down"
	 * links of index nodes (as opposed to mutable "left" fields in
	 * true trees) makes this tractable using only CAS operations.
	 *
	 * Notation guide for local variables
	 * Node:         b, n, f    for  predecessor, node, successor
	 * Index:        q, r, d    for index node, right, down.
	 *               t          for another index node
	 * Head:         h
	 * Levels:       j
	 * Keys:         k, key
	 * Values:       v, value
	 * Comparisons:  c
	 */

private:
	/* ---------------- View Classes -------------- */

	/*
	 * View classes are static, delegating to a ConcurrentNavigableMap
	 * to allow use by SubMaps, which outweighs the ugliness of
	 * needing type-tests for Iterator methods.
	 */

	template<typename E>
	static EList<E>* toList(ECollection<E>* c) {
		// Using size() here would be a pessimization.
		EList<E>* list = new EArrayList<E>();
		EIterator<E>* iter = c->iterator();
		while (iter->hasNext()) {
			E e = iter->next();
			list->add(e);
		}
		delete iter;
		return list;
	}

	class KeySet : virtual public EConcurrentSet<K>, virtual public EConcurrentNavigableSet<K> {
	private:
		EConcurrentNavigableMap<K,V>* m;
	public:
		KeySet(EConcurrentNavigableMap<K,V>* map) { m = map; }
		int size() { return m->size(); }
		boolean isEmpty() { return m->isEmpty(); }
		boolean contains(K* o) { return m->containsKey(o); }
		boolean remove(K* o) { return m->remove(o) != null; }
		void clear() { m->clear(); }
		sp<K> lower(K* e) { return m->lowerKey(e); }
		sp<K> floor(K* e) { return m->floorKey(e); }
		sp<K> ceiling(K* e) { return m->ceilingKey(e); }
		sp<K> higher(K* e) { return m->higherKey(e); }
		EComparator<K*>* comparator() { return m->comparator(); }
		sp<K> first() { return m->firstKey(); }
		sp<K> last() { return m->lastKey(); }
		sp<K> pollFirst() {
			sp<EConcurrentMapEntry<K,V> > e = m->pollFirstEntry();
			return (e == null) ? null : e->getKey();
		}
		sp<K> pollLast() {
			sp<EConcurrentMapEntry<K,V> > e = m->pollLastEntry();
			return (e == null) ? null : e->getKey();
		}
		sp<EConcurrentIterator<K> > iterator() {
			/*
			if (m instanceof ConcurrentSkipListMap)
				return ((ConcurrentSkipListMap<E,Object>)m)->keyIterator();
			else
				return ((ConcurrentSkipListMap::SubMap<E,Object>)m)->keyIterator();
			*/
			EConcurrentSkipListMap* mm = dynamic_cast<EConcurrentSkipListMap*>(m);
			if (mm)
				return mm->keyIterator();
			else
				throw EToDoException(__FILE__, __LINE__);
		}
		boolean equals(KeySet* o) {
			/*
			if (o == this)
				return true;
			if (!(o instanceof Set))
				return false;
			Collection<?> c = (Collection<?>) o;
			try {
				return containsAll(c) && c->containsAll(this);
			} catch (ClassCastException unused)   {
				return false;
			} catch (NullPointerException unused) {
				return false;
			}
			*/
			throw EToDoException(__FILE__, __LINE__);
		}
		sp<EConcurrentIterator<K> > descendingIterator() {
			//@see: return descendingSet().iterator();
			EConcurrentNavigableSet<K>* ns = descendingSet();
			sp<EConcurrentIterator<K> > iter = ns->iterator();
			delete ns;
			return iter;
		}
		EConcurrentNavigableSet<K>* subSet(K* fromElement,
									  boolean fromInclusive,
									  K* toElement,
									  boolean toInclusive) {
			return new KeySet(m->subMap(fromElement, fromInclusive,
										  toElement,   toInclusive));
		}
		EConcurrentNavigableSet<K>* headSet(K* toElement, boolean inclusive) {
			return new KeySet(m->headMap(toElement, inclusive));
		}
		EConcurrentNavigableSet<K>* tailSet(K* fromElement, boolean inclusive) {
			return new KeySet(m->tailMap(fromElement, inclusive));
		}
		EConcurrentNavigableSet<K>* subSet(K* fromElement, K* toElement) {
			return subSet(fromElement, true, toElement, false);
		}
		EConcurrentNavigableSet<K>* headSet(K* toElement) {
			return headSet(toElement, false);
		}
		EConcurrentNavigableSet<K>* tailSet(K* fromElement) {
			return tailSet(fromElement, true);
		}
		EConcurrentNavigableSet<K>* descendingSet() {
			return new KeySet(m->descendingMap().get());
		}
		boolean add(K* e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean add(sp<K> e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

	class Values : public EAbstractConcurrentCollection<V> {
	private:
		EConcurrentNavigableMap<K, V>* m;
	public:
		Values(EConcurrentNavigableMap<K, V>* map) {
			m = map;
		}
		sp<EConcurrentIterator<V> > iterator() {
			/*
			if (m instanceof ConcurrentSkipListMap)
				return ((ConcurrentSkipListMap<Object,E>)m).valueIterator();
			else
				return ((SubMap<Object,E>)m).valueIterator();
			*/
			EConcurrentSkipListMap* mm = dynamic_cast<EConcurrentSkipListMap*>(m);
			if (mm)
				return mm->valueIterator();
			else
				throw EToDoException(__FILE__, __LINE__);
		}
		boolean isEmpty() {
			return m->isEmpty();
		}
		int size() {
			return m->size();
		}
		boolean contains(V* o) {
			return m->containsValue(o);
		}
		void clear() {
			m->clear();
		}
	};

	class EntrySet : public EConcurrentSet<EConcurrentMapEntry<K,V> > {
	private:
		EConcurrentNavigableMap<K, V>* m;
	public:
		EntrySet(EConcurrentNavigableMap<K, V>* map) {
			m = map;
		}

		sp<EConcurrentIterator<EConcurrentMapEntry<K,V> > > iterator() {
			/*
			if (m instanceof ConcurrentSkipListMap)
				return ((ConcurrentSkipListMap<K1,V1>)m).entryIterator();
			else
				return ((SubMap<K1,V1>)m).entryIterator();
			*/
			EConcurrentSkipListMap* mm = dynamic_cast<EConcurrentSkipListMap*>(m);
			if (mm)
				return mm->entryIterator();
			else
				throw EToDoException(__FILE__, __LINE__);
		}

		boolean contains(EConcurrentMapEntry<K,V>* e) {
			sp<V> v = m->get(e->getKey().get());
			return v != null && v->equals(e->getValue().get());
		}
		boolean remove(EConcurrentMapEntry<K,V>* e) {
			return m->remove(e->getKey().get(),
							e->getValue().get());
		}
		boolean isEmpty() {
			return m->isEmpty();
		}
		int size() {
			return m->size();
		}
		void clear() {
			m->clear();
		}
		boolean equals(EConcurrentMapEntry<K,V>* o) {
			/*
			if (o == this)
				return true;
			if (!(o instanceof Set))
				return false;
			Collection<?> c = (Collection<?>) o;
			try {
				return containsAll(c) && c.containsAll(this);
			} catch (ClassCastException unused)   {
				return false;
			} catch (NullPointerException unused) {
				return false;
			}
			*/
			throw EToDoException(__FILE__, __LINE__);
		}
		boolean add(EConcurrentMapEntry<K,V>* e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean add(sp<EConcurrentMapEntry<K,V> > e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

public:
	virtual ~EConcurrentSkipListMap() {
		delete head_->node;
		delete head_;
	}

	/* ---------------- Constructors -------------- */

	/**
	 * Constructs a new, empty map, sorted according to the
	 * {@linkplain Comparable natural ordering} of the keys.
	 */
	EConcurrentSkipListMap() {
		this->comparator_ = null;
		initialize();
	}

	/**
	 * Constructs a new map containing the same mappings as the given map,
	 * sorted according to the {@linkplain Comparable natural ordering} of
	 * the keys.
	 *
	 * @param  m the map whose mappings are to be placed in this map
	 * @throws ClassCastException if the keys in <tt>m</tt> are not
	 *         {@link Comparable}, or are not mutually comparable
	 * @throws NullPointerException if the specified map or any of its keys
	 *         or values are null
	 */
	EConcurrentSkipListMap(EMap<K*, V*>* m) {
		this->comparator_ = null;
		initialize();
		putAll(m);
	}

	/**
	 * Constructs a new map containing the same mappings and using the
	 * same ordering as the specified sorted map.
	 *
	 * @param m the sorted map whose mappings are to be placed in this
	 *        map, and whose comparator is to be used to sort this map
	 * @throws NullPointerException if the specified sorted map or any of
	 *         its keys or values are null
	 */
	EConcurrentSkipListMap(ESortedMap<K*, V*>* m) {
		this->comparator_ = m->comparator();
		initialize();
		buildFromSorted(m);
	}

	/**
	 * Returns a shallow copy of this <tt>ConcurrentSkipListMap</tt>
	 * instance. (The keys and values themselves are not cloned.)
	 *
	 * @return a shallow copy of this map
	 */
	EConcurrentSkipListMap<K,V>* clone() {
		/*
		ConcurrentSkipListMap<K,V> clone = null;
		try {
			clone = (ConcurrentSkipListMap<K,V>) super.clone();
		} catch (CloneNotSupportedException e) {
			throw new InternalError();
		}

		clone.initialize();
		clone.buildFromSorted(this);
		return clone;
		*/

		//TODO...

		return null;
	}

	/* ------ Map API methods ------ */

	/**
	 * Returns <tt>true</tt> if this map contains a mapping for the specified
	 * key.
	 *
	 * @param key key whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map contains a mapping for the specified key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 */
	boolean containsKey(K* key) {
		return doGet(key) != null;
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
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 */
	sp<V> get(K* key) {
		return doGet(key);
	}

	/**
	 * Associates the specified value with the specified key in this map.
	 * If the map previously contained a mapping for the key, the old
	 * value is replaced.
	 *
	 * @param key key with which the specified value is to be associated
	 * @param value value to be associated with the specified key
	 * @return the previous value associated with the specified key, or
	 *         <tt>null</tt> if there was no mapping for the key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key or value is null
	 */
	sp<V> put(K* key, V* value) {
		sp<K> k(key);
		sp<V> v(value);
		return put(k, v);
	}
	sp<V> put(sp<K> key, sp<V> value) {
		if (value == null)
			throw ENullPointerException(__FILE__, __LINE__);
		return doPut(key, value, false);
	}

	/**
	 * Removes the mapping for the specified key from this map if present.
	 *
	 * @param  key key for which mapping should be removed
	 * @return the previous value associated with the specified key, or
	 *         <tt>null</tt> if there was no mapping for the key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 */
	sp<V> remove(K* key) {
		return doRemove(key, null);
	}

	/**
	 * Returns <tt>true</tt> if this map maps one or more keys to the
	 * specified value.  This operation requires time linear in the
	 * map size. Additionally, it is possible for the map to change
	 * during execution of this method, in which case the returned
	 * result may be inaccurate.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if a mapping to <tt>value</tt> exists;
	 *         <tt>false</tt> otherwise
	 * @throws NullPointerException if the specified value is null
	 */
	boolean containsValue(V* value) {
		if (value == null)
			throw ENullPointerException(__FILE__, __LINE__);
		for (Node* n = findFirst(); n != null; n = n->next) {
			sp<V> v = n->getValidValue();
			if (v != null && v->equals(value))
				return true;
		}
		return false;
	}

	/**
	 * Returns the number of key-value mappings in this map.  If this map
	 * contains more than <tt>Integer.MAX_VALUE</tt> elements, it
	 * returns <tt>Integer.MAX_VALUE</tt>.
	 *
	 * <p>Beware that, unlike in most collections, this method is
	 * <em>NOT</em> a constant-time operation. Because of the
	 * asynchronous nature of these maps, determining the current
	 * number of elements requires traversing them all to count them.
	 * Additionally, it is possible for the size to change during
	 * execution of this method, in which case the returned result
	 * will be inaccurate. Thus, this method is typically not very
	 * useful in concurrent applications.
	 *
	 * @return the number of elements in this map
	 */
	int size() {
		long count = 0;
		for (Node* n = findFirst(); n != null; n = n->next) {
			if (n->getValidValue() != null)
				++count;
		}
		return (count >= EInteger::MAX_VALUE) ? EInteger::MAX_VALUE : (int) count;
	}

	/**
	 * Returns <tt>true</tt> if this map contains no key-value mappings.
	 * @return <tt>true</tt> if this map contains no key-value mappings
	 */
	boolean isEmpty() {
		return findFirst() == null;
	}

	/**
	 * Removes all of the mappings from this map.
	 */
	void clear() {
		delete head_->node;
		delete head_;
		initialize();
	}

	/* ---------------- View methods -------------- */

	/*
	 * Note: Lazy initialization works for views because view classes
	 * are stateless/immutable so it doesn't matter wrt correctness if
	 * more than one is created (which will only rarely happen).  Even
	 * so, the following idiom conservatively ensures that the method
	 * returns the one it created if it does so, not one created by
	 * another racing thread.
	 */

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
	 * <p>This method is equivalent to method {@code navigableKeySet}.
	 *
	 * @return a navigable set view of the keys in this map
	 */
	sp<EConcurrentSet<K> > keySet() {
		if (!keySet_) {
			keySet_ = new KeySet(this);
		}
		return dynamic_pointer_cast<EConcurrentSet<K> >(keySet_);
	}

	sp<EConcurrentNavigableSet<K> > navigableKeySet() {
		if (!keySet_) {
			keySet_ = new KeySet(this);
		}
		return dynamic_pointer_cast<EConcurrentNavigableSet<K> >(keySet_);
	}

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection's iterator returns the values in ascending order
	 * of the corresponding keys.
	 * The collection is backed by the map, so changes to the map are
	 * reflected in the collection, and vice-versa.  The collection
	 * supports element removal, which removes the corresponding
	 * mapping from the map, via the <tt>Iterator.remove</tt>,
	 * <tt>Collection.remove</tt>, <tt>removeAll</tt>,
	 * <tt>retainAll</tt> and <tt>clear</tt> operations.  It does not
	 * support the <tt>add</tt> or <tt>addAll</tt> operations.
	 *
	 * <p>The view's <tt>iterator</tt> is a "weakly consistent" iterator
	 * that will never throw {@link ConcurrentModificationException},
	 * and guarantees to traverse elements as they existed upon
	 * construction of the iterator, and may (but is not guaranteed to)
	 * reflect any modifications subsequent to construction.
	 */
	sp<EConcurrentCollection<V> > values() {
		if (!values_) {
			values_ = new Values(this);
		}
		return dynamic_pointer_cast<EConcurrentCollection<V> >(values_);
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * The set's iterator returns the entries in ascending key order.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  The set supports element
	 * removal, which removes the corresponding mapping from the map,
	 * via the <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt> and <tt>clear</tt>
	 * operations.  It does not support the <tt>add</tt> or
	 * <tt>addAll</tt> operations.
	 *
	 * <p>The view's <tt>iterator</tt> is a "weakly consistent" iterator
	 * that will never throw {@link ConcurrentModificationException},
	 * and guarantees to traverse elements as they existed upon
	 * construction of the iterator, and may (but is not guaranteed to)
	 * reflect any modifications subsequent to construction.
	 *
	 * <p>The <tt>Map.Entry</tt> elements returned by
	 * <tt>iterator.next()</tt> do <em>not</em> support the
	 * <tt>setValue</tt> operation.
	 *
	 * @return a set view of the mappings contained in this map,
	 *         sorted in ascending key order
	 */
	sp<EConcurrentSet<EConcurrentMapEntry<K,V> > > entrySet() {
		if (!entrySet_) {
			entrySet_ = new EntrySet(this);
		}
		return dynamic_pointer_cast<EConcurrentSet<EConcurrentMapEntry<K,V> > >(entrySet_);
	}

	sp<EConcurrentNavigableMap<K,V> > descendingMap() {
		/*
		if (!descendingMap_) {
			descendingMap_ = new SubMap<K,V>(this, null, false, null, false, true);
		}
		return descendingMap_;
		*/
		throw EToDoException(__FILE__, __LINE__);
	}

	sp<EConcurrentNavigableSet<K> > descendingKeySet() {
		sp<EConcurrentNavigableMap<K,V> > dm = descendingMap();
		return dm->navigableKeySet();
	}

	/* ---------------- AbstractMap Overrides -------------- */

	/**
	 * Compares the specified object with this map for equality.
	 * Returns <tt>true</tt> if the given object is also a map and the
	 * two maps represent the same mappings.  More formally, two maps
	 * <tt>m1</tt> and <tt>m2</tt> represent the same mappings if
	 * <tt>m1.entrySet().equals(m2.entrySet())</tt>.  This
	 * operation may return misleading results if either map is
	 * concurrently modified during execution of this method.
	 *
	 * @param o object to be compared for equality with this map
	 * @return <tt>true</tt> if the specified object is equal to this map
	 */
	boolean equals(EConcurrentSkipListMap<K,V>* o) {
		/*
		if (o == this)
			return true;
		if (!(o instanceof Map))
			return false;
		Map<?,?> m = (Map<?,?>) o;
		try {
			for (Map.Entry<K,V> e : this.entrySet())
				if (! e.getValue().equals(m.get(e.getKey())))
					return false;
			for (Map.Entry<?,?> e : m.entrySet()) {
				Object k = e.getKey();
				Object v = e.getValue();
				if (k == null || v == null || !v.equals(get(k)))
					return false;
			}
			return true;
		} catch (ClassCastException unused) {
			return false;
		} catch (NullPointerException unused) {
			return false;
		}
		*/
		throw EToDoException(__FILE__, __LINE__);
	}

	/* ------ ConcurrentMap API methods ------ */

	/**
	 * {@inheritDoc}
	 *
	 * @return the previous value associated with the specified key,
	 *         or <tt>null</tt> if there was no mapping for the key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key or value is null
	 */
	sp<V> putIfAbsent(K* key, V* value) {
		sp<K> k(key);
		sp<V> v(value);
		return putIfAbsent(k, v);
	}
	sp<V> putIfAbsent(sp<K> key, sp<V> value) {
		if (value == null)
			throw ENullPointerException(__FILE__, __LINE__);
		return doPut(key, value, true);
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key is null
	 */
	boolean remove(K* key, V* value) {
		if (key == null)
			throw ENullPointerException(__FILE__, __LINE__);
		if (value == null)
			return false;
		return doRemove(key, value) != null;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if any of the arguments are null
	 */
	boolean replace(K* key, V* oldValue, V* newValue) {
		sp<V> nv(newValue);
		return replace(key, oldValue, nv);
	}
	boolean replace(K* key, V* oldValue, sp<V> newValue) {
		if (key == null || oldValue == null || newValue == null)
			throw ENullPointerException(__FILE__, __LINE__);
		for (;;) {
			Node* n = findNode(key);
			if (n == null)
				return false;
			if (n->status != DELETE_NODE) {
				sp<V> v = atomic_load(&n->value);
				if (!v->equals(oldValue))
					return false;
				if (n->casValue(v, newValue))
					return true;
			}
		}
		return false;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @return the previous value associated with the specified key,
	 *         or <tt>null</tt> if there was no mapping for the key
	 * @throws ClassCastException if the specified key cannot be compared
	 *         with the keys currently in the map
	 * @throws NullPointerException if the specified key or value is null
	 */
	sp<V> replace(K* key, V* value) {
		sp<V> v(value);
		return replace(key, v);
	}
	sp<V> replace(K* key, sp<V> value) {
		if (key == null || value == null)
			throw ENullPointerException(__FILE__, __LINE__);
		for (;;) {
			Node* n = findNode(key);
			if (n == null)
				return null;
			int s = n->status;
			sp<V> v = atomic_load(&n->value);
			if (s != DELETE_NODE && n->casValue(v, value))
				return v;
		}
		//not reach here.
		return null;
	}

	/* ------ SortedMap API methods ------ */

	EComparator<K*>* comparator() {
		return comparator_;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	sp<K> firstKey() {
		Node* n = findFirst();
		if (n == null)
			throw ENullPointerException(__FILE__, __LINE__);
		return n->key;
	}

	/**
	 * @throws NoSuchElementException {@inheritDoc}
	 */
	sp<K> lastKey() {
		Node* n = findLast();
		if (n == null)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return n->key;
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromKey} or {@code toKey} is null
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	EConcurrentNavigableMap<K,V>* subMap(K* fromKey,
											  boolean fromInclusive,
											  K* toKey,
											  boolean toInclusive) {
		if (fromKey == null || toKey == null)
			throw ENullPointerException(__FILE__, __LINE__);
		/*
		return new SubMap<K,V>
			(this, fromKey, fromInclusive, toKey, toInclusive, false);
		*/
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code toKey} is null
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	EConcurrentNavigableMap<K,V>* headMap(K* toKey,
											   boolean inclusive) {
		if (toKey == null)
			throw ENullPointerException(__FILE__, __LINE__);
		/*
		return new SubMap<K,V>
			(this, null, false, toKey, inclusive, false);
		*/
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromKey} is null
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	EConcurrentNavigableMap<K,V>* tailMap(K* fromKey,
											   boolean inclusive) {
		if (fromKey == null)
			throw ENullPointerException(__FILE__, __LINE__);
		/*
		return new SubMap<K,V>
			(this, fromKey, inclusive, null, false, false);
		*/
		throw EToDoException(__FILE__, __LINE__);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromKey} or {@code toKey} is null
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	EConcurrentNavigableMap<K,V>* subMap(K* fromKey, K* toKey) {
		return subMap(fromKey, true, toKey, false);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code toKey} is null
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	EConcurrentNavigableMap<K,V>* headMap(K* toKey) {
		return headMap(toKey, false);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if {@code fromKey} is null
	 * @throws IllegalArgumentException {@inheritDoc}
	 */
	EConcurrentNavigableMap<K,V>* tailMap(K* fromKey) {
		return tailMap(fromKey, true);
	}

	/* ---------------- Relational operations -------------- */

	/**
	 * Returns a key-value mapping associated with the greatest key
	 * strictly less than the given key, or <tt>null</tt> if there is
	 * no such key. The returned entry does <em>not</em> support the
	 * <tt>Entry.setValue</tt> method.
	 *
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<EConcurrentMapEntry<K,V> > lowerEntry(K* key) {
		return getNear(key, LT);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<K> lowerKey(K* key) {
		Node* n = findNear(key, LT, comparator_);
		return (n == null) ? null : n->key;
	}

	/**
	 * Returns a key-value mapping associated with the greatest key
	 * less than or equal to the given key, or <tt>null</tt> if there
	 * is no such key. The returned entry does <em>not</em> support
	 * the <tt>Entry.setValue</tt> method.
	 *
	 * @param key the key
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<EConcurrentMapEntry<K,V> > floorEntry(K* key) {
		return getNear(key, LT|EQ);
	}

	/**
	 * @param key the key
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<K> floorKey(K* key) {
		Node* n = findNear(key, LT|EQ, comparator_);
		return (n == null) ? null : n->key;
	}

	/**
	 * Returns a key-value mapping associated with the least key
	 * greater than or equal to the given key, or <tt>null</tt> if
	 * there is no such entry. The returned entry does <em>not</em>
	 * support the <tt>Entry.setValue</tt> method.
	 *
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<EConcurrentMapEntry<K,V> > ceilingEntry(K* key) {
		return getNear(key, GT|EQ);
	}

	/**
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<K> ceilingKey(K* key) {
		Node* n = findNear(key, GT|EQ, comparator_);
		return (n == null) ? null : n->key;
	}

	/**
	 * Returns a key-value mapping associated with the least key
	 * strictly greater than the given key, or <tt>null</tt> if there
	 * is no such key. The returned entry does <em>not</em> support
	 * the <tt>Entry.setValue</tt> method.
	 *
	 * @param key the key
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<EConcurrentMapEntry<K,V> > higherEntry(K* key) {
		return getNear(key, GT);
	}

	/**
	 * @param key the key
	 * @throws ClassCastException {@inheritDoc}
	 * @throws NullPointerException if the specified key is null
	 */
	sp<K> higherKey(K* key) {
		Node* n = findNear(key, GT, comparator_);
		return (n == null) ? null : n->key;
	}

	/**
	 * Returns a key-value mapping associated with the least
	 * key in this map, or <tt>null</tt> if the map is empty.
	 * The returned entry does <em>not</em> support
	 * the <tt>Entry.setValue</tt> method.
	 */
	sp<EConcurrentMapEntry<K,V> > firstEntry() {
		for (;;) {
			Node* n = findFirst();
			if (n == null)
				return null;
			EConcurrentMapEntry<K,V>* e = n->createSnapshot();
			if (e != null)
				return e;
		}
		//not reach here.
		return null;
	}

	/**
	 * Returns a key-value mapping associated with the greatest
	 * key in this map, or <tt>null</tt> if the map is empty.
	 * The returned entry does <em>not</em> support
	 * the <tt>Entry.setValue</tt> method.
	 */
	sp<EConcurrentMapEntry<K,V> > lastEntry() {
		for (;;) {
			Node* n = findLast();
			if (n == null)
				return null;
			EConcurrentMapEntry<K,V>* e = n->createSnapshot();
			if (e != null)
				return e;
		}
		//not reach here.
		return null;
	}

	/**
	 * Removes and returns a key-value mapping associated with
	 * the least key in this map, or <tt>null</tt> if the map is empty.
	 * The returned entry does <em>not</em> support
	 * the <tt>Entry.setValue</tt> method.
	 */
	sp<EConcurrentMapEntry<K,V> > pollFirstEntry() {
		return doRemoveFirstEntry();
	}

	/**
	 * Removes and returns a key-value mapping associated with
	 * the greatest key in this map, or <tt>null</tt> if the map is empty.
	 * The returned entry does <em>not</em> support
	 * the <tt>Entry.setValue</tt> method.
	 */
	sp<EConcurrentMapEntry<K,V> > pollLastEntry() {
		return doRemoveLastEntry();
	}


protected:
	/**
	 * Initializes or resets state. Needed by constructors, clone,
	 * clear, readObject. and ConcurrentSkipListSet.clone.
	 * (Note that comparator must be separately initialized.)
	 */
	void initialize() {
		head_ = new HeadIndex(new Node(), null, null, 1);
	}

private:
	/**
	 * {@inheritDoc}
	 *
	 * <p>This implementation iterates over the specified map's
	 * <tt>entrySet()</tt> collection, and calls this map's <tt>put</tt>
	 * operation once for each entry returned by the iteration.
	 *
	 * <p>Note that this implementation throws an
	 * <tt>UnsupportedOperationException</tt> if this map does not support
	 * the <tt>put</tt> operation and the specified map is nonempty.
	 *
	 * @throws UnsupportedOperationException {@inheritDoc}
	 * @throws ClassCastException            {@inheritDoc}
	 * @throws NullPointerException          {@inheritDoc}
	 * @throws IllegalArgumentException      {@inheritDoc}
	 */
	void putAll(EMap<K*, V*>* m) {
		EIterator<EMapEntry<K*,V*>*>* iter = m->entrySet()->iterator();
		while (iter->hasNext()) {
			EMapEntry<K*,V*>* e = iter->next();
			put(e->getKey(), e->getValue());
		}
		delete iter;
	}

	/**
	 * Streamlined bulk insertion to initialize from elements of
	 * given sorted map.  Call only from constructor or clone
	 * method.
	 */
	void buildFromSorted(ESortedMap<K*, V*>* map) {
		if (map == null)
			throw ENullPointerException(__FILE__, __LINE__);

		HeadIndex* h = head_;
		Node* basepred = h->node;

		// Track the current rightmost node at each level. Uses an
		// ArrayList to avoid committing to initial or maximum level.
		EArrayList<Index*> preds(false);

		// initialize
		for (int i = 0; i <= h->level; ++i)
			preds.add(null);
		Index* q = h;
		for (int i = h->level; i > 0; --i) {
			preds.setAt(i, q);
			q = q->down;
		}

		EIterator<EMapEntry<K*, V*>*>* it = map->entrySet()->iterator();
		while (it->hasNext()) {
			EMapEntry<K*, V*>* e = it->next();
			unsigned int rnd = (unsigned int)EThreadLocalRandom::current()->nextInt();
			int j = 0;
			if ((rnd & 0x80000001) == 0) {
				do {
					++j;
				} while (((rnd >>= 1) & 1) != 0);
				if (j > h->level) j = h->level + 1;
			}
			K* k = e->getKey();
			V* v = e->getValue();
			if (k == null || v == null) {
				delete it;
				throw ENullPointerException(__FILE__, __LINE__);
			}
			Node* z = new Node(k, v, null);
			basepred->next = z;
			basepred = z;
			if (j > 0) {
				Index* idx = null;
				for (int i = 1; i <= j; ++i) {
					idx = new Index(z, idx, null);
					if (i > h->level)
						h = new HeadIndex(h->node, h, idx, i);

					if (i < preds.size()) {
						preds.getAt(i)->right = idx;
						preds.setAt(i, idx);
					} else
						preds.add(idx);
				}
			}
		}
		delete it;
		head_ = h;
	}

private:
	/* ---------------- Nodes -------------- */

	/**
	 * Nodes hold keys and values, and are singly linked in sorted
	 * order, possibly with some intervening marker nodes. The list is
	 * headed by a dummy node accessible as head.node. The value field
	 * is declared only as Object because it takes special non-V
	 * values for marker and header nodes.
	 */
	class Node {
	public:
		sp<K> key;
		sp<V> value; //volatile?
		Node* volatile next;
		volatile int status ES_ALIGN;

	public:

		~Node() {
			Node* n = next;
			Node* tmp;
			while (n) {
				tmp = n;
				n = n->next;
				tmp->next = null;
				delete tmp;
			}
		}

		/**
		 * Creates a new regular node.
		 */
        Node(K* key, V* value, Node* next) {
			this->key = key;
			this->value = value;
			this->next = next;
			this->status = NORMAL_NODE;
		}
		Node(sp<K> key, sp<V> value, Node* next) {
			this->key = key;
			this->value = value;
			this->next = next;
			this->status = NORMAL_NODE;
		}

		/**
		 * Creates a new header node.
		 */
		Node() : next(null) {
			this->status = BASE_HEADER;
		}

		/**
		 * Creates a new marker node. A marker is distinguished by
		 * having its value field point to itself.  Marker nodes also
		 * have null keys, a fact that is exploited in a few places,
		 * but this doesn't distinguish markers from the base-level
		 * header node (head.node), which also has a null key.
		 */
		Node(Node* next) {
			this->next = next;
			this->status = MARKER_NODE;
		}

		/**
		 * compareAndSet value field
		 */
		inline boolean casValue(sp<V> cmp, sp<V> val) {
			return atomic_compare_exchange(&value, &cmp, val);
		}

		/**
		 * compareAndSet next field
		 */
		inline boolean casNext(Node* cmp, Node* val) {
			return EUnsafe::compareAndSwapObject(&next, cmp, val);
		}

		/**
		 * compareAndSet status field
		 */
		inline boolean casStatus(int cmp, int val) {
			return EUnsafe::compareAndSwapInt(&status, cmp, val);
		}

		/**
		 * Returns true if this node is a marker. This method isn't
		 * actually called in any current code checking for markers
		 * because callers will have already read value field and need
		 * to use that read (not another done here) and so directly
		 * test if value points to node.
		 * @return true if this node is a marker node
		 */
		inline boolean isMarker() {
			return status == MARKER_NODE;
		}

		/**
		 * Returns true if this node is the header of base-level list.
		 * @return true if this node is header node
		 */
		inline boolean isBaseHeader() {
			return status == BASE_HEADER;
		}

		/**
		 * Tries to append a deletion marker to this node.
		 * @param f the assumed current successor of this node
		 * @return true if successful
		 */
		inline boolean appendMarker(Node* f) {
			Node* n = new Node(f);
			boolean r = casNext(f, n);
			if (!r) {
				nodeSafeDelete(n);
			}
			return r;
		}

		/**
		 * Helps out a deletion by appending marker or unlinking from
		 * predecessor. This is called during traversals when value
		 * field seen to be null.
		 * @param b predecessor
		 * @param f successor
		 */
		void helpDelete(Node* b, Node* f) {
            /*
			 * Rechecking links and then doing only one of the
			 * help-out stages per call tends to minimize CAS
			 * interference among helping threads.
			 */
			if (f == next && this == b->next) {
				if (f == null || f->status != MARKER_NODE) { // not already marked
					Node* n = new Node(f);
					if (!casNext(f, n)) {
						nodeSafeDelete(n);
					}
				}
				else {
					if (b->casNext(this, f->next)) {
						nodeSafeDelete(this); //!
						nodeSafeDelete(f); //!
					}
				}
             }
		}

		/**
		 * Returns value if this node contains a valid key-value pair,
		 * else null.
		 * @return this node's value if it isn't a marker or header or
		 * is deleted, else null.
		 */
		sp<V> getValidValue() {
			int s = status;
			if (s == DELETE_NODE || s == MARKER_NODE || s == BASE_HEADER)
				return null;
			return atomic_load(&value);
		}

		/**
		 * Creates and returns a new SimpleImmutableEntry holding current
		 * mapping if this node holds a valid value, else null.
		 * @return new entry or null
		 */
		EConcurrentMapEntry<K,V>* createSnapshot() {
			sp<V> v = getValidValue();
			if (v == null)
				return null;
			return new EConcurrentImmutableEntry<K,V>(key, v);
		}
	};

	static inline void nodeSafeDelete(Node* node) {
		if (node) {
			node->next = null;
			delete node;
		}
	}

	/* ---------------- Indexing -------------- */

	/**
	 * Index nodes represent the levels of the skip list.  Note that
	 * even though both Nodes and Indexes have forward-pointing
	 * fields, they have different types and are handled in different
	 * ways, that can't nicely be captured by placing field in a
	 * shared abstract class.
	 */
	class Index : public EObject {
	public:
		Node* node;
		Index* down;
		Index* volatile right;

	public:
		virtual ~Index() {
			Index* n = right;
			Index* tmp;
			while (n) {
				tmp = n;
				n = n->right;
				tmp->right = null;
				delete tmp;
			}
		}

		/**
		 * Creates index node with given values.
		 */
		Index(Node* node, Index* down, Index* right) {
			this->node = node;
			this->down = down;
			this->right = right;
		}

		/**
		 * compareAndSet right field
		 */
		inline boolean casRight(Index* cmp, Index* val) {
			return EUnsafe::compareAndSwapObject(&right, cmp, val);
		}

		/**
		 * Returns true if the node this indexes has been deleted.
		 * @return true if indexed node is known to be deleted
		 */
		inline boolean indexesDeletedNode() {
			return node->status == DELETE_NODE;
		}

		/**
		 * Tries to CAS newSucc as successor.  To minimize races with
		 * unlink that may lose this index node, if the node being
		 * indexed is known to be deleted, it doesn't try to link in.
		 * @param succ the expected current successor
		 * @param newSucc the new successor
		 * @return true if successful
		 */
		boolean link(Index* succ, Index* newSucc) {
			Node* n = node;
			newSucc->right = succ;
			return n->status != DELETE_NODE && casRight(succ, newSucc);
		}

		/**
		 * Tries to CAS right field to skip over apparent successor
		 * succ.  Fails (forcing a retraversal by caller) if this node
		 * is known to be deleted.
		 * @param succ the expected current successor
		 * @return true if successful
		 */
		boolean unlink(Index* succ) {
			//@see:	return !indexesDeletedNode() && casRight(succ, succ->right);
			if (!indexesDeletedNode()) {
				if (casRight(succ, succ->right)) {
					//FIXME: memory leak!
//					succ->down = null;
//					succ->right = null;
//					delete succ; //! if delete here then other threads memory crash!
					return true;
				}
			}
			return false;
		}
	};

	/* ---------------- Head nodes -------------- */

	/**
	 * Nodes heading each level keep track of their level.
	 */
	class HeadIndex : public Index {
	public:
		int level;
		virtual ~HeadIndex() {
			delete Index::down;
		}
		HeadIndex(Node* node, Index* down, Index* right, int level) :
				Index(node, down, right) {
			this->level = level;
		}
	};
    
    /* ---------------- Comparison utilities -------------- */
    
	/**
	 * Compares using comparator or natural ordering if null.
	 * Called only by methods that have performed required type checks.
	 */
	static int cpr(EComparator<K*>* c, K* x, K* y) {
        if (c != null) {
            return c->compare(x, y);
        }
        else {
            EComparable<K*>* cc = dynamic_cast<EComparable<K*>*>(x);
            if (!cc) {
                throw EClassCastException(__FILE__, __LINE__);
            }
            return cc->compareTo(y);
        }
	}

    /* ---------------- Traversal -------------- */

	/**
	 * Returns a base-level node with key strictly less than given key,
	 * or the base-level header if there is no such node.  Also
	 * unlinks indexes to deleted nodes found along the way.  Callers
	 * rely on this side-effect of clearing indices to deleted nodes.
	 * @param key the key
	 * @return a predecessor of key
	 */
	Node* findPredecessor(K* key, EComparator<K*>* cmp) {
		if (key == null)
			throw ENullPointerException(__FILE__, __LINE__); // don't postpone errors
		for (;;) {
			for (Index* q = head_, *r = q->right, *d;;) {
				if (r != null) {
					Node* n = r->node;
					if (n->status == DELETE_NODE) {
						if (!q->unlink(r))
							break;           // restart
						r = q->right;         // reread r
						continue;
					}
					if (cpr(cmp, key, n->key.get()) > 0) {
						q = r;
						r = r->right;
						continue;
					}
				}
				if ((d = q->down) == null)
					return q->node;
				q = d;
				r = d->right;
			}
		}
		//not reach here.
		return null;
	}

	/**
	 * Returns node holding key or null if no such, clearing out any
	 * deleted nodes seen along the way.  Repeatedly traverses at
	 * base-level looking for key starting at predecessor returned
	 * from findPredecessor, processing base-level deletions as
	 * encountered. Some callers rely on this side-effect of clearing
	 * deleted nodes.
	 *
	 * Restarts occur, at traversal step centered on node n, if:
	 *
	 *   (1) After reading n's next field, n is no longer assumed
	 *       predecessor b's current successor, which means that
	 *       we don't have a consistent 3-node snapshot and so cannot
	 *       unlink any subsequent deleted nodes encountered.
	 *
	 *   (2) n's value field is null, indicating n is deleted, in
	 *       which case we help out an ongoing structural deletion
	 *       before retrying.  Even though there are cases where such
	 *       unlinking doesn't require restart, they aren't sorted out
	 *       here because doing so would not usually outweigh cost of
	 *       restarting.
	 *
	 *   (3) n is a marker or n's predecessor's value field is null,
	 *       indicating (among other possibilities) that
	 *       findPredecessor returned a deleted node. We can't unlink
	 *       the node because we don't know its predecessor, so rely
	 *       on another call to findPredecessor to notice and return
	 *       some earlier predecessor, which it will do. This check is
	 *       only strictly needed at beginning of loop, (and the
	 *       b.value check isn't strictly needed at all) but is done
	 *       each iteration to help avoid contention with other
	 *       threads by callers that will fail to be able to change
	 *       links, and so will retry anyway.
	 *
	 * The traversal loops in doPut, doRemove, and findNear all
	 * include the same three kinds of checks. And specialized
	 * versions appear in findFirst, and findLast and their
	 * variants. They can't easily share code because each uses the
	 * reads of fields held in locals occurring in the orders they
	 * were performed.
	 *
	 * @param key the key
	 * @return node holding key, or null if no such
	 */
	Node* findNode(K* key) {
    	if (key == null)
			throw ENullPointerException(__FILE__, __LINE__); // don't postpone errors
    	EComparator<K*>* cmp = comparator_;
		for (;;) {
			Node* b = findPredecessor(key, cmp);
			Node* n = b->next;
			for (;;) {
				int c, s;
				if (n == null)
					goto OUTER;
				Node* f = n->next;
				if (n != b->next)                // inconsistent read
					break;
				if ((s = n->status) == DELETE_NODE) {    // n is deleted
					n->helpDelete(b, f);
					break;
				}
				if (b->status == DELETE_NODE || s == MARKER_NODE)  // b is deleted
					break;
				if ((c = cpr(cmp, key, n->key.get())) == 0)
					return n;
				if (c < 0)
					goto OUTER;
				b = n;
				n = f;
			}
		}
OUTER:
		return null;
	}

	/**
	 * Gets value for key using findNode.
	 * @param okey the key
	 * @return the value, or null if absent
	 */
	sp<V> doGet(K* key) {
		if (key == null)
			throw ENullPointerException(__FILE__, __LINE__); // don't postpone errors
		EComparator<K*>* cmp = comparator_;
		for (;;) {
			for (Node* b = findPredecessor(key, cmp), *n = b->next;;) {
				int c, s;
				if (n == null)
					goto OUTER;
				Node* f = n->next;
				if (n != b->next)                // inconsistent read
					break;
				if ((s = n->status) == DELETE_NODE) {    // n is deleted
					n->helpDelete(b, f);
					break;
				}
				if (b->status == DELETE_NODE || s == MARKER_NODE)  // b is deleted
					break;
				if ((c = cpr(cmp, key, n->key.get())) == 0) {
					return atomic_load(&n->value);
				}
				if (c < 0)
					goto OUTER;
				b = n;
				n = f;
			}
		}
OUTER:
		return null;
	}

	/* ---------------- Insertion -------------- */

	/**
	 * Main insertion method.  Adds element if not present, or
	 * replaces value if present and onlyIfAbsent is false.
	 * @param key the key
	 * @param value the value that must be associated with key
	 * @param onlyIfAbsent if should not insert if already present
	 * @return the old value, or null if newly inserted
	 */
	sp<V> doPut(sp<K>& key, sp<V>& value, boolean onlyIfAbsent) {
		Node* z;             // added node
		if (key == null)
			throw ENullPointerException(__FILE__, __LINE__); // don't postpone errors
		EComparator<K*>* cmp = comparator_;
		for (;;) {
			for (Node* b = findPredecessor(key.get(), cmp), *n = b->next;;) {
				if (n != null) {
					int c, s;
					Node* f = n->next;
					if (n != b->next)               // inconsistent read
						break;
					if ((s = n->status) == DELETE_NODE) {   // n is deleted
						n->helpDelete(b, f);
						break;
					}
					if (b->status == DELETE_NODE || s == MARKER_NODE) // b is deleted
						break;
					if ((c = cpr(cmp, key.get(), n->key.get())) > 0) {
						b = n;
						n = f;
						continue;
					}
					if (c == 0) {
						sp<V> v = atomic_load(&n->value);
						if (onlyIfAbsent || n->casValue(v, value)) {
							return v;
						}
						break; // restart if lost race to replace value
					}
					// else c < 0; fall through
				}

				z = new Node(key, value, n);
				if (!b->casNext(n, z)) {
					nodeSafeDelete(z);
					break;         // restart if lost race to append to b
				}
				goto OUTER;
			}
		}

OUTER:
		//@see: int rnd = ThreadLocalRandom.nextSecondarySeed();
		unsigned int rnd = EThreadLocalRandom::current()->nextInt();
		if ((rnd & 0x80000001) == 0) { // test highest and lowest bits
			int level = 1, max;
			while (((rnd >>= 1) & 1) != 0)
				++level;
			Index* idx = null;
			HeadIndex* h = head_;
			if (level <= (max = h->level)) {
				for (int i = 1; i <= level; ++i)
					idx = new Index(z, idx, null);
			}
			else { // try to grow by one level
				level = max + 1; // hold in array and later pick the one to use
				EA<Index*> idxs(level+1, false);
				for (int i = 1; i <= level; ++i)
					idxs[i] = idx = new Index(z, idx, null);
				for (;;) {
					h = head_;
					int oldLevel = h->level;
					if (level <= oldLevel) // lost race to add level
						break;
					HeadIndex* newh = h;
					Node* oldbase = h->node;
					for (int j = oldLevel+1; j <= level; ++j)
						newh = new HeadIndex(oldbase, newh, idxs[j], j);
					if (casHead(h, newh)) {
						h = newh;
						idx = idxs[level = oldLevel];
						break;
					}
					else {
						//if fail then need free.
						Index* tmp = newh;;
						while (tmp) {
							tmp->right = null;
							tmp = tmp->down;
						}
						delete newh;
					}
				}
			}
			// find insertion points and splice in
			for (int insertionLevel = level;;) {
				int j = h->level;
				for (Index* q = h, *r = q->right, *t = idx;;) {
					if (q == null || t == null)
						goto SPLICE;
					if (r != null) {
						Node* n = r->node;
						// compare before deletion check avoids needing recheck
						int c = cpr(cmp, key.get(), n->key.get());
						if (n->status == DELETE_NODE) {
							if (!q->unlink(r))
								break;
							r = q->right;
							continue;
						}
						if (c > 0) {
							q = r;
							r = r->right;
							continue;
						}
					}

					if (j == insertionLevel) {
						if (!q->link(r, t))
							break; // restart
						if (t->node->status == DELETE_NODE) {
							findNode(key.get());
							goto SPLICE;
						}
						if (--insertionLevel == 0)
							goto SPLICE;
					}

					if (--j >= insertionLevel && j < level)
						t = t->down;
					q = q->down;
					r = q->right;
				}
			}
		}
SPLICE:
		return null;
	}

	/* ---------------- Deletion -------------- */

	/**
	 * Main deletion method. Locates node, nulls value, appends a
	 * deletion marker, unlinks predecessor, removes associated index
	 * nodes, and possibly reduces head index level.
	 *
	 * Index nodes are cleared out simply by calling findPredecessor.
	 * which unlinks indexes to deleted nodes found along path to key,
	 * which will include the indexes to this node.  This is done
	 * unconditionally. We can't check beforehand whether there are
	 * index nodes because it might be the case that some or all
	 * indexes hadn't been inserted yet for this node during initial
	 * search for it, and we'd like to ensure lack of garbage
	 * retention, so must call to be sure.
	 *
	 * @param okey the key
	 * @param value if non-null, the value that must be
	 * associated with key
	 * @return the node, or null if not found
	 */
	sp<V> doRemove(K* key, V* value) {
		return null; /* FIXME: where is the bug? */

		if (key == null)
			throw ENullPointerException(__FILE__, __LINE__);
		EComparator<K*>* cmp = comparator_;
		for (;;) {
			for (Node* b = findPredecessor(key, cmp), *n = b->next;;) {
				int c, s;
				if (n == null)
					goto OUTER;
				Node* f = n->next;
				if (n != b->next)                    // inconsistent read
					break;
				if ((s = n->status) == DELETE_NODE) {        // n is deleted
					n->helpDelete(b, f);
					break;
				}
				if (b->status == DELETE_NODE || s == MARKER_NODE)      // b is deleted
					break;
				if ((c = cpr(cmp, key, n->key.get())) < 0)
					goto OUTER;
				if (c > 0) {
					b = n;
					n = f;
					continue;
				}
				sp<V> v = atomic_load(&n->value);
				if (value != null && !v->equals(value))
					goto OUTER;
				if (!n->casStatus(s, DELETE_NODE))
					break;
				if (!n->appendMarker(f) || !b->casNext(n, f))
					findNode(key);                  // retry via findNode
				else {
					findPredecessor(key, cmp);      // clean index
					if (head_->right == null)
						tryReduceLevel();
				}
				return v;
			}
		}
    OUTER:
		return null;
	}

	/**
	 * Possibly reduce head level if it has no nodes.  This method can
	 * (rarely) make mistakes, in which case levels can disappear even
	 * though they are about to contain index nodes. This impacts
	 * performance, not correctness.  To minimize mistakes as well as
	 * to reduce hysteresis, the level is reduced by one only if the
	 * topmost three levels look empty. Also, if the removed level
	 * looks non-empty after CAS, we try to change it back quick
	 * before anyone notices our mistake! (This trick works pretty
	 * well because this method will practically never make mistakes
	 * unless current thread stalls immediately before first CAS, in
	 * which case it is very unlikely to stall again immediately
	 * afterwards, so will recover.)
	 *
	 * We put up with all this rather than just let levels grow
	 * because otherwise, even a small map that has undergone a large
	 * number of insertions and removals will have a lot of levels,
	 * slowing down access more than would an occasional unwanted
	 * reduction.
	 */
	void tryReduceLevel() {
		HeadIndex* h = head_;
		HeadIndex* d;
		HeadIndex* e;
		if (h->level > 3 &&
			(d = (HeadIndex*)h->down) != null &&
			(e = (HeadIndex*)d->down) != null &&
			e->right == null &&
			d->right == null &&
			h->right == null &&
			casHead(h, d) && // try to set
			h->right != null) // recheck
			casHead(d, h);   // try to backout
	}

	/* ---------------- Finding and removing first element -------------- */

	/**
	 * Specialized variant of findNode to get first valid node.
	 * @return first node or null if empty
	 */
	Node* findFirst() {
		for (;;) {
			Node* b = head_->node;
			Node* n = b->next;
			if (n == null)
				return null;
			if (n->status != DELETE_NODE)
				return n;
			n->helpDelete(b, n->next);
		}
		//not reach here.
		return null;
	}

	/**
	 * Removes first entry; returns its snapshot.
	 * @return null if empty, else snapshot of first entry
	 */
	sp<EConcurrentMapEntry<K,V> > doRemoveFirstEntry() {
		for (;;) {
			Node* b = head_->node;
			Node* n = b->next;
			if (n == null)
				return null;
			Node* f = n->next;
			if (n != b->next)
				continue;
			int s = n->status;
			if (s == DELETE_NODE) {
				n->helpDelete(b, f);
				continue;
			}
			if (!n->casStatus(s, DELETE_NODE))
				continue;
			if (!n->appendMarker(f) || !b->casNext(n, f))
				findFirst(); // retry
			clearIndexToFirst();
			return new EConcurrentImmutableEntry<K,V>(n->key, atomic_load(&n->value));
		}
		//not reach here.
		return null;
	}

	/**
	 * Clears out index nodes associated with deleted first entry.
	 */
	void clearIndexToFirst() {
		for (;;) {
			Index* q = head_;
			for (;;) {
				Index* r = q->right;
				if (r != null && r->indexesDeletedNode() && !q->unlink(r))
					break;
				if ((q = q->down) == null) {
					if (head_->right == null)
						tryReduceLevel();
					return;
				}
			}
		}
	}

	/* ---------------- Finding and removing last element -------------- */

	/**
	 * Specialized version of find to get last valid node.
	 * @return last node or null if empty
	 */
	Node* findLast() {
		/*
		 * findPredecessor can't be used to traverse index level
		 * because this doesn't use comparisons.  So traversals of
		 * both levels are folded together.
		 */
		Index* q = head_;
		for (;;) {
			Index* d, *r;
			if ((r = q->right) != null) {
				if (r->indexesDeletedNode()) {
					q->unlink(r);
					q = head_; // restart
				}
				else
					q = r;
			} else if ((d = q->down) != null) {
				q = d;
			} else {
				Node* b = q->node;
				Node* n = b->next;
				for (;;) {
					if (n == null)
						return b->isBaseHeader() ? null : b;
					Node* f = n->next;            // inconsistent read
					if (n != b->next)
						break;
					int s = n->status;
					if (s == DELETE_NODE) {                 // n is deleted
						n->helpDelete(b, f);
						break;
					}
					if (b->status == DELETE_NODE || s == MARKER_NODE)   // b is deleted
						break;
					b = n;
					n = f;
				}
				q = head_; // restart
			}
		}
		//not reach here.
		return null;
	}

	/**
	 * Specialized variant of findPredecessor to get predecessor of last
	 * valid node.  Needed when removing the last entry.  It is possible
	 * that all successors of returned node will have been deleted upon
	 * return, in which case this method can be retried.
	 * @return likely predecessor of last node
	 */
	Node* findPredecessorOfLast() {
		for (;;) {
			Index* q = head_;
			for (;;) {
				Index* d, *r;
				if ((r = q->right) != null) {
					if (r->indexesDeletedNode()) {
						q->unlink(r);
						break;    // must restart
					}
					// proceed as far across as possible without overshooting
					if (r->node->next != null) {
						q = r;
						continue;
					}
				}
				if ((d = q->down) != null)
					q = d;
				else
					return q->node;
			}
		}
		//not reach here.
		return null;
	}

	/**
	 * Removes last entry; returns its snapshot.
	 * Specialized variant of doRemove.
	 * @return null if empty, else snapshot of last entry
	 */
	sp<EConcurrentMapEntry<K,V> > doRemoveLastEntry() {
		for (;;) {
			Node* b = findPredecessorOfLast();
			Node* n = b->next;
			if (n == null) {
				if (b->isBaseHeader())               // empty
					return null;
				else
					continue; // all b's successors are deleted; retry
			}
			for (;;) {
				Node* f = n->next;
				if (n != b->next)                    // inconsistent read
					break;
				int s = n->status;
				if (s == DELETE_NODE) {                    // n is deleted
					n->helpDelete(b, f);
					break;
				}
				if (b->status == DELETE_NODE || s == MARKER_NODE)      // b is deleted
					break;
				if (f != null) {
					b = n;
					n = f;
					continue;
				}
				if (!n->casStatus(s, DELETE_NODE))
					break;
				sp<K> key = n->key;
				if (!n->appendMarker(f) || !b->casNext(n, f))
					findNode(key.get());                  // Retry via findNode
				else {
					findPredecessor(key.get(), comparator_);           // Clean index
					if (head_->right == null)
						tryReduceLevel();
				}
				return new EConcurrentImmutableEntry<K,V>(key, atomic_load(&n->value));
			}
		}
		//not reach here.
		return null;
	}

	/* ---------------- Relational operations -------------- */

	// Control values OR'ed as arguments to findNear

	static const int EQ = 1;
	static const int LT = 2;
	static const int GT = 0; // Actually checked as !LT

	/**
	 * Utility for ceiling, floor, lower, higher methods.
	 * @param key the key
	 * @param rel the relation -- OR'ed combination of EQ, LT, GT
	 * @return nearest node fitting relation, or null if no such
	 */
	Node* findNear(K* key, int rel, EComparator<K*>* cmp) {
		if (key == null)
			throw ENullPointerException(__FILE__, __LINE__);
		for (;;) {
			Node* b = findPredecessor(key, cmp);
			Node* n = b->next;
			for (;;) {
				int s;
				if (n == null)
					return ((rel & LT) == 0 || b->isBaseHeader()) ? null : b;
				Node* f = n->next;
				if (n != b->next)                  // inconsistent read
					break;
				if ((s = n->status) == DELETE_NODE) {      // n is deleted
					n->helpDelete(b, f);
					break;
				}
				if (b->status == DELETE_NODE || s == MARKER_NODE)      // b is deleted
					break;
				int c = cpr(cmp, key, n->key.get());
				if ((c == 0 && (rel & EQ) != 0) ||
					(c <  0 && (rel & LT) == 0))
					return n;
				if ( c <= 0 && (rel & LT) != 0)
					return b->isBaseHeader() ? null : b;
				b = n;
				n = f;
			}
		}
		//not reach here.
		return null;
	}

	/**
	 * Returns SimpleImmutableEntry for results of findNear.
	 * @param key the key
	 * @param rel the relation -- OR'ed combination of EQ, LT, GT
	 * @return Entry fitting relation, or null if no such
	 */
	EConcurrentMapEntry<K,V>* getNear(K* key, int rel) {
		EComparator<K*>* cmp = comparator_;
		for (;;) {
			Node* n = findNear(key, rel, cmp);
			if (n == null)
				return null;
			EConcurrentMapEntry<K,V>* e = n->createSnapshot();
			if (e != null)
				return e;
		}
		//not reach here.
		return null;
	}

protected:
	/* ---------------- Iterators -------------- */

	/**
	 * Base of iterator classes:
	 */
	template<typename T>
	abstract class Iter : public EConcurrentIterator<T> {
	private:
		EConcurrentSkipListMap<K,V>* m;
	protected:
		/** the last node returned by next() */
		Node* lastReturned;
		/** the next node to return from next(); */
		Node* next_;
		/** Cache of next value field to maintain weak consistency */
		sp<V> nextValue;

		/** Initializes ascending iterator for entire range. */
		Iter(EConcurrentSkipListMap<K,V>* map) : m(map), lastReturned(null) {
			while ((next_ = m->findFirst()) != null) {
				int s = next_->status;
				if (s != DELETE_NODE && s != MARKER_NODE) {
					nextValue = atomic_load(&next_->value);
					break;
				}
			}
		}

	public:
		boolean hasNext() {
			return next_ != null;
		}

		/** Advances next to higher entry. */
		void advance() {
			if (next_ == null)
				throw ENoSuchElementException(__FILE__, __LINE__);
			lastReturned = next_;
			while ((next_ = next_->next) != null) {
				int s = next_->status;
				if (s != DELETE_NODE && s != MARKER_NODE) {
					nextValue = atomic_load(&next_->value);
					break;
				}
			}
		}

		void remove() {
			Node* l = lastReturned;
			if (l == null)
				throw EIllegalStateException(__FILE__, __LINE__);
			// It would not be worth all of the overhead to directly
			// unlink from here. Using remove is fast enough.
			m->remove(l->key.get());
			lastReturned = null;
		}

	};

	class KeyIterator : public Iter<K> {
	public:
		KeyIterator(EConcurrentSkipListMap<K,V>* map) : Iter<K>(map) {
		}
		sp<K> next() {
			Node* n = Iter<K>::next_;
			Iter<K>::advance();
			return n->key;
		}
	};

	class ValueIterator : public Iter<V> {
	public:
		ValueIterator(EConcurrentSkipListMap<K,V>* map) : Iter<V>(map) {
		}
		sp<V> next() {
			sp<V> v = Iter<V>::nextValue;
			Iter<V>::advance();
			return v;
		}
	};

	class EntryIterator : public Iter<EConcurrentMapEntry<K,V> > {
	public:
		EntryIterator(EConcurrentSkipListMap<K,V>* map) : Iter<EConcurrentMapEntry<K,V> >(map) {
		}
		sp<EConcurrentMapEntry<K,V> > next() {
			Node* n = Iter<EConcurrentMapEntry<K,V> >::next_;
			sp<V> v = Iter<EConcurrentMapEntry<K,V> >::nextValue;
			Iter<EConcurrentMapEntry<K,V> >::advance();
			return new EConcurrentImmutableEntry<K,V>(n->key, v);
		}
	};

	// Factory methods for iterators needed by ConcurrentSkipListSet etc

	sp<EConcurrentIterator<K> > keyIterator() {
		return new KeyIterator(this);
	}

	sp<EConcurrentIterator<V> > valueIterator() {
		return new ValueIterator(this);
	}

	sp<EConcurrentIterator<EConcurrentMapEntry<K,V> > > entryIterator() {
		return new EntryIterator(this);
	}

private:
	friend class Node;

	/**
	 * The topmost head index of the skiplist.
	 */
	HeadIndex* volatile head_;

	/**
	 * The comparator used to maintain order in this map, or null if
	 * using natural ordering.  (Non-private to simplify access in
	 * nested classes.)
	 * @serial
	 */
	EComparator<K*>* comparator_;

	/**
	 * Special value used to identify node
	 */
	static const int NORMAL_NODE = 0;
	static const int BASE_HEADER = 1;
	static const int MARKER_NODE = 2;
	static const int DELETE_NODE = 3;

	/** Lazily initialized entry set */
	sp<EntrySet> entrySet_;
	/** Lazily initialized key set */
	sp<KeySet> keySet_;
	/** Lazily initialized values collection */
	sp<Values> values_;
	/** Lazily initialized descending key set */
	sp<EConcurrentNavigableMap<K,V> > descendingMap_;

	/**
	 * compareAndSet head node
	 */
	boolean casHead(HeadIndex* cmp, HeadIndex* val) {
		return EUnsafe::compareAndSwapObject(&head_, cmp, val);
	}
};

} /* namespace efc */
#endif /* ECONCURRENTSKIPLISTMAP_HH_ */
