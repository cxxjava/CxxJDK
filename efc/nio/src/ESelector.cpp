/*
 * ESelector.cpp
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#include "../inc/ESelector.hh"
#include "../inc/ESelectionKey.hh"
#include "../../inc/EHashSet.hh"
#include "../../inc/concurrent/EReentrantLock.hh"
#include "../../inc/EIterator.hh"
#include "../../inc/ESocketException.hh"
#include "./EPipeWrapper.hh"
#include "./ENIOUtil.hh"

#include "./EEPollSelectorImpl.hh"
#include "./EKQueueSelectorImpl.hh"
#include "./EPollSelectorImpl.hh"

namespace efc {
namespace nio {

ESelector* ESelector::open() {
#ifdef HAVE_EPOLL
	return new EEPollSelectorImpl();
#elif defined(HAVE_KQUEUE)
	return new EKQueueSelectorImpl();
#else //win etc.
	return new EPollSelectorImpl();
#endif
}

boolean ESelector::isOpen() {
	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
	return selectorOpen_.get();
}

void ESelector::close()
{
	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
	boolean open = selectorOpen_.getAndSet(false);
	if (!open)
		return;

	//@see: openjdk-8/src/share/classes/sun/nio/ch/SelectorImpl.java
	wakeup();

	SYNCHRONIZED (this) {
		SYNCBLOCK (keysLock_) {
			SYNCBLOCK (selectedKeysLock_) {
				implClose();
            }}
        }}
    }}
}

ESet<sp<ESelectionKey> >* ESelector::cancelledKeys() {
	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
	return cancelledKeys_;
}

ESelector::ESelector() : interruptor_(null),
		selectorOpen_(true) {
	keys_ = new EHashSet<sp<ESelectionKey> >();
	keysLock_ = new EReentrantLock();

	selectedKeys_ = new EHashSet<sp<ESelectionKey> >();
	selectedKeysLock_ = new EReentrantLock();

	cancelledKeys_ = new EHashSet<sp<ESelectionKey> >();
	cancelledKeysLock_ = new EReentrantLock();
}

ESelector::~ESelector()
{
	delete cancelledKeys_;
	delete cancelledKeysLock_;

	delete selectedKeys_;
	delete selectedKeysLock_;

	if (keys_) {
		delete keys_;
	}
	delete keysLock_;

	delete interruptor_;
}

void ESelector::cancel(sp<ESelectionKey> k) {
	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java

	SYNCBLOCK(cancelledKeysLock_) {
		cancelledKeys_->add(k);
    }}
}

ESet<sp<ESelectionKey> >* ESelector::keys() {
	if (!isOpen())
		throw ECLOSEDSELECTOREXCEPTION;
	return keys_;
}

ESet<sp<ESelectionKey> >* ESelector::selectedKeys() {
	if (!isOpen())
		throw ECLOSEDSELECTOREXCEPTION;
	return selectedKeys_;
}

int ESelector::select(llong timeout) {
	if (timeout < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative timeout");
	return lockAndDoSelect((timeout == 0) ? -1 : timeout);
}

int ESelector::selectNow() THROWS(EIOException) {
	return lockAndDoSelect(0);
}

int ESelector::lockAndDoSelect(llong timeout) {
	SYNCHRONIZED (this) {
		if (!isOpen())
			throw ECLOSEDSELECTOREXCEPTION;
		SYNCBLOCK (keysLock_) {
			SYNCBLOCK (selectedKeysLock_) {
				return doSelect(timeout);
            }}
        }}
    }}
}

sp<ESelectionKey> ESelector::register_(sp<ESelectableChannel> ch, int ops, EObject* att) {
	//@see: openjdk-8/jdk/src/share/classes/sun/nio/ch/SelectorImpl.java register()

	sp<ESelectionKey> k = new ESelectionKey(ch, this);
	delete k->attach(att);
	SYNCBLOCK (keysLock_) {
		implRegister(k);
    }}
	k->interestOps(ops);
	return k;
}

void ESelector::processDeregisterQueue() {
    //@see: openjdk-8/jdk/src/share/classes/sun/nio/ch/SelectorImpl.java
    
	// Precondition: Synchronized on this, keys, and selectedKeys
	SYNCBLOCK (cancelledKeysLock_) {
		if (!cancelledKeys_->isEmpty()) {
			sp<EIterator<sp<ESelectionKey> > > i = cancelledKeys_->iterator();
			while (i->hasNext()) {
				sp<ESelectionKey> ski = i->next();
				try {
					implDereg(ski);
				} catch (ESocketException& se) {
					finally {
						i->remove();
					}
					throw EIOException(__FILE__, __LINE__, "Error deregistering key");
				} catch (...) {
					finally {
						i->remove();
					}
					throw; //!
				} finally {
					i->remove();
				}
				ski->channel_ = null; //#added by cxxjava: sp<> breaking!
			}
		}
    }}
}

void ESelector::deregister(sp<ESelectionKey> key) {
	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
	key->channel()->removeKey(key);
}

void ESelector::begin() {
	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java begin()
	class Interruptible : public EInterruptible {
	private:
		ESelector* self;
	public:
		Interruptible(ESelector* sel) : self(sel) {
		}

		virtual void interrupt(EThread* ignore) {
			self->wakeup();
		}
	};

	if (interruptor_ == null) {
		interruptor_ = new Interruptible(this);
	}
	EInterruptibleChannel::blockedOn(interruptor_);
	EThread* me = EThread::currentThread();
	if (me->isInterrupted())
		interruptor_->interrupt(me);
}

void ESelector::end() {
	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java end()
	EInterruptibleChannel::blockedOn(null);
}

void ESelector::putEventOps(ESelectionKey* sk, int ops) {
	//
}

} /* namespace nio */
} /* namespace efc */
