/*
 * ESemaphore.cpp
 *
 *  Created on: 2015-2-7
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/ESemaphore.hh"
#include "../../inc/concurrent/EAbstractQueuedSynchronizer.hh"
#include "../../inc/EIllegalArgumentException.hh"

namespace efc {

namespace semaphore {
/**
 * Synchronization implementation for semaphore.  Uses AQS state
 * to represent permits. Subclassed into fair and nonfair
 * versions.
 */
abstract class Sync : public EAbstractQueuedSynchronizer {
public:
	Sync(int permits) {
		setState(permits);
	}

	int getPermits() {
		return getState();
	}

	int nonfairTryAcquireShared(int acquires) {
		for (;;) {
			int available = getState();
			int remaining = available - acquires;
			if (remaining < 0 ||
				compareAndSetState(available, remaining))
				return remaining;
		}
	}

	boolean tryReleaseShared(int releases) {
		for (;;) {
			int current = getState();
			int next = current + releases;
			if (next < current) // overflow
				throw ERuntimeException(__FILE__, __LINE__, "Maximum permit count exceeded");
			if (compareAndSetState(current, next))
				return true;
		}
	}

	void reducePermits(int reductions) {
		for (;;) {
			int current = getState();
			int next = current - reductions;
			if (next > current) // underflow
				throw ERuntimeException(__FILE__, __LINE__, "Permit count underflow");
			if (compareAndSetState(current, next))
				return;
		}
	}

	int drainPermits() {
		for (;;) {
			int current = getState();
			if (current == 0 || compareAndSetState(current, 0))
				return current;
		}
	}

	/**
	 * Returns {@code true} if this lock has fairness set true.
	 *
	 * @return {@code true} if this lock has fairness set true
	 */
	virtual boolean isFair() const = 0;
};

/**
 * NonFair version
 */
class NonfairSync : public Sync {
public:
	NonfairSync(int permits) : Sync(permits) {
	}

	int tryAcquireShared(int acquires) {
		return nonfairTryAcquireShared(acquires);
	}

	boolean isFair() const {
		return false;
	}
};

/**
 * Fair version
 */
class FairSync : public Sync {
public:
	FairSync(int permits) : Sync(permits) {
	}

	int tryAcquireShared(int acquires) {
		for (;;) {
			if (hasQueuedPredecessors())
				return -1;
			int available = getState();
			int remaining = available - acquires;
			if (remaining < 0 ||
				compareAndSetState(available, remaining))
				return remaining;
		}
	}

	boolean isFair() const {
		return true;
	}
};
} /* namespace semaphore */

ESemaphore::~ESemaphore() {
	delete sync;
}

ESemaphore::ESemaphore(int permits) {
	sync = new semaphore::NonfairSync(permits);
}

ESemaphore::ESemaphore(int permits, boolean fair) {
	if (fair)
		sync =new semaphore::FairSync(permits);
	else
		sync = new semaphore::NonfairSync(permits);
}

void ESemaphore::acquire() {
	sync->acquireSharedInterruptibly(1);
}

void ESemaphore::acquireUninterruptibly() {
	sync->acquireShared(1);
}

boolean ESemaphore::tryAcquire() {
	return sync->nonfairTryAcquireShared(1) >= 0;
}

boolean ESemaphore::tryAcquire(llong timeout, ETimeUnit* unit) {
	return sync->tryAcquireSharedNanos(1, unit->toNanos(timeout));
}

void ESemaphore::release() {
	sync->releaseShared(1);
}

void ESemaphore::acquire(int permits) {
	if (permits < 0) throw EIllegalArgumentException(__FILE__, __LINE__);
	sync->acquireSharedInterruptibly(permits);
}

void ESemaphore::acquireUninterruptibly(int permits) {
	if (permits < 0) throw EIllegalArgumentException(__FILE__, __LINE__);
	sync->acquireShared(permits);
}

boolean ESemaphore::tryAcquire(int permits) {
	if (permits < 0) throw EIllegalArgumentException(__FILE__, __LINE__);
	return sync->nonfairTryAcquireShared(permits) >= 0;
}

boolean ESemaphore::tryAcquire(int permits, llong timeout, ETimeUnit* unit) {
	if (permits < 0) throw EIllegalArgumentException(__FILE__, __LINE__);
	return sync->tryAcquireSharedNanos(permits, unit->toNanos(timeout));
}

void ESemaphore::release(int permits) {
	if (permits < 0) throw EIllegalArgumentException(__FILE__, __LINE__);
	sync->releaseShared(permits);
}

int ESemaphore::availablePermits() {
	return sync->getPermits();
}

int ESemaphore::drainPermits() {
	return sync->drainPermits();
}

boolean ESemaphore::isFair() {
	return sync->isFair();
}

boolean ESemaphore::hasQueuedThreads() {
	return sync->hasQueuedThreads();
}

int ESemaphore::getQueueLength() {
	return sync->getQueueLength();
}

EString ESemaphore::toString() {
	return EString::formatOf("ESemaphore[Permits = %d]", sync->getPermits());
}

void ESemaphore::reducePermits(int reduction) {
	if (reduction < 0) throw EIllegalArgumentException(__FILE__, __LINE__);
	sync->reducePermits(reduction);
}

ECollection<EThread*>* ESemaphore::getQueuedThreads() {
	return sync->getQueuedThreads();
}

} /* namespace efc */
