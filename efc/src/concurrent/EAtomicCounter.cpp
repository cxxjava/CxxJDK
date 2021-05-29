/*
 * EAtomicCounter.cpp
 *
 *  Created on: 2014-04-08
 *      Author: cxxjava@163.com
 */

// @see: poco/1.4/Foundation/src/AtomicCounter.cpp

#include "../../inc/concurrent/EAtomicCounter.hh"

namespace efc {

EAtomicCounter::EAtomicCounter():
	_counter(0)
{
}
	
EAtomicCounter::EAtomicCounter(int initialValue):
	_counter(initialValue)
{
}

EAtomicCounter::EAtomicCounter(const EAtomicCounter& counter):
	_counter(counter.value())
{
}

EAtomicCounter::~EAtomicCounter()
{
}

EAtomicCounter& EAtomicCounter::operator = (const EAtomicCounter& counter)
{
	eso_atomic_test_and_set32(&_counter, counter.value());
	return *this;
}
	
EAtomicCounter& EAtomicCounter::operator = (int value)
{
	eso_atomic_test_and_set32(&_counter, value);
	return *this;
}

} // namespace efc
