/*
 * ESentry.cpp
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#include "ESentry.hh"

namespace efc {

ESentry::ESentry(ELock* lock) :
		_lock(lock) {
	if (_lock) {
		_lock->lock();
	}
}

ESentry::~ESentry()
{
	if (_lock) {
		_lock->unlock();
	}
}

} /* namespace efc */
