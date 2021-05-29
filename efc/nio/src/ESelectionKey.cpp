/*
 * ESelectionKey.cpp
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#include "../inc/ESelectionKey.hh"

namespace efc {
namespace nio {

ESelectionKey::~ESelectionKey() {
	//
}

ESelectionKey::ESelectionKey(sp<ESelectableChannel> ch, ESelector* sel) :
		channel_(ch), selector_(sel), attachment_(null), valid_(true ), readyOps_(
				0), index_(0) {
}

sp<ESelectableChannel> ESelectionKey::channel() {
	return channel_;
}

ESelector* ESelectionKey::selector() {
	return selector_;
}

boolean ESelectionKey::isValid() {
	return valid_;
}

void ESelectionKey::cancel() {
	// Synchronizing "this" to prevent this key from getting canceled
	// multiple times by different threads, which might cause race
	// condition between selector's select() and channel's close().
	SYNCHRONIZED (this) {
		if (valid_) {
			valid_ = false;
			selector_->cancel(shared_from_this());
		}
    }}
}

void ESelectionKey::ensureValid() {
	if (!isValid())
		throw ECANCELLEDKEYEXCEPTION;
}

int ESelectionKey::interestOps() {
	ensureValid();
	return interestOps_;
}

void ESelectionKey::interestOps(int ops) {
	ensureValid();
	nioInterestOps(ops);
}

int ESelectionKey::readyOps() {
	ensureValid();
	return readyOps_;
}

boolean ESelectionKey::isReadable() {
	return (readyOps() & OP_READ) != 0;
}

boolean ESelectionKey::isWritable() {
	return (readyOps() & OP_WRITE) != 0;
}

boolean ESelectionKey::isConnectable() {
	return (readyOps() & OP_CONNECT) != 0;
}

boolean ESelectionKey::isAcceptable() {
	return (readyOps() & OP_ACCEPT) != 0;
}

EObject* ESelectionKey::attach(EObject* ob) {
	return attachment_.getAndSet(ob);
}

EObject* ESelectionKey::attachment() {
	return attachment_.get();
}

int ESelectionKey::getIndex() {
	return index_;
}

void ESelectionKey::setIndex(int i) {
	index_ = i;
}

void ESelectionKey::nioReadyOps(int ops) {
	readyOps_ = ops;
}

int ESelectionKey::nioReadyOps() {
	return readyOps_;
}

void ESelectionKey::nioInterestOps(int ops) {
	if ((ops & ~this->channel()->validOps()) != 0)
		throw EILLEGALARGUMENTEXCEPTION;
	channel_->translateAndSetInterestOps(ops, this);
	interestOps_ = ops;
}

int ESelectionKey::nioInterestOps() {
	return interestOps_;
}

void ESelectionKey::invalidate() {
	valid_ = false;
}

} /* namespace nio */
} /* namespace efc */
