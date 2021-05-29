/*
 * EThreadPoolExecutor.cpp
 *
 *  Created on: 2015-3-5
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EExecutors.hh"
#include "../../inc/concurrent/EThreadPoolExecutor.hh"
#include "../../inc/ENullPointerException.hh"
#include "../../inc/EIllegalArgumentException.hh"
#include "../../inc/EIllegalThreadStateException.hh"
#include "../../inc/EConcurrentModificationException.hh"

namespace efc {

/**
 * The main pool control state, ctl, is an atomic integer packing
 * two conceptual fields
 *   workerCount, indicating the effective number of threads
 *   runState,    indicating whether running, shutting down etc
 *
 * In order to pack them into one int, we limit workerCount to
 * (2^29)-1 (about 500 million) threads rather than (2^31)-1 (2
 * billion) otherwise representable. If this is ever an issue in
 * the future, the variable can be changed to be an AtomicLong,
 * and the shift/mask constants below adjusted. But until the need
 * arises, this code is a bit faster and simpler using an int.
 *
 * The workerCount is the number of workers that have been
 * permitted to start and not permitted to stop.  The value may be
 * transiently different from the actual number of live threads,
 * for example when a ThreadFactory fails to create a thread when
 * asked, and when exiting threads are still performing
 * bookkeeping before terminating. The user-visible pool size is
 * reported as the current size of the workers set.
 *
 * The runState provides the main lifecyle control, taking on values:
 *
 *   RUNNING:  Accept new tasks and process queued tasks
 *   SHUTDOWN: Don't accept new tasks, but process queued tasks
 *   STOP:     Don't accept new tasks, don't process queued tasks,
 *             and interrupt in-progress tasks
 *   TIDYING:  All tasks have terminated, workerCount is zero,
 *             the thread transitioning to state TIDYING
 *             will run the terminated() hook method
 *   TERMINATED: terminated() has completed
 *
 * The numerical order among these values matters, to allow
 * ordered comparisons. The runState monotonically increases over
 * time, but need not hit each state. The transitions are:
 *
 * RUNNING -> SHUTDOWN
 *    On invocation of shutdown(), perhaps implicitly in finalize()
 * (RUNNING or SHUTDOWN) -> STOP
 *    On invocation of shutdownNow()
 * SHUTDOWN -> TIDYING
 *    When both queue and pool are empty
 * STOP -> TIDYING
 *    When pool is empty
 * TIDYING -> TERMINATED
 *    When the terminated() hook method has completed
 *
 * Threads waiting in awaitTermination() will return when the
 * state reaches TERMINATED.
 *
 * Detecting the transition from SHUTDOWN to TIDYING is less
 * straightforward than you'd like because the queue may become
 * empty after non-empty and vice versa during SHUTDOWN state, but
 * we can only terminate if, after seeing that it is empty, we see
 * that workerCount is 0 (which sometimes entails a recheck -- see
 * below).
 */

sp<ERejectedExecutionHandler> EThreadPoolExecutor::defaultHandler = new EThreadPoolExecutor::AbortPolicy();

DEFINE_STATIC_INITZZ_BEGIN(EThreadPoolExecutor)
EThread::_initzz_();
DEFINE_STATIC_INITZZ_END

namespace tpe {

/**
 * Class Worker mainly maintains interrupt control state for
 * threads running tasks, along with other minor bookkeeping.
 * This class opportunistically extends AbstractQueuedSynchronizer
 * to simplify acquiring and releasing a lock surrounding each
 * task execution.  This protects against interrupts that are
 * intended to wake up a worker thread waiting for a task from
 * instead interrupting a task being run.  We implement a simple
 * non-reentrant mutual exclusion lock rather than use
 * ReentrantLock because we do not want worker tasks to be able to
 * reacquire the lock when they invoke pool control methods like
 * setCorePoolSize.  Additionally, to suppress interrupts until
 * the thread actually starts running tasks, we initialize lock
 * state to a negative value, and clear it upon start (in
 * runWorker).
 */
class Worker: public EAbstractQueuedSynchronizer, public ERunnable, public enable_shared_from_this<Worker> {
public:
	EThreadPoolExecutor* self;

	/** Thread this worker is running in.  Null if factory fails. */
	EThread* thread;
	/** Initial task to run.  Possibly null. */
	sp<ERunnable> firstTask;
	/** Per-thread task counter */
	volatile long completedTasks;

