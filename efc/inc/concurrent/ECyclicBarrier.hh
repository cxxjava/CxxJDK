/*
 * ECyclicBarrier.hh
 *
 *  Created on: 2015-2-7
 *      Author: cxxjava@163.com
 */

#ifndef ECYCLICBARRIER_HH_
#define ECYCLICBARRIER_HH_

#include "../ERunnable.hh"
#include "../ETimeUnit.hh"
#include "../ESharedPtr.hh"
#include "./EReentrantLock.hh"
#include "./ETimeoutException.hh"
#include "../EInterruptedException.hh"
#include "./EBrokenBarrierException.hh"

namespace efc {

/**
 * A synchronization aid that allows a set of threads to all wait for
 * each other to reach a common barrier point.  CyclicBarriers are
 * useful in programs involving a fixed sized party of threads that
 * must occasionally wait for each other. The barrier is called
 * <em>cyclic</em> because it can be re-used after the waiting threads
 * are released.
 *
 * <p>A <tt>CyclicBarrier</tt> supports an optional {@link Runnable} command
 * that is run once per barrier point, after the last thread in the party
 * arrives, but before any threads are released.
 * This <em>barrier action</em> is useful
 * for updating shared-state before any of the parties continue.
 *
 * <p><b>Sample usage:</b> Here is an example of
 *  using a barrier in a parallel decomposition design:
 * <pre>
 * class Solver {
 *   final int N;
 *   final float[][] data;
 *   final CyclicBarrier barrier;
 *
 *   class Worker implements Runnable {
 *     int myRow;
 *     Worker(int row) { myRow = row; }
 *     public void run() {
 *       while (!done()) {
 *         processRow(myRow);
 *
 *         try {
 *           barrier.await();
 *         } catch (InterruptedException ex) {
 *           return;
 *         } catch (BrokenBarrierException ex) {
 *           return;
 *         }
 *       }
 *     }
 *   }
 *
 *   public Solver(float[][] matrix) {
 *     data = matrix;
 *     N = matrix.length;
 *     barrier = new CyclicBarrier(N,
 *                                 new Runnable() {
 *                                   public void run() {
 *                                     mergeRows(...);
 *                                   }
 *                                 });
 *     for (int i = 0; i < N; ++i)
 *       new Thread(new Worker(i)).start();
 *
 *     waitUntilDone();
 *   }
 * }
 * </pre>
 * Here, each worker thread processes a row of the matrix then waits at the
 * barrier until all rows have been processed. When all rows are processed
 * the supplied {@link Runnable} barrier action is executed and merges the
 * rows. If the merger
 * determines that a solution has been found then <tt>done()</tt> will return
 * <tt>true</tt> and each worker will terminate.
 *
 * <p>If the barrier action does not rely on the parties being suspended when
 * it is executed, then any of the threads in the party could execute that
 * action when it is released. To facilitate this, each invocation of
 * {@link #await} returns the arrival index of that thread at the barrier.
 * You can then choose which thread should execute the barrier action, for
 * example:
 * <pre>  if (barrier.await() == 0) {
 *     // log the completion of this iteration
 *   }</pre>
 *
 * <p>The <tt>CyclicBarrier</tt> uses an all-or-none breakage model
 * for failed synchronization attempts: If a thread leaves a barrier
 * point prematurely because of interruption, failure, or timeout, all
 * other threads waiting at that barrier point will also leave
 * abnormally via {@link BrokenBarrierException} (or
 * {@link InterruptedException} if they too were interrupted at about
 * the same time).
 *
 * <p>Memory consistency effects: Actions in a thread prior to calling
 * {@code await()}
 * <a href="package-summary.html#MemoryVisibility"><i>happen-before</i></a>
 * actions that are part of the barrier action, which in turn
 * <i>happen-before</i> actions following a successful return from the
 * corresponding {@code await()} in other threads.
 *
 * @since 1.5
 * @see CountDownLatch
 */

class ECyclicBarrier: public EObject {
public:
	virtual ~ECyclicBarrier();

	/**
	 * Creates a new <tt>CyclicBarrier</tt> that will trip when the
	 * given number of parties (threads) are waiting upon it, and which
	 * will execute the given barrier action when the barrier is tripped,
	 * performed by the last thread entering the barrier.
	 *
	 * @param parties the number of threads that must invoke {@link #await}
	 *        before the barrier is tripped
	 * @param barrierAction the command to execute when the barrier is
	 *        tripped, or {@code null} if there is no action
	 * @throws IllegalArgumentException if {@code parties} is less than 1
	 */
	ECyclicBarrier(uint parties, sp<ERunnable> barrierAction);

