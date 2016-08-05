/*
 * ESimpleLock.hh
 *
 *  Created on: 2013-3-18
 *      Author: Administrator
 */

#ifndef ESIMPLELOCK_HH_
#define ESIMPLELOCK_HH_

#include "ELock.hh"
#include "ETimeUnit.hh"
#include "ECondition.hh"

namespace efc {

/**
 * A Simple lock like reentrant lock.
 */

class ESimpleLock : virtual public ELock{
public:
	~ESimpleLock();
	ESimpleLock(int flag=ES_THREAD_MUTEX_DEFAULT);

	//TODO:
	ESimpleLock(const ESimpleLock& that);
	ESimpleLock& operator= (const ESimpleLock& that);

	/**
	 * Acquires the lock.
	 *
	 * <p>Acquires the lock if it is not held by another thread and returns
	 * immediately, setting the lock hold count to one.
	 *
	 * <p>If the current thread already holds the lock then the hold
	 * count is incremented by one and the method returns immediately.
	 *
	 * <p>If the lock is held by another thread then the
	 * current thread becomes disabled for thread scheduling
	 * purposes and lies dormant until the lock has been acquired,
	 * at which time the lock hold count is set to one.
	 */
	void lock();

	/**
	 * Acquires the lock unless the current thread is
	 * {@linkplain Thread#interrupt interrupted}.
	 *
	 * <p>Acquires the lock if it is available and returns immediately.
	 *
	 * <p>If the lock is not available then the current thread becomes
	 * disabled for thread scheduling purposes and lies dormant until
	 * one of two things happens:
	 *
	 * <ul>
	 * <li>The lock is acquired by the current thread; or
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts} the
	 * current thread, and interruption of lock acquisition is supported.
	 * </ul>
	 *
	 * <p>If the current thread:
	 * <ul>
	 * <li>has its interrupted status set on entry to this method; or
	 * <li>is {@linkplain Thread#interrupt interrupted} while acquiring the
	 * lock, and interruption of lock acquisition is supported,
	 * </ul>
	 * then {@link InterruptedException} is thrown and the current thread's
	 * interrupted status is cleared.
	 *
	 * <p><b>Implementation Considerations</b>
	 *
	 * <p>The ability to interrupt a lock acquisition in some
	 * implementations may not be possible, and if possible may be an
	 * expensive operation.  The programmer should be aware that this
	 * may be the case. An implementation should document when this is
	 * the case.
	 *
	 * <p>An implementation can favor responding to an interrupt over
	 * normal method return.
	 *
	 * <p>A {@code Lock} implementation may be able to detect
	 * erroneous use of the lock, such as an invocation that would
	 * cause deadlock, and may throw an (unchecked) exception in such
	 * circumstances.  The circumstances and the exception type must
	 * be documented by that {@code Lock} implementation.
	 *
	 * @throws InterruptedException if the current thread is
	 *         interrupted while acquiring the lock (and interruption
	 *         of lock acquisition is supported).
	 */
	void lockInterruptibly() THROWS(EInterruptedException);

	/**
	 * Acquires the lock only if it is not held by another thread at the time
	 * of invocation.
	 *
	 * <p>Acquires the lock if it is not held by another thread and
	 * returns immediately with the value {@code true}, setting the
	 * lock hold count to one. Even when this lock has been set to use a
	 * fair ordering policy, a call to {@code tryLock()} <em>will</em>
	 * immediately acquire the lock if it is available, whether or not
	 * other threads are currently waiting for the lock.
	 * This &quot;barging&quot; behavior can be useful in certain
	 * circumstances, even though it breaks fairness. If you want to honor
	 * the fairness setting for this lock, then use
	 * {@link #tryLock(long, TimeUnit) tryLock(0, TimeUnit.SECONDS) }
	 * which is almost equivalent (it also detects interruption).
	 *
	 * <p> If the current thread already holds this lock then the hold
	 * count is incremented by one and the method returns {@code true}.
	 *
	 * <p>If the lock is held by another thread then this method will return
	 * immediately with the value {@code false}.
	 *
	 * @return {@code true} if the lock was free and was acquired by the
	 *         current thread, or the lock was already held by the current
	 *         thread; and {@code false} otherwise
	 */
	boolean tryLock();

