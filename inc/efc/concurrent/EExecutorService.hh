/*
 * EExecutorService.hh
 *
 *  Created on: 2014-9-22
 *      Author: cxxjava@163.com
 */

#ifndef EEXECUTORSERVICE_HH_
#define EEXECUTORSERVICE_HH_

#include "EList.hh"
#include "EFuture.hh"
#include "ECallable.hh"
#include "ERunnable.hh"
#include "EExecutor.hh"
#include "ETimeUnit.hh"
#include "ERunnableFuture.hh"
#include "EFutureTask.hh"
#include "ESharedArrLst.hh"
#include "EExecutorCompletionService.hh"
#include "EInterruptedException.hh"
#include "EExecutionException.hh"
#include "ETimeoutException.hh"

namespace efc {

/**
 * An {@link Executor} that provides methods to manage termination and
 * methods that can produce a {@link Future} for tracking progress of
 * one or more asynchronous tasks.
 *
 * <p> An <tt>ExecutorService</tt> can be shut down, which will cause
 * it to reject new tasks.  Two different methods are provided for
 * shutting down an <tt>ExecutorService</tt>. The {@link #shutdown}
 * method will allow previously submitted tasks to execute before
 * terminating, while the {@link #shutdownNow} method prevents waiting
 * tasks from starting and attempts to stop currently executing tasks.
 * Upon termination, an executor has no tasks actively executing, no
 * tasks awaiting execution, and no new tasks can be submitted.  An
 * unused <tt>ExecutorService</tt> should be shut down to allow
 * reclamation of its resources.
 *
 * <p> Method <tt>submit</tt> extends base method {@link
 * Executor#execute} by creating and returning a {@link Future} that
 * can be used to cancel execution and/or wait for completion.
 * Methods <tt>invokeAny</tt> and <tt>invokeAll</tt> perform the most
 * commonly useful forms of bulk execution, executing a collection of
 * tasks and then waiting for at least one, or all, to
 * complete. (Class {@link ExecutorCompletionService} can be used to
 * write customized variants of these methods.)
 *
 * <p>The {@link Executors} class provides factory methods for the
 * executor services provided in this package.
 *
 * <h3>Usage Examples</h3>
 *
 * Here is a sketch of a network service in which threads in a thread
 * pool service incoming requests. It uses the preconfigured {@link
 * Executors#newFixedThreadPool} factory method:
 *
 * <pre>
 * class NetworkService implements Runnable {
 *   private final ServerSocket serverSocket;
 *   private final ExecutorService pool;
 *
 *   public NetworkService(int port, int poolSize)
 *       throws IOException {
 *     serverSocket = new ServerSocket(port);
 *     pool = Executors.newFixedThreadPool(poolSize);
 *   }
 *
 *   public void run() { // run the service
 *     try {
 *       for (;;) {
 *         pool.execute(new Handler(serverSocket.accept()));
 *       }
 *     } catch (IOException ex) {
 *       pool.shutdown();
 *     }
 *   }
 * }
 *
 * class Handler implements Runnable {
 *   private final Socket socket;
 *   Handler(Socket socket) { this.socket = socket; }
 *   public void run() {
 *     // read and service request on socket
 *   }
 * }
 * </pre>
 *
 * The following method shuts down an <tt>ExecutorService</tt> in two phases,
 * first by calling <tt>shutdown</tt> to reject incoming tasks, and then
 * calling <tt>shutdownNow</tt>, if necessary, to cancel any lingering tasks:
 *
 * <pre>
 * void shutdownAndAwaitTermination(ExecutorService pool) {
 *   pool.shutdown(); // Disable new tasks from being submitted
 *   try {
 *     // Wait a while for existing tasks to terminate
 *     if (!pool.awaitTermination(60, TimeUnit.SECONDS)) {
 *       pool.shutdownNow(); // Cancel currently executing tasks
 *       // Wait a while for tasks to respond to being cancelled
 *       if (!pool.awaitTermination(60, TimeUnit.SECONDS))
 *           System.err.println("Pool did not terminate");
 *     }
 *   } catch (InterruptedException ie) {
 *     // (Re-)Cancel if current thread also interrupted
 *     pool.shutdownNow();
 *     // Preserve interrupt status
 *     Thread.currentThread().interrupt();
 *   }
 * }
 * </pre>
 *
 * <p>Memory consistency effects: Actions in a thread prior to the
 * submission of a {@code Runnable} or {@code Callable} task to an
 * {@code ExecutorService}
 * <a href="package-summary.html#MemoryVisibility"><i>happen-before</i></a>
 * any actions taken by that task, which in turn <i>happen-before</i> the
 * result is retrieved via {@code Future.get()}.
 *
 * @since 1.5
 */

interface EExecutorService : virtual public EExecutor {
	virtual ~EExecutorService(){}

