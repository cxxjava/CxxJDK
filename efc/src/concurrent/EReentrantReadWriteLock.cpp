/*
 * EReentrantReadWriteLock.cpp
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EReentrantReadWriteLock.hh"
#include "../../inc/ERuntimeException.hh"
#include "../../inc/concurrent/EAbstractQueuedSynchronizer.hh"
#include "../../inc/EIllegalStateException.hh"
#include "../../inc/EThreadLocal.hh"

using namespace efc::rwlock;

namespace efc {

namespace rwlock {

//@see: openjdk-6/share/classes/java/util/concurrent/locks/ReentrantReadWriteLock.java
//@see: openjdk-7/share/classes/java/util/concurrent/locks/ReentrantReadWriteLock.java
//@mark: complex openjdk-6 & openjdk-7

/**
 * Synchronization implementation for ReentrantReadWriteLock.
 * Subclassed into fair and nonfair versions.
 */
abstract class Sync : public EAbstractQueuedSynchronizer {
public:
	/*
	 * Read vs write count extraction constants and functions.
	 * Lock state is logically divided into two shorts: The lower
	 * one representing the exclusive (writer) lock hold count,
	 * and the upper the shared (reader) hold count.
	 */
	enum Constants {
		SHARED_SHIFT   = 16,
		SHARED_UNIT    = (1 << SHARED_SHIFT),
		MAX_COUNT      = (1 << SHARED_SHIFT) - 1,
		EXCLUSIVE_MASK = (1 << SHARED_SHIFT) - 1
	};

	/** Returns the number of shared holds represented in count  */
	static int sharedCount(int c)    { return (uint)c >> SHARED_SHIFT; }
	/** Returns the number of exclusive holds represented in count  */
	static int exclusiveCount(int c) { return c & EXCLUSIVE_MASK; }

	/**
	 * A counter for per-thread read hold counts.
	 * Maintained as a ThreadLocal; cached in cachedHoldCounter
	 */
	class HoldCounter : public EObject {
	public:
		int count;
		// Use id, not reference, to avoid garbage retention
		long tid;

		HoldCounter() : count(0) {
			tid = EThread::currentThread()->getId();
		}

//		virtual ~HoldCounter() {
//			printf("~HoldCounter()\n");
//		}

		/** Decrement if positive; return previous value */
		int tryDecrement() {
			int c = count;
			if (c > 0)
				count = c - 1;
			return c;
		}
	};

	/**
	 * ThreadLocal subclass. Easiest to explicitly define for sake
	 * of deserialization mechanics.
	 */
	class ThreadLocalHoldCounter
		: public EThreadLocal {
	public:
		virtual EObject* initialValue() {
			return new HoldCounter();
		}
	};

	/**
	 * The number of read locks held by current thread.
	 * Initialized only in constructor and readObject.
	 */
	EThreadLocalVariable<ThreadLocalHoldCounter, HoldCounter>* readHolds;

	/**
	 * The hold count of the last thread to successfully acquire
	 * readLock. This saves ThreadLocal lookup in the common case
	 * where the next thread to release is the last one to
	 * acquire. This is non-volatile since it is just used
	 * as a heuristic, and would be great for threads to cache.
	 *
	 * <p>Can outlive the Thread for which it is caching the read
	 * hold count, but avoids garbage retention by not retaining a
	 * reference to the Thread.
	 *
	 * <p>Accessed via a benign data race; relies on the memory
	 * model's final field and out-of-thin-air guarantees.
	 */
	HoldCounter* cachedHoldCounter;

	/**
	 * firstReader is the first thread to have acquired the read lock.
	 * firstReaderHoldCount is firstReader's hold count.
	 *
	 * <p>More precisely, firstReader is the unique thread that last
	 * changed the shared count from 0 to 1, and has not released the
	 * read lock since then; null if there is no such thread.
	 *
	 * <p>Cannot cause garbage retention unless the thread terminated
	 * without relinquishing its read locks, since tryReleaseShared
	 * sets it to null.
	 *
	 * <p>Accessed via a benign data race; relies on the memory
	 * model's out-of-thin-air guarantees for references.
	 *
	 * <p>This allows tracking of read holds for uncontended read
	 * locks to be very cheap.
	 */
	EThread* firstReader;// = null;
	int firstReaderHoldCount;

