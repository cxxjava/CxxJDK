/*
 * EAtomicDouble.hh
 *
 *  Created on: 2017-4-24
 *      Author: cxxjava@163.com
 */

#ifndef EATOMICDOUBLE_HH_
#define EATOMICDOUBLE_HH_

#include "../ENumber.hh"
#include "../EString.hh"

namespace efc {

/**
 * A {@code double} value that may be updated atomically. See the {@link
 * java.util.concurrent.atomic} package specification for description of the properties of atomic
 * variables. An {@code AtomicDouble} is used in applications such as atomic accumulation, and
 * cannot be used as a replacement for a {@link Double}. However, this class does extend {@code
 * Number} to allow uniform access by tools and utilities that deal with numerically-based classes.
 *
 * <p><a name="bitEquals"></a>This class compares primitive {@code double} values in methods such as
 * {@link #compareAndSet} by comparing their bitwise representation using {@link
 * Double#doubleToRawLongBits}, which differs from both the primitive double {@code ==} operator and
 * from {@link Double#equals}, as if implemented by:
 *
 * <pre>{@code
 * static boolean bitEquals(double x, double y) {
 *   long xBits = Double.doubleToRawLongBits(x);
 *   long yBits = Double.doubleToRawLongBits(y);
 *   return xBits == yBits;
 * }
 * }</pre>
 *
 * <p>It is possible to write a more scalable updater, at the cost of giving up strict atomicity.
 * See for example <a
 * href="http://gee.cs.oswego.edu/dl/jsr166/dist/jsr166edocs/jsr166e/DoubleAdder.html">
 * DoubleAdder</a> and <a
 * href="http://gee.cs.oswego.edu/dl/jsr166/dist/jsr166edocs/jsr166e/DoubleMaxUpdater.html">
 * DoubleMaxUpdater</a>.
 *
 * @since 11.0
 */

class EAtomicDouble: public ENumber {
public:
	/**
	 * Creates a new {@code AtomicDouble} with the given initial value.
	 *
	 * @param initialValue the initial value
	 */
	EAtomicDouble(double initialValue);

	/**
	 * Creates a new {@code AtomicDouble} with initial value {@code 0.0}.
	 */
	EAtomicDouble();

	/**
	 * Gets the current value.
	 *
	 * @return the current value
	 */
	double get();

	/**
	 * Sets to the given value.
	 *
	 * @param newValue the new value
	 */
	void set(double newValue);

	/**
	 * Eventually sets to the given value.
	 *
	 * @param newValue the new value
	 */
	void lazySet(double newValue);

	/**
	 * Atomically sets to the given value and returns the old value.
	 *
	 * @param newValue the new value
	 * @return the previous value
	 */
	double getAndSet(double newValue);

	/**
	 * Atomically sets the value to the given updated value
	 * if the current value is <a href="#bitEquals">bitwise equal</a>
	 * to the expected value.
	 *
	 * @param expect the expected value
	 * @param update the new value
	 * @return {@code true} if successful. False return indicates that
	 * the actual value was not bitwise equal to the expected value.
	 */
	boolean compareAndSet(double expect, double update);

	/**
	 * Atomically sets the value to the given updated value
	 * if the current value is <a href="#bitEquals">bitwise equal</a>
	 * to the expected value.
	 *
	 * <p>May <a
	 * href="http://download.oracle.com/javase/7/docs/api/java/util/concurrent/atomic/package-summary.html#Spurious">
	 * fail spuriously</a>
	 * and does not provide ordering guarantees, so is only rarely an
	 * appropriate alternative to {@code compareAndSet}.
	 *
	 * @param expect the expected value
	 * @param update the new value
	 * @return {@code true} if successful
	 */
	boolean weakCompareAndSet(double expect, double update);

	/**
	 * Atomically adds the given value to the current value.
	 *
	 * @param delta the value to add
	 * @return the previous value
	 */
	double getAndAdd(double delta);

	/**
	 * Atomically adds the given value to the current value.
	 *
	 * @param delta the value to add
	 * @return the updated value
	 */
	double addAndGet(double delta);

	/**
	 * Returns the String representation of the current value.
	 * @return the String representation of the current value
	 */
	virtual EString toString();

	virtual int intValue();
	virtual llong llongValue();
	virtual float floatValue();
	virtual double doubleValue();

private:
	volatile llong value ES_ALIGN;
};

} /* namespace efc */
#endif /* EATOMICDOUBLE_HH_ */
