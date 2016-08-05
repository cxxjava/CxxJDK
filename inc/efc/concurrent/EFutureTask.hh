/*
 * EFutureTask.hh
 *
 *  Created on: 2015-2-7
 *      Author: cxxjava@163.com
 */

#ifndef EFUTURETASK_HH_
#define EFUTURETASK_HH_

#include "ECallable.hh"
#include "ERunnable.hh"
#include "ERunnableFuture.hh"
#include "EUnsafe.hh"
#include "ESystem.hh"
#include "ELockSupport.hh"
#include "ENullPointerException.hh"
#include "ECancellationException.hh"

namespace efc {

//@see: openjdk-8/src/share/classes/java/util/concurrent/FutureTask.java

/**
 * A cancellable asynchronous computation.  This class provides a base
 * implementation of {@link Future}, with methods to start and cancel
 * a computation, query to see if the computation is complete, and
 * retrieve the result of the computation.  The result can only be
 * retrieved when the computation has completed; the {@code get}
 * methods will block if the computation has not yet completed.  Once
 * the computation has completed, the computation cannot be restarted
 * or cancelled (unless the computation is invoked using
 * {@link #runAndReset}).
 *
 * <p>A {@code FutureTask} can be used to wrap a {@link Callable} or
 * {@link Runnable} object.  Because {@code FutureTask} implements
 * {@code Runnable}, a {@code FutureTask} can be submitted to an
 * {@link Executor} for execution.
 *
 * <p>In addition to serving as a standalone class, this class provides
 * {@code protected} functionality that may be useful when creating
 * customized task classes.
 *
 * @since 1.5
 * @param <V> The result type returned by this FutureTask's {@code get} methods
 */

template<typename V>
class EFutureTask: public ERunnableFuture<V> {
private:
	// Non-public classes supporting the public methods

	/**
	 * A callable that runs given task and returns given result
	 */
	template<typename T>
	class RunnableAdapter : public ECallable<T> {
	private:
		sp<ERunnable> task;
		sp<T> result;
	public:
		RunnableAdapter(sp<ERunnable> task, sp<T> result) {
			this->task = task;
			this->result = result;
		}
		virtual sp<T> call() {
			task->run();
			return result;
		}
	};

protected:
	/**
	 * Simple linked list nodes to record waiting threads in a Treiber
	 * stack.  See other classes such as Phaser and SynchronousQueue
	 * for more detailed explanation.
	 */
	class WaitNode {
	public:
		EThread* volatile thread;
		WaitNode* volatile next;
		WaitNode() : next(null) { thread = EThread::currentThread(); }
	};

public:
	virtual ~EFutureTask(){
		WaitNode* p = waiters;
		while (p) {
			WaitNode* n = p->next;
			delete p; //!
			p = n;
		}
	}

	/*
	 * Revision notes: This differs from previous versions of this
	 * class that relied on AbstractQueuedSynchronizer, mainly to
	 * avoid surprising users about retaining interrupt status during
	 * cancellation races. Sync control in the current design relies
	 * on a "state" field updated via CAS to track completion, along
	 * with a simple Treiber stack to hold waiting threads.
	 *
	 * Style note: As usual, we bypass overhead of using
	 * AtomicXFieldUpdaters and instead directly use Unsafe intrinsics.
	 */

	/**
	 * Creates a {@code FutureTask} that will, upon running, execute the
	 * given {@code Callable}.
	 *
	 * @param  callable the callable task
	 * @throws NullPointerException if the callable is null
	 */
	EFutureTask(sp<ECallable<V> > callable) : runner(null), waiters(null) {
		if (callable == null)
			throw ENullPointerException(__FILE__, __LINE__);
		this->callable = callable;
		this->state = NEW;       // ensure visibility of callable
	}

