/*
 * ELockSupport.cpp
 *
 *  Created on: 2014-9-25
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/ELockSupport.hh"
#include "../../inc/concurrent/EUnsafe.hh"

namespace efc {

void ELockSupport::unpark(EThread* thread) {
	if (thread != null)
		EUnsafe::unpark(thread);
}

void ELockSupport::park() {
	EUnsafe::park(false, 0L);
}

void ELockSupport::parkNanos(llong nanos) {
	if (nanos > 0)
		EUnsafe::park(false, nanos);
}

void ELockSupport::parkUntil(llong deadline) {
	EUnsafe::park(true, deadline);
}

} /* namespace efc */
