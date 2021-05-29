/*
 * EExecutors.cpp
 *
 *  Created on: 2015-3-25
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EExecutors.hh"
#include "../../inc/concurrent/ELinkedBlockingQueue.hh"
#include "../../inc/concurrent/ESynchronousQueue.hh"

namespace efc {

EExecutorService* EExecutors::newFixedThreadPool(int nThreads) {
	return new EThreadPoolExecutor(nThreads, nThreads,
								  0L, ETimeUnit::MILLISECONDS,
								  new ELinkedBlockingQueue<ERunnable>());
}

EExecutorService* EExecutors::newFixedThreadPool(int nThreads, sp<EThreadFactory> threadFactory) {
	 return new EThreadPoolExecutor(nThreads, nThreads,
								  0L, ETimeUnit::MILLISECONDS,
								  new ELinkedBlockingQueue<ERunnable>(),
								  threadFactory);
}

EExecutorService* EExecutors::newSingleThreadExecutor() {
	/*
	return new FinalizableDelegatedExecutorService
		(new ThreadPoolExecutor(1, 1,
								0L, TimeUnit.MILLISECONDS,
								new LinkedBlockingQueue<Runnable>()));
	*/
	return new EThreadPoolExecutor(1, 1,
			0L, ETimeUnit::MILLISECONDS,
			new ELinkedBlockingQueue<ERunnable>());
}

EExecutorService* EExecutors::newSingleThreadExecutor(sp<EThreadFactory> threadFactory) {
	/*
    return new FinalizableDelegatedExecutorService
        (new ThreadPoolExecutor(1, 1,
                                0L, TimeUnit.MILLISECONDS,
                                new LinkedBlockingQueue<Runnable>(),
                                threadFactory));
	*/
	return new EThreadPoolExecutor(1, 1,
				0L, ETimeUnit::MILLISECONDS,
				new ELinkedBlockingQueue<ERunnable>(),
				threadFactory);
}

EExecutorService* EExecutors::newCachedThreadPool() {
	return new EThreadPoolExecutor(0, EInteger::MAX_VALUE,
								  60L, ETimeUnit::SECONDS,
								  new ESynchronousQueue<ERunnable>());
}

EExecutorService* EExecutors::newCachedThreadPool(sp<EThreadFactory> threadFactory) {
	return new EThreadPoolExecutor(0, EInteger::MAX_VALUE,
								  60L, ETimeUnit::SECONDS,
								  new ESynchronousQueue<ERunnable>(),
								  threadFactory);
}

//=============================================================================

EAtomicInteger* EExecutors::DefaultThreadFactory::poolNumber = new EAtomicInteger(1);

sp<EThreadFactory> EExecutors::defaultThreadFactory() {
	return new EExecutors::DefaultThreadFactory();
}

EExecutors::DefaultThreadFactory::DefaultThreadFactory() {
	threadNumber = new EAtomicInteger(1);
	/*
	SecurityManager s = System.getSecurityManager();
	group = (s != null) ? s.getThreadGroup() :
						  Thread.currentThread().getThreadGroup()
	*/
	namePrefix = EString::formatOf("pool-%d-thread-", poolNumber->getAndIncrement());
}

EExecutors::DefaultThreadFactory::~DefaultThreadFactory() {
	delete threadNumber;
}

EThread* EExecutors::DefaultThreadFactory::newThread(sp<ERunnable> r) {
    EString name(namePrefix);
    name.append(threadNumber->getAndIncrement());
	EThread* t = new EThread(/*group,*/ r, name.c_str() /*, 0*/);
    if (t->isDaemon())
		EThread::setDaemon(t, false);
	if (t->getPriority() != EThread::NORM_PRIORITY)
		t->setPriority(EThread::NORM_PRIORITY);
	return t;
}

sp<EThreadFactory> EExecutors::privilegedThreadFactory() {
	//FIXME: return new PrivilegedThreadFactory();
	return new EExecutors::DefaultThreadFactory();
}

} /* namespace efc */
