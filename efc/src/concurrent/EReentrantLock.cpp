/*
 * EReentrantLock.cpp
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EReentrantLock.hh"
#include "../../inc/concurrent/EAbstractQueuedSynchronizer.hh"

using namespace efc::lock;

namespace efc {

namespace lock {
/**
 * Base of synchronization control for this lock. Subclassed
 * into fair and nonfair versions below. Uses AQS state to
 * represent the number of holds on the lock.
 */
abstract class Sync: public EAbstractQueuedSynchronizer {
public:
	/**
	 * Performs {@link Lock#lock}. The main reason for subclassing
	 * is to allow fast path for nonfair version.
	 */
	virtual void lock() = 0;

	/**
	 * Returns {@code true} if this lock has fairness set true.
	 *
	 * @return {@code true} if this lock has fairness set true
	 */
	virtual boolean isFair() const = 0;

	/**
	 * Performs non-fair tryLock.  tryAcquire is
	 * implemented in subclasses, but both need nonfair
	 * try for trylock method.
	 */
	boolean nonfairTryAcquire(int acquires) {
		EThread* current = EThread::currentThread();
		int c = getState();
		if (c == 0) {
			if (compareAndSetState(0, acquires)) {
				setExclusiveOwnerThread(current);
				return true;
			}
		}
		else if (current == getExclusiveOwnerThread()) {
			int nextc = c + acquires;
			if (nextc < 0) // overflow
				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
			setState(nextc);
			return true;
		}
		return false;
	}

	boolean tryRelease(int releases) {
		int c = getState() - releases;
		if (EThread::currentThread() != getExclusiveOwnerThread())
			throw EIllegalStateException(__FILE__, __LINE__);
		boolean free = false;
		if (c == 0) {
			free = true;
			setExclusiveOwnerThread(null);
		}
		setState(c);
		return free;
	}

	boolean isHeldExclusively() {
		// While we must in general read state before owner,
		// we don't need to do so to check if current thread is owner
		return getExclusiveOwnerThread() == EThread::currentThread();
	}

	EAbstractQueuedSynchronizer::ConditionObject* newCondition() {
		return new EAbstractQueuedSynchronizer::ConditionObject(this);
	}

	// Methods relayed from outer class

	EThread* getOwner() {
		return getState() == 0 ? null : getExclusiveOwnerThread();
	}

	int getHoldCount() {
		return isHeldExclusively() ? getState() : 0;
	}

	boolean isLocked() {
		return getState() != 0;
	}
};


/**
 * Sync object for non-fair locks
 */
class NonfairSync : public Sync {
public:
	/**
	 * Performs lock.  Try immediate barge, backing up to normal
	 * acquire on failure.
	 */
	void lock() {
		if (compareAndSetState(0, 1))
			setExclusiveOwnerThread(EThread::currentThread());
		else
			acquire(1);
	}

	boolean tryAcquire(int acquires) {
		return nonfairTryAcquire(acquires);
	}

	boolean isFair() const {
		return false;
	}
};

/**
 * Sync object for fair locks
 */
class FairSync: public Sync {
public:
	void lock() {
		acquire(1);
	}

	/**
	 * Fair version of tryAcquire.  Don't grant access unless
	 * recursive call or no waiters or is first.
	 */
	boolean tryAcquire(int acquires) {
		EThread* current = EThread::currentThread();
		int c = getState();
		if (c == 0) {
			if (!hasQueuedPredecessors() && compareAndSetState(0, acquires)) {
//			if (isFirst(current) && compareAndSetState(0, acquires)) {
				setExclusiveOwnerThread(current);
				return true ;
			}
		} else if (current == getExclusiveOwnerThread()) {
			int nextc = c + acquires;
			if (nextc < 0)
				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
			setState(nextc);
			return true ;
		}
		return false ;
	}

	boolean isFair() const {
		return true;
	}
};

} /* namespace lock */

EReentrantLock::EReentrantLock() {
	sync = new NonfairSync();
}

EReentrantLock::EReentrantLock(boolean fair) {
	if (fair)
		sync = new FairSync();
	else
		sync = new NonfairSync();
}

EReentrantLock::~EReentrantLock() {
	delete sync;
}

void EReentrantLock::lock()
{
	sync->lock();
}

void EReentrantLock::lockInterruptibly()
{
	sync->acquireInterruptibly(1);
}

boolean EReentrantLock::tryLock()
{
	return sync->nonfairTryAcquire(1);
}

boolean EReentrantLock::tryLock(llong timeout, ETimeUnit* unit)
{
	return sync->tryAcquireNanos(1, unit->toNanos(timeout));
}

void EReentrantLock::unlock()
{
	sync->release(1);
}

ECondition* EReentrantLock::newCondition()
{
	return sync->newCondition();
}

int EReentrantLock::getHoldCount() {
	return sync->getHoldCount();
}

boolean EReentrantLock::isHeldByCurrentThread() {
	return sync->isHeldExclusively();
}

boolean EReentrantLock::isLocked() {
	return sync->isLocked();
}

boolean EReentrantLock::isFair() {
	return sync->isFair();
}

boolean EReentrantLock::hasQueuedThreads() {
	return sync->hasQueuedThreads();
}

boolean EReentrantLock::hasQueuedThread(EThread* thread) {
	return sync->isQueued(thread);
}

int EReentrantLock::getQueueLength() {
	return sync->getQueueLength();
}

ECollection<EThread*>* EReentrantLock::getQueuedThreads() {
	return sync->getQueuedThreads();
}

boolean EReentrantLock::hasWaiters(ECondition* condition) {
	if (condition == null)
		throw ENullPointerException(__FILE__, __LINE__);

	const EAbstractQueuedSynchronizer::ConditionObject* cond =
		dynamic_cast<const EAbstractQueuedSynchronizer::ConditionObject*>(condition);
	if (cond == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Condition is not associated with this Lock");
	}

	return sync->hasWaiters((EAbstractQueuedSynchronizer::ConditionObject*)condition);
}

int EReentrantLock::getWaitQueueLength(ECondition* condition) {
	if (condition == null)
		throw ENullPointerException(__FILE__, __LINE__);

	const EAbstractQueuedSynchronizer::ConditionObject* cond =
		dynamic_cast<const EAbstractQueuedSynchronizer::ConditionObject*>(condition);
	if (cond == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "Condition is not associated with this Lock");
	}
	return sync->getWaitQueueLength((EAbstractQueuedSynchronizer::ConditionObject*)condition);
}

EString EReentrantLock::toString() {
	EThread* o = sync->getOwner();
	return (o == null) ?
			EString("EReentrantLock[Unlocked]") :
			EString::formatOf("EReentrantLock[Locked by thread %s]", o->getName());
}

} /* namespace efc */
