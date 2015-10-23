/*
 * EThreadPoolExecutor.hh
 *
 *  Created on: 2015-3-5
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADPOOLEXECUTOR_HH_
#define ETHREADPOOLEXECUTOR_HH_

#include "ETimeUnit.hh"
#include "ERunnable.hh"
#include "EBlockingQueue.hh"
#include "EList.hh"
#include "ELinkedList.hh"
#include "ETimer.hh"
#include "EInteger.hh"
#include "ESharedArrLst.hh"
#include "EAtomicInteger.hh"
#include "EAtomicCounter.hh"
#include "EThreadFactory.hh"
#include "EAbstractExecutorService.hh"
#include "EAbstractQueuedSynchronizer.hh"
#include "ESecurityException.hh"
#include "ERejectedExecutionHandler.hh"
#include "EInterruptedException.hh"
#include "ERejectedExecutionException.hh"

namespace efc {

namespace tpe {
	class Worker;
	class WorkerKiller;
}

/**
 * An {@link ExecutorService} that executes each submitted task using
 * one of possibly several pooled threads, normally configured
 * using {@link Executors} factory methods.
 *
 * <p>Thread pools address two different problems: they usually
 * provide improved performance when executing large numbers of
 * asynchronous tasks, due to reduced per-task invocation overhead,
 * and they provide a means of bounding and managing the resources,
 * including threads, consumed when executing a collection of tasks.
 * Each {@code ThreadPoolExecutor} also maintains some basic
 * statistics, such as the number of completed tasks.
 *
 * <p>To be useful across a wide range of contexts, this class
 * provides many adjustable parameters and extensibility
 * hooks. However, programmers are urged to use the more convenient
 * {@link Executors} factory methods {@link
 * Executors#newCachedThreadPool} (unbounded thread pool, with
 * automatic thread reclamation), {@link Executors#newFixedThreadPool}
 * (fixed size thread pool) and {@link
 * Executors#newSingleThreadExecutor} (single background thread), that
 * preconfigure settings for the most common usage
 * scenarios. Otherwise, use the following guide when manually
 * configuring and tuning this class:
 *
 * <dl>
 *
 * <dt>Core and maximum pool sizes</dt>
 *
 * <dd>A {@code ThreadPoolExecutor} will automatically adjust the
 * pool size (see {@link #getPoolSize})
 * according to the bounds set by
 * corePoolSize (see {@link #getCorePoolSize}) and
 * maximumPoolSize (see {@link #getMaximumPoolSize}).
 *
 * When a new task is submitted in method {@link #execute}, and fewer
 * than corePoolSize threads are running, a new thread is created to
 * handle the request, even if other worker threads are idle.  If
 * there are more than corePoolSize but less than maximumPoolSize
 * threads running, a new thread will be created only if the queue is
 * full.  By setting corePoolSize and maximumPoolSize the same, you
 * create a fixed-size thread pool. By setting maximumPoolSize to an
 * essentially unbounded value such as {@code Integer.MAX_VALUE}, you
 * allow the pool to accommodate an arbitrary number of concurrent
 * tasks. Most typically, core and maximum pool sizes are set only
 * upon construction, but they may also be changed dynamically using
 * {@link #setCorePoolSize} and {@link #setMaximumPoolSize}. </dd>
 *
 * <dt>On-demand construction</dt>
 *
 * <dd> By default, even core threads are initially created and
 * started only when new tasks arrive, but this can be overridden
 * dynamically using method {@link #prestartCoreThread} or {@link
 * #prestartAllCoreThreads}.  You probably want to prestart threads if
 * you construct the pool with a non-empty queue. </dd>
 *
 * <dt>Creating new threads</dt>
 *
 * <dd>New threads are created using a {@link ThreadFactory}.  If not
 * otherwise specified, a {@link Executors#defaultThreadFactory} is
 * used, that creates threads to all be in the same {@link
 * ThreadGroup} and with the same {@code NORM_PRIORITY} priority and
 * non-daemon status. By supplying a different ThreadFactory, you can
 * alter the thread's name, thread group, priority, daemon status,
 * etc. If a {@code ThreadFactory} fails to create a thread when asked
 * by returning null from {@code newThread}, the executor will
 * continue, but might not be able to execute any tasks. Threads
 * should possess the "modifyThread" {@code RuntimePermission}. If
 * worker threads or other threads using the pool do not possess this
 * permission, service may be degraded: configuration changes may not
 * take effect in a timely manner, and a shutdown pool may remain in a
 * state in which termination is possible but not completed.</dd>
 *
 * <dt>Keep-alive times</dt>
 *
 * <dd>If the pool currently has more than corePoolSize threads,
 * excess threads will be terminated if they have been idle for more
 * than the keepAliveTime (see {@link #getKeepAliveTime}). This
 * provides a means of reducing resource consumption when the pool is
 * not being actively used. If the pool becomes more active later, new
 * threads will be constructed. This parameter can also be changed
 * dynamically using method {@link #setKeepAliveTime}. Using a value
 * of {@code Long.MAX_VALUE} {@link TimeUnit#NANOSECONDS} effectively
 * disables idle threads from ever terminating prior to shut down. By
 * default, the keep-alive policy applies only when there are more
 * than corePoolSizeThreads. But method {@link
 * #allowCoreThreadTimeOut(boolean)} can be used to apply this
 * time-out policy to core threads as well, so long as the
 * keepAliveTime value is non-zero. </dd>
 *
 * <dt>Queuing</dt>
 *
 * <dd>Any {@link BlockingQueue} may be used to transfer and hold
 * submitted tasks.  The use of this queue interacts with pool sizing:
 *
 * <ul>
 *
 * <li> If fewer than corePoolSize threads are running, the Executor
 * always prefers adding a new thread
 * rather than queuing.</li>
 *
 * <li> If corePoolSize or more threads are running, the Executor
 * always prefers queuing a request rather than adding a new
 * thread.</li>
 *
 * <li> If a request cannot be queued, a new thread is created unless
 * this would exceed maximumPoolSize, in which case, the task will be
 * rejected.</li>
 *
 * </ul>
 *
 * There are three general strategies for queuing:
 * <ol>
 *
 * <li> <em> Direct handoffs.</em> A good default choice for a work
 * queue is a {@link SynchronousQueue} that hands off tasks to threads
 * without otherwise holding them. Here, an attempt to queue a task
 * will fail if no threads are immediately available to run it, so a
 * new thread will be constructed. This policy avoids lockups when
 * handling sets of requests that might have internal dependencies.
 * Direct handoffs generally require unbounded maximumPoolSizes to
 * avoid rejection of new submitted tasks. This in turn admits the
 * possibility of unbounded thread growth when commands continue to
 * arrive on average faster than they can be processed.  </li>
 *
 * <li><em> Unbounded queues.</em> Using an unbounded queue (for
 * example a {@link LinkedBlockingQueue} without a predefined
 * capacity) will cause new tasks to wait in the queue when all
 * corePoolSize threads are busy. Thus, no more than corePoolSize
 * threads will ever be created. (And the value of the maximumPoolSize
 * therefore doesn't have any effect.)  This may be appropriate when
 * each task is completely independent of others, so tasks cannot
 * affect each others execution; for example, in a web page server.
 * While this style of queuing can be useful in smoothing out
 * transient bursts of requests, it admits the possibility of
 * unbounded work queue growth when commands continue to arrive on
 * average faster than they can be processed.  </li>
 *
 * <li><em>Bounded queues.</em> A bounded queue (for example, an
 * {@link ArrayBlockingQueue}) helps prevent resource exhaustion when
 * used with finite maximumPoolSizes, but can be more difficult to
 * tune and control.  Queue sizes and maximum pool sizes may be traded
 * off for each other: Using large queues and small pools minimizes
 * CPU usage, OS resources, and context-switching overhead, but can
 * lead to artificially low throughput.  If tasks frequently block (for
 * example if they are I/O bound), a system may be able to schedule
 * time for more threads than you otherwise allow. Use of small queues
 * generally requires larger pool sizes, which keeps CPUs busier but
 * may encounter unacceptable scheduling overhead, which also
 * decreases throughput.  </li>
 *
 * </ol>
 *
 * </dd>
 *
 * <dt>Rejected tasks</dt>
 *
 * <dd> New tasks submitted in method {@link #execute} will be
 * <em>rejected</em> when the Executor has been shut down, and also
 * when the Executor uses finite bounds for both maximum threads and
 * work queue capacity, and is saturated.  In either case, the {@code
 * execute} method invokes the {@link
 * RejectedExecutionHandler#rejectedExecution} method of its {@link
 * RejectedExecutionHandler}.  Four predefined handler policies are
 * provided:
 *
 * <ol>
 *
 * <li> In the default {@link ThreadPoolExecutor.AbortPolicy}, the
 * handler throws a runtime {@link RejectedExecutionException} upon
 * rejection. </li>
 *
 * <li> In {@link ThreadPoolExecutor.CallerRunsPolicy}, the thread
 * that invokes {@code execute} itself runs the task. This provides a
 * simple feedback control mechanism that will slow down the rate that
 * new tasks are submitted. </li>
 *
 * <li> In {@link ThreadPoolExecutor.DiscardPolicy}, a task that
 * cannot be executed is simply dropped.  </li>
 *
 * <li>In {@link ThreadPoolExecutor.DiscardOldestPolicy}, if the
 * executor is not shut down, the task at the head of the work queue
 * is dropped, and then execution is retried (which can fail again,
 * causing this to be repeated.) </li>
 *
 * </ol>
 *
 * It is possible to define and use other kinds of {@link
 * RejectedExecutionHandler} classes. Doing so requires some care
 * especially when policies are designed to work only under particular
 * capacity or queuing policies. </dd>
 *
 * <dt>Hook methods</dt>
 *
 * <dd>This class provides {@code protected} overridable {@link
 * #beforeExecute} and {@link #afterExecute} methods that are called
 * before and after execution of each task.  These can be used to
 * manipulate the execution environment; for example, reinitializing
 * ThreadLocals, gathering statistics, or adding log
 * entries. Additionally, method {@link #terminated} can be overridden
 * to perform any special processing that needs to be done once the
 * Executor has fully terminated.
 *
 * <p>If hook or callback methods throw exceptions, internal worker
 * threads may in turn fail and abruptly terminate.</dd>
 *
 * <dt>Queue maintenance</dt>
 *
 * <dd> Method {@link #getQueue} allows access to the work queue for
 * purposes of monitoring and debugging.  Use of this method for any
 * other purpose is strongly discouraged.  Two supplied methods,
 * {@link #remove} and {@link #purge} are available to assist in
 * storage reclamation when large numbers of queued tasks become
 * cancelled.</dd>
 *
 * <dt>Finalization</dt>
 *
 * <dd> A pool that is no longer referenced in a program <em>AND</em>
 * has no remaining threads will be {@code shutdown} automatically. If
 * you would like to ensure that unreferenced pools are reclaimed even
 * if users forget to call {@link #shutdown}, then you must arrange
 * that unused threads eventually die, by setting appropriate
 * keep-alive times, using a lower bound of zero core threads and/or
 * setting {@link #allowCoreThreadTimeOut(boolean)}.  </dd>
 *
 * </dl>
 *
 * <p> <b>Extension example</b>. Most extensions of this class
 * override one or more of the protected hook methods. For example,
 * here is a subclass that adds a simple pause/resume feature:
 *
 *  <pre> {@code
 * class PausableThreadPoolExecutor extends ThreadPoolExecutor {
 *   private boolean isPaused;
 *   private ReentrantLock pauseLock = new ReentrantLock();
 *   private Condition unpaused = pauseLock.newCondition();
 *
 *   public PausableThreadPoolExecutor(...) { super(...); }
 *
 *   protected void beforeExecute(Thread t, Runnable r) {
 *     super.beforeExecute(t, r);
 *     pauseLock.lock();
 *     try {
 *       while (isPaused) unpaused.await();
 *     } catch (InterruptedException ie) {
 *       t.interrupt();
 *     } finally {
 *       pauseLock.unlock();
 *     }
 *   }
 *
 *   public void pause() {
 *     pauseLock.lock();
 *     try {
 *       isPaused = true;
 *     } finally {
 *       pauseLock.unlock();
 *     }
 *   }
 *
 *   public void resume() {
 *     pauseLock.lock();
 *     try {
 *       isPaused = false;
 *       unpaused.signalAll();
 *     } finally {
 *       pauseLock.unlock();
 *     }
 *   }
 * }}</pre>
 *
 * @since 1.5
 * @author Doug Lea
 */

