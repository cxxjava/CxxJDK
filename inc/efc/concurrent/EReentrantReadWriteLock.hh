/*
 * EReentrantReadWriteLock.hh
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#ifndef EREENTRANTREADWRITELOCK_HH_
#define EREENTRANTREADWRITELOCK_HH_

#include "EThread.hh"
#include "ETimeUnit.hh"
#include "ECondition.hh"
#include "ECollection.hh"
#include "EReadWriteLock.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {
	namespace rwlock {
		class Sync;
	}
}

namespace efc {

/**
 * An implementation of {@link ReadWriteLock} supporting similar
 * semantics to {@link ReentrantLock}.
 * <p>This class has the following properties:
 *
 * <ul>
 * <li><b>Acquisition order</b>
 *
 * <p> This class does not impose a reader or writer preference
 * ordering for lock access.  However, it does support an optional
 * <em>fairness</em> policy.
 *
 * <dl>
 * <dt><b><i>Non-fair mode (default)</i></b>
 * <dd>When constructed as non-fair (the default), the order of entry
 * to the read and write lock is unspecified, subject to reentrancy
 * constraints.  A nonfair lock that is continously contended may
 * indefinitely postpone one or more reader or writer threads, but
 * will normally have higher throughput than a fair lock.
 * <p>
 *
 * <dt><b><i>Fair mode</i></b>
 * <dd> When constructed as fair, threads contend for entry using an
 * approximately arrival-order policy. When the currently held lock
 * is released either the longest-waiting single writer thread will
 * be assigned the write lock, or if there is a group of reader threads
 * waiting longer than all waiting writer threads, that group will be
 * assigned the read lock.
 *
 * <p>A thread that tries to acquire a fair read lock (non-reentrantly)
 * will block if either the write lock is held, or there is a waiting
 * writer thread. The thread will not acquire the read lock until
 * after the oldest currently waiting writer thread has acquired and
 * released the write lock. Of course, if a waiting writer abandons
 * its wait, leaving one or more reader threads as the longest waiters
 * in the queue with the write lock free, then those readers will be
 * assigned the read lock.
 *
 * <p>A thread that tries to acquire a fair write lock (non-reentrantly)
 * will block unless both the read lock and write lock are free (which
 * implies there are no waiting threads).  (Note that the non-blocking
 * {@link ReadLock#tryLock()} and {@link WriteLock#tryLock()} methods
 * do not honor this fair setting and will acquire the lock if it is
 * possible, regardless of waiting threads.)
 * <p>
 * </dl>
 *
 * <li><b>Reentrancy</b>
 *
 * <p>This lock allows both readers and writers to reacquire read or
 * write locks in the style of a {@link ReentrantLock}. Non-reentrant
 * readers are not allowed until all write locks held by the writing
 * thread have been released.
 *
 * <p>Additionally, a writer can acquire the read lock, but not
 * vice-versa.  Among other applications, reentrancy can be useful
 * when write locks are held during calls or callbacks to methods that
 * perform reads under read locks.  If a reader tries to acquire the
 * write lock it will never succeed.
 *
 * <li><b>Lock downgrading</b>
 * <p>Reentrancy also allows downgrading from the write lock to a read lock,
 * by acquiring the write lock, then the read lock and then releasing the
 * write lock. However, upgrading from a read lock to the write lock is
 * <b>not</b> possible.
 *
 * <li><b>Interruption of lock acquisition</b>
 * <p>The read lock and write lock both support interruption during lock
 * acquisition.
 *
 * <li><b>{@link Condition} support</b>
 * <p>The write lock provides a {@link Condition} implementation that
 * behaves in the same way, with respect to the write lock, as the
 * {@link Condition} implementation provided by
 * {@link ReentrantLock#newCondition} does for {@link ReentrantLock}.
 * This {@link Condition} can, of course, only be used with the write lock.
 *
 * <p>The read lock does not support a {@link Condition} and
 * {@code readLock().newCondition()} throws
 * {@code UnsupportedOperationException}.
 *
 * <li><b>Instrumentation</b>
 * <p>This class supports methods to determine whether locks
 * are held or contended. These methods are designed for monitoring
 * system state, not for synchronization control.
 * </ul>
 *
 * <p>Serialization of this class behaves in the same way as built-in
 * locks: a deserialized lock is in the unlocked state, regardless of
 * its state when serialized.
 *
 * <p><b>Sample usages</b>. Here is a code sketch showing how to exploit
 * reentrancy to perform lock downgrading after updating a cache (exception
 * handling is elided for simplicity):
 * <pre>
 * class CachedData {
 *   Object data;
 *   volatile boolean cacheValid;
 *   ReentrantReadWriteLock rwl = new ReentrantReadWriteLock();
 *
 *   void processCachedData() {
 *     rwl.readLock().lock();
 *     if (!cacheValid) {
 *        // Must release read lock before acquiring write lock
 *        rwl.readLock().unlock();
 *        rwl.writeLock().lock();
 *        // Recheck state because another thread might have acquired
 *        //   write lock and changed state before we did.
 *        if (!cacheValid) {
 *          data = ...
 *          cacheValid = true;
 *        }
 *        // Downgrade by acquiring read lock before releasing write lock
 *        rwl.readLock().lock();
 *        rwl.writeLock().unlock(); // Unlock write, still hold read
 *     }
 *
 *     use(data);
 *     rwl.readLock().unlock();
 *   }
 * }
 * </pre>
 *
 * ReentrantReadWriteLocks can be used to improve concurrency in some
 * uses of some kinds of Collections. This is typically worthwhile
 * only when the collections are expected to be large, accessed by
 * more reader threads than writer threads, and entail operations with
 * overhead that outweighs synchronization overhead. For example, here
 * is a class using a TreeMap that is expected to be large and
 * concurrently accessed.
 *
 * <pre>{@code
 * class RWDictionary {
 *    private final Map<String, Data> m = new TreeMap<String, Data>();
 *    private final ReentrantReadWriteLock rwl = new ReentrantReadWriteLock();
 *    private final Lock r = rwl.readLock();
 *    private final Lock w = rwl.writeLock();
 *
 *    public Data get(String key) {
 *        r.lock();
 *        try { return m.get(key); }
 *        finally { r.unlock(); }
 *    }
 *    public String[] allKeys() {
 *        r.lock();
 *        try { return m.keySet().toArray(); }
 *        finally { r.unlock(); }
 *    }
 *    public Data put(String key, Data value) {
 *        w.lock();
 *        try { return m.put(key, value); }
 *        finally { w.unlock(); }
 *    }
 *    public void clear() {
 *        w.lock();
 *        try { m.clear(); }
 *        finally { w.unlock(); }
 *    }
 * }}</pre>
 *
 * <h3>Implementation Notes</h3>
 *
 * <p>This lock supports a maximum of 65535 recursive write locks
 * and 65535 read locks. Attempts to exceed these limits result in
 * {@link Error} throws from locking methods.
 */

