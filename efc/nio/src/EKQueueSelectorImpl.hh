/*
 * EKQueueSelectorImpl.hh
 *
 *  Created on: 2016-2-16
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_KQUEUE

#ifndef EKQUEUESELECTORIMPL_HH_
#define EKQUEUESELECTORIMPL_HH_

#include "../inc/ESelector.hh"
#include "./EPipeWrapper.hh"
#include "./EKQueueArrayWrapper.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/src/macosx/classes/sun/nio/ch/KQueueSelectorImpl.java

/*
 * KQueueSelectorImpl.java
 * Implementation of Selector using FreeBSD / Mac OS X kqueues
 * Derived from Sun's DevPollSelectorImpl
 */

class EKQueueSelectorImpl: public ESelector {
public:
	virtual ~EKQueueSelectorImpl();

	/**
	 * Package private constructor called by factory method in
	 * the abstract superclass Selector.
	 */
	EKQueueSelectorImpl();

	virtual ESelector* wakeup();
	virtual void putEventOps(ESelectionKey* sk, int ops);

protected:
	// File descriptors used for interrupt
	EPipeWrapper* pipeWrapper_;

	// The kqueue manipulator
	EKQueueArrayWrapper* kqueueWrapper_;

	virtual void implRegister(sp<ESelectionKey> ski);
	virtual void implDereg(sp<ESelectionKey> ski) THROWS(EIOException);
	virtual void implClose() THROWS(EIOException);
	virtual int doSelect(llong timeout) THROWS(EIOException);

private:
	// Used to map file descriptors to a selection key and "update count"
	// (see updateSelectedKeys for usage).
	class MapEntry : public EObject {
	public:
		sp<ESelectionKey> ski;
		long updateCount;
		MapEntry(sp<ESelectionKey> ski) {
			this->ski = ski;
		}
	};

	// Count of registered descriptors (including interrupt)
	int totalChannels_;

	// Map from a file descriptor to an entry containing the selection key
	EHashMap<int,MapEntry*>* fdMap_;

	// True if this Selector has been closed
	boolean closed_;// = false;

	// Lock for interrupt triggering and clearing
	EReentrantLock interruptLock_;// = new Object();
	boolean interruptTriggered_;// = false;

	// used by updateSelectedKeys to handle cases where the same file
	// descriptor is polled by more than one filter
	long updateCount_;

	/**
	 * Update the keys whose fd's have been selected by kqueue.
	 * Add the ready keys to the selected key set.
	 * If the interrupt fd has been selected, drain it and clear the interrupt.
	 */
	int updateSelectedKeys(int entries) THROWS(EIOException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EKQUEUESELECTORIMPL_HH_ */

#endif //!HAVE_KQUEUE
