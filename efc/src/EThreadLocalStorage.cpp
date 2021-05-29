/*
 * EThreadLocalStorage.cpp
 *
 *  Created on: 2014-1-27
 *      Author: cxxjava@163.com
 */

#include "EThreadLocalStorage.hh"
#include "EException.hh"

namespace efc {

#ifdef WIN32

EThreadLocalStorage::EThreadLocalStorage() {
	thread_key = TlsAlloc();
}

EThreadLocalStorage::~EThreadLocalStorage() {
	TlsFree(thread_key);
}

void* EThreadLocalStorage::get() {
	return TlsGetValue(thread_key);
}

void EThreadLocalStorage::set(void* value) {
	TlsSetValue(thread_key, value);
}

#elif defined(__solaris__)

EThreadLocalStorage::EThreadLocalStorage() {
	thread_key_t key;
	if (thr_keycreate(&key, NULL)) {
		throw EException(__FILE__, __LINE__, "thr_keycreate error");
	}
	thread_key = (long)key;
}

EThreadLocalStorage::~EThreadLocalStorage() {
	/* no-op */
}

void* EThreadLocalStorage::get() {
	void* r = NULL;
	thr_getspecific((thread_key_t)thread_key, &r);
	return r;
}

void EThreadLocalStorage::set(void* value) {
	thr_setspecific((thread_key_t)thread_key, value) ;
}

#else //linux||bsd

EThreadLocalStorage::EThreadLocalStorage() {
	pthread_key_t key;
	if (pthread_key_create(&key, NULL)) {
		throw EException(__FILE__, __LINE__, "pthread_key_create error");
	}
	thread_key = (long)key;
}

EThreadLocalStorage::~EThreadLocalStorage() {
	pthread_key_delete((pthread_key_t)thread_key);
}

void* EThreadLocalStorage::get() {
	return pthread_getspecific((pthread_key_t)thread_key);
}

void EThreadLocalStorage::set(void* value) {
	pthread_setspecific((pthread_key_t)thread_key, value) ;
}

#endif //!WIN32

} /* namespace efc */
