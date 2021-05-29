/*
 * EAtomicBoolean.cpp
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EAtomicBoolean.hh"
#include "../../inc/EBoolean.hh"
#include "../../inc/concurrent/EUnsafe.hh"

namespace efc {

EAtomicBoolean::EAtomicBoolean(boolean initialValue) {
	value = initialValue ? 1 : 0;
}

EAtomicBoolean::EAtomicBoolean() : value(0) {
}

boolean EAtomicBoolean::get() {
	return value != 0;
}

boolean EAtomicBoolean::compareAndSet(boolean expect, boolean update) {
	int e = expect ? 1 : 0;
	int u = update ? 1 : 0;
//	return EUnsafe::compareAndSwapInt(&value, e, u);
	return eso_atomic_compare_and_swap32(&value, e, u);
}

boolean EAtomicBoolean::weakCompareAndSet(boolean expect, boolean update) {
	int e = expect ? 1 : 0;
	int u = update ? 1 : 0;
//	return EUnsafe::compareAndSwapInt(&value, e, u);
	return eso_atomic_compare_and_swap32(&value, e, u);
}

void EAtomicBoolean::set(boolean newValue) {
	value = newValue ? 1 : 0;
}

void EAtomicBoolean::lazySet(boolean newValue) {
	int v = newValue ? 1 : 0;
	EUnsafe::putOrdered(&value, v);
}

boolean EAtomicBoolean::getAndSet(boolean newValue) {
	int n = newValue ? 1 : 0;
	return eso_atomic_test_and_set32(&value, n);

//	for (;;) {
//		boolean current = get();
//		if (compareAndSet(current, newValue))
//			return current;
//	}
}

EString EAtomicBoolean::toString() {
	return EBoolean::toString(get()).c_str();
}

} /* namespace efc */