	/**
	 * Creates a {@code FutureTask} that will, upon running, execute the
	 * given {@code Runnable}, and arrange that {@code get} will return the
	 * given result on successful completion.
	 *
	 * @param runnable the runnable task
	 * @param result the result to return on successful completion. If
	 * you don't need a particular result, consider using
	 * constructions of the form:
	 * {@code Future<?> f = new FutureTask<Void>(runnable, null)}
	 * @throws NullPointerException if the runnable is null
	 */
	EFutureTask(sp<ERunnable> runnable, sp<V> result) : runner(null), waiters(null) {
		//@see: this.callable = Executors.callable(runnable, result);
		/*
		 * this->callable = EExecutors::callable(runnable, result);
		 * Not use this code because of #include "EExecutors" cycling.
		 */
		this->callable = new RunnableAdapter<V>(runnable, result);
		this->state = NEW;       // ensure visibility of callable
	}

	boolean isCancelled() {
		return state >= CANCELLED;
	}

	boolean isDone() {
		return state != NEW;
	}

	boolean cancel(boolean mayInterruptIfRunning) {
		if (!(state == NEW &&
			  EUnsafe::compareAndSwapInt(&this->state, NEW,
				  mayInterruptIfRunning ? INTERRUPTING : CANCELLED)))
			return false;
		try {    // in case call to interrupt throws exception
			if (mayInterruptIfRunning) {
				try {
					EThread *t = runner;
					if (t != null)
						t->interrupt();
				} catch (...) {
					EUnsafe::putOrdered(&this->state, (int)INTERRUPTED);
					throw; //!
				} finally { // final state
					EUnsafe::putOrdered(&this->state, (int)INTERRUPTED);
				}
			}
		} catch (...) {
			finishCompletion();
			throw; //!
		} finally {
			finishCompletion();
		}
		return true;
	}

	/**
	 * @throws CancellationException {@inheritDoc}
	 */
	sp<V> get() THROWS2(EInterruptedException, EExecutionException) {
		int s = state;
		if (s <= COMPLETING)
			s = awaitDone(false, 0L);
		return report(s);
	}

	/**
	 * @throws CancellationException {@inheritDoc}
	 */
	sp<V> get(llong timeout, ETimeUnit *unit)
		THROWS3(EInterruptedException, EExecutionException, ETimeoutException)
	{
		if (unit == null)
			throw ENullPointerException(__FILE__, __LINE__);
		int s = state;
		if (s <= COMPLETING &&
			(s = awaitDone(true, unit->toNanos(timeout))) <= COMPLETING)
			throw ETimeoutException(__FILE__, __LINE__);
		return report(s);
	}

	/**
	 *
	 */
	virtual void run() {
		if (state != NEW ||
			!EUnsafe::compareAndSwapObject(&this->runner,
										 null, EThread::currentThread()))
			return;
		try {
			sp<ECallable<V> > c = callable;
			if (c != null && state == NEW) {
				sp<V> result;
				boolean ran;
				try {
					result = c->call();
					ran = true;
				} catch (EThrowable& ex) {
					result = null;
					ran = false;
					setException(ex);
				}
				if (ran)
					set(result);
			}
		} catch (...) {
			runner = null;
			int s = state;
			if (s >= INTERRUPTING)
				handlePossibleCancellationInterrupt(s);

			throw; //!
		} finally {
			// runner must be non-null until state is settled to
			// prevent concurrent calls to run()
			runner = null;
			// state must be re-read after nulling runner to prevent
			// leaked interrupts
			int s = state;
			if (s >= INTERRUPTING)
				handlePossibleCancellationInterrupt(s);
		}
	}

protected:
	/**
	 * Protected method invoked when this task transitions to state
	 * {@code isDone} (whether normally or via cancellation). The
	 * default implementation does nothing.  Subclasses may override
	 * this method to invoke completion callbacks or perform
	 * bookkeeping. Note that you can query status inside the
	 * implementation of this method to determine whether this task
	 * has been cancelled.
	 */
	virtual void done() { }

	/**
	 * Sets the result of this future to the given value unless
	 * this future has already been set or has been cancelled.
	 *
	 * <p>This method is invoked internally by the {@link #run} method
	 * upon successful completion of the computation.
	 *
	 * @param v the value
	 */
	void set(sp<V> v) {
		if (EUnsafe::compareAndSwapInt(&this->state, NEW, COMPLETING)) {
			outcome = v;
			EUnsafe::putOrdered(&this->state, (int)NORMAL); // final state
			finishCompletion();
		}
	}

