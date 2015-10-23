/*
 * EThreadLocal.hh
 *
 *  Created on: 2014-1-26
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADLOCAL_HH_
#define ETHREADLOCAL_HH_

#include "EThread.hh"
#include "EReference.hh"
#include "ESharedPtr.hh"
#include "EAtomicInteger.hh"

namespace efc {

//@see: android422/libcore/luni/src/main/java/java/lang/ThreadLocal.java

/**
 * This class provides thread-local variables.  These variables differ from
 * their normal counterparts in that each thread that accesses one (via its
 * <tt>get</tt> or <tt>set</tt> method) has its own, independently initialized
 * copy of the variable.  <tt>ThreadLocal</tt> instances are typically private
 * static fields in classes that wish to associate state with a thread (e.g.,
 * a user ID or Transaction ID).
 *
 * <p>For example, the class below generates unique identifiers local to each
 * thread.
 * A thread's id is assigned the first time it invokes <tt>ThreadId.get()</tt>
 * and remains unchanged on subsequent calls.
 * <pre>
 * import java.util.concurrent.atomic.AtomicInteger;
 *
 * public class ThreadId {
 *     // Atomic integer containing the next thread ID to be assigned
 *     private static final AtomicInteger nextId = new AtomicInteger(0);
 *
 *     // Thread local variable containing each thread's ID
 *     private static final ThreadLocal&lt;Integer> threadId =
 *         new ThreadLocal&lt;Integer>() {
 *             &#64;Override protected Integer initialValue() {
 *                 return nextId.getAndIncrement();
 *         }
 *     };
 *
 *     // Returns the current thread's unique ID, assigning it if necessary
 *     public static int get() {
 *         return threadId.get();
 *     }
 * }
 * </pre>
 * <p>Each thread holds an implicit reference to its copy of a thread-local
 * variable as long as the thread is alive and the <tt>ThreadLocal</tt>
 * instance is accessible; after a thread goes away, all of its copies of
 * thread-local instances are subject to garbage collection (unless other
 * references to these copies exist).
 *
 * @author  Josh Bloch and Doug Lea
 * @since   1.2
 */

/**
 * Size must always be a power of 2.
 */
#define EETHREAD_LOCAL_INITIAL_SIZE 16

/**
 * Placeholder for deleted entries.
 */
#define EETHREAD_LOCAL_TOMBSTONE  (EObject*)0x1

template<typename T>
class EInheritableThreadLocal;

template<typename _T>
class EThreadLocal : public EObject {
private:
	template<typename>
	friend class EInheritableThreadLocal;

	/**
	 * Per-thread map of ThreadLocal instances to values.
	 */
	class Values : public EObject {
	public:
		virtual ~Values() {
			for (int i=0; i<table->length(); i += 2) {
				EObject* k = static_cast<EObject*>((*table)[i]);
				if (k != null && k != EETHREAD_LOCAL_TOMBSTONE) {
					delete k;
				}
				delete (*table)[i+1];
			}
			delete table;
		}

		/**
		 * Constructs a new, empty instance.
		 */
		Values() {
			initializeTable(EETHREAD_LOCAL_INITIAL_SIZE);
			this->size = 0;
			this->tombstones = 0;
		}

		/**
		 * Used for InheritableThreadLocals.
		 */
		Values(Values* fromParent) {
			this->table = fromParent->table->clone();
			this->mask = fromParent->mask;
			this->size = fromParent->size;
			this->tombstones = fromParent->tombstones;
			this->maximumLoad = fromParent->maximumLoad;
			this->clean = fromParent->clean;
			inheritValues(fromParent);
		}

		void check() {
			return; //!

			for (int i=0; i<table->length(); i += 2) {
				EObject* k = static_cast<EObject*>((*table)[i]);
				if (k != null && k != EETHREAD_LOCAL_TOMBSTONE) {
					if ( (*table)[i+1] == null) {
						printf("0 null null null null\n");
					}
				}
				else if (k == null) {
					if ( (*table)[i+1] != null) {
						printf("1 null null null null\n");
					}
				}
			}
		}

	public:
		/**
		 * Map entries. Contains alternating keys (ThreadLocal) and values.
		 * The length is always a power of 2.
		 */
		EA<EObject*>* table;

		/** Used to turn hashes into indices. */
		int mask;

		/** Number of live entries. */
		int size;

		/** Number of tombstones. */
		int tombstones;

		/** Maximum number of live entries and tombstones. */
		int maximumLoad;

		/** Points to the next cell to clean up. */
		int clean;