	Sync() : cachedHoldCounter(null), firstReader(null), firstReaderHoldCount(0) {
		readHolds = new EThreadLocalVariable<ThreadLocalHoldCounter, HoldCounter>();
		setState(getState()); // ensures visibility of readHolds
	}

	~Sync() {
		delete readHolds;
	}

	/*
	 * Acquires and releases use the same code for fair and
	 * nonfair locks, but differ in whether/how they allow barging
	 * when queues are non-empty.
	 */

	/**
	 * Returns true if the current thread, when trying to acquire
	 * the read lock, and otherwise eligible to do so, should block
	 * because of policy for overtaking other waiting threads.
	 */
	virtual boolean readerShouldBlock() = 0;

	/**
	 * Returns true if the current thread, when trying to acquire
	 * the write lock, and otherwise eligible to do so, should block
	 * because of policy for overtaking other waiting threads.
	 */
	virtual boolean writerShouldBlock() = 0;

	/**
	 * Returns {@code true} if this lock has fairness set true.
	 *
	 * @return {@code true} if this lock has fairness set true
	 */
	virtual boolean isFair() const = 0;

	/*
	 * Note that tryRelease and tryAcquire can be called by
	 * Conditions. So it is possible that their arguments contain
	 * both read and write holds that are all released during a
	 * condition wait and re-established in tryAcquire.
	 */

	boolean tryRelease(int releases) {
		if (!isHeldExclusively())
			throw EIllegalStateException(__FILE__, __LINE__);
		int nextc = getState() - releases;
		boolean free = exclusiveCount(nextc) == 0;
		if (free)
			setExclusiveOwnerThread(null);
		setState(nextc);
		return free;
	}

	boolean tryAcquire(int acquires) {
		/*
		 * Walkthrough:
		 * 1. If read count nonzero or write count nonzero
		 *    and owner is a different thread, fail.
		 * 2. If count would saturate, fail. (This can only
		 *    happen if count is already nonzero.)
		 * 3. Otherwise, this thread is eligible for lock if
		 *    it is either a reentrant acquire or
		 *    queue policy allows it. If so, update state
		 *    and set owner.
		 */
		EThread* current = EThread::currentThread();
		int c = getState();
		int w = exclusiveCount(c);
		if (c != 0) {
			// (Note: if c != 0 and w == 0 then shared count != 0)
			if (w == 0 || current != getExclusiveOwnerThread())
				return false;
			if (w + exclusiveCount(acquires) > MAX_COUNT)
				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
			// Reentrant acquire
			setState(c + acquires);
			return true;
		}
		if (writerShouldBlock() ||
			!compareAndSetState(c, c + acquires))
			return false;
		setExclusiveOwnerThread(current);
		return true;
	}

	boolean tryReleaseShared(int unused) {
		EThread* current = EThread::currentThread();
		if (firstReader == current) {
			// assert firstReaderHoldCount > 0;
			if (firstReaderHoldCount == 1)
				firstReader = null;
			else
				firstReaderHoldCount--;
		} else {
			HoldCounter* rh = cachedHoldCounter;
			if (rh == null || rh->tid != current->getId())
				rh = readHolds->get();
//			int count = rh->count;
//			if (count <= 1) {
//				readHolds->remove(false);
//				if (count <= 0)
//					throw EIllegalStateException(__FILE__, __LINE__);
//			}
//			--rh->count;
			if (rh->tryDecrement() <= 0)
				throw EIllegalStateException(__FILE__, __LINE__, "attempt to unlock read lock, not locked by current thread");

//			HoldCounter* rh = readHolds->get();
//			int count = rh->count;
//			if (count <= 1) {
////				readHolds->remove();
//				if (count <= 0) {
//					throw EIllegalStateException(__FILE__, __LINE__,
//						  "attempt to unlock read lock, not locked by current thread");
//				}
//			}
//			--rh->count;
////			delete readHolds->set(rh); //!
		}
		for (;;) {
			int c = getState();
			int nextc = c - SHARED_UNIT;
			if (compareAndSetState(c, nextc))
				// Releasing the read lock has no effect on readers,
				// but it may allow waiting writers to proceed if
				// both read and write locks are now free.
				return nextc == 0;
		}

//		HoldCounter* rh = cachedHoldCounter;
//		EThread* current = EThread::currentThread();
//		if (rh == null || rh->tid != current->getId())
//			rh = readHolds->get();
//		if (rh->tryDecrement() <= 0)
//			throw EIllegalStateException(__FILE__, __LINE__);
//		for (;;) {
//			int c = getState();
//			int nextc = c - SHARED_UNIT;
//			if (compareAndSetState(c, nextc))
//				return nextc == 0;
//		}

		//not reach here!
		return false;
	}