class EReentrantReadWriteLock : virtual public EReadWriteLock {
public:
	class ReadLock : public ELock {
	protected:
		friend class EReentrantReadWriteLock;
		ReadLock(EReentrantReadWriteLock *lock);

	public:
		~ReadLock();

		/**
		 * Acquires the read lock.
		 *
		 * <p>Acquires the read lock if the write lock is not held by
		 * another thread and returns immediately.
		 *
		 * <p>If the write lock is held by another thread then
		 * the current thread becomes disabled for thread scheduling
		 * purposes and lies dormant until the read lock has been acquired.
		 */
		void lock();

		/**
		 * Acquires the read lock unless the current thread is
		 * {@linkplain Thread#interrupt interrupted}.
		 *
		 * <p>Acquires the read lock if the write lock is not held
		 * by another thread and returns immediately.
		 *
		 * <p>If the write lock is held by another thread then the
		 * current thread becomes disabled for thread scheduling
		 * purposes and lies dormant until one of two things happens:
		 *
		 * <ul>
		 *
		 * <li>The read lock is acquired by the current thread; or
		 *
		 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
		 * the current thread.
		 *
		 * </ul>
		 *
		 * <p>If the current thread:
		 *
		 * <ul>
		 *
		 * <li>has its interrupted status set on entry to this method; or
		 *
		 * <li>is {@linkplain Thread#interrupt interrupted} while
		 * acquiring the read lock,
		 *
		 * </ul>
		 *
		 * then {@link InterruptedException} is thrown and the current
		 * thread's interrupted status is cleared.
		 *
		 * <p>In this implementation, as this method is an explicit
		 * interruption point, preference is given to responding to
		 * the interrupt over normal or reentrant acquisition of the
		 * lock.
		 *
		 * @throws InterruptedException if the current thread is interrupted
		 */
		void lockInterruptibly() THROWS(EInterruptedException);