		/**
		 * Inherits values from a parent thread.
		 */
		void inheritValues(Values* fromParent) {
			// Transfer values from parent to child thread.
			EA<EObject*>* table = this->table;
			for (int i = table->length() - 2; i >= 0; i -= 2) {
				EObject* k = static_cast<EObject*>((*table)[i]);

				if (k == null || k == EETHREAD_LOCAL_TOMBSTONE) {
					// Skip this entry.
					continue;
				}

				// The table can only contain null, tombstones and references.
				sp<EReference<EInheritableThreadLocal<_T>*> >* reference
				                        = (sp<EReference<EInheritableThreadLocal<_T>*> >*) k;
				// Raw type enables us to pass in an Object below.
                EInheritableThreadLocal<_T>* key = (*reference)->get();
				if (key != null) {
					// Replace value with filtered value.
					// We should just let exceptions bubble out and tank
					// the thread creation
					(*table)[i + 1] = static_cast<EObject*>(key->childValue((*fromParent->table)[i + 1]));
				} else {
					delete reference; //!

					// The key was reclaimed.
					(*table)[i] = EETHREAD_LOCAL_TOMBSTONE;
					(*table)[i + 1] = null;
					(*fromParent->table)[i] = EETHREAD_LOCAL_TOMBSTONE;
					(*fromParent->table)[i + 1] = null;

					tombstones++;
					fromParent->tombstones++;

					size--;
					fromParent->size--;
				}
			}
		}

		/**
		 * Creates a new, empty table with the given capacity.
		 */
		void initializeTable(int capacity) {
			this->table = new EA<EObject*>(capacity * 2, false);
			this->mask = table->length() - 1;
			this->clean = 0;
			this->maximumLoad = capacity * 2 / 3; // 2/3
		}

		/**
		 * Cleans up after garbage-collected thread locals.
		 */
		void cleanUp() {
			if (rehash()) {
				// If we rehashed, we needn't clean up (clean up happens as
				// a side effect).
				return;
			}

			if (size == 0) {
				// No live entries == nothing to clean.
				return;
			}

			// Clean log(table.length) entries picking up where we left off
			// last time.
			int index = clean;
			EA<EObject*>* table = this->table;
			for (int counter = table->length(); counter > 0; counter >>= 1,
					index = next(index)) {
				EObject* k = static_cast<EObject*>((*table)[index]);

				if (k == EETHREAD_LOCAL_TOMBSTONE || k == null) {
					continue; // on to next entry
				}

				// The table can only contain null, tombstones and references.
				sp<EReference<EThreadLocal<_T>*> >* reference
										= (sp<EReference<EThreadLocal<_T>*> >*) k;
				if ((*reference)->get() == null) {
					delete reference; //!

					// This thread local was reclaimed by the garbage collector.
					(*table)[index] = EETHREAD_LOCAL_TOMBSTONE;
					delete (*table)[index + 1]; //!!
					(*table)[index + 1] = null;
					tombstones++;
					size--;
				}
			}

			// Point cursor to next index.
			clean = index;
		}

		/**
		 * Rehashes the table, expanding or contracting it as necessary.
		 * Gets rid of tombstones. Returns true if a rehash occurred.
		 * We must rehash every time we fill a null slot; we depend on the
		 * presence of null slots to end searches (otherwise, we'll infinitely
		 * loop).
		 */
		boolean rehash() {
			if (tombstones + size < maximumLoad) {
				return false;
			}

			int capacity = table->length() >> 1;

			// Default to the same capacity. This will create a table of the
			// same size and move over the live entries, analogous to a
			// garbage collection. This should only happen if you churn a
			// bunch of thread local garbage (removing and reinserting
			// the same thread locals over and over will overwrite tombstones
			// and not fill up the table).
			int newCapacity = capacity;

			if (size > (capacity >> 1)) {
				// More than 1/2 filled w/ live entries.
				// Double size.
				newCapacity = capacity * 2;
			}

			EA<EObject*>* oldTable = this->table;

			// Allocate new table.
			initializeTable(newCapacity);

			// We won't have any tombstones after this.
			this->tombstones = 0;

			// If we have no live entries, we can quit here.
			if (size == 0) {
                delete oldTable; //!
				return true;
			}

			// Move over entries.
			for (int i = oldTable->length() - 2; i >= 0; i -= 2) {
				EObject* k = static_cast<EObject*>((*oldTable)[i]);
				if (k == null || k == EETHREAD_LOCAL_TOMBSTONE) {
					// Skip this entry.
					continue;
				}
				
				// The table can only contain null, tombstones and references.
				sp<EReference<EThreadLocal<_T>*> >* reference
										= (sp<EReference<EThreadLocal<_T>*> >*) k;
				EThreadLocal<_T>* key = (*reference)->get();
				if (key == null) {
					// The key was reclaimed.
					delete (*oldTable)[i + 1]; //!
					size--;
				} else {
					// Entry is still live. Move it over.
					add(key, (*oldTable)[i + 1]);
				}
				delete reference; //!
			}

			delete oldTable;
			return true;
		}