	~Worker() {
		//
	}

	/**
	 * Creates with given first task and thread from ThreadFactory.
	 * @param firstTask the first task (null if none)
	 */
	Worker(EThreadPoolExecutor* tpe, sp<ERunnable> f) :
			self(tpe), firstTask(f), completedTasks(0) {
		setState(-1); // inhibit interrupts until runWorker
	}

	void init() {
		self->aliveThreadCount2++;
		sp<Worker> w = shared_from_this();
		this->thread = self->getThreadFactory()->newThread(w);
		EThread::setDaemon(this->thread, true);
		this->thread->injectExitCallback(worker_thread_exit_callback, new sp<Worker>(w));
	}

	/** Delegates main run loop to outer runWorker  */
	virtual void run() {
		self->runWorker(shared_from_this());
	}

	// Lock methods
	//
	// The value 0 represents the unlocked state.
	// The value 1 represents the locked state.

	boolean isHeldExclusively() {
		return getState() != 0;
	}

	boolean tryAcquire(int unused) {
		if (compareAndSetState(0, 1)) {
			setExclusiveOwnerThread(EThread::currentThread());
			return true ;
		}
		return false ;
	}

	boolean tryRelease(int unused) {
		setExclusiveOwnerThread(null);
		setState(0);
		return true ;
	}

	void lock() {
		acquire(1);
	}
	boolean tryLock() {
		return tryAcquire(1);
	}
	void unlock() {
		release(1);
	}
	boolean isLocked() {
		return isHeldExclusively();
	}

	void interruptIfStarted() {
		EThread* t;
		if (getState() >= 0 && (t = thread) != null
				&& !t->isInterrupted()) {
			try {
				t->interrupt();
			} catch (ESecurityException& ignore) {
			}
		}
	}

	static void worker_thread_exit_callback(void* arg) {
		sp<tpe::Worker>* pw = (sp<tpe::Worker>*)arg;
		ES_ASSERT(pw);
		sp<tpe::Worker> w = *pw;

		SYNCBLOCK(&w->self->mainLock) {
			w->self->aliveThreadCount1--;
			w->self->aliveThreadCount2--;

			if (w->self->isTerminated() && w->self->aliveThreadCount2 == 0) {
				w->self->termination->signalAll();
			}
        }}

		delete pw;
	}
};

} /* namespace tpe */

//=============================================================================

EThreadPoolExecutor::~EThreadPoolExecutor() {
	this->shutdown();
	this->awaitTermination();
	delete termination;
	delete ctl;
	delete workers;
}

EThreadPoolExecutor::EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
		llong keepAliveTime, ETimeUnit* unit,
		sp<EBlockingQueue<ERunnable> > workQueue): mainLock(ES_THREAD_MUTEX_NESTED) {
	init(corePoolSize, maximumPoolSize, keepAliveTime, unit, workQueue,
			EExecutors::defaultThreadFactory(), defaultHandler);
}

EThreadPoolExecutor::EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
		llong keepAliveTime, ETimeUnit* unit,
		sp<EBlockingQueue<ERunnable> > workQueue, sp<EThreadFactory> threadFactory): mainLock(ES_THREAD_MUTEX_NESTED) {
	init(corePoolSize, maximumPoolSize, keepAliveTime, unit, workQueue,
			threadFactory, defaultHandler);
}

EThreadPoolExecutor::EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
		llong keepAliveTime, ETimeUnit* unit,
		sp<EBlockingQueue<ERunnable> > workQueue,
		sp<ERejectedExecutionHandler > handler): mainLock(ES_THREAD_MUTEX_NESTED) {
	init(corePoolSize, maximumPoolSize, keepAliveTime, unit, workQueue,
			EExecutors::defaultThreadFactory(), handler);
}

EThreadPoolExecutor::EThreadPoolExecutor(int corePoolSize, int maximumPoolSize,
		llong keepAliveTime, ETimeUnit* unit,
		sp<EBlockingQueue<ERunnable> > workQueue, sp<EThreadFactory> threadFactory,
		sp<ERejectedExecutionHandler> handler): mainLock(ES_THREAD_MUTEX_NESTED) {
	init(corePoolSize, maximumPoolSize, keepAliveTime, unit, workQueue,
			threadFactory, handler);
}