	int tryAcquireShared(int unused) {
		/*
		 * Walkthrough:
		 * 1. If write lock held by another thread, fail.
		 * 2. Otherwise, this thread is eligible for
		 *    lock wrt state, so ask if it should block
		 *    because of queue policy. If not, try
		 *    to grant by CASing state and updating count.
		 *    Note that step does not check for reentrant
		 *    acquires, which is postponed to full version
		 *    to avoid having to check hold count in
		 *    the more typical non-reentrant case.
		 * 3. If step 2 fails either because thread
		 *    apparently not eligible or CAS fails or count
		 *    saturated, chain to version with full retry loop.
		 */
		EThread* current = EThread::currentThread();
		int c = getState();
		if (exclusiveCount(c) != 0 &&
			getExclusiveOwnerThread() != current)
			return -1;

		int r = sharedCount(c);
		if (!readerShouldBlock() &&
			r < MAX_COUNT &&
			compareAndSetState(c, c + SHARED_UNIT)) {
			if (r == 0) {
				firstReader = current;
				firstReaderHoldCount = 1;
			} else if (firstReader == current) {
				firstReaderHoldCount++;
			} else {
				HoldCounter* rh = cachedHoldCounter;
                if (rh == null || rh->tid != current->getId()) {
					cachedHoldCounter = rh = readHolds->get();
                    ES_ASSERT(rh);
                }
                else if (rh->count == 0) {
					delete readHolds->set(rh);
                }
				rh->count++;
			}
//			} else {
//				HoldCounter* rh = readHolds->get();
//				rh->count++;
////				delete readHolds->set(rh);
//			}
			return 1;
		}
		return fullTryAcquireShared(current);

//		if (sharedCount(c) == MAX_COUNT)
//			throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
//		if (!readerShouldBlock() &&
//			compareAndSetState(c, c + SHARED_UNIT)) {
//			HoldCounter* rh = cachedHoldCounter;
//			if (rh == null || rh->tid != current->getId())
//				cachedHoldCounter = rh = readHolds->get();
//			rh->count++;
//			return 1;
//		}
//		return fullTryAcquireShared(current);
	}

	/**
	 * Full version of acquire for reads, that handles CAS misses
	 * and reentrant reads not dealt with in tryAcquireShared.
	 */
	int fullTryAcquireShared(EThread* current) {
		/*
		 * This code is in part redundant with that in
		 * tryAcquireShared but is simpler overall by not
		 * complicating tryAcquireShared with interactions between
		 * retries and lazily reading hold counts.
		 */
//		HoldCounter* rh = cachedHoldCounter;
//		if (rh == null || rh->tid != current->getId())
//			rh = readHolds->get();
//		for (;;) {
//			int c = getState();
//			int w = exclusiveCount(c);
//			if ((w != 0 && getExclusiveOwnerThread() != current) ||
//				((rh->count | w) == 0 && readerShouldBlock()))
//				return -1;
//			if (sharedCount(c) == MAX_COUNT)
//				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
//			if (compareAndSetState(c, c + SHARED_UNIT)) {
//				cachedHoldCounter = rh; // cache for release
//				rh->count++;
//				return 1;
//			}
//		}

		HoldCounter* rh = null;
		for (;;) {
			int c = getState();
			if (exclusiveCount(c) != 0) {
				if (getExclusiveOwnerThread() != current)
					return -1;
				// else we hold the exclusive lock; blocking here
				// would cause deadlock.
			} else if (readerShouldBlock()) {
				// Make sure we're not acquiring read lock reentrantly
				if (firstReader == current) {
					// assert firstReaderHoldCount > 0;
					if (firstReaderHoldCount > 0) {
						throw ERuntimeException(__FILE__, __LINE__, "Read lock should not be aquired reentrantlly.");
					}
				} else {
					if (rh == null) {
						rh = cachedHoldCounter;
						if (rh == null || rh->tid != current->getId()) {
							rh = readHolds->get();
//							if (rh->count == 0)
//								readHolds->remove(false);
						}
					}
					if (rh->count == 0)
						return -1;

//					rh = readHolds->get();
//					if (rh->count == 0) {
////						readHolds->remove();
//					}
//
//					if (rh->count == 0) {
//						return -1;
//					}
				}
			}
			if (sharedCount(c) == MAX_COUNT)
				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
			if (compareAndSetState(c, c + SHARED_UNIT)) {
				if (sharedCount(c) == 0) {
					firstReader = current;
					firstReaderHoldCount = 1;
				} else if (firstReader == current) {
					firstReaderHoldCount++;
				} else {
					if (rh == null)
						rh = cachedHoldCounter;
					if (rh == null || rh->tid != current->getId())
						rh = readHolds->get();
					else if (rh->count == 0)
						delete readHolds->set(rh);
					rh->count++;
					cachedHoldCounter = rh; // cache for release

//					rh = readHolds->get();
//					rh->count++;
////					delete readHolds->set(rh);
				}
				return 1;
			}
		}

		//not reach here!
		return 0;
	}

