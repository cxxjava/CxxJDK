/*
 * EExecutors.hh
 *
 *  Created on: 2015-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EEXECUTORS_HH_
#define EEXECUTORS_HH_

#include "EString.hh"
#include "ETimeUnit.hh"
#include "EThreadPoolExecutor.hh"

namespace efc {

interface EExecutorService;

/**
 * Factory and utility methods for {@link Executor}, {@link
 * ExecutorService}, {@link ScheduledExecutorService}, {@link
 * ThreadFactory}, and {@link Callable} classes defined in this
 * package. This class supports the following kinds of methods:
 *
 * <ul>
 *   <li> Methods that create and return an {@link ExecutorService}
 *        set up with commonly useful configuration settings.
 *   <li> Methods that create and return a {@link ScheduledExecutorService}
 *        set up with commonly useful configuration settings.
 *   <li> Methods that create and return a "wrapped" ExecutorService, that
 *        disables reconfiguration by making implementation-specific methods
 *        inaccessible.
 *   <li> Methods that create and return a {@link ThreadFactory}
 *        that sets newly created threads to a known state.
 *   <li> Methods that create and return a {@link Callable}
 *        out of other closure-like forms, so they can be used
 *        in execution methods requiring <tt>Callable</tt>.
 * </ul>
 *
 * @since 1.5
 * @author Doug Lea
 */

class EExecutors : public EObject {
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

public:
	/**
	 * Creates a thread pool that reuses a fixed number of threads
	 * operating off a shared unbounded queue.  At any point, at most
	 * <tt>nThreads</tt> threads will be active processing tasks.
	 * If additional tasks are submitted when all threads are active,
	 * they will wait in the queue until a thread is available.
	 * If any thread terminates due to a failure during execution
	 * prior to shutdown, a new one will take its place if needed to
	 * execute subsequent tasks.  The threads in the pool will exist
	 * until it is explicitly {@link ExecutorService#shutdown shutdown}.
	 *
	 * @param nThreads the number of threads in the pool
	 * @return the newly created thread pool
	 * @throws IllegalArgumentException if {@code nThreads <= 0}
	 */
	static EExecutorService* newFixedThreadPool(int nThreads);

	/**
	 * Creates a thread pool that reuses a fixed number of threads
	 * operating off a shared unbounded queue, using the provided
	 * ThreadFactory to create new threads when needed.  At any point,
	 * at most <tt>nThreads</tt> threads will be active processing
	 * tasks.  If additional tasks are submitted when all threads are
	 * active, they will wait in the queue until a thread is
	 * available.  If any thread terminates due to a failure during
	 * execution prior to shutdown, a new one will take its place if
	 * needed to execute subsequent tasks.  The threads in the pool will
	 * exist until it is explicitly {@link ExecutorService#shutdown
	 * shutdown}.
	 *
	 * @param nThreads the number of threads in the pool
	 * @param threadFactory the factory to use when creating new threads
	 * @return the newly created thread pool
	 * @throws NullPointerException if threadFactory is null
	 * @throws IllegalArgumentException if {@code nThreads <= 0}
	 */
	static EExecutorService* newFixedThreadPool(int nThreads, EThreadFactory* threadFactory);

	/**
	 * Creates an Executor that uses a single worker thread operating
	 * off an unbounded queue. (Note however that if this single
	 * thread terminates due to a failure during execution prior to
	 * shutdown, a new one will take its place if needed to execute
	 * subsequent tasks.)  Tasks are guaranteed to execute
	 * sequentially, and no more than one task will be active at any
	 * given time. Unlike the otherwise equivalent
	 * <tt>newFixedThreadPool(1)</tt> the returned executor is
	 * guaranteed not to be reconfigurable to use additional threads.
	 *
	 * @return the newly created single-threaded Executor
	 */
	static EExecutorService* newSingleThreadExecutor();

	/**
	 * Creates an Executor that uses a single worker thread operating
	 * off an unbounded queue, and uses the provided ThreadFactory to
	 * create a new thread when needed. Unlike the otherwise
	 * equivalent <tt>newFixedThreadPool(1, threadFactory)</tt> the
	 * returned executor is guaranteed not to be reconfigurable to use
	 * additional threads.
	 *
	 * @param threadFactory the factory to use when creating new
	 * threads
	 *
	 * @return the newly created single-threaded Executor
	 * @throws NullPointerException if threadFactory is null
	 */
	static EExecutorService* newSingleThreadExecutor(EThreadFactory* threadFactory);