		/**
		 * Acquires the read lock only if the write lock is not held by
		 * another thread at the time of invocation.
		 *
		 * <p>Acquires the read lock if the write lock is not held by
		 * another thread and returns immediately with the value
		 * {@code true}. Even when this lock has been set to use a
		 * fair ordering policy, a call to {@code tryLock()}
		 * <em>will</em> immediately acquire the read lock if it is
		 * available, whether or not other threads are currently
		 * waiting for the read lock.  This &quot;barging&quot; behavior
		 * can be useful in certain circumstances, even though it
		 * breaks fairness. If you want to honor the fairness setting
		 * for this lock, then use {@link #tryLock(long, TimeUnit)
		 * tryLock(0, TimeUnit.SECONDS) } which is almost equivalent
		 * (it also detects interruption).
		 *
		 * <p>If the write lock is held by another thread then
		 * this method will return immediately with the value
		 * {@code false}.
		 *
		 * @return {@code true} if the read lock was acquired
		 */
		boolean tryLock();

		/**
		 * Acquires the read lock if the write lock is not held by
		 * another thread within the given waiting time and the
		 * current thread has not been {@linkplain Thread#interrupt
		 * interrupted}.
		 *
		 * <p>Acquires the read lock if the write lock is not held by
		 * another thread and returns immediately with the value
		 * {@code true}. If this lock has been set to use a fair
		 * ordering policy then an available lock <em>will not</em> be
		 * acquired if any other threads are waiting for the
		 * lock. This is in contrast to the {@link #tryLock()}
		 * method. If you want a timed {@code tryLock} that does
		 * permit barging on a fair lock then combine the timed and
		 * un-timed forms together:
		 *
		 * <pre>if (lock.tryLock() || lock.tryLock(timeout, unit) ) { ... }
		 * </pre>
		 *
		 * <p>If the write lock is held by another thread then the
		 * current thread becomes disabled for thread scheduling
		 * purposes and lies dormant until one of three things happens:
		 *
		 * <ul>
		 *
		 * <li>The read lock is acquired by the current thread; or
		 *
		 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
		 * the current thread; or
		 *
		 * <li>The specified waiting time elapses.
		 *
		 * </ul>
		 *
		 * <p>If the read lock is acquired then the value {@code true} is
		 * returned.
		 *
		 * <p>If the current thread:
		 *
		 * <ul>
		 *
		 * <li>has its interrupted status set on entry to this method; or
		 *
		 * <li>is {@linkplain Thread#interrupt interrupted} while
		 * acquiring the read lock,
		 *
		 * </ul> then {@link InterruptedException} is thrown and the
		 * current thread's interrupted status is cleared.
		 *
		 * <p>If the specified waiting time elapses then the value
		 * {@code false} is returned.  If the time is less than or
		 * equal to zero, the method will not wait at all.
		 *
		 * <p>In this implementation, as this method is an explicit
		 * interruption point, preference is given to responding to
		 * the interrupt over normal or reentrant acquisition of the
		 * lock, and over reporting the elapse of the waiting time.
		 *
		 * @param timeout the time to wait for the read lock
		 * @param unit the time unit of the timeout argument
		 * @return {@code true} if the read lock was acquired
		 * @throws InterruptedException if the current thread is interrupted
		 * @throws NullPointerException if the time unit is null
		 *
		 */
		boolean tryLock(llong time, ETimeUnit* unit) THROWS(EInterruptedException);