void EThreadPoolExecutor::init(int corePoolSize, int maximumPoolSize,
		llong keepAliveTime, ETimeUnit* unit,
		sp<EBlockingQueue<ERunnable> > workQueue, sp<EThreadFactory> threadFactory,
		sp<ERejectedExecutionHandler> handler) {
	if (corePoolSize < 0 ||
		maximumPoolSize <= 0 ||
		maximumPoolSize < corePoolSize ||
		keepAliveTime < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	if (workQueue == null || threadFactory == null || handler == null)
		throw ENullPointerException(__FILE__, __LINE__);
	this->corePoolSize = corePoolSize;
	this->maximumPoolSize = maximumPoolSize;
	this->workQueue = workQueue;
	this->keepAliveTime = unit->toNanos(keepAliveTime);
	this->threadFactory = threadFactory;
	this->handler = handler;

	this->ctl = new EAtomicInteger(ctlOf(RUNNING, 0));
	this->workers = new EArrayList<sp<tpe::Worker> >();
	this->termination = mainLock.newCondition();
	this->largestPoolSize = 0;
	this->completedTaskCount = 0;

	this->allowCoreThreadTimeOut_ = false;
}

void EThreadPoolExecutor::execute(sp<ERunnable> command) {
	if (command == null)
		throw ENullPointerException(__FILE__, __LINE__);
	/*
	 * Proceed in 3 steps:
	 *
	 * 1. If fewer than corePoolSize threads are running, try to
	 * start a new thread with the given command as its first
	 * task.  The call to addWorker atomically checks runState and
	 * workerCount, and so prevents false alarms that would add
	 * threads when it shouldn't, by returning false.
	 *
	 * 2. If a task can be successfully queued, then we still need
	 * to double-check whether we should have added a thread
	 * (because existing ones died since last checking) or that
	 * the pool shut down since entry into this method. So we
	 * recheck state and if necessary roll back the enqueuing if
	 * stopped, or start a new thread if there are none.
	 *
	 * 3. If we cannot queue task, then we try to add a new
	 * thread.  If it fails, we know we are shut down or saturated
	 * and so reject the task.
	 */
	int c = ctl->get();
	if (workerCountOf(c) < corePoolSize) {
		if (addWorker(command, true))
			return;
		c = ctl->get();
	}
	if (isRunning(c) && workQueue->offer(command)) {
		int recheck = ctl->get();
		if (! isRunning(recheck) && remove(command))
			reject(command);
		else if (workerCountOf(recheck) == 0)
			addWorker(null, false);
	}
	else if (!addWorker(command, false))
		reject(command);
}

void EThreadPoolExecutor::shutdown() {
	SYNCBLOCK(&mainLock) {
		checkShutdownAccess();
		advanceRunState(SHUTDOWN);
		interruptIdleWorkers();
		onShutdown(); // hook for ScheduledThreadPoolExecutor
    }}
	tryTerminate();
}

EArrayList<sp<ERunnable> > EThreadPoolExecutor::shutdownNow() {
	EArrayList<sp<ERunnable> > tasks;
	SYNCBLOCK(&mainLock) {
		checkShutdownAccess();
		advanceRunState(STOP);
		interruptWorkers();
		tasks = drainQueue();
    }}
	tryTerminate();
	return tasks;
}

boolean EThreadPoolExecutor::isShutdown() {
	return ! isRunning(ctl->get());
}

boolean EThreadPoolExecutor::isTerminating() {
	int c = ctl->get();
	return ! isRunning(c) && runStateLessThan(c, TERMINATED);
}

boolean EThreadPoolExecutor::isTerminated() {
	//@see: return runStateAtLeast(ctl.get(), TERMINATED);
	return runStateAtLeast(ctl->get(), TERMINATED) && (aliveThreadCount1 == 0);
}

boolean EThreadPoolExecutor::awaitTermination() {
	SYNCBLOCK(&mainLock) {
		for (;;) {
			if (isTerminated()) {
				break;
			}
			termination->await();
		}
    }}

	return true;
}

boolean EThreadPoolExecutor::awaitTermination(llong timeout, ETimeUnit* unit) {
	llong nanos = unit->toNanos(timeout);
	SYNCBLOCK(&mainLock) {
		for (;;) {
			if (isTerminated())
				return true;
			if (nanos <= 0)
				return false;
			nanos = termination->awaitNanos(nanos);
		}
    }}
	//not reach here!
	return false;
}

