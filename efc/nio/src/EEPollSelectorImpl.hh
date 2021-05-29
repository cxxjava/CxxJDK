/*
 * EEPollSelectorImpl.hh
 *
 *  Created on: 2016-2-14
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_EPOLL

#ifndef EEPOLLSELECTORIMPL_HH_
#define EEPOLLSELECTORIMPL_HH_

#include "../inc/ESelector.hh"
#include "./EEPollArrayWrapper.hh"
#include "./EPipeWrapper.hh"
#include "../../inc/concurrent/EAtomicBoolean.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/src/solaris/classes/sun/nio/ch/EPollSelectorImpl.java

/**
 * An implementation of Selector for Linux 2.6+ kernels that uses
 * the epoll event notification facility.
 */

class EEPollSelectorImpl: public ESelector {
public:
	virtual ~EEPollSelectorImpl();

	/**
	 * Package private constructor called by factory method in
	 * the abstract superclass Selector.
	 */
	EEPollSelectorImpl() THROWS(EIOException);

	virtual ESelector* wakeup();
	virtual void putEventOps(ESelectionKey* sk, int ops);

protected:
	// pipe used for interrupt
	EPipeWrapper* pipeWrapper_;

	// The poll object
	EEPollArrayWrapper *pollWrapper_;

	virtual void implRegister(sp<ESelectionKey> ski);
	virtual void implDereg(sp<ESelectionKey> ski) THROWS(EIOException);
	virtual void implClose() THROWS(EIOException);
	virtual int doSelect(llong timeout) THROWS(EIOException);

private:
	// Maps from file descriptors to keys
	EHashMap<int, sp<ESelectionKey> >* fdToKey_;

	// True if this Selector has been closed
	volatile boolean closed_;// = false;

	// Lock for interrupt triggering and clearing
	EReentrantLock interruptLock_;
	boolean interruptTriggered_;// = false;

    /**
	 * Update the keys whose fd's have been selected by the epoll.
	 * Add the ready keys to the ready queue.
	 */
	int updateSelectedKeys(int entries);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EEPOLLSELECTORIMPL_HH_ */

#endif //!HAVE_EPOLL