		/**
		 * Attempts to release this lock.
		 *
		 * <p> If the number of readers is now zero then the lock
		 * is made available for write lock attempts.
		 */
		void unlock();

		/**
		 * Throws {@code UnsupportedOperationException} because
		 * {@code ReadLocks} do not support conditions.
		 *
		 * @throws UnsupportedOperationException always
		 */
		ECondition* newCondition() THROWS(EUnsupportedOperationException);

		/**
		 * Returns a string identifying this lock, as well as its lock state.
		 * The state, in brackets, includes the String {@code "Read locks ="}
		 * followed by the number of held read locks.
		 *
		 * @return a string identifying this lock, as well as its lock state
		 */
		virtual EStringBase toString();

	private:
		rwlock::Sync *sync;
	};

	class WriteLock : public ELock {
	protected:
		friend class EReentrantReadWriteLock;
		WriteLock(EReentrantReadWriteLock *lock);

	public:
		~WriteLock();

		/**
		 * Acquires the write lock.
		 *
		 * <p>Acquires the write lock if neither the read nor write lock
		 * are held by another thread
		 * and returns immediately, setting the write lock hold count to
		 * one.
		 *
		 * <p>If the current thread already holds the write lock then the
		 * hold count is incremented by one and the method returns
		 * immediately.
		 *
		 * <p>If the lock is held by another thread then the current
		 * thread becomes disabled for thread scheduling purposes and
		 * lies dormant until the write lock has been acquired, at which
		 * time the write lock hold count is set to one.
		 */
		void lock();

		/**
		 * Acquires the write lock unless the current thread is
		 * {@linkplain Thread#interrupt interrupted}.
		 *
		 * <p>Acquires the write lock if neither the read nor write lock
		 * are held by another thread
		 * and returns immediately, setting the write lock hold count to
		 * one.
		 *
		 * <p>If the current thread already holds this lock then the
		 * hold count is incremented by one and the method returns
		 * immediately.
		 *
		 * <p>If the lock is held by another thread then the current
		 * thread becomes disabled for thread scheduling purposes and
		 * lies dormant until one of two things happens:
		 *
		 * <ul>
		 *
		 * <li>The write lock is acquired by the current thread; or
		 *
		 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
		 * the current thread.
		 *
		 * </ul>
		 *
		 * <p>If the write lock is acquired by the current thread then the
		 * lock hold count is set to one.
		 *
		 * <p>If the current thread:
		 *
		 * <ul>
		 *
		 * <li>has its interrupted status set on entry to this method;
		 * or
		 *
		 * <li>is {@linkplain Thread#interrupt interrupted} while
		 * acquiring the write lock,
		 *
		 * </ul>
		 *
		 * then {@link InterruptedException} is thrown and the current
		 * thread's interrupted status is cleared.
		 *
		 * <p>In this implementation, as this method is an explicit
		 * interruption point, preference is given to responding to
		 * the interrupt over normal or reentrant acquisition of the
		 * lock.
		 *
		 * @throws InterruptedException if the current thread is interrupted
		 */
		void lockInterruptibly() THROWS(EInterruptedException);