class EThreadPoolExecutor: public EAbstractExecutorService {
public:
	DECLARE_STATIC_INITZZ;

public:
	 /* Predefined RejectedExecutionHandlers */

	 /**
	  * A handler for rejected tasks that runs the rejected task
	  * directly in the calling thread of the {@code execute} method,
	  * unless the executor has been shut down, in which case the task
	  * is discarded.
	  */
	 struct CallerRunsPolicy : public ERejectedExecutionHandler {
		 /**
		  * Creates a {@code CallerRunsPolicy}.
		  */
		 CallerRunsPolicy() { }

		 /**
		  * Executes task r in the caller's thread, unless the executor
		  * has been shut down, in which case the task is discarded.
		  *
		  * @param r the runnable task requested to be executed
		  * @param e the executor attempting to execute this task
		  */
		 void rejectedExecution(sp<ERunnable> r, EThreadPoolExecutor* e) {
			 if (!e->isShutdown()) {
				 r->run();
			 }
		 }
	 };

	 /**
	  * A handler for rejected tasks that throws a
	  * {@code RejectedExecutionException}.
	  */
	 struct AbortPolicy : public ERejectedExecutionHandler {
		 /**
		  * Creates an {@code AbortPolicy}.
		  */
		 AbortPolicy() { }

		 /**
		  * Always throws RejectedExecutionException.
		  *
		  * @param r the runnable task requested to be executed
		  * @param e the executor attempting to execute this task
		  * @throws RejectedExecutionException always.
		  */
		 void rejectedExecution(sp<ERunnable> r, EThreadPoolExecutor* e) {
			 throw ERejectedExecutionException("Unable to execute task.", __FILE__, __LINE__);
		 }
	 };

