/*
 * EPollSelectorImpl.cpp
 *
 *  Created on: 2016-2-16
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_SELECT

#include "./EPollSelectorImpl.hh"
#include "./ENIOUtil.hh"
#include "../inc/EClosedSelectorException.hh"

namespace efc {
namespace nio {


EPollSelectorImpl::~EPollSelectorImpl() {
	delete channelArray_;
	delete pollWrapper_;
	delete pipeWrapper_;
}

EPollSelectorImpl::EPollSelectorImpl() : totalChannels_(1),
		closed_(false),
		interruptTriggered_(false) {
	pollWrapper_ = new EPollArrayWrapper(INIT_CAP);
	pipeWrapper_ = new EPipeWrapper();
	pollWrapper_->initInterrupt(pipeWrapper_->inFD(), pipeWrapper_->outFD());

	channelArray_ = new EA<sp<ESelectionKey> >(INIT_CAP);
}

ESelector* EPollSelectorImpl::wakeup() {
	SYNCBLOCK (&interruptLock_) {
		if (!interruptTriggered_) {
			pollWrapper_->interrupt();
			interruptTriggered_ = true;
		}
    }}
	return this;
}

void EPollSelectorImpl::putEventOps(ESelectionKey* sk, int ops) {
	SYNCBLOCK (&closeLock_) {
		if (closed_)
			throw EClosedSelectorException(__FILE__, __LINE__);
		pollWrapper_->putEventOps(sk->getIndex(), ops);
    }}
}

void EPollSelectorImpl::implRegister(sp<ESelectionKey> ski) {
	SYNCBLOCK (&closeLock_) {
		if (closed_)
			throw EClosedSelectorException(__FILE__, __LINE__);
		// Check to see if the array is large enough
		if (channelArray_->length() == totalChannels_) {
			// Make a larger array
			int newSize = pollWrapper_->totalChannels_ * 2;
			EA<sp<ESelectionKey> >* temp = new EA<sp<ESelectionKey> >(newSize);
			// Copy over
			for (int i=1; i<totalChannels_; i++)
				(*temp)[i] = (*channelArray_)[i];
			delete channelArray_; //!
			channelArray_ = temp;
			// Grow the NativeObject poll array
			pollWrapper_->grow(newSize);
		}
		(*channelArray_)[totalChannels_] = ski;
		ski->setIndex(totalChannels_);
		pollWrapper_->addEntry(ski->channel()->getFDVal());
		totalChannels_++;
		keys_->add(ski);
    }}
}

void EPollSelectorImpl::implDereg(sp<ESelectionKey> ski) {
	// Algorithm: Copy the sc from the end of the list and put it into
	// the location of the sc to be removed (since order doesn't
	// matter). Decrement the sc count. Update the index of the sc
	// that is moved.
	int i = ski->getIndex();
	ES_ASSERT (i >= 0);
	if (i != totalChannels_ - 1) {
		// Copy end one over it
		sp<ESelectionKey> endChannel = (*channelArray_)[totalChannels_-1];
		(*channelArray_)[i] = endChannel;
		endChannel->setIndex(i);
		pollWrapper_->release(i);
		pollWrapper_->replaceEntry(totalChannels_ - 1, i);
	} else {
		pollWrapper_->release(i);
	}
	// Destroy the last one
	(*channelArray_)[totalChannels_-1] = null;
	totalChannels_--;
	pollWrapper_->totalChannels_--;
	ski->setIndex(-1);
	// Remove the key from keys and selectedKeys
	keys_->remove(ski.get());
	selectedKeys_->remove(ski.get());

	/**
	 * Must be locked, but not used SYNCHRONIZED(ch),
	 * otherwise if channel is registered on two selector's,
	 * access channel is dangerous.
	 */
	sp<ESelectableChannel> c = null;
	sp<ESelectableChannel> ch = ski->channel();
	SCOPED_SLOCK1(ch.get()) {
		deregister(ski);
		if (!ch->isOpen() && !ch->isRegistered()) {
			ch->kill();
			c = ch;
		}
    }}
}

void EPollSelectorImpl::implClose() {
	SYNCBLOCK (&closeLock_) {
		if (closed_)
			return;
		closed_ = true;
		// Deregister channels
		for (int i=1; i<totalChannels_; i++) {
			sp<ESelectionKey> ski = (*channelArray_)[i];
			ES_ASSERT (ski->getIndex() != -1);
			ski->setIndex(-1);

			sp<ESelectableChannel> c = null;
			sp<ESelectableChannel> selch = ski->channel();
			SCOPED_SLOCK1(selch.get()) {
				deregister(ski);
				if (!selch->isOpen() && !selch->isRegistered()) {
					selch->kill();
					c = selch;
				}
            }}
		}

		//@see: implCloseInterrupt();
		// prevent further wakeup
		SYNCBLOCK(&interruptLock_) {
			interruptTriggered_ = true;
        }}
		if (pipeWrapper_ != null) {
			delete pipeWrapper_;
			pipeWrapper_ = null;
		}

		pollWrapper_->close();
		delete pollWrapper_; //!
		pollWrapper_ = null;
		delete selectedKeys_; //!
		selectedKeys_ = null;
		delete channelArray_; //!
		channelArray_ = null;
		totalChannels_ = 0;
    }}
}

int EPollSelectorImpl::doSelect(llong timeout) {
	if (channelArray_ == null)
		throw EClosedSelectorException(__FILE__, __LINE__);
	processDeregisterQueue();
	try {
		begin();
		pollWrapper_->poll(totalChannels_, 0, timeout);
	} catch (...) {
		end();
        throw;
	}
    end();
	processDeregisterQueue();
	int numKeysUpdated = updateSelectedKeys();
	if (pollWrapper_->getReventOps(0) != 0) {
		// Clear the wakeup pipe
		pollWrapper_->putReventOps(0, 0);
		SYNCBLOCK(&interruptLock_) {
			ENIOUtil::drain(pipeWrapper_->inFD());
			interruptTriggered_ = false;
        }}
	}
	return numKeysUpdated;
}

int EPollSelectorImpl::updateSelectedKeys() {
	int numKeysUpdated = 0;
	// Skip zeroth entry; it is for interrupts only
	for (int i=1; i<totalChannels_; i++) {
		int rOps = pollWrapper_->getReventOps(i);
		if (rOps != 0) {
			sp<ESelectionKey> sk = (*channelArray_)[i];
			pollWrapper_->putReventOps(i, 0);
			if (selectedKeys_->contains(sk.get())) {
				if (sk->channel()->translateAndSetReadyOps(rOps, sk.get())) {
					numKeysUpdated++;
				}
			} else {
				sk->channel()->translateAndSetReadyOps(rOps, sk.get());
				if ((sk->nioReadyOps() & sk->nioInterestOps()) != 0) {
					selectedKeys_->add(sk);
					numKeysUpdated++;
				}
			}
		}
	}
	return numKeysUpdated;
}

} /* namespace nio */
} /* namespace efc */

#endif //!HAVE_SELECT
