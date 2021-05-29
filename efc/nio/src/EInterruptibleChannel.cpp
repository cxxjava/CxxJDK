/*
 * EInterruptibleChannel.cpp
 *
 *  Created on: 2015-10-27
 *      Author: cxxjava@163.com
 */

#include "../inc/EInterruptibleChannel.hh"
#include "../inc/EClosedByInterruptException.hh"

namespace efc {
namespace nio {

EInterruptibleChannel::Interruptible::Interruptible(
		EInterruptibleChannel* ic) :
		self(ic) {
}

void EInterruptibleChannel::Interruptible::interrupt(EThread* target) {
	SYNCBLOCK(&self->closeLock_) {
		if (!self->open_)
			return;
		self->open_ = false;
		self->interrupted_ = target;
		try {
			self->implCloseChannel();
		} catch (EIOException& x) {
		}
    }}
}

EInterruptibleChannel::~EInterruptibleChannel() {
	delete interruptor_;
}

void EInterruptibleChannel::close() {
	SYNCBLOCK(&closeLock_) {
		if (!open_)
			return;
		open_ = false;
		implCloseChannel();
    }}
}

boolean EInterruptibleChannel::isOpen() {
	return open_;
}

EInterruptibleChannel::EInterruptibleChannel() :
		open_(true ), interruptor_(null), interrupted_(null) {
}

void EInterruptibleChannel::begin() {
	//@see: openjdk-6-src-b27-26_oct_2012.tar/jdk/src/share/classes/java/nio/channels/spi/AbstractInterruptibleChannel.java begin()
	if (interruptor_ == null) {
		interruptor_ = new Interruptible(this);
	}
	blockedOn(interruptor_);
	EThread* me = EThread::currentThread();
	if (me->isInterrupted())
		interruptor_->interrupt(me);
}

void EInterruptibleChannel::end(boolean completed) {
	//@see: openjdk-6-src-b27-26_oct_2012.tar/jdk/src/share/classes/java/nio/channels/spi/AbstractInterruptibleChannel.java end()
	blockedOn(null);
	EThread* interrupted = this->interrupted_;
	if (interrupted != null && interrupted == EThread::currentThread()) {
		interrupted = null;
		throw EClosedByInterruptException(__FILE__, __LINE__);
	}
	if (!completed && !open_)
		throw EAsynchronousCloseException(__FILE__, __LINE__);
}

void EInterruptibleChannel::blockedOn(EInterruptible* intr) {
	//@see: http://www.oschina.net/question/138146_26027
	EThread::currentThread()->blockedOn(intr);
}

} /* namespace nio */
} /* namespace efc */
