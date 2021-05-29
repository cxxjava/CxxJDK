/*
 * EEPollArrayWrapper.cpp
 *
 *  Created on: 2013-12-27
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_EPOLL

#include "./EEPollArrayWrapper.hh"
#include "./ENIOUtil.hh"
#include "../inc/EFileDispatcher.hh"
#include "../../inc/ESystem.hh"

#include <sys/epoll.h>
#include <sys/time.h>

namespace efc {
namespace nio {

int EEPollArrayWrapper::OPEN_MAX = ENIOUtil::fdLimit();
int EEPollArrayWrapper::NUM_EPOLLEVENTS = ES_MIN(OPEN_MAX, 8192);
int EEPollArrayWrapper::MAX_UPDATE_ARRAY_SIZE = ES_MIN(OPEN_MAX, 64*1024);

static int iepoll(int epfd, struct epoll_event *events, int numfds, long timeout)
{
    long start, now;
    int remaining = timeout;
    struct timeval t;
    int diff;

    gettimeofday(&t, NULL);
    start = t.tv_sec * 1000 + t.tv_usec / 1000;

    for (;;) {
        int res = epoll_wait(epfd, events, numfds, timeout);
        if (res < 0 && errno == EINTR) {
            if (remaining >= 0) {
                gettimeofday(&t, NULL);
                now = t.tv_sec * 1000 + t.tv_usec / 1000;
                diff = now - start;
                remaining -= diff;
                if (diff < 0 || remaining <= 0) {
                    return 0;
                }
                start = now;
            }
        } else {
            return res;
        }
    }
}

EEPollArrayWrapper::EEPollArrayWrapper() :
		outgoingInterruptFD_(0),
		incomingInterruptFD_(0),
		interruptedIndex_(0),
		interrupted_(false),
		updated_(0),
		updateCount_(0),
		eventsLow_(MAX_UPDATE_ARRAY_SIZE),
		eventsHigh_(null) {
	// creates the epoll file descriptor
	epfd_ = epoll_create(256);
	if (epfd_ == -1) {
		throw EIOEXCEPTION;
	}
	pollArray_ = (void*)eso_calloc(NUM_EPOLLEVENTS * sizeof(struct epoll_event));
	if (!pollArray_) {
		throw ERUNTIMEEXCEPTION;
	}

	// eventHigh needed when using file descriptors > 64k
	if (OPEN_MAX > MAX_UPDATE_ARRAY_SIZE)
		eventsHigh_ = new EHashMap<int,EByte*>();

	updateDescriptors_ = new EA<int>(INITIAL_PENDING_UPDATE_SIZE);
}

EEPollArrayWrapper::~EEPollArrayWrapper() {
	::close(epfd_);

	if (pollArray_) {
		eso_free(pollArray_);
	}

	delete eventsHigh_;
	delete updateDescriptors_;
}

void EEPollArrayWrapper::interrupt() {
	char c = 1;
	if (1 != ::write(outgoingInterruptFD_, &c, 1)) {
		throw EIOException(__FILE__, __LINE__, "EPollArrayWrapper: interrupt failed");
	}
}

void EEPollArrayWrapper::initInterrupt(int fd0, int fd1) {
	outgoingInterruptFD_ = fd1;
	incomingInterruptFD_ = fd0;
	epollCtl(epfd_, EPOLL_CTL_ADD, fd0, EPOLLIN);
}

boolean EEPollArrayWrapper::interrupted() {
	return interrupted_;
}

int EEPollArrayWrapper::interruptedIndex() {
	return interruptedIndex_;
}

void EEPollArrayWrapper::clearInterrupted() {
	interrupted_ = false;
}

void EEPollArrayWrapper::updateRegistrations() {
	SYNCBLOCK (&updateLock_) {
		int j = 0;
		while (j < updateCount_) {
			int fd = (*updateDescriptors_)[j];
			short events = getUpdateEvents(fd);
			boolean isRegistered = registered_.get(fd);
			int opcode = 0;

			if (events != KILLED) {
				if (isRegistered) {
					opcode = (events != 0) ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
				} else {
					opcode = (events != 0) ? EPOLL_CTL_ADD : 0;
				}
				if (opcode != 0) {
					epollCtl(epfd_, opcode, fd, events);
					if (opcode == EPOLL_CTL_ADD) {
						registered_.set(fd);
					} else if (opcode == EPOLL_CTL_DEL) {
						registered_.clear(fd);
					}
				}
			}
			j++;
		}
		updateCount_ = 0;
    }}
}

void EEPollArrayWrapper::putEventOps(int i, int event) {
	struct epoll_event *ee = (struct epoll_event*)pollArray_ + i;
	ee->events = event;
}

void EEPollArrayWrapper::putDescriptor(int i, int fd) {
	struct epoll_event *ee = (struct epoll_event*)pollArray_ + i;
	ee->data.fd = fd;
}

int EEPollArrayWrapper::getEventOps(int i) {
	struct epoll_event *ee = (struct epoll_event*)pollArray_ + i;
	return ee->events;
}

int EEPollArrayWrapper::getDescriptor(int i) {
	struct epoll_event *ee = (struct epoll_event*)pollArray_ + i;
	return (int)(ee->data.fd);
}

int EEPollArrayWrapper::poll(llong timeout) {
	updateRegistrations();
	//@see: updated_ = epollWait(pollArrayAddress, NUM_EPOLLEVENTS, timeout, epfd);
	updated_ = epollWait(pollArray_, NUM_EPOLLEVENTS, timeout, epfd_);
	for (int i=0; i<updated_; i++) {
		if (getDescriptor(i) == incomingInterruptFD_) {
			interruptedIndex_ = i;
			interrupted_ = true;
			break;
		}
	}
	return updated_;
}

void EEPollArrayWrapper::add(int fd) {
	// force the initial update events to 0 as it may be KILLED by a
	// previous registration.
	SYNCBLOCK (&updateLock_) {
		//assert !registered.get(fd);
		setUpdateEvents(fd, (byte)0, true);
    }}
}

void EEPollArrayWrapper::remove(int fd) {
	SYNCBLOCK (&updateLock_) {
		// kill pending and future update for this file descriptor
		setUpdateEvents(fd, KILLED, false);

		// remove from epoll
		if (registered_.get(fd)) {
			epollCtl(epfd_, EPOLL_CTL_DEL, fd, 0);
			registered_.clear(fd);
		}
    }}
}

void EEPollArrayWrapper::setInterest(int fd, int mask) {
	SYNCBLOCK (&updateLock_) {
		// record the file descriptor and events
		int oldCapacity = updateDescriptors_->length();
		if (updateCount_ == oldCapacity) {
			int newCapacity = oldCapacity + INITIAL_PENDING_UPDATE_SIZE;
			EA<int>* newDescriptors = new EA<int>(newCapacity);
			ESystem::arraycopy(*updateDescriptors_, 0, *newDescriptors, 0, oldCapacity);
			delete updateDescriptors_; //!
			updateDescriptors_ = newDescriptors;
		}
		(*updateDescriptors_)[updateCount_++] = fd;

		// events are stored as bytes for efficiency reasons
		byte b = (byte)mask;
		ES_ASSERT((b == mask) && (b != KILLED));
		setUpdateEvents(fd, b, false);
    }}
}

//@see: closeEPollFD()
void EEPollArrayWrapper::close() {
	EFileDispatcher::close(epfd_);
	if (pollArray_) {
		eso_free(pollArray_);
		pollArray_ = NULL;
	}
}

byte EEPollArrayWrapper::getUpdateEvents(int fd) {
	if (fd < MAX_UPDATE_ARRAY_SIZE) {
		return eventsLow_[fd];
	} else {
		EByte* result = eventsHigh_->get(fd);
		// result should never be null
		return result->byteValue();
	}
}

void EEPollArrayWrapper::setUpdateEvents(int fd, byte events, boolean force) {
	if (fd < MAX_UPDATE_ARRAY_SIZE) {
		if ((eventsLow_[fd] != KILLED) || force) {
			eventsLow_[fd] = events;
		}
	} else {
		if (!isEventsHighKilled(fd) || force) {
			eventsHigh_->put(fd, new EByte(events));
		}
	}
}

boolean EEPollArrayWrapper::isEventsHighKilled(int key) {
	ES_ASSERT(key >= MAX_UPDATE_ARRAY_SIZE);
	EByte* value = eventsHigh_->get(key);
	return (value != null && value->byteValue() == KILLED);
}

void EEPollArrayWrapper::epollCtl(int epfd, int opcode, int fd, int events) {
	struct epoll_event event;
	int res;

	event.events = events;
	event.data.fd = fd;

	RESTARTABLE(epoll_ctl(epfd, (int)opcode, (int)fd, &event), res);

	/*
	 * A channel may be registered with several Selectors. When each Selector
	 * is polled a EPOLL_CTL_DEL op will be inserted into its pending update
	 * list to remove the file descriptor from epoll. The "last" Selector will
	 * close the file descriptor which automatically unregisters it from each
	 * epoll descriptor. To avoid costly synchronization between Selectors we
	 * allow pending updates to be processed, ignoring errors. The errors are
	 * harmless as the last update for the file descriptor is guaranteed to
	 * be EPOLL_CTL_DEL.
	 */
	if (res < 0 && errno != EBADF && errno != ENOENT && errno != EPERM) {
		throw EIOException(__FILE__, __LINE__, "epoll_ctl failed");
	}
}

int EEPollArrayWrapper::epollWait(void* pollAddress, int numfds, llong timeout, int epfd) {
	struct epoll_event *events = (struct epoll_event *)pollAddress;
	int res;

	if (timeout <= 0) {           /* Indefinite or no wait */
		RESTARTABLE(epoll_wait(epfd, events, numfds, timeout), res);
	} else {                      /* Bounded wait; bounded restarts */
		res = iepoll(epfd, events, numfds, timeout);
	}

	if (res < 0) {
		throw EIOException(__FILE__, __LINE__, "epoll_wait failed");
	}
	return res;
}

} /* namespace nio */
} /* namespace efc */

#endif //!HAVE_EPOLL