	/**
	 * Creates a thread pool that creates new threads as needed, but
	 * will reuse previously constructed threads when they are
	 * available.  These pools will typically improve the performance
	 * of programs that execute many short-lived asynchronous tasks.
	 * Calls to <tt>execute</tt> will reuse previously constructed
	 * threads if available. If no existing thread is available, a new
	 * thread will be created and added to the pool. Threads that have
	 * not been used for sixty seconds are terminated and removed from
	 * the cache. Thus, a pool that remains idle for long enough will
	 * not consume any resources. Note that pools with similar
	 * properties but different details (for example, timeout parameters)
	 * may be created using {@link ThreadPoolExecutor} constructors.
	 *
	 * @return the newly created thread pool
	 */
	static EExecutorService* newCachedThreadPool();

	/**
	 * Creates a thread pool that creates new threads as needed, but
	 * will reuse previously constructed threads when they are
	 * available, and uses the provided
	 * ThreadFactory to create new threads when needed.
	 * @param threadFactory the factory to use when creating new threads
	 * @return the newly created thread pool
	 * @throws NullPointerException if threadFactory is null
	 */
	static EExecutorService* newCachedThreadPool(EThreadFactory* threadFactory);

//	    /**
//	     * Creates a single-threaded executor that can schedule commands
//	     * to run after a given delay, or to execute periodically.
//	     * (Note however that if this single
//	     * thread terminates due to a failure during execution prior to
//	     * shutdown, a new one will take its place if needed to execute
//	     * subsequent tasks.)  Tasks are guaranteed to execute
//	     * sequentially, and no more than one task will be active at any
//	     * given time. Unlike the otherwise equivalent
//	     * <tt>newScheduledThreadPool(1)</tt> the returned executor is
//	     * guaranteed not to be reconfigurable to use additional threads.
//	     * @return the newly created scheduled executor
//	     */
//	    public static ScheduledExecutorService newSingleThreadScheduledExecutor() {
//	        return new DelegatedScheduledExecutorService
//	            (new ScheduledThreadPoolExecutor(1));
//	    }
//
//	    /**
//	     * Creates a single-threaded executor that can schedule commands
//	     * to run after a given delay, or to execute periodically.  (Note
//	     * however that if this single thread terminates due to a failure
//	     * during execution prior to shutdown, a new one will take its
//	     * place if needed to execute subsequent tasks.)  Tasks are
//	     * guaranteed to execute sequentially, and no more than one task
//	     * will be active at any given time. Unlike the otherwise
//	     * equivalent <tt>newScheduledThreadPool(1, threadFactory)</tt>
//	     * the returned executor is guaranteed not to be reconfigurable to
//	     * use additional threads.
//	     * @param threadFactory the factory to use when creating new
//	     * threads
//	     * @return a newly created scheduled executor
//	     * @throws NullPointerException if threadFactory is null
//	     */
//	    public static ScheduledExecutorService newSingleThreadScheduledExecutor(ThreadFactory threadFactory) {
//	        return new DelegatedScheduledExecutorService
//	            (new ScheduledThreadPoolExecutor(1, threadFactory));
//	    }
//
//	    /**
//	     * Creates a thread pool that can schedule commands to run after a
//	     * given delay, or to execute periodically.
//	     * @param corePoolSize the number of threads to keep in the pool,
//	     * even if they are idle.
//	     * @return a newly created scheduled thread pool
//	     * @throws IllegalArgumentException if {@code corePoolSize < 0}
//	     */
//	    public static ScheduledExecutorService newScheduledThreadPool(int corePoolSize) {
//	        return new ScheduledThreadPoolExecutor(corePoolSize);
//	    }
//
//	    /**
//	     * Creates a thread pool that can schedule commands to run after a
//	     * given delay, or to execute periodically.
//	     * @param corePoolSize the number of threads to keep in the pool,
//	     * even if they are idle.
//	     * @param threadFactory the factory to use when the executor
//	     * creates a new thread.
//	     * @return a newly created scheduled thread pool
//	     * @throws IllegalArgumentException if {@code corePoolSize < 0}
//	     * @throws NullPointerException if threadFactory is null
//	     */
//	    public static ScheduledExecutorService newScheduledThreadPool(
//	            int corePoolSize, ThreadFactory threadFactory) {
//	        return new ScheduledThreadPoolExecutor(corePoolSize, threadFactory);
//	    }
//
//
//	    /**
//	     * Returns an object that delegates all defined {@link
//	     * ExecutorService} methods to the given executor, but not any
//	     * other methods that might otherwise be accessible using
//	     * casts. This provides a way to safely "freeze" configuration and
//	     * disallow tuning of a given concrete implementation.
//	     * @param executor the underlying implementation
//	     * @return an <tt>ExecutorService</tt> instance
//	     * @throws NullPointerException if executor null
//	     */
//	    public static ExecutorService unconfigurableExecutorService(ExecutorService executor) {
//	        if (executor == null)
//	            throw new NullPointerException();
//	        return new DelegatedExecutorService(executor);
//	    }
//
//	    /**
//	     * Returns an object that delegates all defined {@link
//	     * ScheduledExecutorService} methods to the given executor, but
//	     * not any other methods that might otherwise be accessible using
//	     * casts. This provides a way to safely "freeze" configuration and
//	     * disallow tuning of a given concrete implementation.
//	     * @param executor the underlying implementation
//	     * @return a <tt>ScheduledExecutorService</tt> instance
//	     * @throws NullPointerException if executor null
//	     */
//	    public static ScheduledExecutorService unconfigurableScheduledExecutorService(ScheduledExecutorService executor) {
//	        if (executor == null)
//	            throw new NullPointerException();
//	        return new DelegatedScheduledExecutorService(executor);
//	    }