	/**
	 * Performs tryLock for write, enabling barging in both modes.
	 * This is identical in effect to tryAcquire except for lack
	 * of calls to writerShouldBlock
	 */
	boolean tryWriteLock() {
		EThread* current = EThread::currentThread();
		int c = getState();
		if (c != 0) {
			int w = exclusiveCount(c);
			if (w == 0 ||current != getExclusiveOwnerThread())
				return false;
			if (w == MAX_COUNT)
				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
		}
		if (!compareAndSetState(c, c + 1))
			return false;
		setExclusiveOwnerThread(current);
		return true;
	}

	/**
	 * Performs tryLock for read, enabling barging in both modes.
	 * This is identical in effect to tryAcquireShared except for
	 * lack of calls to readerShouldBlock
	 */
	boolean tryReadLock() {
//		EThread* current = EThread::currentThread();
//		for (;;) {
//			int c = getState();
//			if (exclusiveCount(c) != 0 &&
//				getExclusiveOwnerThread() != current)
//				return false;
//			if (sharedCount(c) == MAX_COUNT)
//				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
//			if (compareAndSetState(c, c + SHARED_UNIT)) {
//				HoldCounter* rh = cachedHoldCounter;
//				if (rh == null || rh->tid != current->getId())
//					cachedHoldCounter = rh = readHolds->get();
//				rh->count++;
//				return true;
//			}
//		}

		EThread* current = EThread::currentThread();
		for (;;) {
			int c = getState();
			if (exclusiveCount(c) != 0 &&
				getExclusiveOwnerThread() != current)
				return false;
			int r = sharedCount(c);
			if (r == MAX_COUNT)
				throw ERuntimeException(__FILE__, __LINE__, "Maximum lock count exceeded");
			if (compareAndSetState(c, c + SHARED_UNIT)) {
				if (r == 0) {
					firstReader = current;
					firstReaderHoldCount = 1;
				} else if (firstReader == current) {
					firstReaderHoldCount++;
				} else {
					HoldCounter* rh = cachedHoldCounter;
					if (rh == null || rh->tid != current->getId())
						cachedHoldCounter = rh = readHolds->get();
					else if (rh->count == 0)
						delete readHolds->set(rh);
					rh->count++;

//					HoldCounter* rh = readHolds->get();//cachedHoldCounter;
//					rh->count++;
////					delete readHolds->set(rh);
				}
				return true;
			}
		}

		//not reach here!
		return false;
	}

	boolean isHeldExclusively() {
		// While we must in general read state before owner,
		// we don't need to do so to check if current thread is owner
		return getExclusiveOwnerThread() == EThread::currentThread();
	}

	// Methods relayed to outer class

	EAbstractQueuedSynchronizer::ConditionObject* newCondition() {
		return new EAbstractQueuedSynchronizer::ConditionObject(this);
	}

	EThread* getOwner() {
		// Must read state before owner to ensure memory consistency
		return ((exclusiveCount(getState()) == 0)?
				null :
				getExclusiveOwnerThread());
	}

	int getReadLockCount() {
		return sharedCount(getState());
	}