		/**
		 * Adds an entry during rehashing. Compared to put(), this method
		 * doesn't have to clean up, check for existing entries, account for
		 * tombstones, etc.
		 */
		void add(EThreadLocal<_T>* key, EObject* value) {
			for (int index = key->hash & mask;; index = next(index)) {
				EObject* k = static_cast<EObject*>((*table)[index]);
				if (k == null) {
					(*table)[index] = new sp<EReference<EThreadLocal<_T>*> >(key->reference);
					(*table)[index + 1] = value;
					return;
				}
			}
		}

		/**
		 * Sets entry for given ThreadLocal to given value, creating an
		 * entry if necessary.
		 */
		_T put(EThreadLocal<_T>* key, EObject* value) {
			cleanUp();

			check();

			// Keep track of first tombstone. That's where we want to go back
			// and add an entry if necessary.
			int firstTombstone = -1;

			for (int index = key->hash & mask;; index = next(index)) {
				EObject* k = static_cast<EObject*>((*table)[index]);

				if (k != null && k != EETHREAD_LOCAL_TOMBSTONE) {
					sp<EReference<EThreadLocal<_T>*> >* reference = (sp<EReference<EThreadLocal<_T>*> >*) k;
					if ((*reference)->get() == key) {
						EObject* v = static_cast<EObject*>((*table)[index + 1]);
						if (v != value) {
//							if (!v) {
//								printf("0 kkkkkkk index=%d\n", index);
//							}
							// Replace existing entry.
							_T oldV = dynamic_cast<_T>(v); //!!
//							if (!oldV) {
//															printf("1 kkkkkkk index=%d\n", index);
//														}
							(*table)[index + 1] = value;

//							EThread* t = EThread::currentThread();
//							printf("2 kkkkkkk t=%p, index=%d, value=%p, oldV=%p\n", t, index, value, oldV);
							return oldV;
						}
						else {
							return null;
						}
					}
				}

				if (k == null) {
					if (firstTombstone == -1) {
						// Fill in null slot.
						(*table)[index] = new sp<EReference<EThreadLocal<_T>*> >(key->reference);
						(*table)[index + 1] = value;
						size++;
						return null;
					}

					// Go back and replace first tombstone.
					(*table)[firstTombstone] = new sp<EReference<EThreadLocal<_T>*> >(key->reference);
					(*table)[firstTombstone + 1] = value;
					tombstones--;
					size++;
					return null;
				}

				// Remember first tombstone.
				if (firstTombstone == -1 && k == EETHREAD_LOCAL_TOMBSTONE) {
					firstTombstone = index;
				}
			}

			throw ERuntimeException(__FILE__, __LINE__);
		}

		/**
		 * Gets value for given ThreadLocal after not finding it in the first
		 * slot.
		 */
		EObject* getAfterMiss(EThreadLocal<_T>* key) {
			EA<EObject*>* table = this->table;
			int index = key->hash & mask;

			check();

			// If the first slot is empty, the search is over.
			if ((*table)[index] == null) {
				EObject* value = static_cast<EObject*>(key->initialValue());

				// If the table is still the same and the slot is still empty...
				if (this->table == table && (*table)[index] == null) {
					(*table)[index] = new sp<EReference<EThreadLocal<_T>*> >(key->reference);
					(*table)[index + 1] = value;
					size++;

					cleanUp();
					return value;
				}

				// The table changed during initialValue().
				put(key, value);
				return value;
			}

			// Keep track of first tombstone. That's where we want to go back
			// and add an entry if necessary.
			int firstTombstone = -1;

			// Continue search.
			for (index = next(index);; index = next(index)) {
				EObject* k = static_cast<EObject*>((*table)[index]);
				if (k && k != EETHREAD_LOCAL_TOMBSTONE) {
						sp<EReference<EThreadLocal<_T>*> >* reference = (sp<EReference<EThreadLocal<_T>*> >*) k;
						if ((*reference)->get() == key) {
							return (*table)[index + 1];
						}
				}

				// If no entry was found...
				if (k == null) {
					EObject* value = static_cast<EObject*>(key->initialValue());

					// If the table is still the same...
					if (this->table == table) {
						// If we passed a tombstone and that slot still
						// contains a tombstone...
						if (firstTombstone > -1
								&& (*table)[firstTombstone] == EETHREAD_LOCAL_TOMBSTONE) {
							(*table)[firstTombstone] = new sp<EReference<EThreadLocal<_T>*> >(key->reference);
							(*table)[firstTombstone + 1] = value;
							tombstones--;
							size++;

							// No need to clean up here. We aren't filling
							// in a null slot.
							return value;
						}

						// If this slot is still empty...
						if ((*table)[index] == null) {
							(*table)[index] = new sp<EReference<EThreadLocal<_T>*> >(key->reference);
							(*table)[index + 1] = value;
							size++;

							cleanUp();
							return value;
						}
					}

					// The table changed during initialValue().
					put(key, value);
					return value;
				}

				if (firstTombstone == -1 && k == EETHREAD_LOCAL_TOMBSTONE) {
					// Keep track of this tombstone so we can overwrite it.
					firstTombstone = index;
				}
			}

			return null;
		}