void EThreadPoolExecutor::setThreadFactory(sp<EThreadFactory> threadFactory) {
	if (threadFactory == null)
		throw ENullPointerException(__FILE__, __LINE__);

	atomic_store(&this->threadFactory, threadFactory);
}

sp<EThreadFactory> EThreadPoolExecutor::getThreadFactory() {
	return atomic_load(&threadFactory);
}

void EThreadPoolExecutor::setRejectedExecutionHandler(
		sp<ERejectedExecutionHandler> handler) {
	if (handler == null)
		throw ENullPointerException(__FILE__, __LINE__);

	atomic_store(&this->handler, handler);
}

sp<ERejectedExecutionHandler> EThreadPoolExecutor::getRejectedExecutionHandler() {
	return atomic_load(&handler);
}

void EThreadPoolExecutor::setCorePoolSize(int corePoolSize) {
	if (corePoolSize < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	int delta = corePoolSize - this->corePoolSize;
	this->corePoolSize = corePoolSize;
	if (workerCountOf(ctl->get()) > corePoolSize)
		interruptIdleWorkers();
	else if (delta > 0) {
		// We don't really know how many new threads are "needed".
		// As a heuristic, prestart enough new workers (up to new
		// core size) to handle the current number of tasks in
		// queue, but stop if queue becomes empty while doing so.
		int k = ES_MIN(delta, workQueue->size());
		while (k-- > 0 && addWorker(null, true)) {
			if (workQueue->isEmpty())
				break;
		}
	}
}

int EThreadPoolExecutor::getCorePoolSize() {
	return corePoolSize;
}

boolean EThreadPoolExecutor::prestartCoreThread() {
	return workerCountOf(ctl->get()) < corePoolSize &&
			addWorker(null, true);
}

int EThreadPoolExecutor::prestartAllCoreThreads() {
	int n = 0;
	while (addWorker(null, true))
		++n;
	return n;
}

boolean EThreadPoolExecutor::allowsCoreThreadTimeOut() {
	return allowCoreThreadTimeOut_;
}

void EThreadPoolExecutor::allowCoreThreadTimeOut(boolean value) {
	if (value && keepAliveTime <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Core threads must have nonzero keep alive times");
	if (value != allowCoreThreadTimeOut_) {
		allowCoreThreadTimeOut_ = value;
		if (value)
			interruptIdleWorkers();
	}
}

void EThreadPoolExecutor::setMaximumPoolSize(int maximumPoolSize) {
	if (maximumPoolSize <= 0 || maximumPoolSize < corePoolSize)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	this->maximumPoolSize = maximumPoolSize;
	if (workerCountOf(ctl->get()) > maximumPoolSize)
		interruptIdleWorkers();
}

int EThreadPoolExecutor::getMaximumPoolSize() {
	return maximumPoolSize;
}

void EThreadPoolExecutor::setKeepAliveTime(llong time, ETimeUnit* unit) {
	if (time < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__);
	if (time == 0 && allowsCoreThreadTimeOut())
		throw EIllegalArgumentException(__FILE__, __LINE__, "Core threads must have nonzero keep alive times");
	llong keepAliveTime = unit->toNanos(time);
	llong delta = keepAliveTime - this->keepAliveTime;
	this->keepAliveTime = keepAliveTime;
	if (delta < 0)
		interruptIdleWorkers();
}

llong EThreadPoolExecutor::getKeepAliveTime(ETimeUnit* unit) {
	return unit->convert(keepAliveTime, ETimeUnit::NANOSECONDS);
}

sp<EBlockingQueue<ERunnable> > EThreadPoolExecutor::getQueue() {
	return workQueue;
}

boolean EThreadPoolExecutor::remove(sp<ERunnable> task) {
	boolean removed = workQueue->remove(task.get());
	tryTerminate(); // In case SHUTDOWN and now empty
	return removed;
}

void EThreadPoolExecutor::purge() {
	sp<EBlockingQueue<ERunnable> > q = workQueue;
	try {
		sp<EIterator<sp<ERunnable> > > it = q->iterator();
		while (it->hasNext()) {
			sp<ERunnable> r = it->next();
			EFutureType* future = dynamic_cast<EFutureType*>(r.get());
			if (r != null && future->isCancelled()) {
				it->remove();
			}
		}
	} catch (EConcurrentModificationException& fallThrough) {
		// Take slow path if we encounter interference during traversal.
		// Make copy for traversal and call remove for cancelled entries.
		// The slow path is more likely to be O(N*N).
		EA<sp<ERunnable> > array = q->toArray();
		for (int i=0; i<array.length(); i++) {
			EFutureType* future = dynamic_cast<EFutureType*>(array[i].get());
			if (future != null && future->isCancelled()) {
				q->remove(array[i].get());
			}
		}
	}

	tryTerminate(); // In case SHUTDOWN and now empty
}

int EThreadPoolExecutor::getPoolSize() {
	SYNCBLOCK(&mainLock) {
		// Remove rare and surprising possibility of
		// isTerminated() && getPoolSize() > 0
		return runStateAtLeast(ctl->get(), TIDYING) ? 0
			: workers->size();
    }}
}

int EThreadPoolExecutor::getActiveCount() {
	SYNCBLOCK(&mainLock) {
		int n = 0;
		for (int i = 0; i < workers->size(); i++) {
			if (workers->getAt(i)->isLocked()) {
				++n;
			}
		}
		return n;
    }}
}