    /**
     * Initiates an orderly shutdown in which previously submitted
     * tasks are executed, but no new tasks will be accepted.
     * Invocation has no additional effect if already shut down.
     *
     * @throws SecurityException if a security manager exists and
     *         shutting down this ExecutorService may manipulate
     *         threads that the caller is not permitted to modify
     *         because it does not hold {@link
     *         java.lang.RuntimePermission}<tt>("modifyThread")</tt>,
     *         or the security manager's <tt>checkAccess</tt> method
     *         denies access.
     */
	virtual void shutdown() = 0;

    /**
     * Attempts to stop all actively executing tasks, halts the
     * processing of waiting tasks, and returns a list of the tasks that were
     * awaiting execution.
     *
     * <p>There are no guarantees beyond best-effort attempts to stop
     * processing actively executing tasks.  For example, typical
     * implementations will cancel via {@link Thread#interrupt}, so any
     * task that fails to respond to interrupts may never terminate.
     *
     * @return list of tasks that never commenced execution
     * @throws SecurityException if a security manager exists and
     *         shutting down this ExecutorService may manipulate
     *         threads that the caller is not permitted to modify
     *         because it does not hold {@link
     *         java.lang.RuntimePermission}<tt>("modifyThread")</tt>,
     *         or the security manager's <tt>checkAccess</tt> method
     *         denies access.
     */
	virtual eal<ERunnable> shutdownNow() = 0;

    /**
     * Returns <tt>true</tt> if this executor has been shut down.
     *
     * @return <tt>true</tt> if this executor has been shut down
     */
	virtual boolean isShutdown() = 0;

    /**
     * Returns <tt>true</tt> if all tasks have completed following shut down.
     * Note that <tt>isTerminated</tt> is never <tt>true</tt> unless
     * either <tt>shutdown</tt> or <tt>shutdownNow</tt> was called first.
     *
     * @return <tt>true</tt> if all tasks have completed following shut down
     */
	virtual boolean isTerminated() = 0;

    /**
     * Blocks until all tasks have completed execution after a shutdown
     * request, or the timeout occurs, or the current thread is
     * interrupted, whichever happens first.
     *
     * @param timeout the maximum time to wait
     * @param unit the time unit of the timeout argument
     * @return <tt>true</tt> if this executor terminated and
     *         <tt>false</tt> if the timeout elapsed before termination
     * @throws InterruptedException if interrupted while waiting
     */
	virtual boolean awaitTermination() THROWS(EInterruptedException) = 0;
	virtual boolean awaitTermination(llong timeout, ETimeUnit* unit)
	                                 THROWS(EInterruptedException) = 0;


    /**
     * Submits a value-returning task for execution and returns a
     * Future representing the pending results of the task. The
     * Future's <tt>get</tt> method will return the task's result upon
     * successful completion.
     *
     * <p>
     * If you would like to immediately block waiting
     * for a task, you can use constructions of the form
     * <tt>result = exec.submit(aCallable).get();</tt>
     *
     * <p> Note: The {@link Executors} class includes a set of methods
     * that can convert some other common closure-like objects,
     * for example, {@link java.security.PrivilegedAction} to
     * {@link Callable} form so they can be submitted.
     *
     * @param task the task to submit
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException if the task cannot be
     *         scheduled for execution
     * @throws NullPointerException if the task is null
     */
	template<typename T>
    sp<EFuture<T> > submit(sp<ECallable<T> > task) {
		if (task == null) throw ENullPointerException(__FILE__, __LINE__);
		sp<ERunnableFuture<T> > ftask = newTaskFor(task);
		execute(ftask);
		return ftask;
	}