	/**
	 * Causes this future to report an {@link ExecutionException}
	 * with the given throwable as its cause, unless this future has
	 * already been set or has been cancelled.
	 *
	 * <p>This method is invoked internally by the {@link #run} method
	 * upon failure of the computation.
	 *
	 * @param t the cause of failure
	 */
	void setException(EThrowable& t) {
		if (EUnsafe::compareAndSwapInt(&this->state, NEW, COMPLETING)) {
			outexception = new EThrowable(t.getSourceFile(), t.getSourceLine(), t.getMessage());
			EUnsafe::putOrdered(&this->state, (int)EXCEPTIONAL); // final state
			finishCompletion();
		}
	}

	/**
	 * Executes the computation without setting its result, and then
	 * resets this future to initial state, failing to do so if the
	 * computation encounters an exception or is cancelled.  This is
	 * designed for use with tasks that intrinsically execute more
	 * than once.
	 *
	 * @return {@code true} if successfully run and reset
	 */
	boolean runAndReset() {
		if (state != NEW ||
			!EUnsafe::compareAndSwapObject(&this->runner,
										 null, EThread::currentThread()))
			return false;
		boolean ran = false;
		int s = state;
		try {
			sp<ECallable<V> > c = callable;
			if (c != null && s == NEW) {
				try {
					c->call(); // don't set result
					ran = true;
				} catch (EThrowable& ex) {
					setException(ex);
				}
			}
		} catch (...) {
			runner = null;
			s = state;
			if (s >= INTERRUPTING)
				handlePossibleCancellationInterrupt(s);

			throw; //!
		} finally {
			// runner must be non-null until state is settled to
			// prevent concurrent calls to run()
			runner = null;
			// state must be re-read after nulling runner to prevent
			// leaked interrupts
			s = state;
			if (s >= INTERRUPTING)
				handlePossibleCancellationInterrupt(s);
		}
		return ran && s == NEW;
	}

private:
	/**
	 * The run state of this task, initially NEW.  The run state
	 * transitions to a terminal state only in methods set,
	 * setException, and cancel.  During completion, state may take on
	 * transient values of COMPLETING (while outcome is being set) or
	 * INTERRUPTING (only while interrupting the runner to satisfy a
	 * cancel(true)). Transitions from these intermediate to final
	 * states use cheaper ordered/lazy writes because values are unique
	 * and cannot be further modified.
	 *
	 * Possible state transitions:
	 * NEW -> COMPLETING -> NORMAL
	 * NEW -> COMPLETING -> EXCEPTIONAL
	 * NEW -> CANCELLED
	 * NEW -> INTERRUPTING -> INTERRUPTED
	 */
	volatile int state;
	enum {
		NEW          = 0,
		COMPLETING   = 1,
		NORMAL       = 2,
		EXCEPTIONAL  = 3,
		CANCELLED    = 4,
		INTERRUPTING = 5,
		INTERRUPTED  = 6
	};

	/** The underlying callable; nulled out after running */
	sp<ECallable<V> > callable;
	/** The result to return or exception to throw from get() */
	sp<V> outcome; // non-volatile, protected by state reads/writes
	sp<EThrowable> outexception;
	/** The thread running the callable; CASed during run() */
	EThread* volatile runner;
	/** Treiber stack of waiting threads */
	WaitNode* volatile waiters;

	/**
	 * Returns result or throws exception for completed task.
	 *
	 * @param s completed state value
	 */
	sp<V> report(int s) THROWS(EExecutionException) {
		sp<V> x = outcome;
		if (s == NORMAL)
			return x;
		if (s >= CANCELLED)
			throw ECancellationException(__FILE__, __LINE__);
		EThrowable* t = outexception.get();
		if (t) {
			throw EExecutionException(t->getSourceFile(), t->getSourceLine(), t->getMessage());
		}
		else {
			throw EExecutionException(__FILE__, __LINE__);
		}
	}

