/*
 * EThreadService.hh
 *
 *  Created on: 2014-11-6
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADSERVICE_HH_
#define ETHREADSERVICE_HH_

namespace efc {

//@see: openjdk-6/hotspot/src/share/vm/services/threadService.hpp

// abstract utility class to set new thread states, and restore previous after the block exits
class EThreadStatusChanger {
private:
	EThread::State _old_state;
	EThread* _thread;

	void save_old_state(EThread* thread) {
		_thread = thread;
		_old_state = _thread->getState();
	}

public:
	static void set_thread_status(EThread* thread, EThread::State state) {
		if (thread->isAlive()) {
			thread->setState(state);
		}
	}

	void set_thread_status(EThread::State state) {
		set_thread_status(_thread, state);
	}

	EThreadStatusChanger(EThread* thread, EThread::State state) {
		save_old_state(thread);
		set_thread_status(state);
	}

	EThreadStatusChanger(EThread* thread) {
		save_old_state(thread);
	}

	~EThreadStatusChanger() {
		set_thread_status(_old_state);
	}
};

// Change status to waiting on an object  (timed or indefinite)
class EThreadInObjectWaitState: public EThreadStatusChanger {
public:
	EThreadInObjectWaitState(EThread *thread, boolean timed) :
			EThreadStatusChanger(thread,
					timed ? EThread::TIMED_WAITING : EThread::WAITING) {
	}

	~EThreadInObjectWaitState() {
		//
	}
};

// Change status to parked (timed or indefinite)
class EThreadParkedState: public EThreadStatusChanger {
public:
	EThreadParkedState(EThread *thread, boolean timed) :
			EThreadStatusChanger(thread,
					timed ? EThread::TIMED_WAITING : EThread::WAITING) {
	}

	~EThreadParkedState() {
		//
	}
};

// Change status to blocked on (re-)entering a synchronization block
//class JavaThreadBlockedOnMonitorEnterState ...

// Change status to sleeping
class EThreadSleepState: public EThreadStatusChanger {
public:
	EThreadSleepState(EThread *thread) :
			EThreadStatusChanger(thread, EThread::TIMED_WAITING) {
	}

	~EThreadSleepState() {
		//
	}
};

} /* namespace efc */
#endif /* ETHREADSERVICE_HH_ */