		 /**
		 * Acquires the write lock only if it is not held by another thread
		 * at the time of invocation.
		 *
		 * <p>Acquires the write lock if neither the read nor write lock
		 * are held by another thread
		 * and returns immediately with the value {@code true},
		 * setting the write lock hold count to one. Even when this lock has
		 * been set to use a fair ordering policy, a call to
		 * {@code tryLock()} <em>will</em> immediately acquire the
		 * lock if it is available, whether or not other threads are
		 * currently waiting for the write lock.  This &quot;barging&quot;
		 * behavior can be useful in certain circumstances, even
		 * though it breaks fairness. If you want to honor the
		 * fairness setting for this lock, then use {@link
		 * #tryLock(long, TimeUnit) tryLock(0, TimeUnit.SECONDS) }
		 * which is almost equivalent (it also detects interruption).
		 *
		 * <p> If the current thread already holds this lock then the
		 * hold count is incremented by one and the method returns
		 * {@code true}.
		 *
		 * <p>If the lock is held by another thread then this method
		 * will return immediately with the value {@code false}.
		 *
		 * @return {@code true} if the lock was free and was acquired
		 * by the current thread, or the write lock was already held
		 * by the current thread; and {@code false} otherwise.
		 */
		boolean tryLock();

		/**
		 * Acquires the write lock if it is not held by another thread
		 * within the given waiting time and the current thread has
		 * not been {@linkplain Thread#interrupt interrupted}.
		 *
		 * <p>Acquires the write lock if neither the read nor write lock
		 * are held by another thread
		 * and returns immediately with the value {@code true},
		 * setting the write lock hold count to one. If this lock has been
		 * set to use a fair ordering policy then an available lock
		 * <em>will not</em> be acquired if any other threads are
		 * waiting for the write lock. This is in contrast to the {@link
		 * #tryLock()} method. If you want a timed {@code tryLock}
		 * that does permit barging on a fair lock then combine the
		 * timed and un-timed forms together:
		 *
		 * <pre>if (lock.tryLock() || lock.tryLock(timeout, unit) ) { ... }
		 * </pre>
		 *
		 * <p>If the current thread already holds this lock then the
		 * hold count is incremented by one and the method returns
		 * {@code true}.
		 *
		 * <p>If the lock is held by another thread then the current
		 * thread becomes disabled for thread scheduling purposes and
		 * lies dormant until one of three things happens:
		 *
		 * <ul>
		 *
		 * <li>The write lock is acquired by the current thread; or
		 *
		 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
		 * the current thread; or
		 *
		 * <li>The specified waiting time elapses
		 *
		 * </ul>
		 *
		 * <p>If the write lock is acquired then the value {@code true} is
		 * returned and the write lock hold count is set to one.
		 *
		 * <p>If the current thread:
		 *
		 * <ul>
		 *
		 * <li>has its interrupted status set on entry to this method;
		 * or
		 *
		 * <li>is {@linkplain Thread#interrupt interrupted} while
		 * acquiring the write lock,
		 *
		 * </ul>
		 *
		 * then {@link InterruptedException} is thrown and the current
		 * thread's interrupted status is cleared.
		 *
		 * <p>If the specified waiting time elapses then the value
		 * {@code false} is returned.  If the time is less than or
		 * equal to zero, the method will not wait at all.
		 *
		 * <p>In this implementation, as this method is an explicit
		 * interruption point, preference is given to responding to
		 * the interrupt over normal or reentrant acquisition of the
		 * lock, and over reporting the elapse of the waiting time.
		 *
		 * @param timeout the time to wait for the write lock
		 * @param unit the time unit of the timeout argument
		 *
		 * @return {@code true} if the lock was free and was acquired
		 * by the current thread, or the write lock was already held by the
		 * current thread; and {@code false} if the waiting time
		 * elapsed before the lock could be acquired.
		 *
		 * @throws InterruptedException if the current thread is interrupted
		 * @throws NullPointerException if the time unit is null
		 *
		 */
		boolean tryLock(llong timeout, ETimeUnit* unit) THROWS(EInterruptedException);
		/**
		 * Attempts to release this lock.
		 *
		 * <p>If the current thread is the holder of this lock then
		 * the hold count is decremented. If the hold count is now
		 * zero then the lock is released.  If the current thread is
		 * not the holder of this lock then {@link
		 * IllegalMonitorStateException} is thrown.
		 *
		 * @throws IllegalMonitorStateException if the current thread does not
		 * hold this lock.
		 */
		void unlock();

