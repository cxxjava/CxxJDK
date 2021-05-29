/*
 * EAtomicLLong.cpp
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EAtomicLLong.hh"
#include "../../inc/ELLong.hh"
#include "../../inc/concurrent/EUnsafe.hh"

namespace efc {

EAtomicLLong::EAtomicLLong(llong initialValue) {
	value = initialValue;
}

EAtomicLLong::EAtomicLLong() {
	value = 0;
}

llong EAtomicLLong::get() {
	return value;
}

void EAtomicLLong::set(llong newValue) {
	value = newValue;
}

void EAtomicLLong::lazySet(llong newValue) {
	EUnsafe::putOrdered(&value, newValue);
}

llong EAtomicLLong::getAndSet(llong newValue) {
	return eso_atomic_test_and_set64(&value, newValue);

//	while (true) {
//		llong current = get();
//		if (compareAndSet(current, newValue))
//			return current;
//	}
}

boolean EAtomicLLong::compareAndSet(llong expect, llong update) {
	return eso_atomic_compare_and_swap64(&value, expect, update);

//	return EUnsafe::compareAndSwapLLong(&value, expect, update);
}

boolean EAtomicLLong::weakCompareAndSet(llong expect, llong update) {
	return eso_atomic_compare_and_swap64(&value, expect, update);

//	return EUnsafe::compareAndSwapLLong(&value, expect, update);
}

llong EAtomicLLong::getAndIncrement() {
	return eso_atomic_fetch_and_add64(&value, 1);

//	while (true) {
//		llong current = get();
//		llong next = current + 1;
//		if (compareAndSet(current, next))
//			return current;
//	}
}

llong EAtomicLLong::getAndDecrement() {
	return eso_atomic_fetch_and_add64(&value, -1);

//	while (true) {
//		llong current = get();
//		llong next = current - 1;
//		if (compareAndSet(current, next))
//			return current;
//	}
}

llong EAtomicLLong::getAndAdd(llong delta) {
	return eso_atomic_fetch_and_add64(&value, delta);

//	while (true) {
//		llong current = get();
//		llong next = current + delta;
//		if (compareAndSet(current, next))
//			return current;
//	}
}

llong EAtomicLLong::incrementAndGet() {
	return eso_atomic_add_and_fetch64(&value, 1);

//	for (;;) {
//		llong current = get();
//		llong next = current + 1;
//		if (compareAndSet(current, next))
//			return next;
//	}
}

llong EAtomicLLong::decrementAndGet() {
	return eso_atomic_add_and_fetch64(&value, -1);

//	for (;;) {
//		llong current = get();
//		llong next = current - 1;
//		if (compareAndSet(current, next))
//			return next;
//	}
}

llong EAtomicLLong::addAndGet(llong delta) {
	return eso_atomic_add_and_fetch64(&value, delta);

//	for (;;) {
//		llong current = get();
//		llong next = current + delta;
//		if (compareAndSet(current, next))
//			return next;
//	}
}

EString EAtomicLLong::toString() {
	 return ELLong::toString(get()).c_str();
}

int EAtomicLLong::intValue() {
	return (int)get();
}

llong EAtomicLLong::llongValue() {
	return get();
}

float EAtomicLLong::floatValue() {
	return (float)get();
}

double EAtomicLLong::doubleValue() {
	return (double)get();
}

} /* namespace efc */
