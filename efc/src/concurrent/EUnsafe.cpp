/*
 * EUnsafe.hh
 *
 *  Created on: 2014-10-9
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EUnsafe.hh"
#include "../../inc/concurrent/EAtomic.hh"
#include "../EPark.hh"
#include "../EThreadService.hh"

namespace efc {

#define USE_JDK_MODE 1
    
//@see: openjdk-7/hotspot/src/share/vm/prims/unsafe.cpp

void EUnsafe::unpark(EThread* thread) {
#if USE_JDK_MODE
	thread->parker->unpark();
#else
    thread->unpark();
#endif
}

void EUnsafe::park(boolean isAbsolute, llong time) {
	EThread* thread = EThread::currentThread();
	EThreadParkedState jtps(thread, time != 0);
#if USE_JDK_MODE
	thread->parker->park(isAbsolute != 0, time);
#else
    if (isAbsolute)
		thread->parkUntil(time);
	else
		thread->parkFor(time);
#endif
}

boolean EUnsafe::compareAndSwapObject(volatile void* o, void* expected, void* x) {
	return (EAtomic::cmpxchg_ptr(x, o, expected) == expected);
}

boolean EUnsafe::compareAndSwapInt(volatile int* o, int expected, int x) {
	return (EAtomic::cmpxchg32(x, o, expected) == expected);
}

boolean EUnsafe::compareAndSwapLLong(volatile llong* o, llong expected, llong x) {
	return (EAtomic::cmpxchg64(x, o, expected) == expected);
}

void* EUnsafe::getObjectVolatile(volatile void* addr) {
	volatile es_intptr_t v = *(volatile es_intptr_t*)addr;
	EOrderAccess::acquire();
	return (void*)v;
}

void EUnsafe::putObjectVolatile(volatile void* addr, void* x) {
	EOrderAccess::release();
	EAtomic::store_ptr(x, addr);
	EOrderAccess::fence();
}

void EUnsafe::putOrderedObject(volatile void* addr, void* x) {
	EOrderAccess::release();
	EAtomic::store_ptr(x, addr);
	EOrderAccess::fence();
}

void EUnsafe::loadFence() {
	EOrderAccess::acquire();
}

void EUnsafe::storeFence() {
	EOrderAccess::release();
}

void EUnsafe::fullFence() {
	EOrderAccess::fence();
}

} /* namespace efc */