	 /**
	  * A handler for rejected tasks that silently discards the
	  * rejected task.
	  */
	 struct DiscardPolicy : public ERejectedExecutionHandler {
		 /**
		  * Creates a {@code DiscardPolicy}.
		  */
		 DiscardPolicy() { }

		 /**
		  * Does nothing, which has the effect of discarding task r.
		  *
		  * @param r the runnable task requested to be executed
		  * @param e the executor attempting to execute this task
		  */
		 void rejectedExecution(sp<ERunnable> r, EThreadPoolExecutor* e) {
			 //
		 }
	 };

	 /**
	  * A handler for rejected tasks that discards the oldest unhandled
	  * request and then retries {@code execute}, unless the executor
	  * is shut down, in which case the task is discarded.
	  */
	 struct DiscardOldestPolicy : public ERejectedExecutionHandler {
		 /**
		  * Creates a {@code DiscardOldestPolicy} for the given executor.
		  */
		 DiscardOldestPolicy() { }

		 /**
		  * Obtains and ignores the next task that the executor
		  * would otherwise execute, if one is immediately available,
		  * and then retries execution of task r, unless the executor
		  * is shut down, in which case task r is instead discarded.
		  *
		  * @param r the runnable task requested to be executed
		  * @param e the executor attempting to execute this task
		  */
		 void rejectedExecution(sp<ERunnable> r, EThreadPoolExecutor* e) {
			 if (!e->isShutdown()) {
				 e->getQueue()->EConcurrentQueue<ERunnable>::poll();
				 e->execute(r);
			 }
		 }
	 };

public:
	 virtual ~EThreadPoolExecutor();

