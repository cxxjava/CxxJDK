/*
 * ESimpleLock.cpp
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#include "ESimpleLock.hh"
#include "ESimpleCondition.hh"

namespace efc {

ESimpleLock::ESimpleLock(int flag) {
	m_Mutex = eso_thread_mutex_create(flag);
}

ESimpleLock::~ESimpleLock() {
	eso_thread_mutex_destroy(&m_Mutex);
}

void ESimpleLock::lock()
{
	eso_thread_mutex_lock(m_Mutex);
}

void ESimpleLock::lockInterruptibly() {
	//TODO...
}

boolean ESimpleLock::tryLock()
{
	es_status_t rv = eso_thread_mutex_trylock(m_Mutex);
	return (rv == ES_SUCCESS) ? true : false;
}

boolean ESimpleLock::tryLock(llong timeout, ETimeUnit* unit)
{
	es_status_t rv = eso_thread_mutex_timedlock(m_Mutex, unit->toMillis(timeout));
	return (rv == ES_SUCCESS) ? true : false;
}

void ESimpleLock::unlock()
{
	eso_thread_mutex_unlock(m_Mutex);
}

ECondition* ESimpleLock::newCondition()
{
	return new ESimpleCondition(m_Mutex);
}

} /* namespace efc */