    /**
     * Submits a Runnable task for execution and returns a Future
     * representing that task. The Future's <tt>get</tt> method will
     * return the given result upon successful completion.
     *
     * @param task the task to submit
     * @param result the result to return
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException if the task cannot be
     *         scheduled for execution
     * @throws NullPointerException if the task is null
     */
	template<typename T>
	sp<EFuture<T> > submit(sp<ERunnable> task, sp<T> result) {
		if (task == null) throw ENullPointerException(__FILE__, __LINE__);
		sp<ERunnableFuture<T> > ftask = newTaskFor(task, result);
		execute(ftask);
		return ftask;
	}

    /**
     * Submits a Runnable task for execution and returns a Future
     * representing that task. The Future's <tt>get</tt> method will
     * return <tt>null</tt> upon <em>successful</em> completion.
     *
     * @param task the task to submit
     * @return a Future representing pending completion of the task
     * @throws RejectedExecutionException if the task cannot be
     *         scheduled for execution
     * @throws NullPointerException if the task is null
     */
	template<typename T>
	sp<EFuture<T> > submit(sp<ERunnable> task) {
		if (task == null) throw ENullPointerException(__FILE__, __LINE__);
		sp<ERunnableFuture<T> > ftask = newTaskFor(task, sp<T>::nullPtr());
		execute(ftask);
		return ftask;
	}

    /**
     * Executes the given tasks, returning a list of Futures holding
     * their status and results when all complete.
     * {@link Future#isDone} is <tt>true</tt> for each
     * element of the returned list.
     * Note that a <em>completed</em> task could have
     * terminated either normally or by throwing an exception.
     * The results of this method are undefined if the given
     * collection is modified while this operation is in progress.
     *
     * @param tasks the collection of tasks
     * @return A list of Futures representing the tasks, in the same
     *         sequential order as produced by the iterator for the
     *         given task list, each of which has completed.
     * @throws InterruptedException if interrupted while waiting, in
     *         which case unfinished tasks are cancelled.
     * @throws NullPointerException if tasks or any of its elements are <tt>null</tt>
     * @throws RejectedExecutionException if any task cannot be
     *         scheduled for execution
     */
	template<typename T>
    eal<EFuture<T> > invokeAll(EConcurrentCollection<ECallable<T> >* tasks)
                                         THROWS(EInterruptedException) {
		if (tasks == null)
			throw ENullPointerException(__FILE__, __LINE__);
		eal<EFuture<T> > futures(tasks->size());
		boolean done = false;
		sp<EConcurrentIterator<ECallable<T> > > iter;
		try {
			iter = tasks->iterator();
			while (iter->hasNext()) {
				sp<ECallable<T> > t = iter->next();
				sp<ERunnableFuture<T> > f = newTaskFor(t);
				futures.add(f);
				execute(f);
			}
			for (int i = 0, size = futures.size(); i < size; i++) {
				sp<EFuture<T> > f = futures.get(i);
				if (!f->isDone()) {
					try {
						f->get();
					} catch (ECancellationException& ignore) {
					} catch (EExecutionException& ignore) {
					}
				}
			}
			done = true;
			goto FINALLY;
		} catch (...) {
			finally {
				if (!done)
					for (int i = 0, size = futures.size(); i < size; i++)
						futures.get(i)->cancel(true);
			}
			throw; //!
		}
		FINALLY:
		finally {
			if (!done)
				for (int i = 0, size = futures.size(); i < size; i++)
					futures.get(i)->cancel(true);
		}
		return futures;
	}