		/**
		 * Removes entry for the given ThreadLocal.
		 */
		EObject* remove(EThreadLocal<_T>* key) {
			cleanUp();

			check();

			for (int index = key->hash & mask;; index = next(index)) {
				EObject* k = static_cast<EObject*>((*table)[index]);

				if (k != null && k != EETHREAD_LOCAL_TOMBSTONE) {
					sp<EReference<EThreadLocal<_T>*> >* reference = (sp<EReference<EThreadLocal<_T>*> >*) k;
					if ((*reference)->get() == key) {
						delete reference; //!!

						// Success!
						(*table)[index] = EETHREAD_LOCAL_TOMBSTONE;
						EObject* v = (*table)[index + 1];
						(*table)[index + 1] = null;
						tombstones++;
						size--;
						return v;
					}
				}

				if (k == null) {
					// No entry found.
					return null;
				}
			}

			return null;
		}

		/**
		 * Gets the next index. If we're at the end of the table, we wrap back
		 * around to 0.
		 */
		int next(int index) {
			return (index + 2) & mask;
		}
	};

public:
	virtual ~EThreadLocal() {
		reference->clear();
	}

	/**
	 * Creates a thread local variable.
	 */
	EThreadLocal() {
		hash = hashCounter.getAndAdd(0x61c88647 * 2);
		reference = new EReference<EThreadLocal<_T>*>(this);
	}

	/**
	 * Returns the value in the current thread's copy of this
	 * thread-local variable.  If the variable has no value for the
	 * current thread, it is first initialized to the value returned
	 * by an invocation of the {@link #initialValue} method.
	 *
	 * @return the current thread's value of this thread-local
	 */
	_T get() {
		// Optimized for the fast path.
		EThread* t = EThread::currentThread();
		Values* v = this->values(t);
		if (v != null) {
			v->check();
			EA<EObject*>* table = v->table;
			int index = hash & v->mask;
			EObject* k = static_cast<EObject*>((*table)[index]);
			if (k && k != EETHREAD_LOCAL_TOMBSTONE) {
				sp<EReference<EThreadLocal<_T>*> >* reference = (sp<EReference<EThreadLocal<_T>*> >*) k;
				if (this == (*reference)->get()) {
//					if ((*table)[index + 1] == null) {
//						printf("llllllllllllll\n");
//					}
					_T x = dynamic_cast<_T>((*table)[index + 1]);
//					if (x == null) {
//						printf("vvvvvvvvvvv t=%p, index=%d, v=%p\n", t, index, (*table)[index + 1]);
//					}
					return x;
				}
			}
		} else {
			v = initializeValues(t);
		}

		return dynamic_cast<_T>(v->getAfterMiss(this));
	}

	/**
	 * Sets the current thread's copy of this thread-local variable
	 * to the specified value.  Most subclasses will have no need to
	 * override this method, relying solely on the {@link #initialValue}
	 * method to set the values of thread-locals.
	 *
	 * @param value the value to be stored in the current thread's copy of
	 *        this thread-local.
	 * @return return old value.
	 */
	_T set(_T value) {
		EThread* t = EThread::currentThread();
		Values* v = values(t);
		if (v == null) {
			v = initializeValues(t);
		}
		return v->put(this, static_cast<EObject*>(value));
	}