int EThreadPoolExecutor::getLargestPoolSize() {
	SYNCBLOCK(&mainLock) {
		return largestPoolSize;
    }}
}

llong EThreadPoolExecutor::getTaskCount() {
	SYNCBLOCK(&mainLock) {
		llong n = completedTaskCount;
		for (int i = 0; i < workers->size(); i++) {
			sp<tpe::Worker> w = workers->getAt(i);
			n += w->completedTasks;
			if (w->isLocked())
				++n;
		}
		return n + workQueue->size();
    }}
}

llong EThreadPoolExecutor::getCompletedTaskCount() {
	SYNCBLOCK(&mainLock) {
		llong n = completedTaskCount;
		for (int i = 0; i < workers->size(); i++) {
			n += workers->getAt(i)->completedTasks;
		}
		return n;
    }}
}

EString EThreadPoolExecutor::toString() {
	llong ncompleted;
	int nworkers, nactive;
	SYNCBLOCK(&mainLock) {
		ncompleted = completedTaskCount;
		nactive = 0;
		nworkers = workers->size();
		for (int i = 0; i < nworkers; i++) {
			sp<tpe::Worker> w = workers->getAt(i);
			ncompleted += w->completedTasks;
			if (w->isLocked())
				++nactive;
		}
    }}
	int c = ctl->get();
	const char* rs = (runStateLessThan(c, SHUTDOWN) ? "Running" :
				 (runStateAtLeast(c, TERMINATED) ? "Terminated" :
				  "Shutting down"));
	return EString::formatOf("EThreadPoolExecutor[%s"
		", pool size = %d"
		", active threads = %d"
		", queued tasks = %ld"
		", completed tasks = %ld"
		"]", rs, nworkers, nactive, workQueue->size(), ncompleted);
}

boolean EThreadPoolExecutor::compareAndIncrementWorkerCount(int expect) {
	return ctl->compareAndSet(expect, expect + 1);
}

boolean EThreadPoolExecutor::compareAndDecrementWorkerCount(int expect) {
	return ctl->compareAndSet(expect, expect - 1);
}

void EThreadPoolExecutor::onShutdown() {
	//
}

void EThreadPoolExecutor::finalize() {
	shutdown();
}

void EThreadPoolExecutor::decrementWorkerCount() {
	do {} while (! compareAndDecrementWorkerCount(ctl->get()));
}

void EThreadPoolExecutor::advanceRunState(int targetState) {
	for (;;) {
		int c = ctl->get();
		if (runStateAtLeast(c, targetState) ||
			ctl->compareAndSet(c, ctlOf(targetState, workerCountOf(c))))
			break;
	}
}

