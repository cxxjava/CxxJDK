/*
 * ESpinLock.cpp
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#include "ESpinLock.hh"

namespace efc {

ESpinLock::ESpinLock() {
	m_Spin = eso_thread_spin_create();
}

ESpinLock::~ESpinLock() {
	eso_thread_spin_destroy(&m_Spin);
}

void ESpinLock::lock()
{
	eso_thread_spin_lock(m_Spin);
}

void ESpinLock::lockInterruptibly() {
	throw EUnsupportedOperationException(__FILE__, __LINE__);
}

boolean ESpinLock::tryLock()
{
	es_status_t rv = eso_thread_spin_trylock(m_Spin);
	return (rv == ES_SUCCESS) ? true : false;
}

boolean ESpinLock::tryLock(llong time, ETimeUnit* unit) {
	es_status_t rv = eso_thread_spin_timedlock(m_Spin, unit->toMillis(time));
	return (rv == ES_SUCCESS) ? true : false;
}

void ESpinLock::unlock()
{
	eso_thread_spin_unlock(m_Spin);
}

} /* namespace efc */
