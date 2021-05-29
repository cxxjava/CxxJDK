/*
 * ETimerTask.cpp
 *
 *  Created on: 2015-2-16
 *      Author: cxxjava@163.com
 */

#include "ETimerTask.hh"
#include "ESynchronizeable.hh"

namespace efc {

ETimerTask::~ETimerTask() {

}

boolean ETimerTask::cancel() {
	SYNCBLOCK(&lock) {
		boolean result = (state == SCHEDULED);
		state = CANCELLED;
		return result;
    }}
}

llong ETimerTask::scheduledExecutionTime() {
	SYNCBLOCK(&lock) {
		return (period < 0 ? nextExecutionTime + period
		                   : nextExecutionTime - period);
    }}
}

ETimerTask::ETimerTask() : state(VIRGIN), nextExecutionTime(0), period(0) {
}

} /* namespace efc */
