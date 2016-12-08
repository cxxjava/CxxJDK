/*
 * EConcurrentHashMap.hh
 *
 *  Created on: 2014-2-24
 *      Author: cxxjava@163.com
 */

#ifndef ECONCURRENTHASHMAP_HH_
#define ECONCURRENTHASHMAP_HH_

#include "EMap.hh"
#include "EMath.hh"
#include "EInteger.hh"
#include "ESharedArr.hh"
#include "EConcurrentSet.hh"
#include "EConcurrentMap.hh"
#include "EAbstractConcurrentCollection.hh"
#include "EConcurrentIterator.hh"
#include "EConcurrentEnumeration.hh"
#include "EReentrantLock.hh"
#include "ENullPointerException.hh"
#include "EIllegalArgumentException.hh"
#include "ENoSuchElementException.hh"
#include "EIllegalStateException.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

/**
 * A hash table supporting full concurrency of retrievals and
 * adjustable expected concurrency for updates. This class obeys the
 * same functional specification as {@link java.util.Hashtable}, and
 * includes versions of methods corresponding to each method of
 * <tt>Hashtable</tt>. However, even though all operations are
 * thread-safe, retrieval operations do <em>not</em> entail locking,
 * and there is <em>not</em> any support for locking the entire table
 * in a way that prevents all access.  This class is fully
 * interoperable with <tt>Hashtable</tt> in programs that rely on its
 * thread safety but not on its synchronization details.
 *
 * <p> Retrieval operations (including <tt>get</tt>) generally do not
 * block, so may overlap with update operations (including
 * <tt>put</tt> and <tt>remove</tt>). Retrievals reflect the results
 * of the most recently <em>completed</em> update operations holding
 * upon their onset.  For aggregate operations such as <tt>putAll</tt>
 * and <tt>clear</tt>, concurrent retrievals may reflect insertion or
 * removal of only some entries.  Similarly, Iterators and
 * Enumerations return elements reflecting the state of the hash table
 * at some point at or since the creation of the iterator/enumeration.
 * They do <em>not</em> throw {@link ConcurrentModificationException}.
 * However, iterators are designed to be used by only one thread at a time.
 *
 * <p> The allowed concurrency among update operations is guided by
 * the optional <tt>concurrencyLevel</tt> constructor argument
 * (default <tt>16</tt>), which is used as a hint for internal sizing.  The
 * table is internally partitioned to try to permit the indicated
 * number of concurrent updates without contention. Because placement
 * in hash tables is essentially random, the actual concurrency will
 * vary.  Ideally, you should choose a value to accommodate as many
 * threads as will ever concurrently modify the table. Using a
 * significantly higher value than you need can waste space and time,
 * and a significantly lower value can lead to thread contention. But
 * overestimates and underestimates within an order of magnitude do
 * not usually have much noticeable impact. A value of one is
 * appropriate when it is known that only one thread will modify and
 * all others will only read. Also, resizing this or any other kind of
 * hash table is a relatively slow operation, so, when possible, it is
 * a good idea to provide estimates of expected table sizes in
 * constructors.
 *
 * <p>This class and its views and iterators implement all of the
 * <em>optional</em> methods of the {@link Map} and {@link Iterator}
 * interfaces.
 *
 * <p> Like {@link Hashtable} but unlike {@link HashMap}, this class
 * does <em>not</em> allow <tt>null</tt> to be used as a key or value.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since 1.5
 * @param <K> the type of keys maintained by this map
 * @param <V> the type of mapped values
 */

/* ---------------- Constants -------------- */

/**
 * The default initial capacity for this table,
 * used when not otherwise specified in a constructor.
 */
#define CHM_DEFAULT_INITIAL_CAPACITY	16

/**
 * The default load factor for this table, used when not
 * otherwise specified in a constructor.
 */
#define CHM_DEFAULT_LOAD_FACTOR			0.75f

/**
 * The default concurrency level for this table, used when not
 * otherwise specified in a constructor.
 */
#define CHM_DEFAULT_CONCURRENCY_LEVEL	16

/**
 * The maximum capacity, used if a higher value is implicitly
 * specified by either of the constructors with arguments.  MUST
 * be a power of two <= 1<<30 to ensure that entries are indexable
 * using ints.
 */
#define CHM_MAXIMUM_CAPACITY		(1 << 30)

/**
 * The maximum number of segments to allow; used to bound
 * constructor arguments.
 */
#define CHM_MAX_SEGMENTS			(1 << 16) // slightly conservative

/**
 * Number of unsynchronized retries in size and containsValue
 * methods before resorting to locking. This is used to avoid
 * unbounded retries if tables undergo continuous modification
 * which would make it impossible to obtain an accurate result.
 */
#define CHM_RETRIES_BEFORE_LOCK		2

template<typename K, typename V>
class EConcurrentHashMap: public EConcurrentMap<K, V> {
public:
	/* ---------------- Inner Classes -------------- */

	/**
	 * ConcurrentHashMap list entry. Note that this is never exported
	 * out as a user-visible Map.Entry.
	 *
	 * Because the value field is volatile, not final, it is legal wrt
	 * the Java Memory Model for an unsynchronized reader to see null
	 * instead of initial value when read via a data race.  Although a
	 * reordering leading to this is not likely to ever actually
	 * occur, the Segment.readValueUnderLock method is used as a
	 * backup in case a null (pre-initialized) value is ever seen in
	 * an unsynchronized access method.
	 */
	class HashEntry {
	public:
		sp<K> key;
		sp<V> value; //volatile?
		sp<HashEntry> next; //volatile?
		int hash;

		~HashEntry() {
			//
		}

		HashEntry(sp<K>& key, int hash,
				sp<HashEntry>& next,
				sp<V> value) {
			this->key = key;
			this->hash = hash;
			this->next = next;
			this->value = value;
		}

		static ea<HashEntry>* newArray(int i) {
			return NEWRC(ea<HashEntry>)(i);
		}
	};

	/**
	 * Segments are specialized versions of hash tables.  This
	 * subclasses from ReentrantLock opportunistically, just to
	 * simplify some locking and avoid separate construction.
	 */
	class Segment : public EReentrantLock {
	public:
		/*
		 * Segments maintain a table of entry lists that are ALWAYS
		 * kept in a consistent state, so can be read without locking.
		 * Next fields of nodes are immutable (final).  All list
		 * additions are performed at the front of each bin. This
		 * makes it easy to check changes, and also fast to traverse.
		 * When nodes would otherwise be changed, new nodes are
		 * created to replace them. This works well for hash tables
		 * since the bin lists tend to be short. (The average length
		 * is less than two for the default load factor threshold.)
		 *
		 * Read operations can thus proceed without locking, but rely
		 * on selected uses of volatiles to ensure that completed
		 * write operations performed by other threads are
		 * noticed. For most purposes, the "count" field, tracking the
		 * number of elements, serves as that volatile variable
		 * ensuring visibility.  This is convenient because this field
		 * needs to be read in many read operations anyway:
		 *
		 *   - All (unsynchronized) read operations must first read the
		 *     "count" field, and should not look at table entries if
		 *     it is 0.
		 *
		 *   - All (synchronized) write operations should write to
		 *     the "count" field after structurally changing any bin.
		 *     The operations must not take any action that could even
		 *     momentarily cause a concurrent read operation to see
		 *     inconsistent data. This is made easier by the nature of
		 *     the read operations in Map. For example, no operation
		 *     can reveal that the table has grown but the threshold
		 *     has not yet been updated, so there are no atomicity
		 *     requirements for this with respect to reads.
		 *
		 * As a guide, all critical volatile reads and writes to the
		 * count field are marked in code comments.
		 */

		/**
		 * The number of elements in this segment's region.
		 */
		volatile int count;

		/**
		 * Number of updates that alter the size of the table. This is
		 * used during bulk-read methods to make sure they see a
		 * consistent snapshot: If modCounts change during a traversal
		 * of segments computing size or checking containsValue, then
		 * we might have an inconsistent view of state so (usually)
		 * must retry.
		 */
		int modCount;

		/**
		 * The table is rehashed when its size exceeds this threshold.
		 * (The value of this field is always <tt>(int)(capacity *
		 * loadFactor)</tt>.)
		 */
		int threshold;

		/**
		 * The per-segment table.
		 */
		ea<HashEntry>* volatile table;

		/**
		 * The load factor for the hash table.  Even though this value
		 * is same for all segments, it is replicated to avoid needing
		 * links to outer object.
		 * @serial
		 */
		float loadFactor;

		EConcurrentHashMap<K,V>* chm;

		~Segment() {
			DELRC(table);
		}

		Segment(int initialCapacity, float lf,
				EConcurrentHashMap<K, V>* chm) :
				count(0), modCount(0), threshold(0), table(null), chm(chm) {
			loadFactor = lf;
			setTable(HashEntry::newArray(initialCapacity));
		}

		static EA<Segment*>* newArray(int i) {
			return new EA<Segment*>(i);
		}