    /**
     * Executes the given tasks, returning a list of Futures holding
     * their status and results
     * when all complete or the timeout expires, whichever happens first.
     * {@link Future#isDone} is <tt>true</tt> for each
     * element of the returned list.
     * Upon return, tasks that have not completed are cancelled.
     * Note that a <em>completed</em> task could have
     * terminated either normally or by throwing an exception.
     * The results of this method are undefined if the given
     * collection is modified while this operation is in progress.
     *
     * @param tasks the collection of tasks
     * @param timeout the maximum time to wait
     * @param unit the time unit of the timeout argument
     * @return a list of Futures representing the tasks, in the same
     *         sequential order as produced by the iterator for the
     *         given task list. If the operation did not time out,
     *         each task will have completed. If it did time out, some
     *         of these tasks will not have completed.
     * @throws InterruptedException if interrupted while waiting, in
     *         which case unfinished tasks are cancelled
     * @throws NullPointerException if tasks, any of its elements, or
     *         unit are <tt>null</tt>
     * @throws RejectedExecutionException if any task cannot be scheduled
     *         for execution
     */
	template<typename T>
	eal<EFuture<T> > invokeAll(EConcurrentCollection<ECallable<T> >* tasks,
                                  llong timeout, ETimeUnit* unit)
                                  THROWS(EInterruptedException) {
		if (tasks == null)
			throw ENullPointerException(__FILE__, __LINE__);
		llong nanos = unit->toNanos(timeout);
		eal<EFuture<T> > futures(tasks->size());
		eal<ERunnableFuture<T> > futures_(tasks->size()); //FIXME?
		boolean done = false;
		sp<EConcurrentIterator<ECallable<T> > > iter;
		try {
			iter = tasks->iterator();
			while (iter->hasNext()) {
				sp<ECallable<T> > t = iter->next();
				sp<ERunnableFuture<T> > f = newTaskFor(t);
				futures.add(f);
				futures_.add(f);
			}

			llong deadline = ESystem::nanoTime() + nanos;
			int size = futures.size();

			// Interleave time checks and calls to execute in case
			// executor doesn't have any/much parallelism.
			for (int i = 0; i < size; i++) {
				execute(futures_.get(i));
				nanos = deadline - ESystem::nanoTime();
				if (nanos <= 0L) {
					goto FINALLY;
				}
			}

			for (int i = 0; i < size; i++) {
				sp<EFuture<T> > f = futures.get(i);
				if (!f->isDone()) {
					if (nanos <= 0L)
						goto FINALLY;
					try {
						f->get(nanos, ETimeUnit::NANOSECONDS);
					} catch (ECancellationException& ignore) {
					} catch (EExecutionException& ignore) {
					} catch (ETimeoutException& toe) {
						goto FINALLY;
					}
					nanos = deadline - ESystem::nanoTime();
				}
			}
			done = true;
			goto FINALLY;
		} catch (...) {
			finally {
				if (!done)
					for (int i = 0, size = futures.size(); i < size; i++)
						futures.get(i)->cancel(true);
			}
			throw; //!
		}
		FINALLY:
		finally {
			if (!done)
				for (int i = 0, size = futures.size(); i < size; i++)
					futures.get(i)->cancel(true);
		}
		return futures;
	}

    /**
     * Executes the given tasks, returning the result
     * of one that has completed successfully (i.e., without throwing
     * an exception), if any do. Upon normal or exceptional return,
     * tasks that have not completed are cancelled.
     * The results of this method are undefined if the given
     * collection is modified while this operation is in progress.
     *
     * @param tasks the collection of tasks
     * @return the result returned by one of the tasks
     * @throws InterruptedException if interrupted while waiting
     * @throws NullPointerException if tasks or any of its elements
     *         are <tt>null</tt>
     * @throws IllegalArgumentException if tasks is empty
     * @throws ExecutionException if no task successfully completes
     * @throws RejectedExecutionException if tasks cannot be scheduled
     *         for execution
     */
	template<typename T>
    sp<T> invokeAny(EConcurrentCollection<ECallable<T> >* tasks)
	                   THROWS2(EInterruptedException, EExecutionException) {
		try {
			return doInvokeAny(tasks, false, 0);
		} catch (ETimeoutException& cannotHappen) {
			ES_ASSERT(false);
			return null;
		}
	}