void EThreadPoolExecutor::tryTerminate() {
	for (;;) {
		int c = ctl->get();
		if (isRunning(c) ||
			runStateAtLeast(c, TIDYING) ||
			(runStateOf(c) == SHUTDOWN && ! workQueue->isEmpty()))
			return;
		if (workerCountOf(c) != 0) { // Eligible to terminate
			interruptIdleWorkers(true);// ONLY_ONE = true;
			return;
		}

		SYNCBLOCK(&mainLock) {
			if (ctl->compareAndSet(c, ctlOf(TIDYING, 0))) {
				try {
					terminated();
				} catch (...) {
					ctl->set(ctlOf(TERMINATED, 0));
//					termination->signalAll();
					throw; //!
				} finally {
					ctl->set(ctlOf(TERMINATED, 0));
//					termination->signalAll();
				}
				return;
			}
        }}
		// else retry on failed CAS
	}
}

void EThreadPoolExecutor::checkShutdownAccess() {
	//
}

void EThreadPoolExecutor::interruptWorkers() {
	SYNCBLOCK(&mainLock) {
		for (int i = 0; i < workers->size(); i++) {
			workers->getAt(i)->interruptIfStarted();
		}
    }}
}

void EThreadPoolExecutor::interruptIdleWorkers(boolean onlyOne) {
	SYNCBLOCK(&mainLock) {
		for (int i = 0; i < workers->size(); i++) {
			sp<tpe::Worker> w = workers->getAt(i);
			EThread* t = w->thread;
			if (!t->isInterrupted() && w->tryLock()) {
				try {
					t->interrupt();
				} catch (ESecurityException& ignore) {
				} catch (...) {
					w->unlock();
					throw; //!
				} finally {
					w->unlock();
				}
			}
			if (onlyOne)
				break;
		}
    }}
}

void EThreadPoolExecutor::interruptIdleWorkers() {
	interruptIdleWorkers(false);
}

void EThreadPoolExecutor::reject(sp<ERunnable> command) {
	atomic_load(&handler)->rejectedExecution(command, this);
}

boolean EThreadPoolExecutor::isRunningOrShutdown(boolean shutdownOK) {
	int rs = runStateOf(ctl->get());
	return rs == RUNNING || (rs == SHUTDOWN && shutdownOK);
}

EArrayList<sp<ERunnable> > EThreadPoolExecutor::drainQueue() {
	sp<EBlockingQueue<ERunnable> > q = workQueue;
	EArrayList<sp<ERunnable> > taskList(workQueue->size());
	q->drainTo(&taskList);
	if (!q->isEmpty()) {
		EA<sp<ERunnable> > x  = q->toArray();
		for (int i = 0; i < x.length(); i++) {
			sp<ERunnable> r = x[i];
			if (q->remove(r.get()))
				taskList.add(r);
		}
	}
	return taskList;
}

boolean EThreadPoolExecutor::addWorker(sp<ERunnable> firstTask, boolean core) {
	retry:
	for (;;) {
		int c = ctl->get();
		int rs = runStateOf(c);

		// Check if queue empty only if necessary.
		if (rs >= SHUTDOWN &&
			! (rs == SHUTDOWN &&
			   firstTask == null &&
			   ! workQueue->isEmpty()))
			return false;

		for (;;) {
			int wc = workerCountOf(c);
			if (wc >= CAPACITY ||
				wc >= (core ? corePoolSize : maximumPoolSize))
				return false;
			if (compareAndIncrementWorkerCount(c))
				goto resume;
			c = ctl->get();  // Re-read ctl
			if (runStateOf(c) != rs)
				goto retry;
			// else CAS failed due to workerCount change; retry inner loop
		}
	}

	resume:

	boolean workerStarted = false;
	boolean workerAdded = false;
	sp<tpe::Worker> w;// = null;
	try {
		w = new tpe::Worker(this, firstTask);
		w->init(); //!!!
		EThread* t = w->thread;
		if (t != null) {
			SYNCBLOCK(&mainLock) {
				// Recheck while holding lock.
				// Back out on ThreadFactory failure or if
				// shut down before lock acquired.
				int c = ctl->get();
				int rs = runStateOf(c);

				if (rs < SHUTDOWN ||
					(rs == SHUTDOWN && firstTask == null)) {
					if (t->isAlive()) // precheck that t is startable
						throw EIllegalThreadStateException(__FILE__, __LINE__);
					workers->add(w);
					int s = workers->size();
					if (s > largestPoolSize)
						largestPoolSize = s;
					workerAdded = true;
				}
            }}
			if (workerAdded) {
				aliveThreadCount1++;
				t->start();
				workerStarted = true;
			}
		}
	} catch (...) {
		if (! workerStarted)
			addWorkerFailed(w);
		throw; //!
	} finally {
		if (! workerStarted)
			addWorkerFailed(w);
	}
	return workerStarted;
}