		/**
		 * Sets table to new HashEntry array.
		 * Call only while holding lock or in constructor.
		 */
		void setTable(ea<HashEntry>* newTable) {
			threshold = (int)(newTable->length() * loadFactor);
			ea<HashEntry>* oldTable = GETRC(table);
			table = newTable;
			DELRC(oldTable); //!
		}

		/**
		 * Returns properly casted first entry of bin for given hash.
		 */
		sp<HashEntry> getFirst(int hash) {
			ea<HashEntry>* tab = GETRC(table);
			sp<HashEntry> e = tab->atomicGet(hash & (tab->length() - 1));
			DELRC(tab);
			return e;
		}

		/**
		 * Reads value field of an entry under lock. Called if value
		 * field ever appears to be null. This is possible only if a
		 * compiler happens to reorder a HashEntry initialization with
		 * its table assignment, which is legal under memory model
		 * but is not known to ever occur.
		 */
		sp<V> readValueUnderLock(sp<HashEntry>& e) {
			sp<V> v;
			SYNCBLOCK(this) {
				v = atomic_load(&e->value);
            }}
			return v;
		}

		/* Specialized implementations of map methods */

		sp<V> get(K* key, int hash) {
			if (count != 0) { // read-volatile
				sp<HashEntry> e = getFirst(hash);
				while (e != null) {
					if (e->hash == hash && e->key->equals(key)) {
						sp<V> v = atomic_load(&e->value);
						if (v != null)
							return v;
						return readValueUnderLock(e); // recheck
					}
					e = e->next;
				}
			}
			return null;
		}

		boolean containsKey(K* key, int hash) {
			if (count != 0) { // read-volatile
				sp<HashEntry> e = getFirst(hash);
				while (e != null) {
					if (e->hash == hash && e->key->equals(key))
						return true;
					e = e->next;
				}
			}
			return false;
		}

		boolean containsValue(V* value) {
			if (count != 0) { // read-volatile
				ea<HashEntry>* tab = GETRC(table);
				int len = tab->length();
				for (int i = 0 ; i < len; i++) {
					for (sp<HashEntry> e = tab->atomicGet(i); e != null; e = e->next) {
						sp<V> v = atomic_load(&e->value);
						if (v == null) // recheck
							v = readValueUnderLock(e);
						if (v->equals(value)) {
							DELRC(tab);
							return true;
						}
					}
				}
				DELRC(tab);
			}
			return false;
		}

		boolean replace(K* key, int hash, V* oldValue, sp<V> newValue) {
			boolean rv;

			SYNCBLOCK(this) {
				sp<HashEntry> e = getFirst(hash);
				while (e != null && (e->hash != hash || !e->key->equals(key)))
					e = e->next;

				boolean replaced = false;
				if (e != null && e->value->equals(oldValue)) {
					replaced = true;
					e->value = newValue;
				}
				rv = replaced;
            }}

