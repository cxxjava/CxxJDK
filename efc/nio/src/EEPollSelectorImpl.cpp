/*
 * EEPollSelectorImpl.cpp
 *
 *  Created on: 2016-2-14
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_EPOLL

#include "./EEPollSelectorImpl.hh"
#include "./ENIOUtil.hh"
#include "../inc/EClosedSelectorException.hh"

namespace efc {
namespace nio {

EEPollSelectorImpl::~EEPollSelectorImpl() {
	delete fdToKey_;
	delete pollWrapper_;
	delete pipeWrapper_;
}

EEPollSelectorImpl::EEPollSelectorImpl() : closed_(false),
		interruptTriggered_(false) {
	pollWrapper_ = new EEPollArrayWrapper();
	pipeWrapper_ = new EPipeWrapper();
	pollWrapper_->initInterrupt(pipeWrapper_->inFD(), pipeWrapper_->outFD());

	fdToKey_ = new EHashMap<int, sp<ESelectionKey> >();
}

ESelector* EEPollSelectorImpl::wakeup() {
	SYNCBLOCK (&interruptLock_) {
		if (!interruptTriggered_) {
			pollWrapper_->interrupt();
			interruptTriggered_ = true;
		}
    }}
	return this;
}

void EEPollSelectorImpl::putEventOps(ESelectionKey* sk, int ops) {
	if (closed_)
		throw EClosedSelectorException(__FILE__, __LINE__);
	pollWrapper_->setInterest(sk->channel()->getFDVal(), ops);
}

void EEPollSelectorImpl::implRegister(sp<ESelectionKey> ski) {
	if (closed_)
		throw EClosedSelectorException(__FILE__, __LINE__);
	sp<ESelectableChannel> ch = ski->channel();
	int fd = ch->getFDVal();
	fdToKey_->put(fd, ski);
	pollWrapper_->add(fd);
	keys_->add(ski);
}

void EEPollSelectorImpl::implDereg(sp<ESelectionKey> ski) {
	ES_ASSERT (ski->getIndex() >= 0);

	sp<ESelectableChannel> ch = ski->channel();
	int fd = ch->getFDVal();
	fdToKey_->remove(fd);
	pollWrapper_->remove(fd);
	ski->setIndex(-1);
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

void EEPollSelectorImpl::implClose() {
	if (closed_)
		return;

	closed_ = true;

	// prevent further wakeup
	SYNCBLOCK (&interruptLock_) {
		interruptTriggered_ = true;
    }}

	if (pipeWrapper_ != null) {
		delete pipeWrapper_;
		pipeWrapper_ = null;
	}

	if (pollWrapper_ != null) {
		pollWrapper_->close();

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

		delete pollWrapper_;
		pollWrapper_ = null;
	}
}

int EEPollSelectorImpl::doSelect(llong timeout) {
	int entries = 0;
	if (closed_)
		throw EClosedSelectorException(__FILE__, __LINE__);
	processDeregisterQueue();
	try {
		begin();
		entries = pollWrapper_->poll(timeout);
	} catch (...) {
		end();
        throw;
	}
    end();
	processDeregisterQueue();
	int numKeysUpdated = updateSelectedKeys(entries);
	if (pollWrapper_->interrupted()) {
		// Clear the wakeup pipe
		pollWrapper_->putEventOps(pollWrapper_->interruptedIndex(), 0);
		SYNCBLOCK (&interruptLock_) {
			pollWrapper_->clearInterrupted();
			//@see: IOUtil.drain(fd0);
			ENIOUtil::drain(pipeWrapper_->inFD());
			interruptTriggered_ = false;
        }}
	}
	return numKeysUpdated;
}

int EEPollSelectorImpl::updateSelectedKeys(int entries) {
	int numKeysUpdated = 0;
	for (int i = 0; i < entries; i++) {
		int nextFD = pollWrapper_->getDescriptor(i);
		{
			sp<ESelectionKey> ski = fdToKey_->get(nextFD);
			// ski is null in the case of an interrupt
			if (ski != null) {
				int rOps = pollWrapper_->getEventOps(i);
				if (selectedKeys_->contains(ski.get())) {
					if (ski->channel()->translateAndSetReadyOps(rOps, ski.get())) {
						numKeysUpdated++;
					}
				} else {
					ski->channel()->translateAndSetReadyOps(rOps, ski.get());
					if ((ski->nioReadyOps() & ski->nioInterestOps()) != 0) {
						selectedKeys_->add(ski);
						numKeysUpdated++;
					}
				}
			}
		}
	}
	return numKeysUpdated;
}

} /* namespace nio */
} /* namespace efc */

#endif //!HAVE_EPOLL
