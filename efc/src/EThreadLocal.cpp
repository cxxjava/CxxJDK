/*
 * EThreadLocal.cpp
 *
 *  Created on: 2016-5-30
 *      Author: cxxjava@163.com
 */

#include "EThreadLocal.hh"

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

EAtomicInteger EThreadLocal::hashCounter;

EThreadLocal::Values::~Values() {
	for (int i=0; i<table->length(); i += 2) {
		EObject* k = (*table)[i];
		if (k != null && k != EETHREAD_LOCAL_TOMBSTONE) {
			delete k;
		}
		delete (*table)[i+1];
	}
	delete table;
}

EThreadLocal::Values::Values() {
	initializeTable(EETHREAD_LOCAL_INITIAL_SIZE);
	this->size = 0;
	this->tombstones = 0;
}

EThreadLocal::Values::Values(Values* fromParent) {
	this->table = fromParent->table->clone();
	this->mask = fromParent->mask;
	this->size = fromParent->size;
	this->tombstones = fromParent->tombstones;
	this->maximumLoad = fromParent->maximumLoad;
	this->clean = fromParent->clean;
	inheritValues(fromParent);
}

void EThreadLocal::Values::inheritValues(Values* fromParent) {
	// Transfer values from parent to child thread.
	EA<EObject*>* table = this->table;
	for (int i = table->length() - 2; i >= 0; i -= 2) {
		EObject* k = (*table)[i];

		if (k == null || k == EETHREAD_LOCAL_TOMBSTONE) {
			// Skip this entry.
			continue;
		}

		// The table can only contain null, tombstones and references.
		sp<EInheritableThreadLocal>* reference
								= (sp<EInheritableThreadLocal>*) k;
		EInheritableThreadLocal* object = (*reference).get();
		// Raw type enables us to pass in an Object below.
		EInheritableThreadLocal* key = object ? dynamic_cast<EInheritableThreadLocal*>(object->ref()) : null;
		if (key != null) {
			// Replace value with filtered value.
			// We should just let exceptions bubble out and tank
			// the thread creation
			(*table)[i + 1] = key->childValue((*fromParent->table)[i + 1]);
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

void EThreadLocal::Values::initializeTable(int capacity) {
	this->table = new EA<EObject*>(capacity * 2, false);
	this->mask = table->length() - 1;
	this->clean = 0;
	this->maximumLoad = capacity * 2 / 3; // 2/3
}

void EThreadLocal::Values::cleanUp() {
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
		EObject* k = (*table)[index];

		if (k == EETHREAD_LOCAL_TOMBSTONE || k == null) {
			continue; // on to next entry
		}

		// The table can only contain null, tombstones and references.
		sp<EThreadLocal>* reference = (sp<EThreadLocal>*) k;
		EThreadLocal* object = (*reference).get();
		if (object == null || object->ref() == null) {
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

boolean EThreadLocal::Values::rehash() {
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
		EObject* k = (*oldTable)[i];
		if (k == null || k == EETHREAD_LOCAL_TOMBSTONE) {
			// Skip this entry.
			continue;
		}

		// The table can only contain null, tombstones and references.
		sp<EThreadLocal>* reference = (sp<EThreadLocal>*) k;
		EThreadLocal* object = (*reference).get();
		EThreadLocal* key = object ? object->ref() : null;
		if (key == null) {
			delete reference; //!

			// The key was reclaimed.
			delete (*oldTable)[i + 1]; //!
			size--;
		} else {
			// Entry is still live. Move it over.
			add(key, reference, (*oldTable)[i + 1]);
		}
	}

	delete oldTable;
	return true;
}

void EThreadLocal::Values::add(EThreadLocal* key, sp<EThreadLocal >* reference, EObject* value) {
	for (int index = key->hash & mask;; index = next(index)) {
		EObject* k = (*table)[index];
		if (k == null) {
			(*table)[index] = reference;
			(*table)[index + 1] = value;
			return;
		}
	}
}

EObject* EThreadLocal::Values::put(EThreadLocal* key, EObject* value) {
	cleanUp();

	// Keep track of first tombstone. That's where we want to go back
	// and add an entry if necessary.
	int firstTombstone = -1;

	for (int index = key->hash & mask;; index = next(index)) {
		EObject* k = (*table)[index];

		if (k != null && k != EETHREAD_LOCAL_TOMBSTONE) {
			sp<EThreadLocal>& reference = * (sp<EThreadLocal>*) k;
			if (reference.get() && reference->ref() == key) {
				EObject* v = (*table)[index + 1];
				if (v != value) {
					// Replace existing entry.
					EObject* oldV = v; //!!
					(*table)[index + 1] = value;
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
				(*table)[index] = new sp<EThreadLocal>(key->reference);
				(*table)[index + 1] = value;
				size++;
				return null;
			}

			// Go back and replace first tombstone.
			(*table)[firstTombstone] = new sp<EThreadLocal>(key->reference);
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

EObject* EThreadLocal::Values::getAfterMiss(EThreadLocal* key) {
	EA<EObject*>* table = this->table;
	int index = key->hash & mask;

	// If the first slot is empty, the search is over.
	if ((*table)[index] == null) {
		EObject* value = key->initialValue();

		// If the table is still the same and the slot is still empty...
		if (this->table == table && (*table)[index] == null) {
			(*table)[index] = new sp<EThreadLocal>(key->reference);
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
		EObject* k = (*table)[index];
		if (k && k != EETHREAD_LOCAL_TOMBSTONE) {
				sp<EThreadLocal>& reference = * (sp<EThreadLocal>*) k;
				if (reference.get() && reference->ref() == key) {
					return (*table)[index + 1];
				}
		}

		// If no entry was found...
		if (k == null) {
			EObject* value = key->initialValue();

			// If the table is still the same...
			if (this->table == table) {
				// If we passed a tombstone and that slot still
				// contains a tombstone...
				if (firstTombstone > -1
						&& (*table)[firstTombstone] == EETHREAD_LOCAL_TOMBSTONE) {
					(*table)[firstTombstone] = new sp<EThreadLocal>(key->reference);
					(*table)[firstTombstone + 1] = value;
					tombstones--;
					size++;

					// No need to clean up here. We aren't filling
					// in a null slot.
					return value;
				}

				// If this slot is still empty...
				if ((*table)[index] == null) {
					(*table)[index] = new sp<EThreadLocal>(key->reference);
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

EObject* EThreadLocal::Values::remove(EThreadLocal* key) {
	cleanUp();

	for (int index = key->hash & mask;; index = next(index)) {
		EObject* k = (*table)[index];

		if (k != null && k != EETHREAD_LOCAL_TOMBSTONE) {
			sp<EThreadLocal>* reference = (sp<EThreadLocal>*) k;
			EThreadLocal* object = (*reference).get();
			if (object == null || object->ref() == key) {
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

int EThreadLocal::Values::next(int index) {
	return (index + 2) & mask;
}

EThreadLocal::~EThreadLocal() {
	//
}

EThreadLocal::EThreadLocal() {
	hash = hashCounter.getAndAdd(0x61c88647 * 2);
}

void EThreadLocal::init(sp<EThreadLocal> self) {
	reference = self;
}

void EThreadLocal::uninit() {
	reference = null;
}

EThreadLocal* EThreadLocal::ref() {
	return reference.get();
}

EObject* EThreadLocal::get() {
	// Optimized for the fast path.
	EThread* t = EThread::currentThread();
	Values* v = this->values(t);
	if (v != null) {
		EA<EObject*>* table = v->table;
		int index = hash & v->mask;
		EObject* k = (*table)[index];
		if (k && k != EETHREAD_LOCAL_TOMBSTONE) {
			sp<EThreadLocal>& reference = * (sp<EThreadLocal>*) k;
			if (reference.get() && reference->ref() == this) {
				return (*table)[index + 1];
			}
		}
	} else {
		v = initializeValues(t);
	}

	return v->getAfterMiss(this);
}

EObject* EThreadLocal::set(EObject* value) {
	EThread* t = EThread::currentThread();
	Values* v = values(t);
	if (v == null) {
		v = initializeValues(t);
	}
	return v->put(this, value);
}

void EThreadLocal::remove() {
	EThread* t = EThread::currentThread();
	Values* v = values(t);
	if (v != null) {
		delete (v->remove(this));
	}
}

EObject* EThreadLocal::clear() {
	EThread* t = EThread::currentThread();
	Values* v = values(t);
	if (v != null) {
		return v->remove(this);
	}
	return null;
}

EObject* EThreadLocal::initialValue() {
	return null;
}

EThreadLocal::Values* EThreadLocal::initializeValues(EThread* current) {
	Values* v = new Values();
	current->localValues = v;
	return v;
}

EThreadLocal::Values* EThreadLocal::values(EThread* current) {
	return (Values*)(current->localValues);
}

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

EObject* EInheritableThreadLocal::childValue(EObject* parentValue) {
	return parentValue;
}

EThreadLocal::Values* EInheritableThreadLocal::values(EThread* current) {
	return (Values*)(current->inheritableValues);
}

EThreadLocal::Values* EInheritableThreadLocal::initializeValues(EThread* current) {
	Values* v = new Values();
	current->inheritableValues = v;
	return v;
}

} /* namespace efc */