		/**
		 * Returns a {@link Condition} instance for use with this
		 * {@link Lock} instance.
		 * <p>The returned {@link Condition} instance supports the same
		 * usages as do the {@link Object} monitor methods ({@link
		 * Object#wait() wait}, {@link Object#notify notify}, and {@link
		 * Object#notifyAll notifyAll}) when used with the built-in
		 * monitor lock.
		 *
		 * <ul>
		 *
		 * <li>If this write lock is not held when any {@link
		 * Condition} method is called then an {@link
		 * IllegalMonitorStateException} is thrown.  (Read locks are
		 * held independently of write locks, so are not checked or
		 * affected. However it is essentially always an error to
		 * invoke a condition waiting method when the current thread
		 * has also acquired read locks, since other threads that
		 * could unblock it will not be able to acquire the write
		 * lock.)
		 *
		 * <li>When the condition {@linkplain Condition#await() waiting}
		 * methods are called the write lock is released and, before
		 * they return, the write lock is reacquired and the lock hold
		 * count restored to what it was when the method was called.
		 *
		 * <li>If a thread is {@linkplain Thread#interrupt interrupted} while
		 * waiting then the wait will terminate, an {@link
		 * InterruptedException} will be thrown, and the thread's
		 * interrupted status will be cleared.
		 *
		 * <li> Waiting threads are signalled in FIFO order.
		 *
		 * <li>The ordering of lock reacquisition for threads returning
		 * from waiting methods is the same as for threads initially
		 * acquiring the lock, which is in the default case not specified,
		 * but for <em>fair</em> locks favors those threads that have been
		 * waiting the longest.
		 *
		 * </ul>
		 *
		 * @return the Condition object
		 */
		ECondition* newCondition() THROWS(EUnsupportedOperationException);

		/**
		 * Returns a string identifying this lock, as well as its lock
		 * state.  The state, in brackets includes either the String
		 * {@code "Unlocked"} or the String {@code "Locked by"}
		 * followed by the {@linkplain Thread#getName name} of the owning thread.
		 *
		 * @return a string identifying this lock, as well as its lock state
		 */
		virtual EStringBase toString();

		/**
		 * Queries if this write lock is held by the current thread.
		 * Identical in effect to {@link
		 * ReentrantReadWriteLock#isWriteLockedByCurrentThread}.
		 *
		 * @return {@code true} if the current thread holds this lock and
		 *         {@code false} otherwise
		 * @since 1.6
		 */
		boolean isHeldByCurrentThread();

		/**
		 * Queries the number of holds on this write lock by the current
		 * thread.  A thread has a hold on a lock for each lock action
		 * that is not matched by an unlock action.  Identical in effect
		 * to {@link ReentrantReadWriteLock#getWriteHoldCount}.
		 *
		 * @return the number of holds on this lock by the current thread,
		 *         or zero if this lock is not held by the current thread
		 * @since 1.6
		 */
		int getHoldCount();

	private:
		rwlock::Sync *sync;
	};

public:
	~EReentrantReadWriteLock();

	/**
	 * Creates a new {@code ReentrantReadWriteLock} with
	 * default (nonfair) ordering properties.
	 */
	EReentrantReadWriteLock();

	/**
	 * Creates a new {@code ReentrantReadWriteLock} with
	 * the given fairness policy.
	 *
	 * @param fair {@code true} if this lock should use a fair ordering policy
	 */
	EReentrantReadWriteLock(boolean fair);

