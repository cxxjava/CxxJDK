/*
 * EPollSelectorImpl.hh
 *
 *  Created on: 2016-2-16
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_SELECT

#ifndef EPOLLSELECTORIMPL_HH_
#define EPOLLSELECTORIMPL_HH_

#include "../inc/ESelector.hh"
#include "./EPollArrayWrapper.hh"
#include "./EPipeWrapper.hh"
#include "../../inc/concurrent/EAtomicBoolean.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/src/share/classes/sun/nio/ch/AbstractPollSelectorImpl.java
//@see: openjdk-8/src/solaris/classes/sun/nio/ch/PollSelectorImpl.java

/*
 * Implementation of Selector using the select/poll event notification facility.
 */

class EPollSelectorImpl: public ESelector {
public:
	virtual ~EPollSelectorImpl();

	/**
	 * Package private constructor called by factory method in
	 * the abstract superclass Selector.
	 */
	EPollSelectorImpl() THROWS(EIOException);

	virtual ESelector* wakeup();
	virtual void putEventOps(ESelectionKey* sk, int ops);

protected:
	// pipe used for interrupt
	EPipeWrapper* pipeWrapper_;

	// The poll object
	EPollArrayWrapper *pollWrapper_;

	virtual void implRegister(sp<ESelectionKey> ski);
	virtual void implDereg(sp<ESelectionKey> ski) THROWS(EIOException);
	virtual void implClose() THROWS(EIOException);
	virtual int doSelect(llong timeout) THROWS(EIOException);

private:
	// Initial capacity of the pollfd array
	static const int INIT_CAP = 10;

	// The list of SelectableChannels serviced by this Selector
	EA<sp<ESelectionKey> >* channelArray_;

	// The number of valid channels in this Selector's poll array
	int totalChannels_;

	// True if this Selector has been closed
	volatile boolean closed_;// = false;

	// Lock for close and cleanup
	EReentrantLock closeLock_;// = new Object();

	// Lock for interrupt triggering and clearing
	EReentrantLock interruptLock_;
	boolean interruptTriggered_;// = false;

	/**
	 * Copy the information in the pollfd structs into the opss
	 * of the corresponding Channels. Add the ready keys to the
	 * ready queue.
	 */
	int updateSelectedKeys();
};

} /* namespace nio */
} /* namespace efc */
#endif /* EPOLLSELECTORIMPL_HH_ */

#endif //!HAVE_SELECT
