/*
 * EAtomicInteger.hh
 *
 *  Created on: 2013-11-5
 *      Author: cxxjava@163.com
 */

#ifndef EATOMIC_INTEGER_HH_
#define EATOMIC_INTEGER_HH_

#include "ENumber.hh"
#include "EString.hh"

namespace efc {

/**
 * An {@code int} value that may be updated atomically.  See the
 * {@link java.util.concurrent.atomic} package specification for
 * description of the properties of atomic variables. An
 * {@code AtomicInteger} is used in applications such as atomically
 * incremented counters, and cannot be used as a replacement for an
 * {@link java.lang.Integer}. However, this class does extend
 * {@code Number} to allow uniform access by tools and utilities that
 * deal with numerically-based classes.
 *
 * @since 1.5
 */

class EAtomicInteger: public ENumber {
public:
	/**
	 * Creates a new AtomicInteger with the given initial value.
	 *
	 * @param initialValue the initial value
	 */
	EAtomicInteger(int initialValue);

	/**
	 * Creates a new AtomicInteger with initial value {@code 0}.
	 */
	EAtomicInteger();

	/**
	 * Gets the current value.
	 *
	 * @return the current value
	 */
	int get();

	/**
	 * Sets to the given value.
	 *
	 * @param newValue the new value
	 */
	void set(int newValue);

	/**
	 * Eventually sets to the given value.
	 *
	 * @param newValue the new value
	 * @since 1.6
	 */
	void lazySet(int newValue);

	/**
	 * Atomically sets to the given value and returns the old value.
	 *
	 * @param newValue the new value
	 * @return the previous value
	 */
	int getAndSet(int newValue);

	/**
	 * Atomically sets the value to the given updated value
	 * if the current value {@code ==} the expected value.
	 *
	 * @param expect the expected value
	 * @param update the new value
	 * @return true if successful. False return indicates that
	 * the actual value was not equal to the expected value.
	 */
	boolean compareAndSet(int expect, int update);

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
	boolean weakCompareAndSet(int expect, int update);

	/**
	 * Atomically increments by one the current value.
	 *
	 * @return the previous value
	 */
	int getAndIncrement();

	/**
	 * Atomically decrements by one the current value.
	 *
	 * @return the previous value
	 */
	int getAndDecrement();

	/**
	 * Atomically adds the given value to the current value.
	 *
	 * @param delta the value to add
	 * @return the previous value
	 */
	int getAndAdd(int delta);

	/**
	 * Atomically increments by one the current value.
	 *
	 * @return the updated value
	 */
	int incrementAndGet();

	/**
	 * Atomically decrements by one the current value.
	 *
	 * @return the updated value
	 */
	int decrementAndGet();

	/**
	 * Atomically adds the given value to the current value.
	 *
	 * @param delta the value to add
	 * @return the updated value
	 */
	int addAndGet(int delta);

	/**
	 * Returns the String representation of the current value.
	 * @return the String representation of the current value.
	 */
	virtual EStringBase toString();

	virtual int intValue();
	virtual llong llongValue();
	virtual float floatValue();
	virtual double doubleValue();

private:
	volatile int value ES_ALIGN;
};

} /* namespace efc */
#endif /* EATOMIC_INTEGER_HH_ */
