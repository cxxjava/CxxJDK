/*
 * EAtomicBoolean.hh
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#ifndef EATOMICBOOLEAN_HH_
#define EATOMICBOOLEAN_HH_

#include "../../EBase.hh"
#include "../EString.hh"

namespace efc {

/*
 * This file is available under and governed by the GNU General Public
 * License version 2 only, as published by the Free Software Foundation.
 * However, the following notice accompanied the original version of this
 * file:
 *
 * Written by Doug Lea with assistance from members of JCP JSR-166
 * Expert Group and released to the public domain, as explained at
 * http://creativecommons.org/licenses/publicdomain
 */

class EAtomicBoolean: public EObject {
public:
	/**
	 * Creates a new {@code AtomicBoolean} with the given initial value.
	 *
	 * @param initialValue the initial value
	 */
	EAtomicBoolean(boolean initialValue);

	/**
	 * Creates a new {@code AtomicBoolean} with initial value {@code false}.
	 */
	EAtomicBoolean();

	/**
	 * Returns the current value.
	 *
	 * @return the current value
	 */
	boolean get();

	/**
	 * Atomically sets the value to the given updated value
	 * if the current value {@code ==} the expected value.
	 *
	 * @param expect the expected value
	 * @param update the new value
	 * @return true if successful. False return indicates that
	 * the actual value was not equal to the expected value.
	 */
	boolean compareAndSet(boolean expect, boolean update);

	/**
	 * Atomically sets the value to the given updated value
	 * if the current value {@code ==} the expected value.
	 *
	 * <p>May <a href="package-summary.html#Spurious">fail spuriously</a>
	 * and does not provide ordering guarantees, so is only rarely an
	 * appropriate alternative to {@code compareAndSet}.
	 *
	 * @param expect the expected value
	 * @param update the new value
	 * @return true if successful.
	 */
	boolean weakCompareAndSet(boolean expect, boolean update);

	/**
	 * Unconditionally sets to the given value.
	 *
	 * @param newValue the new value
	 */
	void set(boolean newValue);

	/**
	 * Eventually sets to the given value.
	 *
	 * @param newValue the new value
	 * @since 1.6
	 */
	void lazySet(boolean newValue);

	/**
	 * Atomically sets to the given value and returns the previous value.
	 *
	 * @param newValue the new value
	 * @return the previous value
	 */
	boolean getAndSet(boolean newValue);

	/**
	 * Returns the String representation of the current value.
	 * @return the String representation of the current value.
	 */
	virtual EString toString();

private:
	volatile int value ES_ALIGN;
};

} /* namespace efc */
#endif /* EATOMICBOOLEAN_HH_ */
