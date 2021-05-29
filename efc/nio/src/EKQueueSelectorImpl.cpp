/*
 * EKQueueSelectorImpl.cpp
 *
 *  Created on: 2016-2-16
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_KQUEUE

#include "./EKQueueSelectorImpl.hh"
#include "./ENIOUtil.hh"

namespace efc {
namespace nio {

EKQueueSelectorImpl::~EKQueueSelectorImpl() {
	delete fdMap_;
	delete kqueueWrapper_;
	delete pipeWrapper_;
}

EKQueueSelectorImpl::EKQueueSelectorImpl() :
		closed_(false),
		interruptTriggered_(false),
		updateCount_(0) {
	kqueueWrapper_ = new EKQueueArrayWrapper();
	pipeWrapper_ = new EPipeWrapper();
	kqueueWrapper_->initInterrupt(pipeWrapper_->inFD(), pipeWrapper_->outFD());
	fdMap_ = new EHashMap<int, MapEntry*>(true);
	totalChannels_ = 1;
}

ESelector* EKQueueSelectorImpl::wakeup() {
	SYNCBLOCK(&interruptLock_) {
		if (!interruptTriggered_) {
			kqueueWrapper_->interrupt();
			interruptTriggered_ = true;
		}
    }}
	return this;
}

void EKQueueSelectorImpl::putEventOps(ESelectionKey* ski, int ops) {
	if (closed_)
		throw EClosedSelectorException(__FILE__, __LINE__);
	kqueueWrapper_->setInterest(ski->channel(), ops);
}

void EKQueueSelectorImpl::implRegister(sp<ESelectionKey> ski) {
	if (closed_)
		throw EClosedSelectorException(__FILE__, __LINE__);
	delete fdMap_->put(ski->channel()->getFDVal(), new MapEntry(ski));
	totalChannels_++;
	keys_->add(ski);
}

void EKQueueSelectorImpl::implDereg(sp<ESelectionKey> ski) {
	sp<ESelectableChannel> ch = ski->channel();

	delete fdMap_->remove(ch->getFDVal());
	kqueueWrapper_->release(ch);
	totalChannels_--;
	keys_->remove(ski.get());
	selectedKeys_->remove(ski.get());

	/**
	 * Must be locked, but not used SYNCHRONIZED(ch),
	 * otherwise if channel is registered on two selector's,
	 * access channel is dangerous.
	 */
	sp<ESelectableChannel> c = null;
	SCOPED_SLOCK1(ch.get()) {
		deregister(ski);
		if (!ch->isOpen() && !ch->isRegistered()) {
			ch->kill();
			c = ch;
		}
    }}
}

void EKQueueSelectorImpl::implClose() {
	if (closed_)
		return;

	closed_ = true;

	// prevent further wakeup
	SYNCBLOCK(&interruptLock_) {
		interruptTriggered_ = true;
    }}

	if (pipeWrapper_ != null) {
		delete pipeWrapper_;
		pipeWrapper_ = null;
	}

	if (kqueueWrapper_ != null) {
		kqueueWrapper_->close();

		// it is possible
		delete selectedKeys_;
		selectedKeys_ = null;

		// Deregister channels
		sp<EIterator<sp<ESelectionKey> > > i = keys_->iterator();
		while (i->hasNext()) {
			sp<ESelectionKey> ski = i->next();


			sp<ESelectableChannel> c = null;
			sp<ESelectableChannel> selch = ski->channel();
			SCOPED_SLOCK1(selch.get()) {
				deregister(ski);
				if (!selch->isOpen() && !selch->isRegistered()) {
					selch->kill();
					c = selch;
				}
            }}

			i->remove();
		}

		totalChannels_ = 0;

		delete kqueueWrapper_;
		kqueueWrapper_ = null;
	}
}

int EKQueueSelectorImpl::doSelect(llong timeout) {
	int entries = 0;
	if (closed_)
		throw EClosedSelectorException(__FILE__, __LINE__);
	processDeregisterQueue();
	try {
		begin();
		entries = kqueueWrapper_->poll(timeout);
	} catch (...) {
		end();
		throw; //!
	} finally {
		end();
	}
	processDeregisterQueue();
	return updateSelectedKeys(entries);
}

int EKQueueSelectorImpl::updateSelectedKeys(int entries) {
	int numKeysUpdated = 0;
	boolean interrupted = false;
	int fd0 = pipeWrapper_->inFD();

	// A file descriptor may be registered with kqueue with more than one
	// filter and so there may be more than one event for a fd. The update
	// count in the MapEntry tracks when the fd was last updated and this
	// ensures that the ready ops are updated rather than replaced by a
	// second or subsequent event.
	updateCount_++;

	for (int i = 0; i < entries; i++) {
		int nextFD = kqueueWrapper_->getDescriptor(i);
		if (nextFD == fd0) {
			interrupted = true;
		} else {
			MapEntry* me = fdMap_->get(nextFD);

			// entry is null in the case of an interrupt
			if (me != null) {
				int rOps = kqueueWrapper_->getReventOps(i);
				sp<ESelectionKey> ski = me->ski;
				if (selectedKeys_->contains(ski.get())) {
					// first time this file descriptor has been encountered on this
					// update?
					if (me->updateCount != updateCount_) {
						if (ski->channel()->translateAndSetReadyOps(rOps, ski.get())) {
							numKeysUpdated++;
							me->updateCount = updateCount_;
						}
					} else {
						// ready ops have already been set on this update
						ski->channel()->translateAndUpdateReadyOps(rOps, ski.get());
					}
				} else {
					ski->channel()->translateAndSetReadyOps(rOps, ski.get());
					if ((ski->nioReadyOps() & ski->nioInterestOps()) != 0) {
						selectedKeys_->add(ski);
						numKeysUpdated++;
						me->updateCount = updateCount_;
					}
				}
			}
		}
	}

	if (interrupted) {
		// Clear the wakeup pipe
		SYNCBLOCK(&interruptLock_) {
			ENIOUtil::drain(fd0);
			interruptTriggered_ = false;
        }}
    }
	return numKeysUpdated;
}

} /* namespace nio */
} /* namespace efc */

#endif //!HAVE_KQUEUE
