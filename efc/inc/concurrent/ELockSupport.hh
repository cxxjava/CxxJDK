/*
 * ELockSupport.hh
 *
 *  Created on: 2014-9-25
 *      Author: cxxjava@163.com
 */

#ifndef ELOCKSUPPORT_HH_
#define ELOCKSUPPORT_HH_

#include "../EThread.hh"

namespace efc {

/**
 * Basic thread blocking primitives for creating locks and other
 * synchronization classes.
 *
 * <p>This class associates, with each thread that uses it, a permit
 * (in the sense of the {@link java.util.concurrent.Semaphore
 * Semaphore} class). A call to {@code park} will return immediately
 * if the permit is available, consuming it in the process; otherwise
 * it <em>may</em> block.  A call to {@code unpark} makes the permit
 * available, if it was not already available. (Unlike with Semaphores
 * though, permits do not accumulate. There is at most one.)
 *
 * <p>Methods {@code park} and {@code unpark} provide efficient
 * means of blocking and unblocking threads that do not encounter the
 * problems that cause the deprecated methods {@code Thread.suspend}
 * and {@code Thread.resume} to be unusable for such purposes: Races
 * between one thread invoking {@code park} and another thread trying
 * to {@code unpark} it will preserve liveness, due to the
 * permit. Additionally, {@code park} will return if the caller's
 * thread was interrupted, and timeout versions are supported. The
 * {@code park} method may also return at any other time, for "no
 * reason", so in general must be invoked within a loop that rechecks
 * conditions upon return. In this sense {@code park} serves as an
 * optimization of a "busy wait" that does not waste as much time
 * spinning, but must be paired with an {@code unpark} to be
 * effective.
 *
 * <p>The three forms of {@code park} each also support a
 * {@code blocker} object parameter. This object is recorded while
 * the thread is blocked to permit monitoring and diagnostic tools to
 * identify the reasons that threads are blocked. (Such tools may
 * access blockers using method {@link #getBlocker}.) The use of these
 * forms rather than the original forms without this parameter is
 * strongly encouraged. The normal argument to supply as a
 * {@code blocker} within a lock implementation is {@code this}.
 *
 * <p>These methods are designed to be used as tools for creating
 * higher-level synchronization utilities, and are not in themselves
 * useful for most concurrency control applications.  The {@code park}
 * method is designed for use only in constructions of the form:
 * <pre>while (!canProceed()) { ... LockSupport.park(this); }</pre>
 * where neither {@code canProceed} nor any other actions prior to the
 * call to {@code park} entail locking or blocking.  Because only one
 * permit is associated with each thread, any intermediary uses of
 * {@code park} could interfere with its intended effects.
 *
 * <p><b>Sample Usage.</b> Here is a sketch of a first-in-first-out
 * non-reentrant lock class:
 * <pre>{@code
 * class FIFOMutex {
 *   private final AtomicBoolean locked = new AtomicBoolean(false);
 *   private final Queue<Thread> waiters
 *     = new ConcurrentLinkedQueue<Thread>();
 *
 *   public void lock() {
 *     boolean wasInterrupted = false;
 *     Thread current = Thread.currentThread();
 *     waiters.add(current);
 *
 *     // Block while not first in queue or cannot acquire lock
 *     while (waiters.peek() != current ||
 *            !locked.compareAndSet(false, true)) {
 *        LockSupport.park(this);
 *        if (Thread.interrupted()) // ignore interrupts while waiting
 *          wasInterrupted = true;
 *     }
 *
 *     waiters.remove();
 *     if (wasInterrupted)          // reassert interrupt status on exit
 *        current.interrupt();
 *   }
 *
 *   public void unlock() {
 *     locked.set(false);
 *     LockSupport.unpark(waiters.peek());
 *   }
 * }}</pre>
 */

class ELockSupport: public EObject {
private:
	ELockSupport() {} // Cannot be instantiated.

public:
	/**
	 * Makes available the permit for the given thread, if it
	 * was not already available.  If the thread was blocked on
	 * {@code park} then it will unblock.  Otherwise, its next call
	 * to {@code park} is guaranteed not to block. This operation
	 * is not guaranteed to have any effect at all if the given
	 * thread has not been started.
	 *
	 * @param thread the thread to unpark, or {@code null}, in which case
	 *        this operation has no effect
	 */
	static void unpark(EThread* thread);

	/**
	 * Disables the current thread for thread scheduling purposes unless the
	 * permit is available.
	 *
	 * <p>If the permit is available then it is consumed and the call
	 * returns immediately; otherwise the current thread becomes disabled
	 * for thread scheduling purposes and lies dormant until one of three
	 * things happens:
	 *
	 * <ul>
	 *
	 * <li>Some other thread invokes {@link #unpark unpark} with the
	 * current thread as the target; or
	 *
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * the current thread; or
	 *
	 * <li>The call spuriously (that is, for no reason) returns.
	 * </ul>
	 *
	 * <p>This method does <em>not</em> report which of these caused the
	 * method to return. Callers should re-check the conditions which caused
	 * the thread to park in the first place. Callers may also determine,
	 * for example, the interrupt status of the thread upon return.
	 */
	static void park();

	/**
	 * Disables the current thread for thread scheduling purposes, for up to
	 * the specified waiting time, unless the permit is available.
	 *
	 * <p>If the permit is available then it is consumed and the call
	 * returns immediately; otherwise the current thread becomes disabled
	 * for thread scheduling purposes and lies dormant until one of four
	 * things happens:
	 *
	 * <ul>
	 * <li>Some other thread invokes {@link #unpark unpark} with the
	 * current thread as the target; or
	 *
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * the current thread; or
	 *
	 * <li>The specified waiting time elapses; or
	 *
	 * <li>The call spuriously (that is, for no reason) returns.
	 * </ul>
	 *
	 * <p>This method does <em>not</em> report which of these caused the
	 * method to return. Callers should re-check the conditions which caused
	 * the thread to park in the first place. Callers may also determine,
	 * for example, the interrupt status of the thread, or the elapsed time
	 * upon return.
	 *
	 * @param nanos the maximum number of nanoseconds to wait
	 */
	static void parkNanos(llong nanos);

	/**
	 * Disables the current thread for thread scheduling purposes, until
	 * the specified deadline, unless the permit is available.
	 *
	 * <p>If the permit is available then it is consumed and the call
	 * returns immediately; otherwise the current thread becomes disabled
	 * for thread scheduling purposes and lies dormant until one of four
	 * things happens:
	 *
	 * <ul>
	 * <li>Some other thread invokes {@link #unpark unpark} with the
	 * current thread as the target; or
	 *
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * the current thread; or
	 *
	 * <li>The specified deadline passes; or
	 *
	 * <li>The call spuriously (that is, for no reason) returns.
	 * </ul>
	 *
	 * <p>This method does <em>not</em> report which of these caused the
	 * method to return. Callers should re-check the conditions which caused
	 * the thread to park in the first place. Callers may also determine,
	 * for example, the interrupt status of the thread, or the current time
	 * upon return.
	 *
	 * @param deadline the absolute time, in milliseconds from the Epoch,
	 *        to wait until
	 */
	static void parkUntil(llong deadline);
};

} /* namespace efc */
#endif /* ELOCKSUPPORT_HH_ */