	/**
	 * Creates a new {@code ThreadPoolExecutor} with the given initial
	 * parameters and default thread factory and rejected execution handler.
	 * It may be more convenient to use one of the {@link Executors} factory
	 * methods instead of this general purpose constructor.
	 *
	 * @param corePoolSize the number of threads to keep in the pool, even
	 *        if they are idle, unless {@code allowCoreThreadTimeOut} is set
	 * @param maximumPoolSize the maximum number of threads to allow in the
	 *        pool
	 * @param keepAliveTime when the number of threads is greater than
	 *        the core, this is the maximum time that excess idle threads
	 *        will wait for new tasks before terminating.
	 * @param unit the time unit for the {@code keepAliveTime} argument
	 * @param workQueue the queue to use for holding tasks before they are
	 *        executed.  This queue will hold only the {@code Runnable}
	 *        tasks submitted by the {@code execute} method.
	 * @throws IllegalArgumentException if one of the following holds:<br>
	 *         {@code corePoolSize < 0}<br>
	 *         {@code keepAliveTime < 0}<br>
	 *         {@code maximumPoolSize <= 0}<br>
	 *         {@code maximumPoolSize < corePoolSize}
	 * @throws NullPointerException if {@code workQueue} is null
	 */
	EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
			llong keepAliveTime, ETimeUnit* unit,
			EBlockingQueue<ERunnable>* workQueue);

	/**
	 * Creates a new {@code ThreadPoolExecutor} with the given initial
	 * parameters and default rejected execution handler.
	 *
	 * @param corePoolSize the number of threads to keep in the pool, even
	 *        if they are idle, unless {@code allowCoreThreadTimeOut} is set
	 * @param maximumPoolSize the maximum number of threads to allow in the
	 *        pool
	 * @param keepAliveTime when the number of threads is greater than
	 *        the core, this is the maximum time that excess idle threads
	 *        will wait for new tasks before terminating.
	 * @param unit the time unit for the {@code keepAliveTime} argument
	 * @param workQueue the queue to use for holding tasks before they are
	 *        executed.  This queue will hold only the {@code Runnable}
	 *        tasks submitted by the {@code execute} method.
	 * @param threadFactory the factory to use when the executor
	 *        creates a new thread
	 * @throws IllegalArgumentException if one of the following holds:<br>
	 *         {@code corePoolSize < 0}<br>
	 *         {@code keepAliveTime < 0}<br>
	 *         {@code maximumPoolSize <= 0}<br>
	 *         {@code maximumPoolSize < corePoolSize}
	 * @throws NullPointerException if {@code workQueue}
	 *         or {@code threadFactory} is null
	 */
	EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
			llong keepAliveTime, ETimeUnit* unit,
			EBlockingQueue<ERunnable>* workQueue, EThreadFactory* threadFactory);

	/**
	 * Creates a new {@code ThreadPoolExecutor} with the given initial
	 * parameters and default thread factory.
	 *
	 * @param corePoolSize the number of threads to keep in the pool, even
	 *        if they are idle, unless {@code allowCoreThreadTimeOut} is set
	 * @param maximumPoolSize the maximum number of threads to allow in the
	 *        pool
	 * @param keepAliveTime when the number of threads is greater than
	 *        the core, this is the maximum time that excess idle threads
	 *        will wait for new tasks before terminating.
	 * @param unit the time unit for the {@code keepAliveTime} argument
	 * @param workQueue the queue to use for holding tasks before they are
	 *        executed.  This queue will hold only the {@code Runnable}
	 *        tasks submitted by the {@code execute} method.
	 * @param handler the handler to use when execution is blocked
	 *        because the thread bounds and queue capacities are reached
	 * @throws IllegalArgumentException if one of the following holds:<br>
	 *         {@code corePoolSize < 0}<br>
	 *         {@code keepAliveTime < 0}<br>
	 *         {@code maximumPoolSize <= 0}<br>
	 *         {@code maximumPoolSize < corePoolSize}
	 * @throws NullPointerException if {@code workQueue}
	 *         or {@code handler} is null
	 */
	EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
			llong keepAliveTime, ETimeUnit* unit,
			EBlockingQueue<ERunnable>* workQueue,
			ERejectedExecutionHandler* handler);

	/**
	 * Creates a new {@code ThreadPoolExecutor} with the given initial
	 * parameters.
	 *
	 * @param corePoolSize the number of threads to keep in the pool, even
	 *        if they are idle, unless {@code allowCoreThreadTimeOut} is set
	 * @param maximumPoolSize the maximum number of threads to allow in the
	 *        pool
	 * @param keepAliveTime when the number of threads is greater than
	 *        the core, this is the maximum time that excess idle threads
	 *        will wait for new tasks before terminating.
	 * @param unit the time unit for the {@code keepAliveTime} argument
	 * @param workQueue the queue to use for holding tasks before they are
	 *        executed.  This queue will hold only the {@code Runnable}
	 *        tasks submitted by the {@code execute} method.
	 * @param threadFactory the factory to use when the executor
	 *        creates a new thread
	 * @param handler the handler to use when execution is blocked
	 *        because the thread bounds and queue capacities are reached
	 * @throws IllegalArgumentException if one of the following holds:<br>
	 *         {@code corePoolSize < 0}<br>
	 *         {@code keepAliveTime < 0}<br>
	 *         {@code maximumPoolSize <= 0}<br>
	 *         {@code maximumPoolSize < corePoolSize}
	 * @throws NullPointerException if {@code workQueue}
	 *         or {@code threadFactory} or {@code handler} is null
	 */
	EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
			llong keepAliveTime, ETimeUnit* unit,
			EBlockingQueue<ERunnable>* workQueue, EThreadFactory* threadFactory,
			ERejectedExecutionHandler* handler);

	/**
	 * Executes the given task sometime in the future.  The task
	 * may execute in a new thread or in an existing pooled thread.
	 *
	 * If the task cannot be submitted for execution, either because this
	 * executor has been shutdown or because its capacity has been reached,
	 * the task is handled by the current {@code RejectedExecutionHandler}.
	 *
	 * @param command the task to execute
	 * @throws RejectedExecutionException at discretion of
	 *         {@code RejectedExecutionHandler}, if the task
	 *         cannot be accepted for execution
	 * @throws NullPointerException if {@code command} is null
	 */
	void execute(sp<ERunnable> command);

	/**
	 * Initiates an orderly shutdown in which previously submitted
	 * tasks are executed, but no new tasks will be accepted.
	 * Invocation has no additional effect if already shut down.
	 *
	 * <p>This method does not wait for previously submitted tasks to
	 * complete execution.  Use {@link #awaitTermination awaitTermination}
	 * to do that.
	 *
	 * @throws SecurityException {@inheritDoc}
	 */
	void shutdown();

	/**
	 * Attempts to stop all actively executing tasks, halts the
	 * processing of waiting tasks, and returns a list of the tasks
	 * that were awaiting execution. These tasks are drained (removed)
	 * from the task queue upon return from this method.
	 *
	 * <p>This method does not wait for actively executing tasks to
	 * terminate.  Use {@link #awaitTermination awaitTermination} to
	 * do that.
	 *
	 * <p>There are no guarantees beyond best-effort attempts to stop
	 * processing actively executing tasks.  This implementation
	 * cancels tasks via {@link Thread#interrupt}, so any task that
	 * fails to respond to interrupts may never terminate.
	 *
	 * @throws SecurityException {@inheritDoc}
	 */
	eal<ERunnable> shutdownNow();

	boolean isShutdown();

	/**
	 * Returns true if this executor is in the process of terminating
	 * after {@link #shutdown} or {@link #shutdownNow} but has not
	 * completely terminated.  This method may be useful for
	 * debugging. A return of {@code true} reported a sufficient
	 * period after shutdown may indicate that submitted tasks have
	 * ignored or suppressed interruption, causing this executor not
	 * to properly terminate.
	 *
	 * @return true if terminating but not yet terminated
	 */
	boolean isTerminating();

	boolean isTerminated();

	boolean awaitTermination() THROWS(EInterruptedException);
	boolean awaitTermination(llong timeout, ETimeUnit* unit) THROWS(EInterruptedException);

	/**
	 * Sets the thread factory used to create new threads.
	 *
	 * @param threadFactory the new thread factory
	 * @throws NullPointerException if threadFactory is null
	 * @see #getThreadFactory
	 */
	void setThreadFactory(EThreadFactory* threadFactory);

	/**
	 * Returns the thread factory used to create new threads.
	 *
	 * @return the current thread factory
	 * @see #setThreadFactory
	 */
	EThreadFactory* getThreadFactory();

	/**
	 * Sets a new handler for unexecutable tasks.
	 *
	 * @param handler the new handler
	 * @throws NullPointerException if handler is null
	 * @see #getRejectedExecutionHandler
	 */
	void setRejectedExecutionHandler(ERejectedExecutionHandler* handler);

	/**
	 * Returns the current handler for unexecutable tasks.
	 *
	 * @return the current handler
	 * @see #setRejectedExecutionHandler
	 */
	ERejectedExecutionHandler* getRejectedExecutionHandler();

	/**
	 * Sets the core number of threads.  This overrides any value set
	 * in the constructor.  If the new value is smaller than the
	 * current value, excess existing threads will be terminated when
	 * they next become idle.  If larger, new threads will, if needed,
	 * be started to execute any queued tasks.
	 *
	 * @param corePoolSize the new core size
	 * @throws IllegalArgumentException if {@code corePoolSize < 0}
	 * @see #getCorePoolSize
	 */
	void setCorePoolSize(int corePoolSize);

	/**
	 * Returns the core number of threads.
	 *
	 * @return the core number of threads
	 * @see #setCorePoolSize
	 */
	int getCorePoolSize();

	/**
	 * Starts a core thread, causing it to idly wait for work. This
	 * overrides the default policy of starting core threads only when
	 * new tasks are executed. This method will return {@code false}
	 * if all core threads have already been started.
	 *
	 * @return {@code true} if a thread was started
	 */
	boolean prestartCoreThread();

	/**
	 * Starts all core threads, causing them to idly wait for work. This
	 * overrides the default policy of starting core threads only when
	 * new tasks are executed.
	 *
	 * @return the number of threads started
	 */
	int prestartAllCoreThreads();

	/**
	 * Returns true if this pool allows core threads to time out and
	 * terminate if no tasks arrive within the keepAlive time, being
	 * replaced if needed when new tasks arrive. When true, the same
	 * keep-alive policy applying to non-core threads applies also to
	 * core threads. When false (the default), core threads are never
	 * terminated due to lack of incoming tasks.
	 *
	 * @return {@code true} if core threads are allowed to time out,
	 *         else {@code false}
	 *
	 * @since 1.6
	 */
	boolean allowsCoreThreadTimeOut();

	/**
	 * Sets the policy governing whether core threads may time out and
	 * terminate if no tasks arrive within the keep-alive time, being
	 * replaced if needed when new tasks arrive. When false, core
	 * threads are never terminated due to lack of incoming
	 * tasks. When true, the same keep-alive policy applying to
	 * non-core threads applies also to core threads. To avoid
	 * continual thread replacement, the keep-alive time must be
	 * greater than zero when setting {@code true}. This method
	 * should in general be called before the pool is actively used.
	 *
	 * @param value {@code true} if should time out, else {@code false}
	 * @throws IllegalArgumentException if value is {@code true}
	 *         and the current keep-alive time is not greater than zero
	 *
	 * @since 1.6
	 */
	void allowCoreThreadTimeOut(boolean value);

	/**
	 * Sets the maximum allowed number of threads. This overrides any
	 * value set in the constructor. If the new value is smaller than
	 * the current value, excess existing threads will be
	 * terminated when they next become idle.
	 *
	 * @param maximumPoolSize the new maximum
	 * @throws IllegalArgumentException if the new maximum is
	 *         less than or equal to zero, or
	 *         less than the {@linkplain #getCorePoolSize core pool size}
	 * @see #getMaximumPoolSize
	 */
	void setMaximumPoolSize(int maximumPoolSize);

	/**
	 * Returns the maximum allowed number of threads.
	 *
	 * @return the maximum allowed number of threads
	 * @see #setMaximumPoolSize
	 */
	int getMaximumPoolSize();

	/**
	 * Sets the time limit for which threads may remain idle before
	 * being terminated.  If there are more than the core number of
	 * threads currently in the pool, after waiting this amount of
	 * time without processing a task, excess threads will be
	 * terminated.  This overrides any value set in the constructor.
	 *
	 * @param time the time to wait.  A time value of zero will cause
	 *        excess threads to terminate immediately after executing tasks.
	 * @param unit the time unit of the {@code time} argument
	 * @throws IllegalArgumentException if {@code time} less than zero or
	 *         if {@code time} is zero and {@code allowsCoreThreadTimeOut}
	 * @see #getKeepAliveTime
	 */
	void setKeepAliveTime(llong time, ETimeUnit* unit);

	/**
	 * Returns the thread keep-alive time, which is the amount of time
	 * that threads in excess of the core pool size may remain
	 * idle before being terminated.
	 *
	 * @param unit the desired time unit of the result
	 * @return the time limit
	 * @see #setKeepAliveTime
	 */
	llong getKeepAliveTime(ETimeUnit* unit);

	/* User-level queue utilities */

	/**
	 * Returns the task queue used by this executor. Access to the
	 * task queue is intended primarily for debugging and monitoring.
	 * This queue may be in active use.  Retrieving the task queue
	 * does not prevent queued tasks from executing.
	 *
	 * @return the task queue
	 */
	EBlockingQueue<ERunnable>* getQueue();

	/**
	 * Removes this task from the executor's internal queue if it is
	 * present, thus causing it not to be run if it has not already
	 * started.
	 *
	 * <p> This method may be useful as one part of a cancellation
	 * scheme.  It may fail to remove tasks that have been converted
	 * into other forms before being placed on the internal queue. For
	 * example, a task entered using {@code submit} might be
	 * converted into a form that maintains {@code Future} status.
	 * However, in such cases, method {@link #purge} may be used to
	 * remove those Futures that have been cancelled.
	 *
	 * @param task the task to remove
	 * @return true if the task was removed
	 */
	boolean remove(sp<ERunnable> task);

	/**
	 * Tries to remove from the work queue all {@link Future}
	 * tasks that have been cancelled. This method can be useful as a
	 * storage reclamation operation, that has no other impact on
	 * functionality. Cancelled tasks are never executed, but may
	 * accumulate in work queues until worker threads can actively
	 * remove them. Invoking this method instead tries to remove them now.
	 * However, this method may fail to remove tasks in
	 * the presence of interference by other threads.
	 */
	void purge();

	/* Statistics */

	/**
	 * Returns the current number of threads in the pool.
	 *
	 * @return the number of threads
	 */
	int getPoolSize();

	/**
	 * Returns the approximate number of threads that are actively
	 * executing tasks.
	 *
	 * @return the number of threads
	 */
	int getActiveCount();

	/**
	 * Returns the largest number of threads that have ever
	 * simultaneously been in the pool.
	 *
	 * @return the number of threads
	 */
	int getLargestPoolSize();

	/**
	 * Returns the approximate total number of tasks that have ever been
	 * scheduled for execution. Because the states of tasks and
	 * threads may change dynamically during computation, the returned
	 * value is only an approximation.
	 *
	 * @return the number of tasks
	 */
	llong getTaskCount();

	/**
	 * Returns the approximate total number of tasks that have
	 * completed execution. Because the states of tasks and threads
	 * may change dynamically during computation, the returned value
	 * is only an approximation, but one that does not ever decrease
	 * across successive calls.
	 *
	 * @return the number of tasks
	 */
	llong getCompletedTaskCount();

	/**
	 * Returns a string identifying this pool, as well as its state,
	 * including indications of run state and estimated worker and
	 * task counts.
	 *
	 * @return a string identifying this pool, as well as its state
	 */
	EString toString();