	EReentrantReadWriteLock::ReadLock* readLock();
	EReentrantReadWriteLock::WriteLock* writeLock();

	// Instrumentation and status

	/**
	 * Returns {@code true} if this lock has fairness set true.
	 *
	 * @return {@code true} if this lock has fairness set true
	 */
	boolean isFair();

	/**
	 * Queries the number of read locks held for this lock. This
	 * method is designed for use in monitoring system state, not for
	 * synchronization control.
	 * @return the number of read locks held.
	 */
	int getReadLockCount();

	/**
	 * Queries if the write lock is held by any thread. This method is
	 * designed for use in monitoring system state, not for
	 * synchronization control.
	 *
	 * @return {@code true} if any thread holds the write lock and
	 *         {@code false} otherwise
	 */
	boolean isWriteLocked();

	/**
	 * Queries if the write lock is held by the current thread.
	 *
	 * @return {@code true} if the current thread holds the write lock and
	 *         {@code false} otherwise
	 */
	boolean isWriteLockedByCurrentThread();

	/**
	 * Queries the number of reentrant write holds on this lock by the
	 * current thread.  A writer thread has a hold on a lock for
	 * each lock action that is not matched by an unlock action.
	 *
	 * @return the number of holds on the write lock by the current thread,
	 *         or zero if the write lock is not held by the current thread
	 */
	int getWriteHoldCount();

	/**
	 * Queries the number of reentrant read holds on this lock by the
	 * current thread.  A reader thread has a hold on a lock for
	 * each lock action that is not matched by an unlock action.
	 *
	 * @return the number of holds on the read lock by the current thread,
	 *         or zero if the read lock is not held by the current thread
	 * @since 1.6
	 */
	int getReadHoldCount();

	/**
	 * Queries whether any threads are waiting to acquire the read or
	 * write lock. Note that because cancellations may occur at any
	 * time, a {@code true} return does not guarantee that any other
	 * thread will ever acquire a lock.  This method is designed
	 * primarily for use in monitoring of the system state.
	 *
	 * @return {@code true} if there may be other threads waiting to
	 *         acquire the lock
	 */
	boolean hasQueuedThreads();

	/**
	 * Queries whether the given thread is waiting to acquire either
	 * the read or write lock. Note that because cancellations may
	 * occur at any time, a {@code true} return does not guarantee
	 * that this thread will ever acquire a lock.  This method is
	 * designed primarily for use in monitoring of the system state.
	 *
	 * @param thread the thread
	 * @return {@code true} if the given thread is queued waiting for this lock
	 * @throws NullPointerException if the thread is null
	 */
	boolean hasQueuedThread(EThread* thread);

	/**
	 * Returns an estimate of the number of threads waiting to acquire
	 * either the read or write lock.  The value is only an estimate
	 * because the number of threads may change dynamically while this
	 * method traverses internal data structures.  This method is
	 * designed for use in monitoring of the system state, not for
	 * synchronization control.
	 *
	 * @return the estimated number of threads waiting for this lock
	 */
	int getQueueLength();

	/**
	 * Queries whether any threads are waiting on the given condition
	 * associated with the write lock. Note that because timeouts and
	 * interrupts may occur at any time, a {@code true} return does
	 * not guarantee that a future {@code signal} will awaken any
	 * threads.  This method is designed primarily for use in
	 * monitoring of the system state.
	 *
	 * @param condition the condition
	 * @return {@code true} if there are any waiting threads
	 * @throws IllegalMonitorStateException if this lock is not held
	 * @throws IllegalArgumentException if the given condition is
	 *         not associated with this lock
	 * @throws NullPointerException if the condition is null
	 */
	boolean hasWaiters(ECondition* condition);

