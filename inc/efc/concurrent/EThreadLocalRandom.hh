/*
 * EThreadLocalRandom.hh
 *
 *  Created on: 2015-7-10
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADLOCALRANDOM_HH_
#define ETHREADLOCALRANDOM_HH_

#include "ERandom.hh"
#include "EThreadLocal.hh"

namespace efc {


/**
 * A random number generator isolated to the current thread.  Like the
 * global {@link java.util.Random} generator used by the {@link
 * java.lang.Math} class, a {@code ThreadLocalRandom} is initialized
 * with an internally generated seed that may not otherwise be
 * modified. When applicable, use of {@code ThreadLocalRandom} rather
 * than shared {@code Random} objects in concurrent programs will
 * typically encounter much less overhead and contention.  Use of
 * {@code ThreadLocalRandom} is particularly appropriate when multiple
 * tasks (for example, each a {@link ForkJoinTask}) use random numbers
 * in parallel in thread pools.
 *
 * <p>Usages of this class should typically be of the form:
 * {@code ThreadLocalRandom.current().nextX(...)} (where
 * {@code X} is {@code Int}, {@code Long}, etc).
 * When all usages are of this form, it is never possible to
 * accidently share a {@code ThreadLocalRandom} across multiple threads.
 *
 * <p>This class also provides additional commonly used bounded random
 * generation methods.
 *
 * @since 1.7
 * @author Doug Lea
 */

class EThreadLocalRandom : public ERandom {
public:
	DECLARE_STATIC_INITZZ;

public:
	/**
	 * Constructor called only by localRandom.initialValue.
	 */
	EThreadLocalRandom();

	/**
	 * Throws {@code UnsupportedOperationException}.  Setting seeds in
	 * this generator is not supported.
	 *
	 * @throws UnsupportedOperationException always
	 */
	void setSeed(llong seed);

	/**
	 * Returns a pseudorandom, uniformly distributed value between the
	 * given least value (inclusive) and bound (exclusive).
	 *
	 * @param least the least value returned
	 * @param bound the upper bound (exclusive)
	 * @throws IllegalArgumentException if least greater than or equal
	 * to bound
	 * @return the next value
	 */
	int nextInt(int least, int bound);

	/**
	 * {@inherit from super for c++ hides overloaded virtual function}
	 */
	using ERandom::nextInt;

	/**
	 * Returns a pseudorandom, uniformly distributed value
	 * between 0 (inclusive) and the specified value (exclusive).
	 *
	 * @param n the bound on the random number to be returned.  Must be
	 *        positive.
	 * @return the next value
	 * @throws IllegalArgumentException if n is not positive
	 */
	llong nextLong(llong n);

	/**
	 * Returns a pseudorandom, uniformly distributed value between the
	 * given least value (inclusive) and bound (exclusive).
	 *
	 * @param least the least value returned
	 * @param bound the upper bound (exclusive)
	 * @return the next value
	 * @throws IllegalArgumentException if least greater than or equal
	 * to bound
	 */
	llong nextLong(llong least, llong bound);

	/**
	 * Returns a pseudorandom, uniformly distributed {@code double} value
	 * between 0 (inclusive) and the specified value (exclusive).
	 *
	 * @param n the bound on the random number to be returned.  Must be
	 *        positive.
	 * @return the next value
	 * @throws IllegalArgumentException if n is not positive
	 */
	double nextDouble(double n);

	/**
	 * Returns a pseudorandom, uniformly distributed value between the
	 * given least value (inclusive) and bound (exclusive).
	 *
	 * @param least the least value returned
	 * @param bound the upper bound (exclusive)
	 * @return the next value
	 * @throws IllegalArgumentException if least greater than or equal
	 * to bound
	 */
	double nextDouble(double least, double bound);

	/**
	 * {@inherit from super for c++ hides overloaded virtual function}
	 */
	using ERandom::nextDouble;

	/**
	 * Returns the current thread's {@code ThreadLocalRandom}.
	 *
	 * @return the current thread's {@code ThreadLocalRandom}
	 */
	static EThreadLocalRandom* current();

protected:
	int next(int bits);

private:
	// same constants as Random, but must be redeclared because private
	static const llong multiplier = 0x5DEECE66DL;
	static const llong addend = 0xBL;
	static const llong mask = 281474976710655L; //(1L << 48) - 1;

	/**
	 * The random seed. We can't use super.seed.
	 */
	llong rnd;

	/**
	 * Initialization flag to permit calls to setSeed to succeed only
	 * while executing the Random constructor.  We can't allow others
	 * since it would cause setting seed in one part of a program to
	 * unintentionally impact other usages by the thread.
	 */
	boolean initialized;

	// Padding to help avoid memory contention among seed updates in
	// different TLRs in the common case that they are located near
	// each other.
	long pad0, pad1, pad2, pad3, pad4, pad5, pad6, pad7;

	/**
	 * The actual ThreadLocal
	 */
	static EThreadLocal<EThreadLocalRandom*>* localRandom;
};

} /* namespace efc */
#endif /* ETHREADLOCALRANDOM_HH_ */