	/**
	 * Creates a new <tt>CyclicBarrier</tt> that will trip when the
	 * given number of parties (threads) are waiting upon it, and
	 * does not perform a predefined action when the barrier is tripped.
	 *
	 * @param parties the number of threads that must invoke {@link #await}
	 *        before the barrier is tripped
	 * @throws IllegalArgumentException if {@code parties} is less than 1
	 */
	ECyclicBarrier(uint parties);

	/**
	 * Returns the number of parties required to trip this barrier.
	 *
	 * @return the number of parties required to trip this barrier
	 */
	int getParties();

	/**
	 * Waits until all {@linkplain #getParties parties} have invoked
	 * <tt>await</tt> on this barrier.
	 *
	 * <p>If the current thread is not the last to arrive then it is
	 * disabled for thread scheduling purposes and lies dormant until
	 * one of the following things happens:
	 * <ul>
	 * <li>The last thread arrives; or
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * the current thread; or
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * one of the other waiting threads; or
	 * <li>Some other thread times out while waiting for barrier; or
	 * <li>Some other thread invokes {@link #reset} on this barrier.
	 * </ul>
	 *
	 * <p>If the current thread:
	 * <ul>
	 * <li>has its interrupted status set on entry to this method; or
	 * <li>is {@linkplain Thread#interrupt interrupted} while waiting
	 * </ul>
	 * then {@link InterruptedException} is thrown and the current thread's
	 * interrupted status is cleared.
	 *
	 * <p>If the barrier is {@link #reset} while any thread is waiting,
	 * or if the barrier {@linkplain #isBroken is broken} when
	 * <tt>await</tt> is invoked, or while any thread is waiting, then
	 * {@link BrokenBarrierException} is thrown.
	 *
	 * <p>If any thread is {@linkplain Thread#interrupt interrupted} while waiting,
	 * then all other waiting threads will throw
	 * {@link BrokenBarrierException} and the barrier is placed in the broken
	 * state.
	 *
	 * <p>If the current thread is the last thread to arrive, and a
	 * non-null barrier action was supplied in the constructor, then the
	 * current thread runs the action before allowing the other threads to
	 * continue.
	 * If an exception occurs during the barrier action then that exception
	 * will be propagated in the current thread and the barrier is placed in
	 * the broken state.
	 *
	 * @return the arrival index of the current thread, where index
	 *         <tt>{@link #getParties()} - 1</tt> indicates the first
	 *         to arrive and zero indicates the last to arrive
	 * @throws InterruptedException if the current thread was interrupted
	 *         while waiting
	 * @throws BrokenBarrierException if <em>another</em> thread was
	 *         interrupted or timed out while the current thread was
	 *         waiting, or the barrier was reset, or the barrier was
	 *         broken when {@code await} was called, or the barrier
	 *         action (if present) failed due an exception.
	 */
	int await() THROWS2(EInterruptedException, EBrokenBarrierException);

