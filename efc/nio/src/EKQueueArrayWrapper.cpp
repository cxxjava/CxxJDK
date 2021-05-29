/*
 * EKQueueArrayWrapper.cpp
 *
 *  Created on: 2016-2-16
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_KQUEUE

#include "./EKQueueArrayWrapper.hh"
#include "../../inc/ENetWrapper.hh"

#include <sys/event.h>

namespace efc {
namespace nio {

static int register0(int kq, int fd, int r, int w)
{
    struct kevent changes[2];
    struct kevent errors[2];
    struct timespec dontBlock = {0, 0};

    // if (r) then { register for read } else { unregister for read }
    // if (w) then { register for write } else { unregister for write }
    // Ignore errors - they're probably complaints about deleting non-
    //   added filters - but provide an error array anyway because
    //   kqueue behaves erratically if some of its registrations fail.
    EV_SET(&changes[0], fd, EVFILT_READ,  r ? EV_ADD : EV_DELETE, 0, 0, 0);
    EV_SET(&changes[1], fd, EVFILT_WRITE, w ? EV_ADD : EV_DELETE, 0, 0, 0);
    return kevent(kq, changes, 2, errors, 2, &dontBlock);
}

EKQueueArrayWrapper::EKQueueArrayWrapper() :
		outgoingInterruptFD_(0),
		incomingInterruptFD_(0) {
	updateList_ = new ELinkedList<Update*>();

	kq_ = kqueue();
	if (kq_ == -1) {
		throw EIOEXCEPTION;
	}
	kevs_ = eso_calloc(sizeof(struct kevent) * NUM_KEVENTS);
	if (!kevs_) {
		throw ERUNTIMEEXCEPTION;
	}
}

EKQueueArrayWrapper::~EKQueueArrayWrapper() {
	::close(kq_);

	if (kevs_) {
		eso_free(kevs_);
	}

	delete updateList_;
}

void EKQueueArrayWrapper::interrupt() {
	char c = 1;
	if (1 != ::write(outgoingInterruptFD_, &c, 1)) {
		throw EIOException(__FILE__, __LINE__, "KQueueArrayWrapper: interrupt failed");
	}
}

void EKQueueArrayWrapper::initInterrupt(int fd0, int fd1) {
	outgoingInterruptFD_ = fd1;
	incomingInterruptFD_ = fd0;

	// add fd0 to kqueue.
	if (register0(kq_, fd0, 1, 0) < 0) {
		throw EIOEXCEPTION;
	}
}

void EKQueueArrayWrapper::release(sp<ESelectableChannel> channel) {
	SYNCBLOCK (&updateListLock_) {
		// flush any pending updates
		sp<EIterator<Update*> > it = null;
		for (it = updateList_->iterator(); it->hasNext();) {
			if (it->next()->channel == channel) {
				it->remove();
			}
		}

		// remove
		if (register0(kq_, channel->getFDVal(), 0, 0) < 0) {
			throw EIOEXCEPTION;
		}
    }}
}

void EKQueueArrayWrapper::updateRegistrations() {
	SYNCBLOCK (&updateListLock_) {
		Update* u = null;
		while ((u = updateList_->poll()) != null) {
			sp<ESelectableChannel> ch = u->channel;
			if (!ch->isOpen()) {
				delete u; //#added by cxxjava
				continue;
			}

			register0(kq_, ch->getFDVal(), u->events & ENetWrapper::POLLIN_, u->events & ENetWrapper::POLLOUT_);

			delete u; //#added by cxxjava
		}
    }}
}

int EKQueueArrayWrapper::getDescriptor(int i) {
	struct kevent *ke = (struct kevent*)kevs_ + i;
	return (int)(ke->ident);
}

int EKQueueArrayWrapper::getReventOps(int index) {
	int result = 0;
	struct kevent *ke = (struct kevent*)kevs_ + index;
	short filter = ke->filter;

	// This is all that's necessary based on inspection of usage:
	//   SinkChannelImpl, SourceChannelImpl, DatagramChannelImpl,
	//   ServerSocketChannelImpl, SocketChannelImpl
	if (filter == EVFILT_READ) {
		result |= ENetWrapper::POLLIN_;
	} else if (filter == EVFILT_WRITE) {
		result |= ENetWrapper::POLLOUT_;
	}

	return result;
}

void EKQueueArrayWrapper::setInterest(sp<ESelectableChannel> channel, int events) {
	SYNCBLOCK (&updateListLock_) {
		// update existing registration
		updateList_->add(new Update(channel, events));
    }}
}

int EKQueueArrayWrapper::poll(llong timeout) {
	updateRegistrations();

	//@see: int updated = kevent0(kq, keventArrayAddress, NUM_KEVENTS, timeout);
	struct timespec ts;
	struct timespec *tsp;

	// Java timeout is in milliseconds. Convert to struct timespec.
	// Java timeout == -1 : wait forever : timespec timeout of NULL
	// Java timeout == 0  : return immediately : timespec timeout of zero
	if (timeout >= 0) {
		ts.tv_sec = timeout / 1000;
		ts.tv_nsec = (timeout % 1000) * 1000000; //nanosec = 1 million millisec
		tsp = &ts;
	} else {
		tsp = NULL;
	}

	int updated = kevent(kq_, NULL, 0, (struct kevent *)kevs_, NUM_KEVENTS, tsp);
	if (updated < 0) {
		if (errno == EINTR) {
			// ignore EINTR, pretend nothing was selected
			updated = 0;
		} else {
			throw EIOException(__FILE__, __LINE__, "KQueueArrayWrapper: kqueue failed");
		}
	}

	return updated;
}

void EKQueueArrayWrapper::close() {
	if (kevs_ != null) {
		eso_free(kevs_);
		kevs_ = null;
	}
	if (kq_ >= 0) {
		::close(kq_);
		kq_ = -1;
	}
}

} /* namespace nio */
} /* namespace efc */

#endif //!HAVE_KQUEUE
