/*
 * EAtomicDouble.cpp
 *
 *  Created on: 2017-4-24
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EAtomicDouble.hh"
#include "../../inc/EDouble.hh"

namespace efc {

#define doubleToRawLongBits eso_double2llongBits
#define longBitsToDouble    eso_llongBits2double

EAtomicDouble::EAtomicDouble(double initialValue) {
	value = doubleToRawLongBits(initialValue);
}

EAtomicDouble::EAtomicDouble(): value(0L) {
}

double EAtomicDouble::get() {
	return longBitsToDouble(value);
}

void EAtomicDouble::set(double newValue) {
	llong next = doubleToRawLongBits(newValue);
	value = next;
}

void EAtomicDouble::lazySet(double newValue) {
	set(newValue);
}

double EAtomicDouble::getAndSet(double newValue) {
	llong next = doubleToRawLongBits(newValue);
	return longBitsToDouble(eso_atomic_test_and_set64(&value, next));
}

boolean EAtomicDouble::compareAndSet(double expect, double update) {
	return eso_atomic_compare_and_swap64(&value,
								 doubleToRawLongBits(expect),
								 doubleToRawLongBits(update));
}

boolean EAtomicDouble::weakCompareAndSet(double expect, double update) {
	return compareAndSet(expect, update);
}

double EAtomicDouble::getAndAdd(double delta) {
	while (true) {
		llong current = value;
		double currentVal = longBitsToDouble(current);
		double nextVal = currentVal + delta;
		llong next = doubleToRawLongBits(nextVal);
		if (compareAndSet(current, next)) {
			return currentVal;
		}
	}
}

double EAtomicDouble::addAndGet(double delta) {
	while (true) {
		llong current = value;
		double currentVal = longBitsToDouble(current);
		double nextVal = currentVal + delta;
		llong next = doubleToRawLongBits(nextVal);
		if (compareAndSet(current, next)) {
			return nextVal;
		}
	}
}

EString EAtomicDouble::toString() {
	return EDouble::toString(get());
}

int EAtomicDouble::intValue() {
	return (int) get();
}

llong EAtomicDouble::llongValue() {
	return (llong) get();
}

float EAtomicDouble::floatValue() {
	return (float) get();
}

double EAtomicDouble::doubleValue() {
	return get();
}

} /* namespace efc */
