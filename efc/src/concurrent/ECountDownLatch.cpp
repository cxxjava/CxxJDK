/*
 * ECountDownLatch.cpp
 *
 *  Created on: 2015-2-6
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/ECountDownLatch.hh"
#include "../../inc/concurrent/EAbstractQueuedSynchronizer.hh"

namespace efc {

namespace cdl {
/**
 * Synchronization control For CountDownLatch.
 * Uses AQS state to represent count.
 */
class Sync: public EAbstractQueuedSynchronizer {
public:
	Sync(int count) {
		setState(count);
	}

	int getCount() {
		return getState();
	}

	int tryAcquireShared(int acquires) {
		return (getState() == 0) ? 1 : -1;
	}

	boolean tryReleaseShared(int releases) {
		// Decrement count; signal when transition to zero
		for (;;) {
			int c = getState();
			if (c == 0)
				return false;
			int nextc = c-1;
			if (compareAndSetState(c, nextc))
				return nextc == 0;
		}
	}
};
} /* namespace cdl */


ECountDownLatch::~ECountDownLatch() {
	delete sync;
}

ECountDownLatch::ECountDownLatch(int count) {
	if (count < 0) throw EIllegalArgumentException(__FILE__, __LINE__, "count < 0");
	this->sync = new cdl::Sync(count);
}

void ECountDownLatch::await() {
	sync->acquireSharedInterruptibly(1);
}

boolean ECountDownLatch::await(llong timeout, ETimeUnit* unit) {
	return sync->tryAcquireSharedNanos(1, unit->toNanos(timeout));
}

void ECountDownLatch::countDown() {
	sync->releaseShared(1);
}

long ECountDownLatch::getCount() {
	return sync->getCount();
}

EString ECountDownLatch::toString() {
	return EString::formatOf("ECountDownLatch[Count = %ld]", sync->getCount());
}

} /* namespace efc */