    /**
     * Executes the given tasks, returning the result
     * of one that has completed successfully (i.e., without throwing
     * an exception), if any do before the given timeout elapses.
     * Upon normal or exceptional return, tasks that have not
     * completed are cancelled.
     * The results of this method are undefined if the given
     * collection is modified while this operation is in progress.
     *
     * @param tasks the collection of tasks
     * @param timeout the maximum time to wait
     * @param unit the time unit of the timeout argument
     * @return the result returned by one of the tasks.
     * @throws InterruptedException if interrupted while waiting
     * @throws NullPointerException if tasks, any of its elements, or
     *         unit are <tt>null</tt>
     * @throws TimeoutException if the given timeout elapses before
     *         any task successfully completes
     * @throws ExecutionException if no task successfully completes
     * @throws RejectedExecutionException if tasks cannot be scheduled
     *         for execution
     */
	template<typename T>
	sp<T> invokeAny(EConcurrentCollection<ECallable<T> >* tasks,
                       llong timeout, ETimeUnit* unit)
	                   THROWS3(EInterruptedException, EExecutionException, ETimeoutException) {
		return doInvokeAny(tasks, true, unit->toNanos(timeout));
	}

private:
	/**
	 * Returns a {@code RunnableFuture} for the given runnable and default
	 * value.
	 *
	 * @param runnable the runnable task being wrapped
	 * @param value the default value for the returned future
	 * @param <T> the type of the given value
	 * @return a {@code RunnableFuture} which, when run, will run the
	 * underlying runnable and which, as a {@code Future}, will yield
	 * the given value as its result and provide for cancellation of
	 * the underlying task
	 * @since 1.6
	 */
	template<typename T>
	ERunnableFuture<T>* newTaskFor(sp<ERunnable> runnable, sp<T> value) {
		return new EFutureTask<T>(runnable, value);
	}

	/**
	 * Returns a {@code RunnableFuture} for the given callable task.
	 *
	 * @param callable the callable task being wrapped
	 * @param <T> the type of the callable's result
	 * @return a {@code RunnableFuture} which, when run, will call the
	 * underlying callable and which, as a {@code Future}, will yield
	 * the callable's result as its result and provide for
	 * cancellation of the underlying task
	 * @since 1.6
	 */
	template<typename T>
	ERunnableFuture<T>* newTaskFor(sp<ECallable<T> > callable) {
		return new EFutureTask<T>(callable);
	}

	/**
	 * the main mechanics of invokeAny.
	 */
	template<typename T>
	sp<T> doInvokeAny(EConcurrentCollection<ECallable<T> >* tasks, boolean timed,
			llong nanos) THROWS3(EInterruptedException, EExecutionException, ETimeoutException) {
		if (tasks == null)
			throw ENullPointerException(__FILE__, __LINE__);
		int ntasks = tasks->size();
		if (ntasks == 0)
			throw EIllegalArgumentException(__FILE__, __LINE__);
		eal<EFuture<T> > futures(ntasks);
		sp<EExecutorCompletionService<T> > ecs =
			new EExecutorCompletionService<T>(this);

		// For efficiency, especially in executors with limited
		// parallelism, check to see if previously submitted tasks are
		// done before submitting more of them. This interleaving
		// plus the exception mechanics account for messiness of main
		// loop.

		sp<T> result = null;
		sp<EConcurrentIterator<ECallable<T> > > it;
		try {
			// Record exceptions so that if we fail to obtain any
			// result, we can throw the last exception we got.
			llong deadline = timed ? ESystem::nanoTime() + nanos : 0L;
			it = tasks->iterator();

			// Start one task for sure; the rest incrementally
			futures.add(ecs->submit(it->next()));
			--ntasks;
			int active = 1;

			for (;;) {
				sp<EFuture<T> > f = ecs->poll();
				if (f == null) {
					if (ntasks > 0) {
						--ntasks;
						futures.add(ecs->submit(it->next()));
						++active;
					}
					else if (active == 0)
						break;
					else if (timed) {
						f = ecs->poll(nanos, ETimeUnit::NANOSECONDS);
						if (f == null)
							throw ETimeoutException(__FILE__, __LINE__);
						nanos = deadline - ESystem::nanoTime();
					}
					else
						f = ecs->take();
				}
				if (f != null) {
					--active;
					try {
						result = f->get();
						goto FINALLY;
					} catch (EExecutionException& eex) {
						throw eex;
					} catch (ERuntimeException& rex) {
						throw EExecutionException(rex.getSourceFile(), rex.getSourceLine(), rex.getMessage());
					}
				}
			}

			throw EExecutionException(__FILE__, __LINE__);

		} catch (...) {
			finally {
				for (int i = 0, size = futures.size(); i < size; i++)
					futures.get(i)->cancel(true);
			}
			throw; //!
		}
		FINALLY:
		finally {
			for (int i = 0, size = futures.size(); i < size; i++)
				futures.get(i)->cancel(true);
		}
		return result;
	}
};

} // namespace efc
#endif /* EEXECUTORSERVICE_HH_ */
