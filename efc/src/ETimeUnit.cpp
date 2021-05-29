/*
 * ETimeUnit.cpp
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#include "ETimeUnit.hh"
#include "EOS.hh"
#include "EThread.hh"
#include "ESynchronizeable.hh"

namespace efc {

_NANOSECONDS* ETimeUnit::NANOSECONDS;// = new _NANOSECONDS();
_MICROSECONDS* ETimeUnit::MICROSECONDS;// = new _MICROSECONDS();
_MILLISECONDS* ETimeUnit::MILLISECONDS;// = new _MILLISECONDS();
_SECONDS* ETimeUnit::SECONDS;// = new _SECONDS();
_MINUTES* ETimeUnit::MINUTES;// = new _MINUTES();
_HOURS* ETimeUnit::HOURS;// = new _HOURS();
_DAYS* ETimeUnit::DAYS;// = new _DAYS();

//const llong ETimeUnit::C0 = 1L;
//const llong ETimeUnit::C1 = C0 * 1000L;
//const llong ETimeUnit::C2 = C1 * 1000L;
//const llong ETimeUnit::C3 = C2 * 1000L;
//const llong ETimeUnit::C4 = C3 * 60L;
//const llong ETimeUnit::C5 = C4 * 60L;
//const llong ETimeUnit::C6 = C5 * 24L;
//
//const llong ETimeUnit::MAX = ELLong::MAX_VALUE;

DEFINE_STATIC_INITZZ_BEGIN(ETimeUnit)
EOS::_initzz_();
NANOSECONDS = new _NANOSECONDS();
MICROSECONDS = new _MICROSECONDS();
MILLISECONDS = new _MILLISECONDS();
SECONDS = new _SECONDS();
MINUTES = new _MINUTES();
HOURS = new _HOURS();
DAYS = new _DAYS();
DEFINE_STATIC_INITZZ_END

void ETimeUnit::timedWait(ESynchronizeable* obj, llong timeout) {
	if (timeout > 0) {
		llong ms = toMillis(timeout);
		int ns = excessNanos(timeout, ms);
		obj->wait(ms, ns);
	}
}

void ETimeUnit::timedJoin(EThread* thread, llong timeout) {
	if (timeout > 0) {
		llong ms = toMillis(timeout);
		int ns = excessNanos(timeout, ms);
		thread->join(ms, ns);
	}
}

void ETimeUnit::sleep(llong timeout) {
	if (timeout > 0) {
		llong ms = toMillis(timeout);
		int ns = excessNanos(timeout, ms);
		EThread::sleep(ms, ns);
	}
}

} /* namespace efc */
