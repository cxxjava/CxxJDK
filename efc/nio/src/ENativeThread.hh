/*
 * ENativeThread.hh
 *
 *  Created on: 2014-1-3
 *      Author: cxxjava@163.com
 */

#ifndef ENATIVETHREAD_HH_
#define ENATIVETHREAD_HH_

#include "../../EBase.hh"

namespace efc {
namespace nio {

class ENativeThread {
public:
	// Returns an opaque token representing the native thread underlying the
	// invoking Java thread.  On systems that do not require signalling, this
	// method always returns -1.
	//
	static es_os_thread_t current();

	// Signals the given native thread so as to release it from a blocking I/O
	// operation.  On systems that do not require signalling, this method has
	// no effect.
	//
	static void signal(es_os_thread_t nt);

private:
	/* File descriptor to which we dup other fd's
	 * before closing them for real
	 */
	static int inited;// = false;
	static void init() THROWS(EIOException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* ENATIVETHREAD_HH_ */