			return rv;
		}

		sp<V> replace(K* key, int hash, sp<V> newValue) {
			sp<V> rv;

			SYNCBLOCK(this) {
				sp<HashEntry> e = getFirst(hash);
				while (e != null && (e->hash != hash || !e->key->equals(key)))
					e = e->next;

				sp<V> oldValue = null;
				if (e != null) {
					oldValue = e->value;
					e->value = newValue;
				}
				rv = oldValue;
            }}

			return rv;
		}

		sp<V> put(sp<K>& key, int hash, sp<V>& value, boolean onlyIfAbsent) {
			SYNCBLOCK(this) {
				int c = count;
				if (c++ > threshold) // ensure capacity
					rehash();
				ea<HashEntry>* tab = GETRC(table);
				int index = hash & (tab->length() - 1);
				sp<HashEntry> first = tab->atomicGet(index);
				sp<HashEntry> e = first;
				while (e != null && (e->hash != hash || !e->key->equals(key.get())))
					e = e->next;

				sp<V> oldValue;
				if (e != null) {
					oldValue = atomic_load(&e->value);
					if (!onlyIfAbsent) {
						atomic_store(&e->value, value);
					}
				}
				else {
					++modCount;
					sp<HashEntry> newEntry(new HashEntry(key, hash, first, value));
					tab->atomicSet(index, newEntry);
					count = c; // write-volatile
				}
				DELRC(tab);
				return oldValue;
            }}
		}

		void rehash() {
			ea<HashEntry>* oldTable = GETRC(table);
			int oldCapacity = oldTable->length();
			if (oldCapacity >= CHM_MAXIMUM_CAPACITY) {
				DELRC(oldTable);
				return;
			}

			/*
			 * Reclassify nodes in each list to new Map.  Because we are
			 * using power-of-two expansion, the elements from each bin
			 * must either stay at same index, or move with a power of two
			 * offset. We eliminate unnecessary node creation by catching
			 * cases where old nodes can be reused because their next
			 * fields won't change. Statistically, at the default
			 * threshold, only about one-sixth of them need cloning when
			 * a table doubles. The nodes they replace will be garbage
			 * collectable as soon as they are no longer referenced by any
			 * reader thread that may be in the midst of traversing table
			 * right now.
			 */

			ea<HashEntry>* newTable = HashEntry::newArray(oldCapacity<<1);
			threshold = (int)(newTable->length() * loadFactor);
			int sizeMask = newTable->length() - 1;
			for (int i = 0; i < oldCapacity ; i++) {
				// We need to guarantee that any existing reads of old Map can
				//  proceed. So we cannot yet null out each bin.
				sp<HashEntry> e = oldTable->atomicGet(i);

				if (e != null) {
					sp<HashEntry> next = e->next;
					int idx = e->hash & sizeMask;

					//  Single node on list
					if (next == null)
						(*newTable)[idx] = e;

					else {
						// Reuse trailing consecutive sequence at same slot
						sp<HashEntry> lastRun = e;
						int lastIdx = idx;
						for (sp<HashEntry> last = next;
							 last != null;
							 last = last->next) {
							int k = last->hash & sizeMask;
							if (k != lastIdx) {
								lastIdx = k;
								lastRun = last;
							}
						}
						(*newTable)[lastIdx] = lastRun;

						// Clone all remaining nodes
						for (sp<HashEntry> p = e; p != lastRun; p = p->next) {
							int k = p->hash & sizeMask;
							sp<HashEntry> n = (*newTable)[k];
							sp<HashEntry> newEntry(new HashEntry(p->key, p->hash, n, atomic_load(&p->value)));
							(*newTable)[k] = newEntry;
						}
					}
				}
			}
			table = newTable;

			//free old table
			DELRC(oldTable);
			DELRC(oldTable);
		}

		/**
		 * Remove; match on key only if value null, else match both.
		 */
		sp<V> remove(K* key, int hash, V* value) {
			sp<V> rv;

			SYNCBLOCK(this) {
				int c = count - 1;
				ea<HashEntry>* tab = GETRC(table);
				int index = hash & (tab->length() - 1);
				sp<HashEntry> first = tab->atomicGet(index);
				sp<HashEntry> e = first;
				while (e != null && (e->hash != hash || !key->equals(e->key.get())))
					e = e->next;

				sp<V> oldValue = null;
				if (e != null) {
					sp<V> v = atomic_load(&e->value);
					if (value == null || value->equals(v.get())) {
						oldValue = v;
						// All entries following removed node can stay
						// in list, but all preceding ones need to be
						// cloned.
						++modCount;
						sp<HashEntry> newFirst = e->next;
						for (sp<HashEntry> p = first; p != e; p = p->next) {
							newFirst = new HashEntry(p->key, p->hash,
													newFirst, atomic_load(&p->value));
						}
						tab->atomicSet(index, newFirst);
						count = c; // write-volatile
					}
				}
				rv = oldValue;
				DELRC(tab);
            }}

			return rv;
		}

		void clear() {
			if (count != 0) {
				SYNCBLOCK(this) {
					ea<HashEntry>* tab = GETRC(table);
					for (int i = 0; i < tab->length() ; i++) {
						sp<HashEntry> nullPtr;
						tab->atomicSet(i, nullPtr);
					}
					++modCount;
					count = 0; // write-volatile
					DELRC(tab);
                }}
			}
		}
	};

	/* ---------------- Iterator Support -------------- */

	abstract class HashIterator {
		EConcurrentHashMap<K,V>* chm;
		int nextSegmentIndex;
		int nextTableIndex;
		ea<HashEntry>* currentTable;
		sp<HashEntry> nextEntry_;
		sp<HashEntry> lastReturned;

		void advance() {
			if (nextEntry_ != null && (nextEntry_ = nextEntry_->next) != null)
				return;

			while (nextTableIndex >= 0) {
				if ( (nextEntry_ = (*currentTable)[nextTableIndex--]) != null)
					return;
			}

			while (nextSegmentIndex >= 0) {
				Segment* seg = (*chm->segments_)[nextSegmentIndex--];
				if (seg->count != 0) {
					DELRC(currentTable); //!
					currentTable = GETRC(seg->table);
					for (int j = currentTable->length() - 1; j >= 0; --j) {
						if ( (nextEntry_ = (*currentTable)[j]) != null) {
							nextTableIndex = j - 1;
							return;
						}
					}
				}
			}
		}

	public:
		HashIterator(EConcurrentHashMap<K,V>* chm) : chm(chm) {
			nextSegmentIndex = chm->segments_->length() - 1;
			nextTableIndex = -1;
			currentTable = null;
			advance();
		}

		virtual ~HashIterator() {
			DELRC(currentTable);
		}

		sp<HashEntry> nextEntry() {
			if (nextEntry_ == null)
				throw ENoSuchElementException(__FILE__, __LINE__);
			lastReturned = nextEntry_;
			advance();
			return lastReturned;
		}

		boolean hasMoreElements() {
			return hasNext();
		}

		boolean hasNext() {
			return nextEntry_ != null;
		}

		void remove() {
			if (lastReturned == null)
				throw EIllegalStateException(__FILE__, __LINE__);
			chm->remove(lastReturned->key.get());
			lastReturned = null;
		}
	};

	class KeyIterator: public HashIterator, public EConcurrentIterator<
	K>, public EConcurrentEnumeration<K> {
	public:
		KeyIterator(EConcurrentHashMap<K,V>* chm) : HashIterator(chm) {}
		boolean hasMoreElements()    { return HashIterator::hasMoreElements();  }
		boolean hasNext()            { return HashIterator::hasNext();          }
		sp<K> next()        { return HashIterator::nextEntry()->key; }
		sp<K> nextElement() { return HashIterator::nextEntry()->key; }
		void remove()                { HashIterator::remove();                  }
	};

	class ValueIterator: public HashIterator, public EConcurrentIterator<
	V>, public EConcurrentEnumeration<V> {
	public:
		ValueIterator(EConcurrentHashMap<K,V>* chm) : HashIterator(chm) {}
		boolean hasMoreElements()      { return HashIterator::hasMoreElements();  }
		boolean hasNext()              { return HashIterator::hasNext();          }
		sp<V> next()        { return HashIterator::nextEntry()->value; }
		sp<V> nextElement() { return HashIterator::nextEntry()->value; }
		void remove()                  { HashIterator::remove();                  }
	};

	/**
	 * Custom Entry class used by EntryIterator.next(), that relays
	 * setValue changes to the underlying map.
	 */
	class WriteThroughEntry: public EConcurrentMapEntry<K, V> {
	private:
		sp<K> key;
		sp<V> value;
		EConcurrentHashMap<K, V>* chm;

		static boolean eq(EObject* o1, EObject* o2) {
			return o1 == null ? o2 == null : o1->equals(o2);
		}
	public:
		/**
		 * Creates an entry representing a mapping from the specified
		 * key to the specified value.
		 *
		 * @param key the key represented by this entry
		 * @param value the value represented by this entry
		 */
		WriteThroughEntry(sp<K>& key, sp<V>& value,
				EConcurrentHashMap<K, V>* chm) : chm(chm) {
			this->key = key;
			this->value = value;
		}

		/**
		 * Returns the key corresponding to this entry.
		 *
		 * @return the key corresponding to this entry
		 */
		sp<K> getKey() {
			return key;
		}

		/**
		 * Returns the value corresponding to this entry.
		 *
		 * @return the value corresponding to this entry
		 */
		sp<V> getValue() {
			return value;
		}

		/**
		 * Set our entry's value and write through to the map. The
		 * value to return is somewhat arbitrary here. Since a
		 * WriteThroughEntry does not necessarily track asynchronous
		 * changes, the most recent "previous" value could be
		 * different from what we return (or could even have been
		 * removed in which case the put will re-establish). We do not
		 * and cannot guarantee more.
		 */
		sp<V> setValue(sp<V> value) {
			if (value == null)
				throw ENullPointerException(__FILE__, __LINE__);

			sp<V> oldValue = this->value;
			this->value = value;
			return oldValue;
		}

		boolean equals(sp<EConcurrentMapEntry<K, V> > o) {
			return eq(key.get(), o->getKey().get()) && eq(value.get(), o->getValue().get());
		}

		virtual int hashCode() {
			return (key   == null ? 0 : key->hashCode()) ^
					(value == null ? 0 : value->hashCode());
		}
	};

	class EntryIterator: public HashIterator,
	                     public EConcurrentIterator<EConcurrentMapEntry<K, V> >
	{
	private:
		EConcurrentHashMap<K, V>* chm;
	public:
		EntryIterator(EConcurrentHashMap<K, V>* chm) :
				HashIterator(chm), chm(chm) {
		}
		sp<EConcurrentMapEntry<K, V> > next() {
			sp<HashEntry> e = HashIterator::nextEntry();
			return new WriteThroughEntry(e->key, e->value, chm);
		}
		boolean hasNext() {
			return HashIterator::hasNext();
		}
		void remove() {
			HashIterator::remove();
		}
	};

	class KeySet : public EConcurrentSet<K> {
	private:
		EConcurrentHashMap<K,V>* chm;
	public:
		KeySet(EConcurrentHashMap<K,V>* chm) : chm(chm) {
		}
		sp<EConcurrentIterator<K> > iterator() {
			return new KeyIterator(chm);
		}
		int size() {
			return chm->size();
		}
		boolean isEmpty() {
			return chm->isEmpty();
		}
		boolean contains(K* o) {
			return chm->containsKey(o);
		}
		boolean remove(K* o) {
			return chm->remove(o) != null;
		}
		void clear() {
			chm->clear();
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
		EConcurrentHashMap<K,V>* chm;
	public:
		Values(EConcurrentHashMap<K,V>* chm) : chm(chm) {
		}
		sp<EConcurrentIterator<V> > iterator() {
			return new ValueIterator(chm);
		}
		int size() {
			return chm->size();
		}
		boolean isEmpty() {
			return chm->isEmpty();
		}
		boolean contains(V* o) {
			return chm->containsValue(o);
		}
		void clear() {
			chm->clear();
		}
//		boolean add(V* e) {
//			throw EUnsupportedOperationException(__FILE__, __LINE__);
//		}
//		boolean add(sp<V> e) {
//			throw EUnsupportedOperationException(__FILE__, __LINE__);
//		}
//		boolean remove(V* o) {
//			sp<EConcurrentIterator<V> > e = iterator();
//			while (e->hasNext()) {
//				if (o->equals(e->next().get())) {
//					e->remove();
//					return true;
//				}
//			}
//			return false;
//		}
	};

	class EntrySet : public EConcurrentSet<EConcurrentMapEntry<K,V> > {
	private:
		EConcurrentHashMap<K,V>* chm;
	public:
		EntrySet(EConcurrentHashMap<K,V>* chm) : chm(chm) {
		}
		sp<EConcurrentIterator<EConcurrentMapEntry<K,V> > > iterator() {
			return new EntryIterator(chm);
		}
		boolean contains(EConcurrentMapEntry<K,V>* e) {
			sp<V> v = chm->get(e->getKey().get());
			return v != null && v->equals(e->getValue().get());
		}
		boolean remove(EConcurrentMapEntry<K,V>* e) {
			return chm->remove(e->getKey().get(), e->getValue().get());
		}
		int size() {
			return chm->size();
		}
		boolean isEmpty() {
			return chm->isEmpty();
		}
		void clear() {
			chm->clear();
		}
		boolean add(EConcurrentMapEntry<K,V>* e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		boolean add(sp<EConcurrentMapEntry<K,V> > e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

public:
	/* ---------------- Public operations -------------- */

	~EConcurrentHashMap() {
		delete segments_;
	}

	/**
	 * Creates a new, empty map with the specified initial
	 * capacity, load factor and concurrency level.
	 *
	 * @param initialCapacity the initial capacity. The implementation
	 * performs internal sizing to accommodate this many elements.
	 * @param loadFactor  the load factor threshold, used to control resizing.
	 * Resizing may be performed when the average number of elements per
	 * bin exceeds this threshold.
	 * @param concurrencyLevel the estimated number of concurrently
	 * updating threads. The implementation performs internal sizing
	 * to try to accommodate this many threads.
	 * @throws IllegalArgumentException if the initial capacity is
	 * negative or the load factor or concurrencyLevel are
	 * nonpositive.
	 */
	EConcurrentHashMap(int initialCapacity, float loadFactor, int concurrencyLevel) {
		init(initialCapacity, loadFactor, concurrencyLevel);
	}

	/**
	 * Creates a new, empty map with the specified initial capacity
	 * and load factor and with the default concurrencyLevel (16).
	 *
	 * @param initialCapacity The implementation performs internal
	 * sizing to accommodate this many elements.
	 * @param loadFactor  the load factor threshold, used to control resizing.
	 * Resizing may be performed when the average number of elements per
	 * bin exceeds this threshold.
	 * @throws IllegalArgumentException if the initial capacity of
	 * elements is negative or the load factor is nonpositive
	 *
	 * @since 1.6
	 */
	EConcurrentHashMap(int initialCapacity, float loadFactor) {
		init(initialCapacity, loadFactor, CHM_DEFAULT_CONCURRENCY_LEVEL);
	}

	/**
	 * Creates a new, empty map with the specified initial capacity,
	 * and with default load factor (0.75) and concurrencyLevel (16).
	 *
	 * @param initialCapacity the initial capacity. The implementation
	 * performs internal sizing to accommodate this many elements.
	 * @throws IllegalArgumentException if the initial capacity of
	 * elements is negative.
	 */
	EConcurrentHashMap(int initialCapacity) {
		init(initialCapacity, CHM_DEFAULT_LOAD_FACTOR, CHM_DEFAULT_CONCURRENCY_LEVEL);
	}

	/**
	 * Creates a new, empty map with a default initial capacity (16),
	 * load factor (0.75) and concurrencyLevel (16).
	 */
	EConcurrentHashMap() {
		init(CHM_DEFAULT_INITIAL_CAPACITY, CHM_DEFAULT_LOAD_FACTOR, CHM_DEFAULT_CONCURRENCY_LEVEL);
	}

	/**
	 * Creates a new map with the same mappings as the given map.
	 * The map is created with a capacity of 1.5 times the number
	 * of mappings in the given map or 16 (whichever is greater),
	 * and a default load factor (0.75) and concurrencyLevel (16).
	 *
	 * @param m the map
	 */
	EConcurrentHashMap(EMap<K, V>* m) :
			segmentMask(0), segmentShift(0), segments_(null) {
		this(EMath::max((int) (m->size() / CHM_DEFAULT_LOAD_FACTOR) + 1,
						CHM_DEFAULT_INITIAL_CAPACITY), CHM_DEFAULT_LOAD_FACTOR,
				CHM_DEFAULT_CONCURRENCY_LEVEL);
		putAll(m);
	}

	/**
	 * Returns <tt>true</tt> if this map contains no key-value mappings.
	 *
	 * @return <tt>true</tt> if this map contains no key-value mappings
	 */
	boolean isEmpty() {
		EA<Segment*>* segments = this->segments_;
		/*
		 * We keep track of per-segment modCounts to avoid ABA
		 * problems in which an element in one segment was added and
		 * in another removed during traversal, in which case the
		 * table was never actually empty at any point. Note the
		 * similar use of modCounts in the size() and containsValue()
		 * methods, which are the only other methods also susceptible
		 * to ABA problems.
		 */
		int* mc = new int[segments->length()]();
		int mcsum = 0;
		for (int i = 0; i < segments->length(); ++i) {
			if ((*segments)[i]->count != 0) {
				delete[] mc;
				return false;
			} else {
				mcsum += mc[i] = (*segments)[i]->modCount;
			}
		}
		// If mcsum happens to be zero, then we know we got a snapshot
		// before any modifications at all were made.  This is
		// probably common enough to bother tracking.
		if (mcsum != 0) {
			for (int i = 0; i < segments->length(); ++i) {
				if ((*segments)[i]->count != 0 ||
					mc[i] != (*segments)[i]->modCount) {
					delete[] mc;
					return false;
				}
			}
		}
		delete[] mc;
		return true;
	}

	/**
	 * Returns the number of key-value mappings in this map.  If the
	 * map contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
	 * <tt>Integer.MAX_VALUE</tt>.
	 *
	 * @return the number of key-value mappings in this map
	 */
	int size() {
		EA<Segment*>* segments = this->segments_;
		long sum = 0;
		long check = 0;
		int* mc = new int[segments->length()]();
		// Try a few times to get accurate count. On failure due to
		// continuous async changes in table, resort to locking.
		for (int k = 0; k < CHM_RETRIES_BEFORE_LOCK; ++k) {
			check = 0;
			sum = 0;
			int mcsum = 0;
			for (int i = 0; i < segments->length(); ++i) {
				sum += (*segments)[i]->count;
				mcsum += mc[i] = (*segments)[i]->modCount;
			}
			if (mcsum != 0) {
				for (int i = 0; i < segments->length(); ++i) {
					check += (*segments)[i]->count;
					if (mc[i] != (*segments)[i]->modCount) {
						check = -1; // force retry
						break;
					}
				}
			}
			if (check == sum)
				break;
		}
		if (check != sum) { // Resort to locking all segments
			sum = 0;
			int i;
			for (i = 0; i < segments->length(); ++i)
				(*segments)[i]->lock();
			for (i = 0; i < segments->length(); ++i)
				sum += (*segments)[i]->count;
			for (i = 0; i < segments->length(); ++i)
				(*segments)[i]->unlock();
		}
		if (sum > EInteger::MAX_VALUE) {
			delete[] mc;
			return EInteger::MAX_VALUE;
		}
		else {
			delete[] mc;
			return (int)sum;
		}
	}

	/**
	 * Returns the value to which the specified key is mapped,
	 * or {@code null} if this map contains no mapping for the key.
	 *
	 * <p>More formally, if this map contains a mapping from a key
	 * {@code k} to a value {@code v} such that {@code key.equals(k)},
	 * then this method returns {@code v}; otherwise it returns
	 * {@code null}.  (There can be at most one such mapping.)
	 *
	 * @throws NullPointerException if the specified key is null
	 */
	sp<V> get(K* key) {
		int hash = hashIt(key->hashCode());
		return segmentFor(hash)->get(key, hash);
	}

	/**
	 * Tests if the specified object is a key in this table.
	 *
	 * @param  key   possible key
	 * @return <tt>true</tt> if and only if the specified object
	 *         is a key in this table, as determined by the
	 *         <tt>equals</tt> method; <tt>false</tt> otherwise.
	 * @throws NullPointerException if the specified key is null
	 */
	boolean containsKey(K* key) {
		int hash = hashIt(key->hashCode());
		return segmentFor(hash)->containsKey(key, hash);
	}

	/**
	 * Returns <tt>true</tt> if this map maps one or more keys to the
	 * specified value. Note: This method requires a full internal
	 * traversal of the hash table, and so is much slower than
	 * method <tt>containsKey</tt>.
	 *
	 * @param value value whose presence in this map is to be tested
	 * @return <tt>true</tt> if this map maps one or more keys to the
	 *         specified value
	 * @throws NullPointerException if the specified value is null
	 */
	boolean containsValue(V* value) {
		if (value == null)
			throw ENullPointerException(__FILE__, __LINE__);

		// See explanation of modCount use above

		EA<Segment*>* segments = this->segments_;
		int* mc = new int[segments->length()]();

		// Try a few times without locking
		for (int k = 0; k < CHM_RETRIES_BEFORE_LOCK; ++k) {
			int mcsum = 0;
			for (int i = 0; i < segments->length(); ++i) {
				mcsum += mc[i] = (*segments)[i]->modCount;
				if ((*segments)[i]->containsValue(value)) {
					delete[] mc;
					return true;
				}
			}
			boolean cleanSweep = true;
			if (mcsum != 0) {
				for (int i = 0; i < segments->length(); ++i) {
					if (mc[i] != (*segments)[i]->modCount) {
						cleanSweep = false;
						break;
					}
				}
			}
			if (cleanSweep) {
				delete[] mc;
				return false;
			}
		}
		// Resort to locking all segments
		for (int i = 0; i < segments->length(); ++i)
			(*segments)[i]->lock();
		boolean found = false;
		try {
			for (int i = 0; i < segments->length(); ++i) {
				if ((*segments)[i]->containsValue(value)) {
					found = true;
					break;
				}
			}
		} catch(...) {
			finally {
				for (int i = 0; i < segments->length(); ++i)
					(*segments)[i]->unlock();
			}
			delete[] mc;
			throw; //!
		} finally {
			for (int i = 0; i < segments->length(); ++i)
				(*segments)[i]->unlock();
		}
		delete[] mc;
		return found;
	}

	/**
	 * Legacy method testing if some key maps into the specified value
	 * in this table.  This method is identical in functionality to
	 * {@link #containsValue}, and exists solely to ensure
	 * full compatibility with class {@link java.util.Hashtable},
	 * which supported this method prior to introduction of the
	 * Java Collections framework.

	 * @param  value a value to search for
	 * @return <tt>true</tt> if and only if some key maps to the
	 *         <tt>value</tt> argument in this table as
	 *         determined by the <tt>equals</tt> method;
	 *         <tt>false</tt> otherwise
	 * @throws NullPointerException if the specified value is null
	 */
	boolean contains(V* value) {
		return containsValue(value);
	}

	/**
	 * Maps the specified key to the specified value in this table.
	 * Neither the key nor the value can be null.
	 *
	 * <p> The value can be retrieved by calling the <tt>get</tt> method
	 * with a key that is equal to the original key.
	 *
	 * @param key key with which the specified value is to be associated
	 * @param value value to be associated with the specified key
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>
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
		int hash = hashIt(key->hashCode());
		return segmentFor(hash)->put(key, hash, value, false);
	}

	/**
	 * {@inheritDoc}
	 *
	 * @return the previous value associated with the specified key,
	 *         or <tt>null</tt> if there was no mapping for the key
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
		int hash = hashIt(key->hashCode());
		return segmentFor(hash)->put(key, hash, value, true);
	}

	/**
	 * Copies all of the mappings from the specified map to this one.
	 * These mappings replace any mappings that this map had for any of the
	 * keys currently in the specified map.
	 *
	 * @param m mappings to be stored in this map
	 */
	void putAll(EMap<K*, V*>* m) {
		sp<EIterator<EMapEntry<K*,V*>*> > it = m->entrySet()->iterator();
		while (it->hasNext()) {
			EMapEntry<K*,V*>* e = it->next();
			put(e->getKey(), e->getValue());
		}
	}

	/**
	 * Removes the key (and its corresponding value) from this map.
	 * This method does nothing if the key is not in the map.
	 *
	 * @param  key the key that needs to be removed
	 * @return the previous value associated with <tt>key</tt>, or
	 *         <tt>null</tt> if there was no mapping for <tt>key</tt>
	 * @throws NullPointerException if the specified key is null
	 */
	sp<V> remove(K* key) {
		int hash = hashIt(key->hashCode());
		return segmentFor(hash)->remove(key, hash, null);
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws NullPointerException if the specified key is null
	 */
	boolean remove(K* key, V* value) {
		int hash = hashIt(key->hashCode());
		if (value == null)
			return false;
		return segmentFor(hash)->remove(key, hash, value) != null;
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws NullPointerException if any of the arguments are null
	 */
	boolean replace(K* key, V* oldValue, V* newValue) {
		sp<V> nv(newValue);
		return replace(key, oldValue, nv);
	}
	boolean replace(K* key, V* oldValue, sp<V> newValue) {
		if (oldValue == null || newValue == null)
			throw ENullPointerException(__FILE__, __LINE__);
		int hash = hashIt(key->hashCode());
		return segmentFor(hash)->replace(key, hash, oldValue, newValue);
	}

	/**
	 * {@inheritDoc}
	 *
	 * @return the previous value associated with the specified key,
	 *         or <tt>null</tt> if there was no mapping for the key
	 * @throws NullPointerException if the specified key or value is null
	 */
	sp<V> replace(K* key, V* value) {
		sp<V> v(value);
		return replace(key, v);
	}
	sp<V> replace(K* key, sp<V> value) {
		if (value == null)
			throw ENullPointerException(__FILE__, __LINE__);
		int hash = hashIt(key->hashCode());
		return segmentFor(hash)->replace(key, hash, value);
	}

	/**
	 * Removes all of the mappings from this map.
	 */
	void clear() {
		for (int i = 0; i < segments_->length(); ++i)
			(*segments_)[i]->clear();
	}

	/**
	 * Returns a {@link Set} view of the keys contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  The set supports element
	 * removal, which removes the corresponding mapping from this map,
	 * via the <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt>, and <tt>clear</tt>
	 * operations.  It does not support the <tt>add</tt> or
	 * <tt>addAll</tt> operations.
	 *
	 * <p>The view's <tt>iterator</tt> is a "weakly consistent" iterator
	 * that will never throw {@link ConcurrentModificationException},
	 * and guarantees to traverse elements as they existed upon
	 * construction of the iterator, and may (but is not guaranteed to)
	 * reflect any modifications subsequent to construction.
	 */
	sp<EConcurrentSet<K> > keySet() {
		if (!keySet_) {
			keySet_ = new KeySet(this);
		}
		return keySet_;
	}

	/**
	 * Returns a {@link Collection} view of the values contained in this map.
	 * The collection is backed by the map, so changes to the map are
	 * reflected in the collection, and vice-versa.  The collection
	 * supports element removal, which removes the corresponding
	 * mapping from this map, via the <tt>Iterator.remove</tt>,
	 * <tt>Collection.remove</tt>, <tt>removeAll</tt>,
	 * <tt>retainAll</tt>, and <tt>clear</tt> operations.  It does not
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
		return values_;
	}

	/**
	 * Returns a {@link Set} view of the mappings contained in this map.
	 * The set is backed by the map, so changes to the map are
	 * reflected in the set, and vice-versa.  The set supports element
	 * removal, which removes the corresponding mapping from the map,
	 * via the <tt>Iterator.remove</tt>, <tt>Set.remove</tt>,
	 * <tt>removeAll</tt>, <tt>retainAll</tt>, and <tt>clear</tt>
	 * operations.  It does not support the <tt>add</tt> or
	 * <tt>addAll</tt> operations.
	 *
	 * <p>The view's <tt>iterator</tt> is a "weakly consistent" iterator
	 * that will never throw {@link ConcurrentModificationException},
	 * and guarantees to traverse elements as they existed upon
	 * construction of the iterator, and may (but is not guaranteed to)
	 * reflect any modifications subsequent to construction.
	 */
	sp<EConcurrentSet<EConcurrentMapEntry<K,V> > > entrySet() {
		if (!entrySet_) {
			entrySet_ = new EntrySet(this);
		}
		return entrySet_;
	}

	/**
	 * Returns an enumeration of the keys in this table.
	 *
	 * @return an enumeration of the keys in this table
	 * @see #keySet()
	 *
	 */
	sp<EConcurrentEnumeration<K> > keys() {
		return new KeyIterator(this);
	}

	/**
	 * Returns an enumeration of the values in this table.
	 *
	 * @return an enumeration of the values in this table
	 * @see #values()
	 *
	 */
	sp<EConcurrentEnumeration<V> > elements() {
		return new ValueIterator(this);
	}

protected:
	friend class HashIterator;

	/* ---------------- Fields -------------- */

	/**
	 * Mask value for indexing into segments. The upper bits of a
	 * key's hash code are used to choose the segment.
	 */
	int segmentMask;

	/**
	 * Shift value for indexing within segments.
	 */
	int segmentShift;

	/**
	 * The segments, each of which is a specialized hash table
	 */
	EA<Segment*>* segments_;

	sp<EConcurrentSet<EConcurrentMapEntry<K,V> > > entrySet_;
	sp<EConcurrentSet<K> > keySet_;
	sp<EConcurrentCollection<V> > values_;

private:

	void init(int initialCapacity, float loadFactor,
			int concurrencyLevel)
	{
		if (!(loadFactor > 0) || initialCapacity < 0 || concurrencyLevel <= 0)
			throw EIllegalArgumentException(__FILE__, __LINE__);

		if (concurrencyLevel > CHM_MAX_SEGMENTS)
			concurrencyLevel = CHM_MAX_SEGMENTS;

		// Find power-of-two sizes best matching arguments
		int sshift = 0;
		int ssize = 1;
		while (ssize < concurrencyLevel) {
			++sshift;
			ssize <<= 1;
		}
		segmentShift = 32 - sshift;
		segmentMask = ssize - 1;
		this->segments_ = Segment::newArray(ssize);

		if (initialCapacity > CHM_MAXIMUM_CAPACITY)
			initialCapacity = CHM_MAXIMUM_CAPACITY;
		int c = initialCapacity / ssize;
		if (c * ssize < initialCapacity)
			++c;
		int cap = 1;
		while (cap < c)
			cap <<= 1;

		for (int i = 0; i < this->segments_->length(); ++i)
			(*this->segments_)[i] = new Segment(cap, loadFactor, this);
	}

	/* ---------------- Small Utilities -------------- */

	/**
	 * Applies a supplemental hash function to a given hashCode, which
	 * defends against poor quality hash functions.  This is critical
	 * because ConcurrentHashMap uses power-of-two length hash tables,
	 * that otherwise encounter collisions for hashCodes that do not
	 * differ in lower or upper bits.
	 */
	static int hashIt(int h) {
		// Spread bits to regularize both segment and index locations,
		// using variant of single-word Wang/Jenkins hash.
		h += (h <<  15) ^ 0xffffcd7d;
		h ^= (((unsigned)h) >> 10);
		h += (h <<   3);
		h ^= (((unsigned)h) >>  6);
		h += (h <<   2) + (h << 14);
		return h ^ (((unsigned)h) >> 16);
	}

	/**
	 * Returns the segment that should be used for key with given hash
	 * @param hash the hash code for the key
	 * @return the segment
	 */
	Segment* segmentFor(int hash) {
		return (*segments_)[(((unsigned)hash) >> segmentShift) & segmentMask];
	}
};

//=============================================================================

#define ECHM_DECLARE(K) template<typename V> \
class EConcurrentHashMap<K, V>: public EConcurrentMap<K, V> { \
public: \
	class HashEntry { \
	public: \
		K key; \
		sp<V> value; \
		sp<HashEntry> next; \
		int hash; \
 \
		HashEntry(K key, int hash, \
				sp<HashEntry>& next, \
				sp<V> value) { \
			this->key = key; \
			this->hash = hash; \
			this->next = next; \
			this->value = value; \
		} \
 \
		static ea<HashEntry>* newArray(int i) { \
			return NEWRC(ea<HashEntry>)(i); \
		} \
	}; \
 \
	class Segment : public EReentrantLock { \
	public: \
		volatile int count; \
		int modCount; \
		int threshold; \
		ea<HashEntry>* volatile table; \
		float loadFactor; \
 \
		EConcurrentHashMap<K,V>* chm; \
 \
		~Segment() { \
			DELRC(table); \
		} \
 \
		Segment(int initialCapacity, float lf, \
				EConcurrentHashMap<K, V>* chm) : \
				count(0), modCount(0), threshold(0), table(null), chm(chm) { \
			loadFactor = lf; \
			setTable(HashEntry::newArray(initialCapacity)); \
		} \
 \
		static EA<Segment*>* newArray(int i) { \
			return new EA<Segment*>(i); \
		} \
 \
		void setTable(ea<HashEntry>* newTable) { \
			threshold = (int)(newTable->length() * loadFactor); \
			ea<HashEntry>* oldTable = GETRC(table); \
			table = newTable; \
			DELRC(oldTable); \
		} \
 \
		sp<HashEntry> getFirst(int hash) { \
			ea<HashEntry>* tab = GETRC(table); \
			sp<HashEntry> e = tab->atomicGet(hash & (tab->length() - 1)); \
			DELRC(tab); \
			return e; \
		} \
 \
		sp<V> readValueUnderLock(sp<HashEntry>& e) { \
			sp<V> v; \
			SYNCBLOCK(this) { \
				v = atomic_load(&e->value); \
            }} \
			return v; \
		} \
 \
		sp<V> get(K key, int hash) { \
			if (count != 0) { \
				sp<HashEntry> e = getFirst(hash); \
				while (e != null) { \
					if (e->hash == hash && e->key == key) { \
						sp<V> v = atomic_load(&e->value); \
						if (v != null) \
							return v; \
						return readValueUnderLock(e); \
					} \
					e = e->next; \
				} \
			} \
			return null; \
		} \
 \
		boolean containsKey(K key, int hash) { \
			if (count != 0) { \
				sp<HashEntry> e = getFirst(hash); \
				while (e != null) { \
					if (e->hash == hash && e->key == key) \
						return true; \
					e = e->next; \
				} \
			} \
			return false; \
		} \
 \
		boolean containsValue(V* value) { \
			if (count != 0) { \
				ea<HashEntry>* tab = GETRC(table); \
				int len = tab->length(); \
				for (int i = 0 ; i < len; i++) { \
					for (sp<HashEntry> e = tab->atomicGet(i); e != null; e = e->next) { \
						sp<V> v = atomic_load(&e->value); \
						if (v == null) \
							v = readValueUnderLock(e); \
						if (v->equals(value)) { \
							DELRC(tab); \
							return true; \
						} \
					} \
				} \
				DELRC(tab); \
			} \
			return false; \
		} \
 \
		boolean replace(K key, int hash, V* oldValue, sp<V> newValue) { \
			boolean rv; \
 \
			SYNCBLOCK(this) { \
				sp<HashEntry> e = getFirst(hash); \
				while (e != null && (e->hash != hash || e->key != key)) \
					e = e->next; \
 \
				boolean replaced = false; \
				if (e != null && e->value->equals(oldValue)) { \
					replaced = true; \
					e->value = newValue; \
				} \
				rv = replaced; \
            }} \
 \
			return rv; \
		} \
 \
		sp<V> replace(K key, int hash, sp<V> newValue) { \
			sp<V> rv; \
 \
			SYNCBLOCK(this) { \
				sp<HashEntry> e = getFirst(hash); \
				while (e != null && (e->hash != hash || e->key != key)) \
					e = e->next; \
 \
				sp<V> oldValue = null; \
				if (e != null) { \
					oldValue = e->value; \
					e->value = newValue; \
				} \
				rv = oldValue; \
            }} \
 \
			return rv; \
		} \
 \
		sp<V> put(K key, int hash, sp<V>& value, boolean onlyIfAbsent) { \
			SYNCBLOCK(this) { \
				int c = count; \
				if (c++ > threshold) \
					rehash(); \
				ea<HashEntry>* tab = GETRC(table); \
				int index = hash & (tab->length() - 1); \
				sp<HashEntry> first = tab->atomicGet(index); \
				sp<HashEntry> e = first; \
				while (e != null && (e->hash != hash || (e->key != key))) \
					e = e->next; \
 \
				sp<V> oldValue; \
				if (e != null) { \
					oldValue = atomic_load(&e->value); \
					if (!onlyIfAbsent) { \
						atomic_store(&e->value, value); \
					} \
				} \
				else { \
					++modCount; \
					sp<HashEntry> newEntry(new HashEntry(key, hash, first, value)); \
					tab->atomicSet(index, newEntry); \
					count = c; \
				} \
				DELRC(tab); \
				return oldValue; \
            }} \
		} \
 \
		void rehash() { \
			ea<HashEntry>* oldTable = GETRC(table); \
			int oldCapacity = oldTable->length(); \
			if (oldCapacity >= CHM_MAXIMUM_CAPACITY) { \
				DELRC(oldTable); \
				return; \
			} \
 \
			ea<HashEntry>* newTable = HashEntry::newArray(oldCapacity<<1); \
			threshold = (int)(newTable->length() * loadFactor); \
			int sizeMask = newTable->length() - 1; \
			for (int i = 0; i < oldCapacity ; i++) { \
				sp<HashEntry> e = oldTable->atomicGet(i); \
 \
				if (e != null) { \
					sp<HashEntry> next = e->next; \
					int idx = e->hash & sizeMask; \
 \
					if (next == null) \
						(*newTable)[idx] = e; \
 \
					else { \
						sp<HashEntry> lastRun = e; \
						int lastIdx = idx; \
						for (sp<HashEntry> last = next; \
							 last != null; \
							 last = last->next) { \
							int k = last->hash & sizeMask; \
							if (k != lastIdx) { \
								lastIdx = k; \
								lastRun = last; \
							} \
						} \
						(*newTable)[lastIdx] = lastRun; \
 \
						for (sp<HashEntry> p = e; p != lastRun; p = p->next) { \
							int k = p->hash & sizeMask; \
							sp<HashEntry> n = (*newTable)[k]; \
							sp<HashEntry> newEntry(new HashEntry(p->key, p->hash, n, atomic_load(&p->value))); \
							(*newTable)[k] = newEntry; \
						} \
					} \
				} \
			} \
			table = newTable; \
 \
			DELRC(oldTable); \
			DELRC(oldTable); \
		} \
 \
		sp<V> remove(K key, int hash, V* value) { \
			sp<V> rv; \
 \
			SYNCBLOCK(this) { \
				int c = count - 1; \
				ea<HashEntry>* tab = GETRC(table); \
				int index = hash & (tab->length() - 1); \
				sp<HashEntry> first = tab->atomicGet(index); \
				sp<HashEntry> e = first; \
				while (e != null && (e->hash != hash || (key != e->key))) \
					e = e->next; \
 \
				sp<V> oldValue = null; \
				if (e != null) { \
					sp<V> v = atomic_load(&e->value); \
					if (value == null || value->equals(v.get())) { \
						oldValue = v; \
						++modCount; \
						sp<HashEntry> newFirst = e->next; \
						for (sp<HashEntry> p = first; p != e; p = p->next) { \
							newFirst = new HashEntry(p->key, p->hash, \
													newFirst, atomic_load(&p->value)); \
						} \
						tab->atomicSet(index, newFirst); \
						count = c; \
					} \
				} \
				rv = oldValue; \
				DELRC(tab); \
            }} \
 \
			return rv; \
		} \
 \
		void clear() { \
			if (count != 0) { \
				SYNCBLOCK(this) { \
					ea<HashEntry>* tab = GETRC(table); \
					for (int i = 0; i < tab->length() ; i++) { \
						sp<HashEntry> nullPtr; \
						tab->atomicSet(i, nullPtr); \
					} \
					++modCount; \
					count = 0; \
					DELRC(tab); \
                }} \
			} \
		} \
	}; \
 \
	abstract class HashIterator { \
		EConcurrentHashMap<K,V>* chm; \
		int nextSegmentIndex; \
		int nextTableIndex; \
		ea<HashEntry>* currentTable; \
		sp<HashEntry> nextEntry_; \
		sp<HashEntry> lastReturned; \
 \
		void advance() { \
			if (nextEntry_ != null && (nextEntry_ = nextEntry_->next) != null) \
				return; \
 \
			while (nextTableIndex >= 0) { \
				if ( (nextEntry_ = (*currentTable)[nextTableIndex--]) != null) \
					return; \
			} \
 \
			while (nextSegmentIndex >= 0) { \
				Segment* seg = (*chm->segments_)[nextSegmentIndex--]; \
				if (seg->count != 0) { \
					DELRC(currentTable); \
					currentTable = GETRC(seg->table); \
					for (int j = currentTable->length() - 1; j >= 0; --j) { \
						if ( (nextEntry_ = (*currentTable)[j]) != null) { \
							nextTableIndex = j - 1; \
							return; \
						} \
					} \
				} \
			} \
		} \
 \
	public: \
		HashIterator(EConcurrentHashMap<K,V>* chm) : chm(chm) { \
			nextSegmentIndex = chm->segments_->length() - 1; \
			nextTableIndex = -1; \
			currentTable = null; \
			advance(); \
		} \
 \
		virtual ~HashIterator() { \
			DELRC(currentTable); \
		} \
 \
		sp<HashEntry> nextEntry() { \
			if (nextEntry_ == null) \
				throw ENoSuchElementException(__FILE__, __LINE__); \
			lastReturned = nextEntry_; \
			advance(); \
			return lastReturned; \
		} \
 \
		boolean hasMoreElements() { \
			return hasNext(); \
		} \
 \
		boolean hasNext() { \
			return nextEntry_ != null; \
		} \
 \
		void remove() { \
			if (lastReturned == null) \
				throw EIllegalStateException(__FILE__, __LINE__); \
			chm->remove(lastReturned->key); \
			lastReturned = null; \
		} \
	}; \
 \
	class KeyIterator: public HashIterator, public EConcurrentIterator< \
	K>, public EConcurrentEnumeration<K> { \
	public: \
		KeyIterator(EConcurrentHashMap<K,V>* chm) : HashIterator(chm) {} \
		boolean hasMoreElements()    { return HashIterator::hasMoreElements();  } \
		boolean hasNext()            { return HashIterator::hasNext();          } \
		K next()        { return HashIterator::nextEntry()->key; } \
		K nextElement() { return HashIterator::nextEntry()->key; } \
		void remove()                { HashIterator::remove();                  } \
	}; \
 \
	class ValueIterator: public HashIterator, public EConcurrentIterator< \
	V>, public EConcurrentEnumeration<V> { \
	public: \
		ValueIterator(EConcurrentHashMap<K,V>* chm) : HashIterator(chm) {} \
		boolean hasMoreElements()      { return HashIterator::hasMoreElements();  } \
		boolean hasNext()              { return HashIterator::hasNext();          } \
		sp<V> next()        { return HashIterator::nextEntry()->value; } \
		sp<V> nextElement() { return HashIterator::nextEntry()->value; } \
		void remove()                  { HashIterator::remove();                  } \
	}; \
 \
	class WriteThroughEntry: public EConcurrentMapEntry<K, V> { \
	private: \
		K key; \
		sp<V> value; \
		EConcurrentHashMap<K, V>* chm; \
 \
		static boolean eq(EObject* o1, EObject* o2) { \
			return o1 == null ? o2 == null : o1->equals(o2); \
		} \
	public: \
		WriteThroughEntry(K key, sp<V>& value, \
				EConcurrentHashMap<K, V>* chm) : chm(chm) { \
			this->key = key; \
			this->value = value; \
		} \
 \
		K getKey() { \
			return key; \
		} \
 \
		sp<V> getValue() { \
			return value; \
		} \
 \
		sp<V> setValue(sp<V> value) { \
			if (value == null) \
				throw ENullPointerException(__FILE__, __LINE__); \
 \
			sp<V> oldValue = this->value; \
			this->value = value; \
			return oldValue; \
		} \
 \
		boolean equals(sp<EConcurrentMapEntry<K, V> > o) { \
			return (key == o->getKey()) && eq(value.get(), o->getValue().get()); \
		} \
 \
		virtual int hashCode() { \
			return (key) ^ \
					(value == null ? 0 : value->hashCode()); \
		} \
	}; \
 \
	class EntryIterator: public HashIterator, \
	                     public EConcurrentIterator<EConcurrentMapEntry<K, V> > \
	{ \
	private: \
		EConcurrentHashMap<K, V>* chm; \
	public: \
		EntryIterator(EConcurrentHashMap<K, V>* chm) : \
				HashIterator(chm), chm(chm) { \
		} \
		sp<EConcurrentMapEntry<K, V> > next() { \
			sp<HashEntry> e = HashIterator::nextEntry(); \
			return new WriteThroughEntry(e->key, e->value, chm); \
		} \
		boolean hasNext() { \
			return HashIterator::hasNext(); \
		} \
		void remove() { \
			HashIterator::remove(); \
		} \
	}; \
 \
	class KeySet : public EConcurrentSet<K> { \
	private: \
		EConcurrentHashMap<K,V>* chm; \
	public: \
		KeySet(EConcurrentHashMap<K,V>* chm) : chm(chm) { \
		} \
		sp<EConcurrentIterator<K> > iterator() { \
			return new KeyIterator(chm); \
		} \
		int size() { \
			return chm->size(); \
		} \
		boolean isEmpty() { \
			return chm->isEmpty(); \
		} \
		boolean contains(K o) { \
			return chm->containsKey(o); \
		} \
		boolean remove(K o) { \
			return chm->remove(o) != null; \
		} \
		void clear() { \
			chm->clear(); \
		} \
		boolean add(K e) { \
			throw EUnsupportedOperationException(__FILE__, __LINE__); \
		} \
	}; \
 \
	class Values : public EAbstractConcurrentCollection<V> { \
	private: \
		EConcurrentHashMap<K,V>* chm; \
	public: \
		Values(EConcurrentHashMap<K,V>* chm) : chm(chm) { \
		} \
		sp<EConcurrentIterator<V> > iterator() { \
			return new ValueIterator(chm); \
		} \
		int size() { \
			return chm->size(); \
		} \
		boolean isEmpty() { \
			return chm->isEmpty(); \
		} \
		boolean contains(V* o) { \
			return chm->containsValue(o); \
		} \
		void clear() { \
			chm->clear(); \
		} \
	}; \
 \
	class EntrySet : public EConcurrentSet<EConcurrentMapEntry<K,V> > { \
	private: \
		EConcurrentHashMap<K,V>* chm; \
	public: \
		EntrySet(EConcurrentHashMap<K,V>* chm) : chm(chm) { \
		} \
		sp<EConcurrentIterator<EConcurrentMapEntry<K,V> > > iterator() { \
			return new EntryIterator(chm); \
		} \
		boolean contains(EConcurrentMapEntry<K,V>* e) { \
			sp<V> v = chm->get(e->getKey()); \
			return v != null && v->equals(e->getValue().get()); \
		} \
		boolean remove(EConcurrentMapEntry<K,V>* e) { \
			return chm->remove(e->getKey(), e->getValue().get()); \
		} \
		int size() { \
			return chm->size(); \
		} \
		boolean isEmpty() { \
			return chm->isEmpty(); \
		} \
		void clear() { \
			chm->clear(); \
		} \
		boolean add(EConcurrentMapEntry<K,V>* e) { \
			throw EUnsupportedOperationException(__FILE__, __LINE__); \
		} \
		boolean add(sp<EConcurrentMapEntry<K,V> > e) { \
			throw EUnsupportedOperationException(__FILE__, __LINE__); \
		} \
	}; \
 \