	/**
	 * Waits until all {@linkplain #getParties parties} have invoked
	 * <tt>await</tt> on this barrier, or the specified waiting time elapses.
	 *
	 * <p>If the current thread is not the last to arrive then it is
	 * disabled for thread scheduling purposes and lies dormant until
	 * one of the following things happens:
	 * <ul>
	 * <li>The last thread arrives; or
	 * <li>The specified timeout elapses; or
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * the current thread; or
	 * <li>Some other thread {@linkplain Thread#interrupt interrupts}
	 * one of the other waiting threads; or
	 * <li>Some other thread times out while waiting for barrier; or
	 * <li>Some other thread invokes {@link #reset} on this barrier.
	 * </ul>
	 *
	 * <p>If the current thread:
	 * <ul>
	 * <li>has its interrupted status set on entry to this method; or
	 * <li>is {@linkplain Thread#interrupt interrupted} while waiting
	 * </ul>
	 * then {@link InterruptedException} is thrown and the current thread's
	 * interrupted status is cleared.
	 *
	 * <p>If the specified waiting time elapses then {@link TimeoutException}
	 * is thrown. If the time is less than or equal to zero, the
	 * method will not wait at all.
	 *
	 * <p>If the barrier is {@link #reset} while any thread is waiting,
	 * or if the barrier {@linkplain #isBroken is broken} when
	 * <tt>await</tt> is invoked, or while any thread is waiting, then
	 * {@link BrokenBarrierException} is thrown.
	 *
	 * <p>If any thread is {@linkplain Thread#interrupt interrupted} while
	 * waiting, then all other waiting threads will throw {@link
	 * BrokenBarrierException} and the barrier is placed in the broken
	 * state.
	 *
	 * <p>If the current thread is the last thread to arrive, and a
	 * non-null barrier action was supplied in the constructor, then the
	 * current thread runs the action before allowing the other threads to
	 * continue.
	 * If an exception occurs during the barrier action then that exception
	 * will be propagated in the current thread and the barrier is placed in
	 * the broken state.
	 *
	 * @param timeout the time to wait for the barrier
	 * @param unit the time unit of the timeout parameter
	 * @return the arrival index of the current thread, where index
	 *         <tt>{@link #getParties()} - 1</tt> indicates the first
	 *         to arrive and zero indicates the last to arrive
	 * @throws InterruptedException if the current thread was interrupted
	 *         while waiting
	 * @throws TimeoutException if the specified timeout elapses
	 * @throws BrokenBarrierException if <em>another</em> thread was
	 *         interrupted or timed out while the current thread was
	 *         waiting, or the barrier was reset, or the barrier was broken
	 *         when {@code await} was called, or the barrier action (if
	 *         present) failed due an exception
	 */
	int await(llong timeout, ETimeUnit *unit) THROWS3(EInterruptedException,
			EBrokenBarrierException,
			ETimeoutException);

	/**
	 * Queries if this barrier is in a broken state.
	 *
	 * @return {@code true} if one or more parties broke out of this
	 *         barrier due to interruption or timeout since
	 *         construction or the last reset, or a barrier action
	 *         failed due to an exception; {@code false} otherwise.
	 */
	boolean isBroken();

	/**
	 * Resets the barrier to its initial state.  If any parties are
	 * currently waiting at the barrier, they will return with a
	 * {@link BrokenBarrierException}. Note that resets <em>after</em>
	 * a breakage has occurred for other reasons can be complicated to
	 * carry out; threads need to re-synchronize in some other way,
	 * and choose one to perform the reset.  It may be preferable to
	 * instead create a new barrier for subsequent use.
	 */
	void reset();

	/**
	 * Returns the number of parties currently waiting at the barrier.
	 * This method is primarily useful for debugging and assertions.
	 *
	 * @return the number of parties currently blocked in {@link #await}
	 */
	int getNumberWaiting();

private:
	/**
	 * Each use of the barrier is represented as a generation instance.
	 * The generation changes whenever the barrier is tripped, or
	 * is reset. There can be many generations associated with threads
	 * using the barrier - due to the non-deterministic way the lock
	 * may be allocated to waiting threads - but only one of these
	 * can be active at a time (the one to which <tt>count</tt> applies)
	 * and all the rest are either broken or tripped.
	 * There need not be an active generation if there has been a break
	 * but no subsequent reset.
	 */
	class Generation {
	public:
		boolean broken;// = false;
		Generation() : broken(false) {}
	};

	/** The lock for guarding barrier entry */
	EReentrantLock *lock;// = new ReentrantLock();
	/** Condition to wait on until tripped */
	ECondition *trip;// = lock.newCondition();
	/** The number of parties */
	uint parties;
	/* The command to run when tripped */
	sp<ERunnable> barrierCommand;
	/** The current generation */
	sp<Generation> generation;// = new Generation();

	/**
	 * Number of parties still waiting. Counts down from parties to 0
	 * on each generation.  It is reset to parties on each new
	 * generation or when broken.
	 */
	uint count;

	/**
	 * init
	 */
	void init(uint parties, sp<ERunnable> barrierAction);

	/**
	 * Updates state on barrier trip and wakes up everyone.
	 * Called only while holding lock.
	 */
	void nextGeneration();

	/**
	 * Sets current barrier generation as broken and wakes up everyone.
	 * Called only while holding lock.
	 */
	void breakBarrier();

	/**
	 * Main barrier code, covering the various policies.
	 */
	int dowait(boolean timed, llong nanos) THROWS3(EInterruptedException,
			EBrokenBarrierException,
			ETimeoutException);
};

} /* namespace efc */
#endif /* ECYCLICBARRIER_HH_ */