void EThreadPoolExecutor::addWorkerFailed(sp<tpe::Worker>& w) {
	SYNCBLOCK(&mainLock) {
		if (w != null) {
			workers->remove(w.get());
		}
		decrementWorkerCount();
		tryTerminate();
    }}
}

void EThreadPoolExecutor::processWorkerExit(sp<tpe::Worker>& w,
		boolean completedAbruptly) {
	if (completedAbruptly) // If abrupt, then workerCount wasn't adjusted
		decrementWorkerCount();

	SYNCBLOCK(&mainLock) {
		completedTaskCount += w->completedTasks;
		workers->remove(w.get());
    }}

	tryTerminate();

	int c = ctl->get();
	if (runStateLessThan(c, STOP)) {
		if (!completedAbruptly) {
			int min = allowCoreThreadTimeOut_ ? 0 : corePoolSize;
			if (min == 0 && ! workQueue->isEmpty())
				min = 1;
			if (workerCountOf(c) >= min)
				return; // replacement not needed
		}
		addWorker(null, false);
	}
}

sp<ERunnable> EThreadPoolExecutor::getTask() {
	boolean timedOut = false; // Did the last poll() time out?

	retry:
	for (;;) {
		int c = ctl->get();
		int rs = runStateOf(c);

		// Check if queue empty only if necessary.
		if (rs >= SHUTDOWN && (rs >= STOP || workQueue->isEmpty())) {
			decrementWorkerCount();
			return null;
		}

		boolean timed;      // Are workers subject to culling?

		for (;;) {
			int wc = workerCountOf(c);
			timed = allowCoreThreadTimeOut_ || wc > corePoolSize;

			if (wc <= maximumPoolSize && ! (timedOut && timed))
				break;
			if (compareAndDecrementWorkerCount(c))
				return null;
			c = ctl->get();  // Re-read ctl
			if (runStateOf(c) != rs)
				goto retry;
			// else CAS failed due to workerCount change; retry inner loop
		}

		try {
			sp<ERunnable> r = timed ?
				workQueue->poll(keepAliveTime, ETimeUnit::NANOSECONDS) :
				workQueue->take();
			if (r != null)
				return r;
			timedOut = true;
		} catch (EInterruptedException& retry) {
			timedOut = false;
		}
	}
}

void EThreadPoolExecutor::ensurePrestart() {
	int wc = workerCountOf(ctl->get());
	if (wc < corePoolSize)
		addWorker(null, true);
	else if (wc == 0)
		addWorker(null, false);
}

void EThreadPoolExecutor::runWorker(sp<tpe::Worker> w) {
	EThread* wt = EThread::currentThread();
	sp<ERunnable> task(w->firstTask);
	w->firstTask = null;
	w->unlock(); // allow interrupts
	boolean completedAbruptly = true;
	try {
		while (task != null || (task = getTask()) != null) {
			w->lock();
			// If pool is stopping, ensure thread is interrupted;
			// if not, ensure thread is not interrupted.  This
			// requires a recheck in second case to deal with
			// shutdownNow race while clearing interrupt
			if ((runStateAtLeast(ctl->get(), STOP) ||
				 (EThread::interrupted() &&
				  runStateAtLeast(ctl->get(), STOP))) &&
				!wt->isInterrupted())
				wt->interrupt();
			try {
				beforeExecute(wt, task);
				EThrowable* thrown = null;
				try {
					task->run();
				} catch (ERuntimeException& x) {
					afterExecute(task, &x);
					throw x;
				} catch (EThrowable& x) {
					afterExecute(task, &x);
					throw x;
				}
				afterExecute(task, thrown);
			} catch (...) {
				task = null;
				w->completedTasks++;
				w->unlock();
				throw; //!
			} finally {
				task = null;
				w->completedTasks++;
				w->unlock();
			}
		}
		completedAbruptly = false;
	} catch (...) {
		processWorkerExit(w, completedAbruptly);
		throw; //!
	} finally {
		processWorkerExit(w, completedAbruptly);
	}
}

} /* namespace efc */