	/**
	 * Returns an estimate of the number of threads waiting on the
	 * given condition associated with the write lock. Note that because
	 * timeouts and interrupts may occur at any time, the estimate
	 * serves only as an upper bound on the actual number of waiters.
	 * This method is designed for use in monitoring of the system
	 * state, not for synchronization control.
	 *
	 * @param condition the condition
	 * @return the estimated number of waiting threads
	 * @throws IllegalMonitorStateException if this lock is not held
	 * @throws IllegalArgumentException if the given condition is
	 *         not associated with this lock
	 * @throws NullPointerException if the condition is null
	 */
	int getWaitQueueLength(ECondition* condition);

	/**
	 * Returns a string identifying this lock, as well as its lock state.
	 * The state, in brackets, includes the String {@code "Write locks ="}
	 * followed by the number of reentrantly held write locks, and the
	 * String {@code "Read locks ="} followed by the number of held
	 * read locks.
	 *
	 * @return a string identifying this lock, as well as its lock state
	 */
	virtual EStringBase toString();

protected:
	/**
	 * Returns the thread that currently owns the write lock, or
	 * {@code null} if not owned. When this method is called by a
	 * thread that is not the owner, the return value reflects a
	 * best-effort approximation of current lock status. For example,
	 * the owner may be momentarily {@code null} even if there are
	 * threads trying to acquire the lock but have not yet done so.
	 * This method is designed to facilitate construction of
	 * subclasses that provide more extensive lock monitoring
	 * facilities.
	 *
	 * @return the owner, or {@code null} if not owned
	 */
	EThread* getOwner();

	/**
	 * Returns a collection containing threads that may be waiting to
	 * acquire the write lock.  Because the actual set of threads may
	 * change dynamically while constructing this result, the returned
	 * collection is only a best-effort estimate.  The elements of the
	 * returned collection are in no particular order.  This method is
	 * designed to facilitate construction of subclasses that provide
	 * more extensive lock monitoring facilities.
	 *
	 * @return the collection of threads
	 */
	ECollection<EThread*>* getQueuedWriterThreads();

	/**
	 * Returns a collection containing threads that may be waiting to
	 * acquire the read lock.  Because the actual set of threads may
	 * change dynamically while constructing this result, the returned
	 * collection is only a best-effort estimate.  The elements of the
	 * returned collection are in no particular order.  This method is
	 * designed to facilitate construction of subclasses that provide
	 * more extensive lock monitoring facilities.
	 *
	 * @return the collection of threads
	 */
	ECollection<EThread*>* getQueuedReaderThreads();

	/**
	 * Returns a collection containing threads that may be waiting to
	 * acquire either the read or write lock.  Because the actual set
	 * of threads may change dynamically while constructing this
	 * result, the returned collection is only a best-effort estimate.
	 * The elements of the returned collection are in no particular
	 * order.  This method is designed to facilitate construction of
	 * subclasses that provide more extensive monitoring facilities.
	 *
	 * @return the collection of threads
	 */
	ECollection<EThread*>* getQueuedThreads();

	/**
	 * Returns a collection containing those threads that may be
	 * waiting on the given condition associated with the write lock.
	 * Because the actual set of threads may change dynamically while
	 * constructing this result, the returned collection is only a
	 * best-effort estimate. The elements of the returned collection
	 * are in no particular order.  This method is designed to
	 * facilitate construction of subclasses that provide more
	 * extensive condition monitoring facilities.
	 *
	 * @param condition the condition
	 * @return the collection of threads
	 * @throws IllegalMonitorStateException if this lock is not held
	 * @throws IllegalArgumentException if the given condition is
	 *         not associated with this lock
	 * @throws NullPointerException if the condition is null
	 */
	ECollection<EThread*>* getWaitingThreads(ECondition* condition);

protected:
	/** Performs all synchronization mechanics */
	rwlock::Sync *sync;

private:
	/** Inner class providing readlock */
	ReadLock  *readerLock;
	/** Inner class providing writelock */
	WriteLock *writerLock;
};

} /* namespace efc */
#endif /* EREENTRANTREADWRITELOCK_HH_ */
