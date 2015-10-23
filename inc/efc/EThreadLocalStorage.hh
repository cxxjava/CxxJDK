/*
 * EThreadLocalStorage.hh
 *
 *  Created on: 2014-1-27
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADLOCALSTORAGE_HH_
#define ETHREADLOCALSTORAGE_HH_

#include "EBase.hh"

namespace efc {

class EThreadLocalStorage {
public:
	virtual ~EThreadLocalStorage();

	EThreadLocalStorage();

	/**
	 * Returns the value in the current thread's copy of this
	 * thread-local variable.  If the variable has no value for the
	 * current thread, it is first initialized to the value returned
	 * by an invocation of the {@link #initialValue} method.
	 *
	 * @return the current thread's value of this thread-local
	 */
	void* get();

	/**
	 * Sets the current thread's copy of this thread-local variable
	 * to the specified value.  Most subclasses will have no need to
	 * override this method, relying solely on the {@link #initialValue}
	 * method to set the values of thread-locals.
	 *
	 * @param value the value to be stored in the current thread's copy of
	 *        this thread-local.
	 */
	void set(void* value);

private:
	int thread_key;
};

} /* namespace efc */
#endif /* ETHREADLOCALSTORAGE_HH_ */
