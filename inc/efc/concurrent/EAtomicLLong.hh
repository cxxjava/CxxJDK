/*
 * EAtomicLLong.hh
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#ifndef EATOMICLLONG_HH_
#define EATOMICLLONG_HH_

#include "EBase.hh"
#include "EString.hh"

namespace efc {

/**
 * A {@code long} value that may be updated atomically.  See the
 * {@link java.util.concurrent.atomic} package specification for
 * description of the properties of atomic variables. An
 * {@code AtomicLong} is used in applications such as atomically
 * incremented sequence numbers, and cannot be used as a replacement
 * for a {@link java.lang.Long}. However, this class does extend
 * {@code Number} to allow uniform access by tools and utilities that
 * deal with numerically-based classes.
 *
 * @since 1.5
 */

class EAtomicLLong: public EObject {
public:
	/**
     * Creates a new AtomicLong with the given initial value.
     *
     * @param initialValue the initial value
     */
	EAtomicLLong(llong initialValue);

    /**
     * Creates a new AtomicLong with initial value {@code 0}.
     */
    EAtomicLLong();

    /**
     * Gets the current value.
     *
     * @return the current value
     */
    llong get();

    /**
     * Sets to the given value.
     *
     * @param newValue the new value
     */
    void set(llong newValue);

    /**
	 * Eventually sets to the given value.
	 *
	 * @param newValue the new value
	 * @since 1.6
	 */
	void lazySet(llong newValue);

    /**
     * Atomically sets to the given value and returns the old value.
     *
     * @param newValue the new value
     * @return the previous value
     */
    llong getAndSet(llong newValue);

    /**
     * Atomically sets the value to the given updated value
     * if the current value {@code ==} the expected value.
     *
     * @param expect the expected value
     * @param update the new value
     * @return true if successful. False return indicates that
     * the actual value was not equal to the expected value.
     */
    boolean compareAndSet(llong expect, llong update);

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
    boolean weakCompareAndSet(llong expect, llong update);

    /**
     * Atomically increments by one the current value.
     *
     * @return the previous value
     */
    llong getAndIncrement();

    /**
     * Atomically decrements by one the current value.
     *
     * @return the previous value
     */
    llong getAndDecrement();

    /**
     * Atomically adds the given value to the current value.
     *
     * @param delta the value to add
     * @return the previous value
     */
    llong getAndAdd(llong delta);

    /**
     * Atomically increments by one the current value.
     *
     * @return the updated value
     */
    llong incrementAndGet();

    /**
     * Atomically decrements by one the current value.
     *
     * @return the updated value
     */
    llong decrementAndGet();

    /**
     * Atomically adds the given value to the current value.
     *
     * @param delta the value to add
     * @return the updated value
     */
    llong addAndGet(llong delta);

    /**
     * Returns the String representation of the current value.
     * @return the String representation of the current value.
     */
    EStringBase toString();


    int intValue();

    llong longValue();

    float floatValue();

    double doubleValue();

private:
    volatile llong value ES_ALIGN;
};

} /* namespace efc */
#endif /* EATOMICLLONG_HH_ */
