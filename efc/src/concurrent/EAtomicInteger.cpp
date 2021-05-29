/*
 * EAtomicInteger.cpp
 *
 *  Created on: 2013-11-5
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EAtomicInteger.hh"
#include "../../inc/EInteger.hh"
#include "../../inc/concurrent/EUnsafe.hh"

namespace efc {

EAtomicInteger::EAtomicInteger(int initialValue) {
	value = initialValue;
}

EAtomicInteger::EAtomicInteger() {
	value = 0;
}

int EAtomicInteger::get() {
	return value;
}

void EAtomicInteger::set(int newValue) {
	value = newValue;
}

void EAtomicInteger::lazySet(int newValue) {
	EUnsafe::putOrdered(&value, newValue);
}

int EAtomicInteger::getAndSet(int newValue) {
	return eso_atomic_test_and_set32(&value, newValue);

//	for (;;) {
//		int current = get();
//		if (compareAndSet(current, newValue))
//			return current;
//	}
}

boolean EAtomicInteger::compareAndSet(int expect, int update) {
	return eso_atomic_compare_and_swap32(&value, expect, update);

//	return EUnsafe::compareAndSwapInt(&value, expect, update);
}

boolean EAtomicInteger::weakCompareAndSet(int expect, int update) {
	return EUnsafe::compareAndSwapInt(&value, expect, update);
}

int EAtomicInteger::getAndIncrement() {
	return eso_atomic_fetch_and_add32(&value, 1);

//	for (;;) {
//		int current = get();
//		int next = current + 1;
//		if (compareAndSet(current, next))
//			return current;
//	}
}

int EAtomicInteger::getAndDecrement() {
	return eso_atomic_fetch_and_add32(&value, -1);

//	for (;;) {
//		int current = get();
//		int next = current - 1;
//		if (compareAndSet(current, next))
//			return current;
//	}
}

int EAtomicInteger::getAndAdd(int delta) {
	return eso_atomic_fetch_and_add32(&value, delta);

//	for (;;) {
//		int current = get();
//		int next = current + delta;
//		if (compareAndSet(current, next))
//			return current;
//	}
}

int EAtomicInteger::incrementAndGet() {
	return eso_atomic_add_and_fetch32(&value, 1);

//	for (;;) {
//		int current = get();
//		int next = current + 1;
//		if (compareAndSet(current, next))
//			return next;
//	}
}

int EAtomicInteger::decrementAndGet() {
	return eso_atomic_add_and_fetch32(&value, -1);

//	for (;;) {
//		int current = get();
//		int next = current - 1;
//		if (compareAndSet(current, next))
//			return next;
//	}
}

int EAtomicInteger::addAndGet(int delta) {
	return eso_atomic_add_and_fetch32(&value, delta);

//	for (;;) {
//		int current = get();
//		int next = current + delta;
//		if (compareAndSet(current, next))
//			return next;
//	}
}

EString EAtomicInteger::toString() {
	 return EInteger::toString(get()).c_str();
}

int EAtomicInteger::intValue() {
	return get();
}

llong EAtomicInteger::llongValue() {
	return (llong)get();
}

float EAtomicInteger::floatValue() {
	return (float)get();
}

double EAtomicInteger::doubleValue() {
	return (double)get();
}

} /* namespace efc */
