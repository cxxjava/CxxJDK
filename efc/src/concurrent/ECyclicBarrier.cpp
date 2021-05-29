/*
 * ECyclicBarrier.cpp
 *
 *  Created on: 2015-2-7
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/ECyclicBarrier.hh"
#include "../../inc/ESynchronizeable.hh"
#include "../../inc/EThread.hh"

namespace efc {

//@see: openjdk-7/jdk/src/share/classes/java/util/concurrent/CyclicBarrier.java

void ECyclicBarrier::init(uint parties, sp<ERunnable> barrierAction) {
	this->parties = parties;
	this->count = parties;
	this->barrierCommand = barrierAction;

	lock = new EReentrantLock();
	trip = lock->newCondition();

	generation = new Generation();
}

ECyclicBarrier::~ECyclicBarrier() {
	delete trip;
	delete lock;
}

ECyclicBarrier::ECyclicBarrier(uint parties, sp<ERunnable> barrierAction) {
	init(parties, barrierAction);
}

ECyclicBarrier::ECyclicBarrier(uint parties) {
	init(parties, null);
}

int ECyclicBarrier::getParties() {
	return parties;
}

int ECyclicBarrier::await() {
	//try {
		return dowait(false, 0L);
	//} catch (ETimeoutException& toe) {
	//	throw Error(toe); // cannot happen;
	//}
}

int ECyclicBarrier::await(llong timeout, ETimeUnit* unit) {
	return dowait(true, unit->toNanos(timeout));
}

boolean ECyclicBarrier::isBroken() {
	SYNCBLOCK(lock) {
		return generation->broken;
    }}
}

void ECyclicBarrier::reset() {
	SYNCBLOCK(lock) {
		breakBarrier();   // break the current generation
		nextGeneration(); // start a new generation
    }}
}

int ECyclicBarrier::getNumberWaiting() {
	SYNCBLOCK(lock) {
		return parties - count;
    }}
}

void ECyclicBarrier::nextGeneration() {
	// signal completion of last generation
	trip->signalAll();
	// set up next generation
	count = parties;
	generation = new Generation();
}

void ECyclicBarrier::breakBarrier() {
	generation->broken = true;
	count = parties;
	trip->signalAll();
}

int ECyclicBarrier::dowait(boolean timed, llong nanos) {
	SYNCBLOCK(lock) {
		sp<Generation> g = generation;

		if (g->broken)
			throw EBrokenBarrierException(__FILE__, __LINE__);

		if (EThread::interrupted()) {
			breakBarrier();
			throw EInterruptedException(__FILE__, __LINE__);
		}

	   int index = --count;
	   if (index == 0) {  // tripped
		   boolean ranAction = false;
		   try {
			   sp<ERunnable> command = barrierCommand;
			   if (command != null)
				   command->run();
			   ranAction = true;
			   nextGeneration();
			   return 0;
		   } catch (...) {
			   finally {
				   if (!ranAction)
					   breakBarrier();
			   }
			   throw; //!
		   }
	   }

		// loop until tripped, broken, interrupted, or timed out
		for (;;) {
			try {
				if (!timed)
					trip->await();
				else if (nanos > 0L)
					nanos = trip->awaitNanos(nanos);
			} catch (EInterruptedException& ie) {
				if (g == generation && ! g->broken) {
					breakBarrier();
					throw ie;
				} else {
					// We're about to finish waiting even if we had not
					// been interrupted, so this interrupt is deemed to
					// "belong" to subsequent execution.
					EThread::currentThread()->interrupt();
				}
			}

			if (g->broken)
				throw EBrokenBarrierException(__FILE__, __LINE__);

			if (g != generation)
				return index;

			if (timed && nanos <= 0L) {
				breakBarrier();
				throw ETimeoutException(__FILE__, __LINE__);
			}
		}
    }}
}

} /* namespace efc */
