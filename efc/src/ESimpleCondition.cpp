/*
 * ESimpleCondition.cpp
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#include "ESimpleCondition.hh"
#include "ESystem.hh"

namespace efc {

ESimpleCondition::~ESimpleCondition() {
	eso_thread_cond_destroy(&m_Cond);
}

ESimpleCondition::ESimpleCondition(es_thread_mutex_t *mutex) {
	m_Cond = eso_thread_cond_create();
	m_Mutex = mutex;
}

void ESimpleCondition::await() {
	eso_thread_cond_wait(m_Cond, m_Mutex);
}

llong ESimpleCondition::awaitNanos(llong nanosTimeout) {
	llong lastTime = ESystem::nanoTime();
	eso_thread_cond_timedwait(m_Cond, m_Mutex, nanosTimeout);
	llong currTime = ESystem::nanoTime();
	return nanosTimeout - (currTime - lastTime);
}

boolean ESimpleCondition::await(llong time, ETimeUnit* unit) {
	llong nanosTimeout = unit->toNanos(time);
	return (awaitNanos(nanosTimeout) > 0) ? false : true;
}

boolean ESimpleCondition::awaitUntil(EDate* deadline) THROWS(ERuntimeException) {
	llong currTime = ESystem::currentTimeMillis();
	llong abstime = deadline->getTime();
	llong t = abstime - currTime;
	if (t < LLONG(0)) {
		throw ERuntimeException(__FILE__, __LINE__);
	}
	eso_thread_cond_timedwait(m_Cond, m_Mutex, (es_uint32_t)t);
	currTime = ESystem::currentTimeMillis();
	return (currTime - abstime) > LLONG(0) ? false : true;
}

void ESimpleCondition::signal() {
	eso_thread_cond_signal(m_Cond);
}

void ESimpleCondition::signalAll() {
	eso_thread_cond_broadcast(m_Cond);
}

} /* namespace efc */