	boolean isWriteLocked() {
		return exclusiveCount(getState()) != 0;
	}

	int getWriteHoldCount() {
		return isHeldExclusively() ? exclusiveCount(getState()) : 0;
	}

	int getReadHoldCount() {
		if (getReadLockCount() == 0)
			return 0;

		EThread* current = EThread::currentThread();
		if (firstReader == current)
			return firstReaderHoldCount;

		HoldCounter* rh = cachedHoldCounter;
		if (rh != null && rh->tid == current->getId())
			return rh->count;

		int count = readHolds->get()->count;
//		if (count == 0) readHolds->remove();
		return count;
	}

	int getCount() { return getState(); }
};

/**
 * Nonfair version of Sync
 */
class NonfairSync : public Sync {
public:
	boolean writerShouldBlock() {
		return false; // writers can always barge
	}
	boolean readerShouldBlock() {
		/* As a heuristic to avoid indefinite writer starvation,
		 * block if the thread that momentarily appears to be head
		 * of queue, if one exists, is a waiting writer.  This is
		 * only a probabilistic effect since a new reader will not
		 * block if there is a waiting writer behind other enabled
		 * readers that have not yet drained from the queue.
		 */
		return apparentlyFirstQueuedIsExclusive();
	}
	boolean isFair() const {
		return false;
	}
};

/**
 * Fair version of Sync
 */
class FairSync : public Sync {
public:
	boolean writerShouldBlock() {
		return hasQueuedPredecessors();
	}
	boolean readerShouldBlock() {
		return hasQueuedPredecessors();
	}
	boolean isFair() const {
		return true;
	}
};

} /* namespace rwlock */

EReentrantReadWriteLock::EReentrantReadWriteLock() {
	sync = new NonfairSync();
	readerLock = new ReadLock(this);
	writerLock = new WriteLock(this);
}

EReentrantReadWriteLock::EReentrantReadWriteLock(boolean fair) {
	if (fair)
		sync = new FairSync();
	else
		sync = new NonfairSync();
	readerLock = new ReadLock(this);
	writerLock = new WriteLock(this);
}

EReentrantReadWriteLock::~EReentrantReadWriteLock() {
	delete sync;
	delete readerLock;
	delete writerLock;
}

EReentrantReadWriteLock::ReadLock* EReentrantReadWriteLock::readLock() {
	return readerLock;
}

EReentrantReadWriteLock::WriteLock* EReentrantReadWriteLock::writeLock() {
	return writerLock;
}

boolean EReentrantReadWriteLock::isFair() {
	return sync->isFair();
}

int EReentrantReadWriteLock::getReadLockCount() {
	return sync->getReadLockCount();
}

boolean EReentrantReadWriteLock::isWriteLocked() {
	return sync->isWriteLocked();
}

boolean EReentrantReadWriteLock::isWriteLockedByCurrentThread() {
	return sync->isHeldExclusively();
}

int EReentrantReadWriteLock::getWriteHoldCount() {
	return sync->getWriteHoldCount();
}

int EReentrantReadWriteLock::getReadHoldCount() {
	return sync->getReadHoldCount();
}

boolean EReentrantReadWriteLock::hasQueuedThreads() {
	return sync->hasQueuedThreads();
}

boolean EReentrantReadWriteLock::hasQueuedThread(EThread* thread) {
	return sync->isQueued(thread);
}

int EReentrantReadWriteLock::getQueueLength() {
	return sync->getQueueLength();
}

boolean EReentrantReadWriteLock::hasWaiters(ECondition* condition) {
	if (condition == null)
		throw ENullPointerException(__FILE__, __LINE__);

	const EAbstractQueuedSynchronizer::ConditionObject* cond =
			dynamic_cast<const EAbstractQueuedSynchronizer::ConditionObject*>(condition);
	if (cond == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "not owner");
	}

	return sync->hasWaiters((EAbstractQueuedSynchronizer::ConditionObject*)condition);
}

int EReentrantReadWriteLock::getWaitQueueLength(ECondition* condition) {
	if (condition == null)
		throw ENullPointerException(__FILE__, __LINE__);

	const EAbstractQueuedSynchronizer::ConditionObject* cond =
		dynamic_cast<const EAbstractQueuedSynchronizer::ConditionObject*>(condition);
	if (cond == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "not owner");
	}
	return sync->getWaitQueueLength((EAbstractQueuedSynchronizer::ConditionObject*)condition);
}

