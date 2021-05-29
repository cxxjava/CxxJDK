/*
 * EPollArrayWrapper.hh
 *
 *  Created on: 2013-12-27
 *      Author: cxxjava@163.com
 */

#include "es_config.h"

#ifdef HAVE_SELECT

#ifndef EPOLLARRAYWRAPPER_HH_
#define EPOLLARRAYWRAPPER_HH_

#include "../../inc/EObject.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/src/share/classes/sun/nio/ch/AbstractPollArrayWrapper.java
//@see: openjdk-8/src/solaris/classes/sun/nio/ch/PollArrayWrapper.java

/**
 * Manipulates a native array of pollfd structs on Solaris:
 *
 * typedef struct pollfd {
 *    int fd;
 *    short events;
 *    short revents;
 * } pollfd_t;
 *
 * @since 1.4
 */

class EPollArrayWrapper {
public:
	// Number of valid entries in the pollArray
	int totalChannels_;// = 0;

public:
	EPollArrayWrapper(int newSize);
	~EPollArrayWrapper();

	void interrupt();
	void initInterrupt(int fd0, int fd1);

	void putReventOps(int i, int revent);
	int getReventOps(int i);
	void putEventOps(int i, int event);
	int getEventOps(int i);
	void putDescriptor(int i, int fd);
	int getDescriptor(int i);

	int poll(int numfds, int offset, llong timeout);
	void release(int i);
	void close();

	/**
	 * Grows the pollfd array to a size that will accommodate newSize
	 * pollfd entries. This method does no checking of the newSize
	 * to determine if it is in fact bigger than the old size: it
	 * always reallocates an array of the new size.
	 */
	void grow(int newSize);

	/**
	 * Prepare another pollfd struct for use.
	 */
	void addEntry(int fd);

	/**
	 * Writes the pollfd entry from the source wrapper at the source index
	 * over the entry in the target wrapper at the target index. The source
	 * array remains unchanged unless the source array and the target are
	 * the same array.
	 */
	void replaceEntry(int sindex, int tindex);

private:
	// The poll fd array
	void* pollArray_;

	// File descriptor to write for interrupt
	int interruptFD_;
};

} /* namespace nio */
} /* namespace efc */
#endif /* EPOLLARRAYWRAPPER_HH_ */

#endif //!HAVE_SELECT