	/**
	 * Ensures that any interrupt from a possible cancel(true) is only
	 * delivered to a task while in run or runAndReset.
	 */
	void handlePossibleCancellationInterrupt(int s) {
		// It is possible for our interrupter to stall before getting a
		// chance to interrupt us.  Let's spin-wait patiently.
		if (s == INTERRUPTING)
			while (state == INTERRUPTING)
				EThread::yield(); // wait out pending interrupt

		// assert state == INTERRUPTED;

		// We want to clear any interrupt we may have received from
		// cancel(true).  However, it is permissible to use interrupts
		// as an independent mechanism for a task to communicate with
		// its caller, and there is no way to clear only the
		// cancellation interrupt.
		//
		// Thread.interrupted();
	}

	/**
     * Removes and signals all waiting threads, invokes done(), and
     * nulls out callable.
     */
    void finishCompletion() {
        // assert state > COMPLETING;
        for (WaitNode* q; (q = waiters) != null;) {
            if (EUnsafe::compareAndSwapObject(&this->waiters, q, null)) {
                for (;;) {
                    EThread* t = q->thread;
                    if (t != null) {
                        q->thread = null;
                        ELockSupport::unpark(t);
                    }
                    WaitNode* next = q->next;
                    if (next == null) {
                    	delete q; //!
                        break;
                    }
                    q->next = null; // unlink to help gc
                    delete q; //!
                    q = next;
                }
                break;
            }
        }

        done();

        //@see: callable = null;        // to reduce footprint
        //moved to ~EFutureTask(){...}
    }

    /**
     * Awaits completion or aborts on interrupt or timeout.
     *
     * @param timed true if use timed waits
     * @param nanos time to wait, if timed
     * @return state upon completion
     */
    int awaitDone(boolean timed, llong nanos)
        THROWS(EInterruptedException) {
        llong deadline = timed ? ESystem::nanoTime() + nanos : 0L;
        WaitNode* q = null;
        boolean queued = false;
        for (;;) {
            if (EThread::interrupted()) {
                removeWaiter(q);
                throw EInterruptedException(__FILE__, __LINE__);
            }

            int s = state;
            if (s > COMPLETING) {
                /* @see:
            	if (q != null)
                    q->thread = null;
                */
            	if (q != null && !queued) { // not in stack yet
            		delete q; //!
            	}
                return s;
            }
            else if (s == COMPLETING) // cannot time out yet
                EThread::yield();
            else if (q == null)
                q = new WaitNode(); //! new
            else if (!queued)
                queued = EUnsafe::compareAndSwapObject(&this->waiters,
                                                     q->next = waiters, q);
            else if (timed) {
                nanos = deadline - ESystem::nanoTime();
                if (nanos <= 0L) {
                	// @see: removeWaiter(q);
                    if (q != null && !queued) { // not in stack yet
						delete q; //!
					}
                    else {
                        removeWaiter(q);
                    }
                    return state;
                }
                ELockSupport::parkNanos(nanos);
            }
            else
                ELockSupport::park();
        }
    }

    /**
     * Tries to unlink a timed-out or interrupted wait node to avoid
     * accumulating garbage.  Internal nodes are simply unspliced
     * without CAS since it is harmless if they are traversed anyway
     * by releasers.  To avoid effects of unsplicing from already
     * removed nodes, the list is retraversed in case of an apparent
     * race.  This is slow when there are a lot of nodes, but we don't
     * expect lists to be long enough to outweigh higher-overhead
     * schemes.
     */
    void removeWaiter(WaitNode* node) {
        if (node != null) {
            node->thread = null;
            retry:
            for (;;) {          // restart on removeWaiter race
                for (WaitNode *pred = null, *q = waiters, *s; q != null; q = s) {
                    s = q->next;
                    if (q->thread != null) {
                        pred = q;
                    }
                    else if (pred != null) {
                        pred->next = s;
                        if (pred->thread == null) // check for race
                            goto retry;
                    }
                    else if (!EUnsafe::compareAndSwapObject(&this->waiters,
                                                          q, s)) {
                    	delete q; //!
                        goto retry;
                    }
                }
                break;
            }
        }
    }
};

} /* namespace efc */
#endif /* EFUTURETASK_HH_ */