EString EReentrantReadWriteLock::toString() {
	int c = sync->getCount();
	int w = Sync::exclusiveCount(c);
	int r = Sync::sharedCount(c);

	return EString::formatOf(
			"EReentrantReadWriteLock[Write locks = %d, Read locks = %d]", w, r);
}

EThread* EReentrantReadWriteLock::getOwner() {
	return sync->getOwner();
}

ECollection<EThread*>* EReentrantReadWriteLock::getQueuedWriterThreads() {
	return sync->getExclusiveQueuedThreads();
}

ECollection<EThread*>* EReentrantReadWriteLock::getQueuedReaderThreads() {
	return sync->getSharedQueuedThreads();
}

ECollection<EThread*>* EReentrantReadWriteLock::getQueuedThreads() {
	return sync->getQueuedThreads();
}

ECollection<EThread*>* EReentrantReadWriteLock::getWaitingThreads(
		ECondition* condition) {
	if (condition == null)
		throw ENullPointerException(__FILE__, __LINE__);

	const EAbstractQueuedSynchronizer::ConditionObject* cond =
		dynamic_cast<const EAbstractQueuedSynchronizer::ConditionObject*>(condition);
	if (cond == null) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "not owner");
	}
	return sync->getWaitingThreads((EAbstractQueuedSynchronizer::ConditionObject*)condition);
}

//=============================================================================

EReentrantReadWriteLock::ReadLock::ReadLock(EReentrantReadWriteLock* lock) {
	sync = lock->sync;
}

EReentrantReadWriteLock::ReadLock::~ReadLock() {
}

void EReentrantReadWriteLock::ReadLock::lock() {
	sync->acquireShared(1);
}

void EReentrantReadWriteLock::ReadLock::lockInterruptibly() {
	sync->acquireSharedInterruptibly(1);
}

boolean EReentrantReadWriteLock::ReadLock::tryLock() {
	return sync->tryReadLock();
}

boolean EReentrantReadWriteLock::ReadLock::tryLock(llong time,
		ETimeUnit* unit) {
	return sync->tryAcquireSharedNanos(1, unit->toNanos(time));
}

void EReentrantReadWriteLock::ReadLock::unlock() {
	sync->releaseShared(1);
}

ECondition* EReentrantReadWriteLock::ReadLock::newCondition() {
	throw EUnsupportedOperationException(__FILE__, __LINE__);
}

EString EReentrantReadWriteLock::ReadLock::toString() {
	int r = sync->getReadLockCount();
	return EString::formatOf("EReentrantReadWriteLock::ReadLock[Read locks = %d]", r);
}

//=============================================================================

EReentrantReadWriteLock::WriteLock::WriteLock(EReentrantReadWriteLock* lock) {
	sync = lock->sync;
}

EReentrantReadWriteLock::WriteLock::~WriteLock() {
}

void EReentrantReadWriteLock::WriteLock::lock() {
	sync->acquire(1);
}

void EReentrantReadWriteLock::WriteLock::lockInterruptibly() {
	sync->acquireInterruptibly(1);
}

boolean EReentrantReadWriteLock::WriteLock::tryLock() {
	return sync->tryWriteLock();
}

boolean EReentrantReadWriteLock::WriteLock::tryLock(llong timeout,
		ETimeUnit* unit) {
	return sync->tryAcquireNanos(1, unit->toNanos(timeout));
}

void EReentrantReadWriteLock::WriteLock::unlock() {
	sync->release(1);
}

ECondition* EReentrantReadWriteLock::WriteLock::newCondition() {
	return sync->newCondition();
}

EString EReentrantReadWriteLock::WriteLock::toString() {
	EThread* o = sync->getOwner();
	if (o == null)
		return EString("EReentrantReadWriteLock::WriteLock[Unlocked]");
	else
		return EString::formatOf("EReentrantReadWriteLock::WriteLock[Locked by thread %s]", o->getName());
}

boolean EReentrantReadWriteLock::WriteLock::isHeldByCurrentThread() {
	return sync->isHeldExclusively();
}

int EReentrantReadWriteLock::WriteLock::getHoldCount() {
	return sync->getWriteHoldCount();
}

} /* namespace efc */