protected:
	/**
	 * Performs any further cleanup following run state transition on
	 * invocation of shutdown.  A no-op here, but used by
	 * ScheduledThreadPoolExecutor to cancel delayed tasks.
	 */
	void onShutdown();

	/**
	 * Invokes {@code shutdown} when this executor is no longer
	 * referenced and it has no threads.
	 */
	void finalize();

	/* Extension hooks */

	/**
	 * Method invoked prior to executing the given Runnable in the
	 * given thread.  This method is invoked by thread {@code t} that
	 * will execute task {@code r}, and may be used to re-initialize
	 * ThreadLocals, or to perform logging.
	 *
	 * <p>This implementation does nothing, but may be customized in
	 * subclasses. Note: To properly nest multiple overridings, subclasses
	 * should generally invoke {@code super.beforeExecute} at the end of
	 * this method.
	 *
	 * @param t the thread that will run task {@code r}
	 * @param r the task that will be executed
	 */
	virtual void beforeExecute(EThread* t, sp<ERunnable> r) {
	}

	/**
	 * Method invoked upon completion of execution of the given Runnable.
	 * This method is invoked by the thread that executed the task. If
	 * non-null, the Throwable is the uncaught {@code RuntimeException}
	 * or {@code Error} that caused execution to terminate abruptly.
	 *
	 * <p>This implementation does nothing, but may be customized in
	 * subclasses. Note: To properly nest multiple overridings, subclasses
	 * should generally invoke {@code super.afterExecute} at the
	 * beginning of this method.
	 *
	 * <p><b>Note:</b> When actions are enclosed in tasks (such as
	 * {@link FutureTask}) either explicitly or via methods such as
	 * {@code submit}, these task objects catch and maintain
	 * computational exceptions, and so they do not cause abrupt
	 * termination, and the internal exceptions are <em>not</em>
	 * passed to this method. If you would like to trap both kinds of
	 * failures in this method, you can further probe for such cases,
	 * as in this sample subclass that prints either the direct cause
	 * or the underlying exception if a task has been aborted:
	 *
	 *  <pre> {@code
	 * class ExtendedExecutor extends ThreadPoolExecutor {
	 *   // ...
	 *   protected void afterExecute(Runnable r, Throwable t) {
	 *     super.afterExecute(r, t);
	 *     if (t == null && r instanceof Future<?>) {
	 *       try {
	 *         Object result = ((Future<?>) r).get();
	 *       } catch (CancellationException ce) {
	 *           t = ce;
	 *       } catch (ExecutionException ee) {
	 *           t = ee.getCause();
	 *       } catch (InterruptedException ie) {
	 *           Thread.currentThread().interrupt(); // ignore/reset
	 *       }
	 *     }
	 *     if (t != null)
	 *       System.out.println(t);
	 *   }
	 * }}</pre>
	 *
	 * @param r the runnable that has completed
	 * @param t the exception that caused termination, or null if
	 * execution completed normally
	 */
	virtual void afterExecute(sp<ERunnable> r, EThrowable* t) {
	}

	/**
	 * Method invoked when the Executor has terminated.  Default
	 * implementation does nothing. Note: To properly nest multiple
	 * overridings, subclasses should generally invoke
	 * {@code super.terminated} within this method.
	 */
	virtual void terminated() {
	}

	/**
	 * Same as prestartCoreThread except arranges that at least one
	 * thread is started even if corePoolSize is 0.
	 */
	void ensurePrestart();

