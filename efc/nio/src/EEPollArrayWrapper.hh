/*
 * EEPollArrayWrapper.hh
 *
 *  Created on: 2016-2-18
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_EPOLL

#ifndef EEPOLLARRAYWRAPPER_HH_
#define EEPOLLARRAYWRAPPER_HH_

#include "../../inc/EByte.hh"
#include "../../inc/EBitSet.hh"
#include "../../inc/EHashSet.hh"
#include "../../inc/ELinkedList.hh"
#include "../../inc/ESynchronizeable.hh"
#include "../inc/ESelectableChannel.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/jdk-f5d77a430a29/src/solaris/classes/sun/nio/ch/EPollArrayWrapper.java

/**
 * Manipulates a native array of epoll_event structs on Linux:
 *
 * typedef union epoll_data {
 *     void *ptr;
 *     int fd;
 *     __uint32_t u32;
 *     __uint64_t u64;
 *  } epoll_data_t;
 *
 * struct epoll_event {
 *     __uint32_t events;
 *     epoll_data_t data;
 * };
 *
 * The system call to wait for I/O events is epoll_wait(2). It populates an
 * array of epoll_event structures that are passed to the call. The data
 * member of the epoll_event structure contains the same data as was set
 * when the file descriptor was registered to epoll via epoll_ctl(2). In
 * this implementation we set data.fd to be the file descriptor that we
 * register. That way, we have the file descriptor available when we
 * process the events.
 */

class EEPollArrayWrapper {
public:
	EEPollArrayWrapper();
	~EEPollArrayWrapper();

	void interrupt();
	void initInterrupt(int fd0, int fd1);
	boolean interrupted();
	int interruptedIndex();
	void clearInterrupted();

	void updateRegistrations();

	void putEventOps(int i, int event);
	int getEventOps(int i);
	void putDescriptor(int i, int fd);
	int getDescriptor(int i);

	/**
	 *
	 */
	int poll(llong timeout) THROWS(EIOException);

	/**
	 * Add a channel's file descriptor to epoll
	 */
	void add(int fd);

	/**
	 * Remove a file descriptor
	 */
	void remove(int fd);

	/**
	 * Update the events for a given file descriptor
	 */
	void setInterest(int fd, int mask);

	/**
	 * Close epoll file descriptor and free poll array
	 */
	void close() THROWS(EIOException);

private:
	// EPOLL_EVENTS
	static const int EPOLLIN      = 0x001;

	// opcodes
	static const int EPOLL_CTL_ADD      = 1;
	static const int EPOLL_CTL_DEL      = 2;
	static const int EPOLL_CTL_MOD      = 3;

	// Miscellaneous constants
	static int OPEN_MAX;//         = IOUtil.fdLimit();
	static int NUM_EPOLLEVENTS;//  = Math.min(OPEN_MAX, 8192);

	// Special value to indicate that an update should be ignored
	static const byte  KILLED = (byte)-1;

	// Initial size of arrays for fd registration changes
	static const int INITIAL_PENDING_UPDATE_SIZE = 64;

	// maximum size of updatesLow
	static int MAX_UPDATE_ARRAY_SIZE;// = AccessController.doPrivileged(new GetIntegerAction("sun.nio.ch.maxUpdateArraySize", Math.min(OPEN_MAX, 64*1024)));

	// The fd of the epoll driver
	int epfd_;

	 // The epoll_event array for results from epoll_wait
	void* pollArray_;

	// The fd of the interrupt line going out
	int outgoingInterruptFD_;

	// The fd of the interrupt line coming in
	int incomingInterruptFD_;

	// The index of the interrupt FD
	int interruptedIndex_;

	// interrupt support
	boolean interrupted_;// = false;

	// Number of updated pollfd entries
	int updated_;

	// object to synchronize fd registration changes
	EReentrantLock updateLock_;

	// number of file descriptors with registration changes pending
	int updateCount_;

	// file descriptors with registration changes pending
	EA<int>* updateDescriptors_;// = new int[INITIAL_PENDING_UPDATE_SIZE];

	// events for file descriptors with registration changes pending, indexed
	// by file descriptor and stored as bytes for efficiency reasons. For
	// file descriptors higher than MAX_UPDATE_ARRAY_SIZE (unlimited case at
	// least) then the update is stored in a map.
	EA<byte> eventsLow_;// = new byte[MAX_UPDATE_ARRAY_SIZE];
	EHashMap<int,EByte*>* eventsHigh_;

	// Used by release and updateRegistrations to track whether a file
	// descriptor is registered with epoll.
	EBitSet registered_;// = new BitSet();


	/**
	 * Returns the pending update events for the given file descriptor.
	 */
	byte getUpdateEvents(int fd);

	/**
	 * Sets the pending update events for the given file descriptor. This
	 * method has no effect if the update events is already set to KILLED,
	 * unless {@code force} is {@code true}.
	 */
	void setUpdateEvents(int fd, byte events, boolean force);

	/**
	 * Returns {@code true} if updates for the given key (file
	 * descriptor) are killed.
	 */
	boolean isEventsHighKilled(int key);

	void epollCtl(int epfd, int opcode, int fd, int events);
	int epollWait(void* pollAddress, int numfds, llong timeout, int epfd) THROWS(EIOException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EEPOLLARRAYWRAPPER_HH_ */

#endif //!HAVE_EPOLL