	/**
	 * Removes the current thread's value for this thread-local
	 * variable.  If this thread-local variable is subsequently
	 * {@linkplain #get read} by the current thread, its value will be
	 * reinitialized by invoking its {@link #initialValue} method,
	 * unless its value is {@linkplain #set set} by the current thread
	 * in the interim.  This may result in multiple invocations of the
	 * <tt>initialValue</tt> method in the current thread.
	 *
	 * @since 1.5
	 */
	void remove() {
		EThread* t = EThread::currentThread();
		Values* v = values(t);
		if (v != null) {
			delete (v->remove(this));
		}
	}

	/**
	 * Only removes the current thread's value for this thread-local
	 * variable reference.
	 *
	 * @return return old value.
	 */
	_T clear() {
		EThread* t = EThread::currentThread();
		Values* v = values(t);
		if (v != null) {
			return dynamic_cast<_T>(v->remove(this));
		}
		return null;
	}

protected:
	/**
	 * Returns the current thread's "initial value" for this
	 * thread-local variable.  This method will be invoked the first
	 * time a thread accesses the variable with the {@link #get}
	 * method, unless the thread previously invoked the {@link #set}
	 * method, in which case the <tt>initialValue</tt> method will not
	 * be invoked for the thread.  Normally, this method is invoked at
	 * most once per thread, but it may be invoked again in case of
	 * subsequent invocations of {@link #remove} followed by {@link #get}.
	 *
	 * <p>This implementation simply returns <tt>null</tt>; if the
	 * programmer desires thread-local variables to have an initial
	 * value other than <tt>null</tt>, <tt>ThreadLocal</tt> must be
	 * subclassed, and this method overridden.  Typically, an
	 * anonymous inner class will be used.
	 *
	 * @return the initial value for this thread-local
	 */
	virtual _T initialValue() {
		return null;
	}

	/**
	 * Creates Values instance for this thread and variable type.
	 */
	virtual Values* initializeValues(EThread* current) {
		Values* v = new Values();
		current->localValues = v;
		return v;
	}

	/**
	 * Gets Values instance for this thread and variable type.
	 */
	virtual Values* values(EThread* current) {
		return (Values*)(current->localValues);
	}

private:
	/** Hash counter. */
	static EAtomicInteger hashCounter;

	/**
	 * Internal hash. We deliberately don't bother with #hashCode().
	 * Hashes must be even. This ensures that the result of
	 * (hash & (table.length - 1)) points to a key and not a value.
	 *
	 * We increment by Doug Lea's Magic Number(TM) (*2 since keys are in
	 * every other bucket) to help prevent clustering.
	 */
	int hash;// = hashCounter.getAndAdd(0x61c88647 * 2);

	/** Weak reference to this thread local instance. */
	sp<EReference<EThreadLocal<_T>*> > reference;
};

template<class T> EAtomicInteger EThreadLocal<T>::hashCounter;
//template<class T> const int EThreadLocal<T>::Values::INITIAL_SIZE = 16;
//template<class T> const EObject* EThreadLocal<T>::Values::TOMBSTONE = new EObject();

//=============================================================================

//@see: android422/libcore/luni/src/main/java/java/lang/InheritableThreadLocal.java

/**
 * A thread-local variable whose value is passed from parent to child thread. By
 * default, the value of an inheritable thread-local variable of a child thread
 * is initialized with the value of the parent thread's variable at thread
 * creation time. However, subclasses may override {code #childValue(Object)}
 * to provide an arbitrary function for passing the value of a parent's
 * thread-local variable to the child's thread-local variable.
 *
 * @see java.lang.Thread
 * @see java.lang.ThreadLocal
 */

template<typename T>
class EInheritableThreadLocal : public EThreadLocal<T> {
public:
    /**
     * Creates a new inheritable thread-local variable.
     */
    EInheritableThreadLocal() {
    }

protected:
    typedef typename EThreadLocal<T>::Values Values;

    /**
     * Computes the initial value of this thread-local variable for the child
     * thread given the parent thread's value. Called from the parent thread when
     * creating a child thread. The default implementation returns the parent
     * thread's value.
     *
     * @param parentValue the value of the variable in the parent thread.
     * @return the initial value of the variable for the child thread.
     */
    T childValue(T parentValue) {
        return parentValue;
    }

    Values* values(EThread* current) {
        return current->inheritableValues;
    }

    Values* initializeValues(EThread* current) {
    	Values* v = new Values();
        current->inheritableValues = v;
        return v;
    }
};

} /* namespace efc */
#endif /* ETHREADLOCAL_HH_ */