private:
	friend class tpe::Worker;
	friend class tpe::WorkerKiller;

	EAtomicInteger* ctl;// = new AtomicInteger(ctlOf(RUNNING, 0));
	static const int COUNT_BITS = EInteger::SIZE - 3;
	static const int CAPACITY   = (1 << COUNT_BITS) - 1;

	// runState is stored in the high-order bits
	static const int RUNNING    = -1 << COUNT_BITS;
	static const int SHUTDOWN   =  0 << COUNT_BITS;
	static const int STOP       =  1 << COUNT_BITS;
	static const int TIDYING    =  2 << COUNT_BITS;
	static const int TERMINATED =  3 << COUNT_BITS;

	// Packing and unpacking ctl
	static inline int runStateOf(int c)     { return c & ~CAPACITY; }
	static inline int workerCountOf(int c)  { return c & CAPACITY; }
	static inline int ctlOf(int rs, int wc) { return rs | wc; }

	/*
	 * Bit field accessors that don't require unpacking ctl.
	 * These depend on the bit layout and on workerCount being never negative.
	 */

	static inline boolean runStateLessThan(int c, int s) {
		return c < s;
	}

	static inline boolean runStateAtLeast(int c, int s) {
		return c >= s;
	}

	static inline boolean isRunning(int c) {
		return c < SHUTDOWN;
	}

	/**
	 *
	 */
	void init(int corePoolSize, int maximumPoolSize,
			llong keepAliveTime, ETimeUnit* unit,
			EBlockingQueue<ERunnable>* workQueue, EThreadFactory* threadFactory,
			ERejectedExecutionHandler* handler);

	/**
	 * Attempt to CAS-increment the workerCount field of ctl.
	 */
	boolean compareAndIncrementWorkerCount(int expect);

	/**
	 * Attempt to CAS-decrement the workerCount field of ctl.
	 */
	boolean compareAndDecrementWorkerCount(int expect);

	/**
	 * Decrements the workerCount field of ctl. This is called only on
	 * abrupt termination of a thread (see processWorkerExit). Other
	 * decrements are performed within getTask.
	 */
	void decrementWorkerCount();

	/**
	 * The queue used for holding tasks and handing off to worker
	 * threads.  We do not require that workQueue.poll() returning
	 * null necessarily means that workQueue.isEmpty(), so rely
	 * solely on isEmpty to see if the queue is empty (which we must
	 * do for example when deciding whether to transition from
	 * SHUTDOWN to TIDYING).  This accommodates special-purpose
	 * queues such as DelayQueues for which poll() is allowed to
	 * return null even if it may later return non-null when delays
	 * expire.
	 */
	EBlockingQueue<ERunnable>* workQueue; // need free!

	/**
	 * Lock held on access to workers set and related bookkeeping.
	 * While we could use a concurrent set of some sort, it turns out
	 * to be generally preferable to use a lock. Among the reasons is
	 * that this serializes interruptIdleWorkers, which avoids
	 * unnecessary interrupt storms, especially during shutdown.
	 * Otherwise exiting threads would concurrently interrupt those
	 * that have not yet interrupted. It also simplifies some of the
	 * associated statistics bookkeeping of largestPoolSize etc. We
	 * also hold mainLock on shutdown and shutdownNow, for the sake of
	 * ensuring workers set is stable while separately checking
	 * permission to interrupt and actually interrupting.
	 */
	EReentrantLock mainLock;

	/**
	 * Set containing all worker threads in pool. Accessed only when
	 * holding mainLock.
	 */
	ELinkedList<tpe::Worker*>* workers; // = new HashSet<Worker>();

	/**
	 * List to hold Worker object that have terminated for some reason. Usually this is
	 * because of a call to setMaximumPoolSize or setCorePoolSize but can also occur
	 * because of an exception from a task that the worker was running.
	 */
	ELinkedList<tpe::Worker*>* deadWorkers;

	/**
	 * Timer used to periodically clean up the dead worker objects.  They must be cleaned
	 * up on a separate thread because the Worker generally adds itself to the deadWorkers
	 * list from the context of its run method and cannot delete itself.
	 */
	ETimer cleanupTimer;

	/**
	 * Wait condition to support awaitTermination
	 */
	ECondition* termination; // = mainLock.newCondition();

	/**
	 * Tracks largest attained pool size. Accessed only under
	 * mainLock.
	 */
	int largestPoolSize;

	/**
	 * Counter for completed tasks. Updated only on termination of
	 * worker threads. Accessed only under mainLock.
	 */
	llong completedTaskCount;

	/*
	 * All user control parameters are declared as volatiles so that
	 * ongoing actions are based on freshest values, but without need
	 * for locking, since no internal invariants depend on them
	 * changing synchronously with respect to other actions.
	 */

	/**
	 * Factory for new threads. All threads are created using this
	 * factory (via method addWorker).  All callers must be prepared
	 * for addWorker to fail, which may reflect a system or user's
	 * policy limiting the number of threads.  Even though it is not
	 * treated as an error, failure to create threads may result in
	 * new tasks being rejected or existing ones remaining stuck in
	 * the queue.
	 *
	 * We go further and preserve pool invariants even in the face of
	 * errors such as OutOfMemoryError, that might be thrown while
	 * trying to create threads.  Such errors are rather common due to
	 * the need to allocate a native stack in Thread#start, and users
	 * will want to perform clean pool shutdown to clean up.  There
	 * will likely be enough memory available for the cleanup code to
	 * complete without encountering yet another OutOfMemoryError.
	 */
	EThreadFactory* volatile threadFactory; // need free!

	/**
	 * Handler called when saturated or shutdown in execute.
	 */
	ERejectedExecutionHandler* volatile handler; // need free!

	/**
	 * Timeout in nanoseconds for idle threads waiting for work.
	 * Threads use this timeout when there are more than corePoolSize
	 * present or if allowCoreThreadTimeOut. Otherwise they wait
	 * forever for new work.
	 */
	volatile llong keepAliveTime;

	/**
	 * If false (default), core threads stay alive even when idle.
	 * If true, core threads use keepAliveTime to time out waiting
	 * for work.
	 */
	volatile boolean allowCoreThreadTimeOut_;

	/**
	 * Core pool size is the minimum number of workers to keep alive
	 * (and not allow to time out etc) unless allowCoreThreadTimeOut
	 * is set, in which case the minimum is zero.
	 */
	volatile int corePoolSize;

	/**
	 * Maximum pool size. Note that the actual maximum is internally
	 * bounded by CAPACITY.
	 */
	volatile int maximumPoolSize;

	/**
	 *
	 */
	EAtomicCounter aliveThreadCount1;
	EAtomicCounter aliveThreadCount2;

	/**
	 * The default rejected execution handler
	 */
	static ERejectedExecutionHandler* defaultHandler; // = new AbortPolicy();

	/*
	 * Methods for setting control state
	 */

	/**
	 * Transitions runState to given target, or leaves it alone if
	 * already at least the given target.
	 *
	 * @param targetState the desired state, either SHUTDOWN or STOP
	 *        (but not TIDYING or TERMINATED -- use tryTerminate for that)
	 */
	void advanceRunState(int targetState);

	/**
	 * Transitions to TERMINATED state if either (SHUTDOWN and pool
	 * and queue empty) or (STOP and pool empty).  If otherwise
	 * eligible to terminate but workerCount is nonzero, interrupts an
	 * idle worker to ensure that shutdown signals propagate. This
	 * method must be called following any action that might make
	 * termination possible -- reducing worker count or removing tasks
	 * from the queue during shutdown. The method is non-private to
	 * allow access from ScheduledThreadPoolExecutor.
	 */
	void tryTerminate();

	/*
	 * Methods for controlling interrupts to worker threads.
	 */

	/**
	 * If there is a security manager, makes sure caller has
	 * permission to shut down threads in general (see shutdownPerm).
	 * If this passes, additionally makes sure the caller is allowed
	 * to interrupt each worker thread. This might not be true even if
	 * first check passed, if the SecurityManager treats some threads
	 * specially.
	 */
	void checkShutdownAccess();

	/**
	 * Interrupts all threads, even if active. Ignores SecurityExceptions
	 * (in which case some threads may remain uninterrupted).
	 */
	void interruptWorkers();

	/**
	 * Interrupts threads that might be waiting for tasks (as
	 * indicated by not being locked) so they can check for
	 * termination or configuration changes. Ignores
	 * SecurityExceptions (in which case some threads may remain
	 * uninterrupted).
	 *
	 * @param onlyOne If true, interrupt at most one worker. This is
	 * called only from tryTerminate when termination is otherwise
	 * enabled but there are still other workers.  In this case, at
	 * most one waiting worker is interrupted to propagate shutdown
	 * signals in case all threads are currently waiting.
	 * Interrupting any arbitrary thread ensures that newly arriving
	 * workers since shutdown began will also eventually exit.
	 * To guarantee eventual termination, it suffices to always
	 * interrupt only one idle worker, but shutdown() interrupts all
	 * idle workers so that redundant workers exit promptly, not
	 * waiting for a straggler task to finish.
	 */
	void interruptIdleWorkers(boolean onlyOne);

	/**
	 * Common form of interruptIdleWorkers, to avoid having to
	 * remember what the boolean argument means.
	 */
	void interruptIdleWorkers();

	static const boolean ONLY_ONE = true;

	/*
	 * Misc utilities, most of which are also exported to
	 * ScheduledThreadPoolExecutor
	 */

	/**
	 * Invokes the rejected execution handler for the given command.
	 * Package-protected for use by ScheduledThreadPoolExecutor.
	 */
	void reject(sp<ERunnable> command);

	/**
	 * State check needed by ScheduledThreadPoolExecutor to
	 * enable running tasks during shutdown.
	 *
	 * @param shutdownOK true if should return true if SHUTDOWN
	 */
	boolean isRunningOrShutdown(boolean shutdownOK);

	/**
	 * Drains the task queue into a new list, normally using
	 * drainTo. But if the queue is a DelayQueue or any other kind of
	 * queue for which poll or drainTo may fail to remove some
	 * elements, it deletes them one by one.
	 */
	eal<ERunnable> drainQueue();

	/*
	 * Methods for creating, running and cleaning up after workers
	 */

	/**
	 * Checks if a new worker can be added with respect to current
	 * pool state and the given bound (either core or maximum). If so,
	 * the worker count is adjusted accordingly, and, if possible, a
	 * new worker is created and started, running firstTask as its
	 * first task. This method returns false if the pool is stopped or
	 * eligible to shut down. It also returns false if the thread
	 * factory fails to create a thread when asked.  If the thread
	 * creation fails, either due to the thread factory returning
	 * null, or due to an exception (typically OutOfMemoryError in
	 * Thread#start), we roll back cleanly.
	 *
	 * @param firstTask the task the new thread should run first (or
	 * null if none). Workers are created with an initial first task
	 * (in method execute()) to bypass queuing when there are fewer
	 * than corePoolSize threads (in which case we always start one),
	 * or when the queue is full (in which case we must bypass queue).
	 * Initially idle threads are usually created via
	 * prestartCoreThread or to replace other dying workers.
	 *
	 * @param core if true use corePoolSize as bound, else
	 * maximumPoolSize. (A boolean indicator is used here rather than a
	 * value to ensure reads of fresh values after checking other pool
	 * state).
	 * @return true if successful
	 */
	boolean addWorker(sp<ERunnable> firstTask, boolean core);

	/**
	 * Rolls back the worker thread creation.
	 * - removes worker from workers, if present
	 * - decrements worker count
	 * - rechecks for termination, in case the existence of this
	 *   worker was holding up termination
	 */
	void addWorkerFailed(tpe::Worker* w);

	/**
	 * Performs cleanup and bookkeeping for a dying worker. Called
	 * only from worker threads. Unless completedAbruptly is set,
	 * assumes that workerCount has already been adjusted to account
	 * for exit.  This method removes thread from worker set, and
	 * possibly terminates the pool or replaces the worker if either
	 * it exited due to user task exception or if fewer than
	 * corePoolSize workers are running or queue is non-empty but
	 * there are no workers.
	 *
	 * @param w the worker
	 * @param completedAbruptly if the worker died due to user exception
	 */
	void processWorkerExit(tpe::Worker* w, boolean completedAbruptly);

	/**
	 * Performs blocking or timed wait for a task, depending on
	 * current configuration settings, or returns null if this worker
	 * must exit because of any of:
	 * 1. There are more than maximumPoolSize workers (due to
	 *    a call to setMaximumPoolSize).
	 * 2. The pool is stopped.
	 * 3. The pool is shutdown and the queue is empty.
	 * 4. This worker timed out waiting for a task, and timed-out
	 *    workers are subject to termination (that is,
	 *    {@code allowCoreThreadTimeOut || workerCount > corePoolSize})
	 *    both before and after the timed wait.
	 *
	 * @return task, or null if the worker must exit, in which case
	 *         workerCount is decremented
	 */
	sp<ERunnable> getTask();

	/**
	 * Main worker run loop.  Repeatedly gets tasks from queue and
	 * executes them, while coping with a number of issues:
	 *
	 * 1. We may start out with an initial task, in which case we
	 * don't need to get the first one. Otherwise, as long as pool is
	 * running, we get tasks from getTask. If it returns null then the
	 * worker exits due to changed pool state or configuration
	 * parameters.  Other exits result from exception throws in
	 * external code, in which case completedAbruptly holds, which
	 * usually leads processWorkerExit to replace this thread.
	 *
	 * 2. Before running any task, the lock is acquired to prevent
	 * other pool interrupts while the task is executing, and
	 * clearInterruptsForTaskRun called to ensure that unless pool is
	 * stopping, this thread does not have its interrupt set.
	 *
	 * 3. Each task run is preceded by a call to beforeExecute, which
	 * might throw an exception, in which case we cause thread to die
	 * (breaking loop with completedAbruptly true) without processing
	 * the task.
	 *
	 * 4. Assuming beforeExecute completes normally, we run the task,
	 * gathering any of its thrown exceptions to send to
	 * afterExecute. We separately handle RuntimeException, Error
	 * (both of which the specs guarantee that we trap) and arbitrary
	 * Throwables.  Because we cannot rethrow Throwables within
	 * Runnable.run, we wrap them within Errors on the way out (to the
	 * thread's UncaughtExceptionHandler).  Any thrown exception also
	 * conservatively causes thread to die.
	 *
	 * 5. After task.run completes, we call afterExecute, which may
	 * also throw an exception, which will also cause thread to
	 * die. According to JLS Sec 14.20, this exception is the one that
	 * will be in effect even if task.run throws.
	 *
	 * The net effect of the exception mechanics is that afterExecute
	 * and the thread's UncaughtExceptionHandler have as accurate
	 * information as we can provide about any problems encountered by
	 * user code.
	 *
	 * @param w the worker
	 */
	void runWorker(tpe::Worker* w);

};

} /* namespace efc */
#endif /* ETHREADPOOLEXECUTOR_HH_ */