	/**
	 * Returns a default thread factory used to create new threads.
	 * This factory creates all new threads used by an Executor in the
	 * same {@link ThreadGroup}. If there is a {@link
	 * java.lang.SecurityManager}, it uses the group of {@link
	 * System#getSecurityManager}, else the group of the thread
	 * invoking this <tt>defaultThreadFactory</tt> method. Each new
	 * thread is created as a non-daemon thread with priority set to
	 * the smaller of <tt>Thread.NORM_PRIORITY</tt> and the maximum
	 * priority permitted in the thread group.  New threads have names
	 * accessible via {@link Thread#getName} of
	 * <em>pool-N-thread-M</em>, where <em>N</em> is the sequence
	 * number of this factory, and <em>M</em> is the sequence number
	 * of the thread created by this factory.
	 * @return a thread factory
	 */
	static EThreadFactory* defaultThreadFactory();

	/**
	 * Returns a thread factory used to create new threads that
	 * have the same permissions as the current thread.
	 * This factory creates threads with the same settings as {@link
	 * Executors#defaultThreadFactory}, additionally setting the
	 * AccessControlContext and contextClassLoader of new threads to
	 * be the same as the thread invoking this
	 * <tt>privilegedThreadFactory</tt> method.  A new
	 * <tt>privilegedThreadFactory</tt> can be created within an
	 * {@link AccessController#doPrivileged} action setting the
	 * current thread's access control context to create threads with
	 * the selected permission settings holding within that action.
	 *
	 * <p> Note that while tasks running within such threads will have
	 * the same access control and class loader settings as the
	 * current thread, they need not have the same {@link
	 * java.lang.ThreadLocal} or {@link
	 * java.lang.InheritableThreadLocal} values. If necessary,
	 * particular values of thread locals can be set or reset before
	 * any task runs in {@link ThreadPoolExecutor} subclasses using
	 * {@link ThreadPoolExecutor#beforeExecute}. Also, if it is
	 * necessary to initialize worker threads to have the same
	 * InheritableThreadLocal settings as some other designated
	 * thread, you can create a custom ThreadFactory in which that
	 * thread waits for and services requests to create others that
	 * will inherit its values.
	 *
	 * @return a thread factory
	 * @throws AccessControlException if the current access control
	 * context does not have permission to both get and set context
	 * class loader.
	 */
	static EThreadFactory* privilegedThreadFactory();