public: \
	~EConcurrentHashMap() { \
		delete segments_; \
	} \
 \
	EConcurrentHashMap(int initialCapacity, float loadFactor, int concurrencyLevel) { \
		init(initialCapacity, loadFactor, concurrencyLevel); \
	} \
 \
	EConcurrentHashMap(int initialCapacity, float loadFactor) { \
		init(initialCapacity, loadFactor, CHM_DEFAULT_CONCURRENCY_LEVEL); \
	} \
 \
	EConcurrentHashMap(int initialCapacity) { \
		init(initialCapacity, CHM_DEFAULT_LOAD_FACTOR, CHM_DEFAULT_CONCURRENCY_LEVEL); \
	} \
 \
	EConcurrentHashMap() { \
		init(CHM_DEFAULT_INITIAL_CAPACITY, CHM_DEFAULT_LOAD_FACTOR, CHM_DEFAULT_CONCURRENCY_LEVEL); \
	} \
 \
	EConcurrentHashMap(EMap<K, V>* m) : \
			segmentMask(0), segmentShift(0), segments_(null) { \
		this(EMath::max((int) (m->size() / CHM_DEFAULT_LOAD_FACTOR) + 1, \
						CHM_DEFAULT_INITIAL_CAPACITY), CHM_DEFAULT_LOAD_FACTOR, \
				CHM_DEFAULT_CONCURRENCY_LEVEL); \
		putAll(m); \
	} \
 \
	boolean isEmpty() { \
		EA<Segment*>* segments = this->segments_; \
		int* mc = new int[segments->length()](); \
		int mcsum = 0; \
		for (int i = 0; i < segments->length(); ++i) { \
			if ((*segments)[i]->count != 0) { \
				delete[] mc; \
				return false; \
			} else { \
				mcsum += mc[i] = (*segments)[i]->modCount; \
			} \
		} \
		if (mcsum != 0) { \
			for (int i = 0; i < segments->length(); ++i) { \
				if ((*segments)[i]->count != 0 || \
					mc[i] != (*segments)[i]->modCount) { \
					delete[] mc; \
					return false; \
				} \
			} \
		} \
		delete[] mc; \
		return true; \
	} \
 \
	int size() { \
		EA<Segment*>* segments = this->segments_; \
		long sum = 0; \
		long check = 0; \
		int* mc = new int[segments->length()](); \
		for (int k = 0; k < CHM_RETRIES_BEFORE_LOCK; ++k) { \
			check = 0; \
			sum = 0; \
			int mcsum = 0; \
			for (int i = 0; i < segments->length(); ++i) { \
				sum += (*segments)[i]->count; \
				mcsum += mc[i] = (*segments)[i]->modCount; \
			} \
			if (mcsum != 0) { \
				for (int i = 0; i < segments->length(); ++i) { \
					check += (*segments)[i]->count; \
					if (mc[i] != (*segments)[i]->modCount) { \
						check = -1; \
						break; \
					} \
				} \
			} \
			if (check == sum) \
				break; \
		} \
		if (check != sum) { \
			sum = 0; \
			int i; \
			for (i = 0; i < segments->length(); ++i) \
				(*segments)[i]->lock(); \
			for (i = 0; i < segments->length(); ++i) \
				sum += (*segments)[i]->count; \
			for (i = 0; i < segments->length(); ++i) \
				(*segments)[i]->unlock(); \
		} \
		if (sum > EInteger::MAX_VALUE) { \
			delete[] mc; \
			return EInteger::MAX_VALUE; \
		} \
		else { \
			delete[] mc; \
			return (int)sum; \
		} \
	} \
 \
	sp<V> get(K key) { \
		int hash = hashIt(key); \
		return segmentFor(hash)->get(key, hash); \
	} \
 \
	boolean containsKey(K key) { \
		int hash = hashIt(key); \
		return segmentFor(hash)->containsKey(key, hash); \
	} \
 \
	boolean containsValue(V* value) { \
		if (value == null) \
			throw ENullPointerException(__FILE__, __LINE__); \
 \
		EA<Segment*>* segments = this->segments_; \
		int* mc = new int[segments->length()](); \
 \
		for (int k = 0; k < CHM_RETRIES_BEFORE_LOCK; ++k) { \
			int mcsum = 0; \
			for (int i = 0; i < segments->length(); ++i) { \
				mcsum += mc[i] = (*segments)[i]->modCount; \
				if ((*segments)[i]->containsValue(value)) { \
					delete[] mc; \
					return true; \
				} \
			} \
			boolean cleanSweep = true; \
			if (mcsum != 0) { \
				for (int i = 0; i < segments->length(); ++i) { \
					if (mc[i] != (*segments)[i]->modCount) { \
						cleanSweep = false; \
						break; \
					} \
				} \
			} \
			if (cleanSweep) { \
				delete[] mc; \
				return false; \
			} \
		} \
		for (int i = 0; i < segments->length(); ++i) \
			(*segments)[i]->lock(); \
		boolean found = false; \
		try { \
			for (int i = 0; i < segments->length(); ++i) { \
				if ((*segments)[i]->containsValue(value)) { \
					found = true; \
					break; \
				} \
			} \
		} catch(...) { \
			finally { \
				for (int i = 0; i < segments->length(); ++i) \
					(*segments)[i]->unlock(); \
			} \
			delete[] mc; \
			throw; \
		} finally { \
			for (int i = 0; i < segments->length(); ++i) \
				(*segments)[i]->unlock(); \
		} \
		delete[] mc; \
		return found; \
	} \
 \
	boolean contains(V* value) { \
		return containsValue(value); \
	} \
 \
	sp<V> put(K key, V* value) { \
		K k(key); \
		sp<V> v(value); \
		return put(k, v); \
	} \
	sp<V> put(K key, sp<V> value) { \
		if (value == null) \
			throw ENullPointerException(__FILE__, __LINE__); \
		int hash = hashIt(key); \
		return segmentFor(hash)->put(key, hash, value, false); \
	} \
 \
	sp<V> putIfAbsent(K key, V* value) { \
		K k(key); \
		sp<V> v(value); \
		return putIfAbsent(k, v); \
	} \
	sp<V> putIfAbsent(K key, sp<V> value) { \
		if (value == null) \
			throw ENullPointerException(__FILE__, __LINE__); \
		int hash = hashIt(key); \
		return segmentFor(hash)->put(key, hash, value, true); \
	} \
 \
	void putAll(EMap<K, V*>* m) { \
		sp<EIterator<EMapEntry<K,V*>*> > it = m->entrySet()->iterator(); \
		while (it->hasNext()) { \
			EMapEntry<K,V*>* e = it->next(); \
			put(e->getKey(), e->getValue()); \
		} \
	} \
 \
	sp<V> remove(K key) { \
		int hash = hashIt(key); \
		return segmentFor(hash)->remove(key, hash, null); \
	} \
 \
	boolean remove(K key, V* value) { \
		int hash = hashIt(key); \
		if (value == null) \
			return false; \
		return segmentFor(hash)->remove(key, hash, value) != null; \
	} \
 \
	boolean replace(K key, V* oldValue, V* newValue) { \
		sp<V> nv(newValue); \
		return replace(key, oldValue, nv); \
	} \
	boolean replace(K key, V* oldValue, sp<V> newValue) { \
		if (oldValue == null || newValue == null) \
			throw ENullPointerException(__FILE__, __LINE__); \
		int hash = hashIt(key); \
		return segmentFor(hash)->replace(key, hash, oldValue, newValue); \
	} \
 \
	sp<V> replace(K key, V* value) { \
		sp<V> v(value); \
		return replace(key, v); \
	} \
	sp<V> replace(K key, sp<V> value) { \
		if (value == null) \
			throw ENullPointerException(__FILE__, __LINE__); \
		int hash = hashIt(key); \
		return segmentFor(hash)->replace(key, hash, value); \
	} \
 \
	void clear() { \
		for (int i = 0; i < segments_->length(); ++i) \
			(*segments_)[i]->clear(); \
	} \
 \
	sp<EConcurrentSet<K> > keySet() { \
		if (!keySet_) { \
			keySet_ = new KeySet(this); \
		} \
		return keySet_; \
	} \
 \
	sp<EConcurrentCollection<V> > values() { \
		if (!values_) { \
			values_ = new Values(this); \
		} \
		return values_; \
	} \
 \
	sp<EConcurrentSet<EConcurrentMapEntry<K,V> > > entrySet() { \
		if (!entrySet_) { \
			entrySet_ = new EntrySet(this); \
		} \
		return entrySet_; \
	} \
 \
	sp<EConcurrentEnumeration<K> > keys() { \
		return new KeyIterator(this); \
	} \
 \
	sp<EConcurrentEnumeration<V> > elements() { \
		return new ValueIterator(this); \
	} \
 \
