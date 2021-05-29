/*
 * EKQueueArrayWrapper.hh
 *
 *  Created on: 2016-2-16
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_KQUEUE

#ifndef EKQUEUEARRAYWRAPPER_HH_
#define EKQUEUEARRAYWRAPPER_HH_

#include "../../inc/ELinkedList.hh"
#include "../../inc/ESynchronizeable.hh"
#include "../inc/ESelectableChannel.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/src/macosx/classes/sun/nio/ch/KQueueArrayWrapper.java

/*
 * KQueueArrayWrapper.java
 * Implementation of Selector using FreeBSD / Mac OS X kqueues
 * Derived from Sun's DevPollArrayWrapper
 */

/*
 * struct kevent {           // 32-bit    64-bit
 *     uintptr_t ident;      //   4         8
 *     short     filter;     //   2         2
 *     u_short   flags;      //   2         2
 *     u_int     fflags;     //   4         4
 *     intptr_t  data;       //   4         8
 *     void      *udata;     //   4         8
 * }                  // Total:  20        32
 *
 * The implementation works in 32-bit and 64-bit world. We do this by calling a
 * native function that actually sets the sizes and offsets of the fields based
 * on which mode we're in.
 */

class EKQueueArrayWrapper {
public:
	// kevent array size
	static const int NUM_KEVENTS = 128;

public:
	EKQueueArrayWrapper();
	~EKQueueArrayWrapper();

	void interrupt();
	void initInterrupt(int fd0, int fd1);
	void release(sp<ESelectableChannel> channel);
	void updateRegistrations();

	int getDescriptor(int i);
	int getReventOps(int index);

	void setInterest(sp<ESelectableChannel> channel, int events);

	int poll(llong timeout) THROWS(EIOException);
	void close() THROWS(EIOException);

private:
	// Used to update file description registrations
	class Update: public EObject {
	public:
		sp<ESelectableChannel> channel;
		int events;

		Update(sp<ESelectableChannel> channel, int events) {
			this->channel = channel;
			this->events = events;
		}
	};

	// The kqueue fd
	int kq_;// = -1;

	// The fd of the interrupt line going out
	int outgoingInterruptFD_;

	// The fd of the interrupt line coming in
	int incomingInterruptFD_;

	 // The kevent array (used for outcoming events only)
	void *kevs_;

	ELinkedList<Update*>* updateList_;
	EReentrantLock updateListLock_;
};

} /* namespace nio */
} /* namespace efc */
#endif /* EKQUEUEARRAYWRAPPER_HH_ */

#endif //!HAVE_KQUEUE
