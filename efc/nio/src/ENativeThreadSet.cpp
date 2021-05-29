/*
 * ENativeThreadSet.cpp
 *
 *  Created on: 2014-2-15
 *      Author: cxxjava@163.com
 */

#include "../inc/ENativeThreadSet.hh"
#include "./ENativeThread.hh"

namespace efc {
namespace nio {

ENativeThreadSet::~ENativeThreadSet() {
	delete elts;
}

ENativeThreadSet::ENativeThreadSet(int n) {
	elts = new EArrayList<llong>(n);
}

int ENativeThreadSet::add() {
	llong th = (llong)ENativeThread::current();
	if (th == -1)
		return -1;
	SYNCHRONIZED (this) {
		int length = elts->size();
		for (int i = 0; i < length; i++) {
			if (elts->getAt(i) == 0) {
				elts->setAt(i, th);
				return i;
			}
		}
		elts->add(th);
		return length;
    }}
}

void ENativeThreadSet::remove(int i) {
	if (i < 0)
		return;
	SYNCHRONIZED (this) {
		elts->setAt(i, 0);
    }}
}

void ENativeThreadSet::signal() {
	SYNCHRONIZED (this) {
		int n = elts->size();
		for (int i = 0; i < n; i++) {
			llong th = elts->getAt(i);
			if (th == 0)
				continue;
			ENativeThread::signal((es_os_thread_t)th);
		}
    }}
}

} /* namespace nio */
} /* namespace efc */
