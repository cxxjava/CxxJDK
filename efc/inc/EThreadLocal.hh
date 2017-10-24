/*
 * EThreadLocal.hh
 *
 *  Created on: 2014-1-26
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADLOCAL_HH_
#define ETHREADLOCAL_HH_

#include "EThread.hh"
#include "ESharedPtr.hh"
#include "./concurrent/EAtomicInteger.hh"

namespace efc {

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

class EInheritableThreadLocal;

class EThreadLocal : public EObject {
public:
	virtual ~EThreadLocal();

	/**
	 * Returns the value in the current thread's copy of this
	 * thread-local variable.  If the variable has no value for the
	 * current thread, it is first initialized to the value returned
	 * by an invocation of the {@link #initialValue} method.
	 *
	 * @return the current thread's value of this thread-local
	 */
	EObject* get();

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
	EObject* set(EObject* value);

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
	void remove();

	/**
	 * Only removes the current thread's value for this thread-local
	 * variable reference.
	 *
	 * @return return old value.
	 */
	EObject* clear();

protected:
	template<typename T, typename V>
	friend class EThreadLocalVariable;

	/**
	 * Creates a thread local variable.
	 */
	EThreadLocal();

	// unsupported.
	EThreadLocal(const EThreadLocal& that);
	EThreadLocal& operator= (const EThreadLocal& that);

	/**
	 * Init reference
	 */
	void init(sp<EThreadLocal> self);

	/**
	 *
	 */
	void uninit();

	/**
	 *
	 */
	EThreadLocal* ref();

	/**
	 * Per-thread map of ThreadLocal instances to values.
	 */
	class Values : public EObject {
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

	public:
		virtual ~Values();

		/**
		 * Constructs a new, empty instance.
		 */
		Values();

		/**
		 * Used for InheritableThreadLocals.
		 */
		Values(Values* fromParent);

		/**
		 * Inherits values from a parent thread.
		 */
		void inheritValues(Values* fromParent);

		/**
		 * Creates a new, empty table with the given capacity.
		 */
		void initializeTable(int capacity);

		/**
		 * Cleans up after garbage-collected thread locals.
		 */
		void cleanUp();

		/**
		 * Rehashes the table, expanding or contracting it as necessary.
		 * Gets rid of tombstones. Returns true if a rehash occurred.
		 * We must rehash every time we fill a null slot; we depend on the
		 * presence of null slots to end searches (otherwise, we'll infinitely
		 * loop).
		 */
		boolean rehash();

		/**
		 * Adds an entry during rehashing. Compared to put(), this method
		 * doesn't have to clean up, check for existing entries, account for
		 * tombstones, etc.
		 */
		void add(EThreadLocal* key, sp<EThreadLocal>* reference, EObject* value);

		/**
		 * Sets entry for given ThreadLocal to given value, creating an
		 * entry if necessary.
		 */
		EObject* put(EThreadLocal* key, EObject* value);

		/**
		 * Gets value for given ThreadLocal after not finding it in the first
		 * slot.
		 */
		EObject* getAfterMiss(EThreadLocal* key);

		/**
		 * Removes entry for the given ThreadLocal.
		 */
		EObject* remove(EThreadLocal* key);

		/**
		 * Gets the next index. If we're at the end of the table, we wrap back
		 * around to 0.
		 */
		int next(int index);
	};

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
	virtual EObject* initialValue();

	/**
	 * Creates Values instance for this thread and variable type.
	 */
	virtual Values* initializeValues(EThread* current);

	/**
	 * Gets Values instance for this thread and variable type.
	 */
	virtual Values* values(EThread* current);

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
	int hash;

	/** Weak reference to this thread local instance. */
	sp<EThreadLocal> reference;
};

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

class EInheritableThreadLocal : public EThreadLocal {
public:
	/**
	 * Computes the initial value of this thread-local variable for the child
	 * thread given the parent thread's value. Called from the parent thread when
	 * creating a child thread. The default implementation returns the parent
	 * thread's value.
	 *
	 * @param parentValue the value of the variable in the parent thread.
	 * @return the initial value of the variable for the child thread.
	 */
	EObject* childValue(EObject* parentValue);

	virtual Values* values(EThread* current);

	virtual Values* initializeValues(EThread* current);
};

template<typename T, typename V>
class EThreadLocalVariable {
public:
	virtual ~EThreadLocalVariable() {
		tli->uninit();
	}

	EThreadLocalVariable() {
		tli = new T();
		tli->init(tli);
	}

	/**
	 * Returns the value in the current thread's copy of this
	 * thread-local variable.  If the variable has no value for the
	 * current thread, it is first initialized to the value returned
	 * by an invocation of the {@link #initialValue} method.
	 *
	 * @return the current thread's value of this thread-local
	 */
	V* get() {
		return dynamic_cast<V*>(tli->get());
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
	V* set(V* value) {
		return dynamic_cast<V*>(tli->set(value));
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
		tli->remove();
	}

	/**
	 * Only removes the current thread's value for this thread-local
	 * variable reference.
	 *
	 * @return return old value.
	 */
	V* clear() {
		return dynamic_cast<V*>(tli->clear());
	}

private:
	sp<T> tli;
};

} /* namespace efc */
#endif /* ETHREADLOCAL_HH_ */