	/**
	 * Acquires the lock if it is not held by another thread within the given
	 * waiting time and the current thread has not been
	 * {@linkplain Thread#interrupt interrupted}.
	 *
	 * <p>Acquires the lock if it is not held by another thread and returns
	 * immediately with the value {@code true}, setting the lock hold count
	 * to one. If this lock has been set to use a fair ordering policy then
	 * an available lock <em>will not</em> be acquired if any other threads
	 * are waiting for the lock. This is in contrast to the {@link #tryLock()}
	 * method. If you want a timed {@code tryLock} that does permit barging on
	 * a fair lock then combine the timed and un-timed forms together:
	 *
	 * <pre>if (lock.tryLock() || lock.tryLock(timeout, unit) ) { ... }
	 * </pre>
	 *
	 * <p>If the current thread
	 * already holds this lock then the hold count is incremented by one and
	 * the method returns {@code true}.
	 *
	 * <p>If the lock is held by another thread then the
	 * current thread becomes disabled for thread scheduling
	 * purposes and lies dormant until one of three things happens:
	 *
	 * <ul>
	 *
	 * <li>The lock is acquired by the current thread; or
	 *
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * the current thread; or
	 *
	 * <li>The specified waiting time elapses
	 *
	 * </ul>
	 *
	 * <p>If the lock is acquired then the value {@code true} is returned and
	 * the lock hold count is set to one.
	 *
	 * <p>If the current thread:
	 *
	 * <ul>
	 *
	 * <li>has its interrupted status set on entry to this method; or
	 *
	 * <li>is {@linkplain Thread#interrupt interrupted} while
	 * acquiring the lock,
	 *
	 * </ul>
	 * then {@link InterruptedException} is thrown and the current thread's
	 * interrupted status is cleared.
	 *
	 * <p>If the specified waiting time elapses then the value {@code false}
	 * is returned.  If the time is less than or equal to zero, the method
	 * will not wait at all.
	 *
	 * <p>In this implementation, as this method is an explicit
	 * interruption point, preference is given to responding to the
	 * interrupt over normal or reentrant acquisition of the lock, and
	 * over reporting the elapse of the waiting time.
	 *
	 * @param timeout the time to wait for the lock
	 * @param unit the time unit of the timeout argument
	 * @return {@code true} if the lock was free and was acquired by the
	 *         current thread, or the lock was already held by the current
	 *         thread; and {@code false} if the waiting time elapsed before
	 *         the lock could be acquired
	 * @throws NullPointerException if the time unit is null
	 *
	 */
	boolean tryLock(llong timeout, ETimeUnit* unit) THROWS(EInterruptedException);

	/**
	 * Attempts to release this lock.
	 *
	 * <p>If the current thread is the holder of this lock then the hold
	 * count is decremented.  If the hold count is now zero then the lock
	 * is released.  If the current thread is not the holder of this
	 * lock then {@link IllegalMonitorStateException} is thrown.
	 *
	 * @throws IllegalMonitorStateException if the current thread does not
	 *         hold this lock
	 */
	void unlock();

	/**
	 * Returns a {@link Condition} instance for use with this
	 * {@link Lock} instance.
	 *
	 * <p>The returned {@link Condition} instance supports the same
	 * usages as do the {@link Object} monitor methods ({@link
	 * Object#wait() wait}, {@link Object#notify notify}, and {@link
	 * Object#notifyAll notifyAll}) when used with the built-in
	 * monitor lock.
	 *
	 * <ul>
	 *
	 * <li>If this lock is not held when any of the {@link Condition}
	 * {@linkplain Condition#await() waiting} or {@linkplain
	 * Condition#signal signalling} methods are called, then an {@link
	 * IllegalMonitorStateException} is thrown.
	 *
	 * <li>When the condition {@linkplain Condition#await() waiting}
	 * methods are called the lock is released and, before they
	 * return, the lock is reacquired and the lock hold count restored
	 * to what it was when the method was called.
	 *
	 * <li>If a thread is {@linkplain Thread#interrupt interrupted}
	 * while waiting then the wait will terminate, an {@link
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
	ECondition* newCondition();

private:
	es_thread_mutex_t *m_Mutex;
};

} /* namespace efc */
#endif /* ESIMPLELOCK_HH_ */