	/**
	 * Returns a {@link Callable} object that, when
	 * called, runs the given task and returns the given result.  This
	 * can be useful when applying methods requiring a
	 * <tt>Callable</tt> to an otherwise resultless action.
	 * @param task the task to run
	 * @param result the result to return
	 * @return a callable object
	 * @throws NullPointerException if task null
	 */
	template<typename T>
	static ECallable<T>* callable(sp<ERunnable> task, sp<T> result) {
		if (task == null)
			throw ENullPointerException(__FILE__, __LINE__);
		return new RunnableAdapter<T>(task, result);
	}

//	    /**
//	     * Returns a {@link Callable} object that, when
//	     * called, runs the given task and returns <tt>null</tt>.
//	     * @param task the task to run
//	     * @return a callable object
//	     * @throws NullPointerException if task null
//	     */
//	    public static Callable<Object> callable(Runnable task) {
//	        if (task == null)
//	            throw new NullPointerException();
//	        return new RunnableAdapter<Object>(task, null);
//	    }
//
//	    /**
//	     * Returns a {@link Callable} object that, when
//	     * called, runs the given privileged action and returns its result.
//	     * @param action the privileged action to run
//	     * @return a callable object
//	     * @throws NullPointerException if action null
//	     */
//	    public static Callable<Object> callable(final PrivilegedAction<?> action) {
//	        if (action == null)
//	            throw new NullPointerException();
//	        return new Callable<Object>() {
//	            public Object call() { return action.run(); }};
//	    }
//
//	    /**
//	     * Returns a {@link Callable} object that, when
//	     * called, runs the given privileged exception action and returns
//	     * its result.
//	     * @param action the privileged exception action to run
//	     * @return a callable object
//	     * @throws NullPointerException if action null
//	     */
//	    public static Callable<Object> callable(final PrivilegedExceptionAction<?> action) {
//	        if (action == null)
//	            throw new NullPointerException();
//	        return new Callable<Object>() {
//	            public Object call() throws Exception { return action.run(); }};
//	    }
//
//	    /**
//	     * Returns a {@link Callable} object that will, when
//	     * called, execute the given <tt>callable</tt> under the current
//	     * access control context. This method should normally be
//	     * invoked within an {@link AccessController#doPrivileged} action
//	     * to create callables that will, if possible, execute under the
//	     * selected permission settings holding within that action; or if
//	     * not possible, throw an associated {@link
//	     * AccessControlException}.
//	     * @param callable the underlying task
//	     * @return a callable object
//	     * @throws NullPointerException if callable null
//	     *
//	     */
//	    public static <T> Callable<T> privilegedCallable(Callable<T> callable) {
//	        if (callable == null)
//	            throw new NullPointerException();
//	        return new PrivilegedCallable<T>(callable);
//	    }
//
//	    /**
//	     * Returns a {@link Callable} object that will, when
//	     * called, execute the given <tt>callable</tt> under the current
//	     * access control context, with the current context class loader
//	     * as the context class loader. This method should normally be
//	     * invoked within an {@link AccessController#doPrivileged} action
//	     * to create callables that will, if possible, execute under the
//	     * selected permission settings holding within that action; or if
//	     * not possible, throw an associated {@link
//	     * AccessControlException}.
//	     * @param callable the underlying task
//	     *
//	     * @return a callable object
//	     * @throws NullPointerException if callable null
//	     * @throws AccessControlException if the current access control
//	     * context does not have permission to both set and get context
//	     * class loader.
//	     */
//	    public static <T> Callable<T> privilegedCallableUsingCurrentClassLoader(Callable<T> callable) {
//	        if (callable == null)
//	            throw new NullPointerException();
//	        return new PrivilegedCallableUsingCurrentClassLoader<T>(callable);
//	    }

protected:
	/**
	 * The default thread factory
	 */
	class DefaultThreadFactory : public EThreadFactory {
	private:
		static EAtomicInteger* poolNumber;// = new AtomicInteger(1);
		//EThreadGroup* group;
		EAtomicInteger* threadNumber;// = new AtomicInteger(1);
		EString namePrefix;

	public:
		virtual ~DefaultThreadFactory();
		DefaultThreadFactory();
		EThread* newThread(ERunnable* r);
	};

private:
	/** Cannot instantiate. */
	EExecutors() {}
};

} /* namespace efc */
#endif /* EEXECUTORS_HH_ */
