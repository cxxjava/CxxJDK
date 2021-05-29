/*
 * EPollArrayWrapper.cpp
 *
 *  Created on: 2013-12-27
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_SELECT

#include "./EPollArrayWrapper.hh"
#include "./ENIOUtil.hh"
#include "../../inc/ENetWrapper.hh"

#ifdef WIN32
#include <winsock2.h>
#include <io.h>
#else
#include <poll.h>
#include <sys/time.h>
#endif

namespace efc {
namespace nio {

#ifdef WIN32
struct pollfd
{
	int     fd;
	short   events;
	short   revents;
};

static int
iselect(struct pollfd fds[], unsigned int numfds, int timeout)
{
	int i, maxfd = -1;
	long result = 0;
	fd_set readfds, writefds;
	struct timeval timevalue, *tv;
	static struct timeval zerotime = {0, 0};

	if (timeout == 0) {
		tv = &zerotime;
	} else if (timeout < 0) {
		tv = NULL;
	} else {
		tv = &timevalue;
		tv->tv_sec =  (long)(timeout / 1000);
		tv->tv_usec = (long)((timeout % 1000) * 1000);
	}

	/* Set FD_SET structures required for select */
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	for (i = 0; i < numfds; i++) {
		if (fds[i].events & ENetWrapper::POLLIN_) {
			FD_SET(fds[i].fd, &readfds);
			maxfd = ES_MAX(maxfd, fds[i].fd);
		}
		if (fds[i].events & (ENetWrapper::POLLOUT_ | ENetWrapper::POLLCONN_)) {
			FD_SET(fds[i].fd, &writefds);
			maxfd = ES_MAX(maxfd, fds[i].fd);
		}
	}

	/* Call select */
	int updated = 0;
	result = ::select(maxfd+1 , &readfds, &writefds, NULL, tv);
	if (result > 0) {
		for (i = 0; i <= numfds; i++) {
				int event = 0;

				if (fds[i].events == 0) continue;
				if ((fds[i].events & ENetWrapper::POLLIN_) && FD_ISSET(fds[i].fd, &readfds))
					event |= ENetWrapper::POLLIN_;
				if ((fds[i].events & (ENetWrapper::POLLOUT_ | ENetWrapper::POLLCONN_)) && FD_ISSET(fds[i].fd, &writefds))
					event |= ENetWrapper::POLLOUT_;

				if (event != 0) {
					fds[i].revents = event;
					updated++;
				}
			}
	}

	return updated;
}

#else //

static int
ipoll(struct pollfd fds[], unsigned int numfds, int timeout) {
	long start, now;
	int remaining = timeout;
	struct timeval t;
	int diff;

	gettimeofday(&t, NULL);
	start = t.tv_sec * 1000 + t.tv_usec / 1000;

	for (;;) {
		int res = ::poll(fds, numfds, remaining);
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
#endif //!WIN32

EPollArrayWrapper::EPollArrayWrapper(int newSize) :
		interruptFD_(-1) {
	newSize = (newSize + 1) * sizeof(struct pollfd);
	pollArray_ = eso_calloc(newSize);
	totalChannels_ = 1;
}

EPollArrayWrapper::~EPollArrayWrapper() {
	if (pollArray_) {
		eso_free(pollArray_);
	}
}

void EPollArrayWrapper::interrupt() {
	const char byte = 1;
#ifdef WIN32
	/* Write one byte into the pipe */
	if (1 != ::send(interruptFD_, &byte, 1, 0))
#else
	if (1 != ::write(interruptFD_, &byte, 1))
#endif
	{
		throw EIOException(__FILE__, __LINE__, "PollArrayWrapper: interrupt failed");
	}
}

void EPollArrayWrapper::initInterrupt(int fd0, int fd1) {
	interruptFD_ = fd1;
	putDescriptor(0, fd0);
	putEventOps(0, ENetWrapper::POLLIN_);
	putReventOps(0, 0);
}

void EPollArrayWrapper::putReventOps(int i, int revent) {
	struct pollfd *se = (struct pollfd*)pollArray_ + i;
	se->revents = revent;
}

int EPollArrayWrapper::getReventOps(int i) {
	struct pollfd *se = (struct pollfd*)pollArray_ + i;
	return se->revents;
}

void EPollArrayWrapper::putEventOps(int i, int event) {
	struct pollfd *se = (struct pollfd*)pollArray_ + i;
	se->events = event;
}

int EPollArrayWrapper::getEventOps(int i) {
	struct pollfd *se = (struct pollfd*)pollArray_ + i;
	return se->events;
}

void EPollArrayWrapper::putDescriptor(int i, int fd) {
	struct pollfd *se = (struct pollfd*)pollArray_ + i;
	se->fd = fd;
}

int EPollArrayWrapper::getDescriptor(int i) {
	struct pollfd *se = (struct pollfd*)pollArray_ + i;
	return se->fd;
}

int EPollArrayWrapper::poll(int numfds, int offset, llong timeout) {
	struct pollfd *a = (struct pollfd*)pollArray_;
	int err = 0;

#ifdef WIN32
	err = iselect(a, numfds, timeout);
#else
	if (timeout <= 0) {           /* Indefinite or no wait */
		RESTARTABLE (::poll(a, numfds, timeout), err);
	} else {                     /* Bounded wait; bounded restarts */
		err = ipoll(a, numfds, timeout);
	}
#endif

	if (err < 0) {
		throw EIOException(__FILE__, __LINE__, "Poll failed");
	}

	return err;
}

void EPollArrayWrapper::release(int i) {
	return;
}

void EPollArrayWrapper::close() {
	if (pollArray_) {
		eso_free(pollArray_);
		pollArray_ = NULL;
	}
}

void EPollArrayWrapper::grow(int newSize) {
	struct pollfd* temp = (struct pollfd*)eso_calloc((newSize + 1) * sizeof(struct pollfd));

	// Copy over existing entries
	for (int i=0; i<totalChannels_; i++) {
		struct pollfd *se = temp + i;
		se->fd = this->getDescriptor(i);
		se->events = this->getEventOps(i);
		se->revents = this->getReventOps(i);
	}

	// Free old
	if (pollArray_)
		eso_free(pollArray_);

	// Swap new array into pollArray field
	pollArray_ = temp;
}

void EPollArrayWrapper::addEntry(int fd) {
	putDescriptor(totalChannels_, fd);
	putEventOps(totalChannels_, 0);
	putReventOps(totalChannels_, 0);
	totalChannels_++;
}

void EPollArrayWrapper::replaceEntry(int sindex, int tindex) {
	this->putDescriptor(tindex, this->getDescriptor(sindex));
	this->putEventOps(tindex, this->getEventOps(sindex));
	this->putReventOps(tindex, this->getReventOps(sindex));
}

} /* namespace nio */
} /* namespace efc */

#endif //!HAVE_SELECT
