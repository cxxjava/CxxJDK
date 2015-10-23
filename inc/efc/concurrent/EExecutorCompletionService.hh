/*
 * EExecutorCompletionService.hh
 *
 *  Created on: 2015-2-7
 *      Author: cxxjava@163.com
 */

#ifndef EEXECUTORCOMPLETIONSERVICE_HH_
#define EEXECUTORCOMPLETIONSERVICE_HH_

#include "EFutureTask.hh"
#include "ECompletionService.hh"
#include "ELinkedBlockingQueue.hh"

namespace efc {

/**
 * A {@link CompletionService} that uses a supplied {@link Executor}
 * to execute tasks.  This class arranges that submitted tasks are,
 * upon completion, placed on a queue accessible using {@code take}.
 * The class is lightweight enough to be suitable for transient use
 * when processing groups of tasks.
 *
 * <p>
 *
 * <b>Usage Examples.</b>
 *
 * Suppose you have a set of solvers for a certain problem, each
 * returning a value of some type {@code Result}, and would like to
 * run them concurrently, processing the results of each of them that
 * return a non-null value, in some method {@code use(Result r)}. You
 * could write this as:
 *
 * <pre> {@code
 * void solve(Executor e,
 *            Collection<Callable<Result>> solvers)
 *     throws InterruptedException, ExecutionException {
 *     CompletionService<Result> ecs
 *         = new ExecutorCompletionService<Result>(e);
 *     for (Callable<Result> s : solvers)
 *         ecs.submit(s);
 *     int n = solvers.size();
 *     for (int i = 0; i < n; ++i) {
 *         Result r = ecs.take().get();
 *         if (r != null)
 *             use(r);
 *     }
 * }}</pre>
 *
 * Suppose instead that you would like to use the first non-null result
 * of the set of tasks, ignoring any that encounter exceptions,
 * and cancelling all other tasks when the first one is ready:
 *
 * <pre> {@code
 * void solve(Executor e,
 *            Collection<Callable<Result>> solvers)
 *     throws InterruptedException {
 *     CompletionService<Result> ecs
 *         = new ExecutorCompletionService<Result>(e);
 *     int n = solvers.size();
 *     List<Future<Result>> futures
 *         = new ArrayList<Future<Result>>(n);
 *     Result result = null;
 *     try {
 *         for (Callable<Result> s : solvers)
 *             futures.add(ecs.submit(s));
 *         for (int i = 0; i < n; ++i) {
 *             try {
 *                 Result r = ecs.take().get();
 *                 if (r != null) {
 *                     result = r;
 *                     break;
 *                 }
 *             } catch (ExecutionException ignore) {}
 *         }
 *     }
 *     finally {
 *         for (Future<Result> f : futures)
 *             f.cancel(true);
 *     }
 *
 *     if (result != null)
 *         use(result);
 * }}</pre>
 */

template<typename V>
class EExecutorCompletionService : public ECompletionService<V> {
private:
    EExecutor *executor;
    sp<EBlockingQueue<EFuture<V> > > completionQueue;
    boolean owns;

    /**
     * FutureTask extension to enqueue upon completion
     */
    class QueueingFuture : public EFutureTask<V> {
    public:
        QueueingFuture(ERunnableFuture<V> *task, sp<EBlockingQueue<EFuture<V> > > queue) : EFutureTask<V>(task, null) {
            this->task = task;
            this->completionQueue = queue;
        }
    protected:
        virtual void done() {
        	completionQueue->add(task);
        }
    private:
        EFuture<V> *task;
        sp<EBlockingQueue<EFuture<V> > > completionQueue;
    };

protected:
    ERunnableFuture<V>* newTaskFor(ECallable<V> *task) {
		return new EFutureTask<V>(task);
    }

private:
    ERunnableFuture<V>* newTaskFor(ERunnable *task, sp<V> result) {
		return new EFutureTask<V>(task, result);
    }

public:
    virtual ~EExecutorCompletionService() {
    	if (!owns) {
    		completionQueue.detach();
    	}
    }

    /**
     * Creates an ExecutorCompletionService using the supplied
     * executor for base task execution and a
     * {@link LinkedBlockingQueue} as a completion queue.
     *
     * @param executor the executor to use
     * @throws NullPointerException if executor is {@code null}
     */
    EExecutorCompletionService(EExecutor *executor) {
        if (executor == null)
            throw ENullPointerException(__FILE__, __LINE__);
        this->executor = executor;
        this->completionQueue = new ELinkedBlockingQueue<EFuture<V> >();
        this->owns = true;
    }

    /**
     * Creates an ExecutorCompletionService using the supplied
     * executor for base task execution and the supplied queue as its
     * completion queue.
     *
     * @param executor the executor to use
     * @param completionQueue the queue to use as the completion queue
     *        normally one dedicated for use by this service. This
     *        queue is treated as unbounded -- failed attempted
     *        {@code Queue.add} operations for completed tasks cause
     *        them not to be retrievable.
     * @throws NullPointerException if executor or completionQueue are {@code null}
     */
    EExecutorCompletionService(EExecutor *executor,
                                     EBlockingQueue<EFuture<V> > *completionQueue) {
        if (executor == null || completionQueue == null)
            throw ENullPointerException(__FILE__, __LINE__);
        this->executor = executor;
        this->completionQueue = completionQueue;
        this->owns = false;
    }

    sp<EFuture<V> > submit(ECallable<V> *task) {
        if (task == null) throw ENullPointerException(__FILE__, __LINE__);
        sp<ERunnableFuture<V> > f = new QueueingFuture(newTaskFor(task), completionQueue);
        executor->execute(f);
        return f;
    }

    sp<EFuture<V> > submit(ERunnable *task, sp<V> result) {
        if (task == null) throw ENullPointerException(__FILE__, __LINE__);
        sp<ERunnableFuture<V> > f = new QueueingFuture(newTaskFor(task, result), completionQueue);
        executor->execute(f);
        return f;
    }

    sp<EFuture<V> > take() THROWS(EInterruptedException) {
        return completionQueue->take();
    }

    sp<EFuture<V> > poll() {
        return completionQueue->poll();
    }

    sp<EFuture<V> > poll(llong timeout, ETimeUnit *unit)
		THROWS(EInterruptedException) {
        return completionQueue->poll(timeout, unit);
    }

};

} /* namespace efc */
#endif /* EEXECUTORCOMPLETIONSERVICE_HH_ */
