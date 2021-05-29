/*
 * ESelectableChannel.cpp
 *
 *  Created on: 2013-12-16
 *      Author: cxxjava@163.com
 */

#include "../inc/ESelectableChannel.hh"
#include "../../inc/ESynchronizeable.hh"
#include "../../inc/concurrent/EReentrantLock.hh"
#include "../inc/ESelectionKey.hh"

namespace efc {
namespace nio {

ESelectableChannel::ESelectableChannel() :
		keys_(null), keyCount_(0), blocking_(true ) {
	keyLock_ = new EReentrantLock();
	regLock_ = new EReentrantLock();
}

ESelectableChannel::~ESelectableChannel() {
	delete keyLock_;
	delete regLock_;
	delete keys_;
}

boolean ESelectableChannel::isRegistered() {
	SYNCBLOCK (keyLock_) {
		return keyCount_ != 0;
    }}
}

sp<ESelectionKey> ESelectableChannel::keyFor(ESelector* sel) {
	return findKey(sel);
}

sp<ESelectionKey> ESelectableChannel::register_(ESelector* sel, int ops, EObject* att) {
	SYNCBLOCK (regLock_) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		if ((ops & ~validOps()) != 0)
			throw EILLEGALARGUMENTEXCEPTION;
		if (blocking_)
			throw EILLEGALBLOCKINGMODEEXCEPTION;
		sp<ESelectionKey> k = findKey(sel);
		if (k != null) {
			k->interestOps(ops);
			delete k->attach(att);
		}
		if (k == null) {
			// New registration
			SYNCBLOCK (keyLock_) {
				if (!isOpen())
					throw EClosedChannelException(__FILE__, __LINE__);
				k = sel->register_(shared_from_this(), ops, att);
				addKey(k);
            }}
		}
		return k;
    }}
}

ESelectableChannel* ESelectableChannel::configureBlocking(boolean block) {
	SYNCBLOCK (regLock_) {
		if (!isOpen())
			throw ECLOSEDCHANNELEXCEPTION;
		if (blocking_ == block)
			return this;
		if (block && haveValidKeys())
			throw EILLEGALBLOCKINGMODEEXCEPTION;
		implConfigureBlocking(block);
		blocking_ = block;
    }}
	return this;
}

boolean ESelectableChannel::isBlocking() {
	SYNCBLOCK (regLock_) {
		return blocking_;
    }}
}

ELock* ESelectableChannel::blockingLock() {
	return regLock_;
}

sp<ESelectionKey> ESelectableChannel::findKey(ESelector* sel) {
	SYNCBLOCK (keyLock_) {
		if (keys_ == null)
			return null;
		for (int i = 0; i < keys_->length(); i++)
			if (((*keys_)[i] != null) && ((*keys_)[i]->selector() == sel))
				return (*keys_)[i];
		return null;
    }}
}

void ESelectableChannel::addKey(sp<ESelectionKey> k) {
	//assert Thread.holdsLock(keyLock);
	int i = 0;
	if ((keys_ != null) && (keyCount_ < keys_->length())) {
		// Find empty element of key array
		for (i = 0; i < keys_->length(); i++)
			if ((*keys_)[i] == null) {
				keys_->setAt(i, k);
				break;
			}
	} else if (keys_ == null) {
		keys_ =  new EArray<sp<ESelectionKey> >(3);
		keys_->add(k);
	} else {
		// Grow key array
		keys_->add(k);
	}

	keyCount_++;
}

boolean ESelectableChannel::haveValidKeys() {
	SYNCBLOCK (keyLock_) {
		if (!keys_ || keyCount_ == 0)
			return false;
		for (int i = 0; i < keys_->length(); i++) {
			if (((*keys_)[i] != null) && (*keys_)[i]->isValid())
			    return true;
		}
		return false;
    }}
}

void ESelectableChannel::removeKey(sp<ESelectionKey> k) {
	SYNCBLOCK (keyLock_) {
		for (int i = 0; i < keys_->length(); i++)
			if ((*keys_)[i] == k) {
				//@see: (*keys_)[i] = null;
                keys_->setAt(i, null);
				keyCount_--;
			}
	    k->invalidate();
    }}
}

void ESelectableChannel::implCloseChannel() {
	implCloseSelectableChannel();
	SYNCBLOCK (keyLock_) {
		int count = (keys_ == null) ? 0 : keys_->length();
		for (int i = 0; i < count; i++) {
			sp<ESelectionKey> k = (*keys_)[i];
			if (k != null)
				k->cancel();
		}
    }}
}

} /* namespace nio */
} /* namespace efc */