protected: \
	friend class HashIterator; \
	int segmentMask; \
	int segmentShift; \
	EA<Segment*>* segments_; \
	sp<EConcurrentSet<EConcurrentMapEntry<K,V> > > entrySet_; \
	sp<EConcurrentSet<K> > keySet_; \
	sp<EConcurrentCollection<V> > values_; \
 \
private: \
 \
	void init(int initialCapacity, float loadFactor, \
			int concurrencyLevel) \
	{ \
		if (!(loadFactor > 0) || initialCapacity < 0 || concurrencyLevel <= 0) \
			throw EIllegalArgumentException(__FILE__, __LINE__); \
 \
		if (concurrencyLevel > CHM_MAX_SEGMENTS) \
			concurrencyLevel = CHM_MAX_SEGMENTS; \
 \
		int sshift = 0; \
		int ssize = 1; \
		while (ssize < concurrencyLevel) { \
			++sshift; \
			ssize <<= 1; \
		} \
		segmentShift = 32 - sshift; \
		segmentMask = ssize - 1; \
		this->segments_ = Segment::newArray(ssize); \
 \
		if (initialCapacity > CHM_MAXIMUM_CAPACITY) \
			initialCapacity = CHM_MAXIMUM_CAPACITY; \
		int c = initialCapacity / ssize; \
		if (c * ssize < initialCapacity) \
			++c; \
		int cap = 1; \
		while (cap < c) \
			cap <<= 1; \
 \
		for (int i = 0; i < this->segments_->length(); ++i) \
			(*this->segments_)[i] = new Segment(cap, loadFactor, this); \
	} \
 \
	static int hashIt(int h) { \
		h += (h <<  15) ^ 0xffffcd7d; \
		h ^= (((unsigned)h) >> 10); \
		h += (h <<   3); \
		h ^= (((unsigned)h) >>  6); \
		h += (h <<   2) + (h << 14); \
		return h ^ (((unsigned)h) >> 16); \
	} \
 \
	Segment* segmentFor(int hash) { \
		return (*segments_)[(((unsigned)hash) >> segmentShift) & segmentMask]; \
	} \
};

ECHM_DECLARE(byte)
ECHM_DECLARE(char)
ECHM_DECLARE(int)
ECHM_DECLARE(short)
ECHM_DECLARE(long)
ECHM_DECLARE(llong)
ECHM_DECLARE(float)
ECHM_DECLARE(double)

} /* namespace efc */
#endif /* ECONCURRENTHASHMAP_HH_ */
