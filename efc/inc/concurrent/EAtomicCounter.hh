/*
 * EAtomicCounter.hh
 *
 *  Created on: 2014-04-08
 *      Author: cxxjava@163.com
 */

// @see: poco/1.4/Foundation/include/Poco/AtomicCounter.h

#ifndef EATOMIC_COUNTER_HH_
#define EATOMIC_COUNTER_HH_

#include "../../EBase.hh"

namespace efc {

/**
 * This class implements a simple counter, which
 * provides atomic operations that are safe to
 * use in a multithreaded environment.
 *
 * Typical usage of AtomicCounter is for implementing
 * reference counting and similar things.
 *
 * On some platforms, the implementation of AtomicCounter
 * is based on atomic primitives specific to the platform
 * (such as InterlockedIncrement, etc. on Windows), and
 * thus very efficient. On platforms that do not support
 * atomic primitives, operations are guarded by a FastMutex.
 *
 * The following platforms currently have atomic
 * primitives:
 *   - Windows
 *   - Mac OS X
 *   - GCC 4.1+ (Intel platforms only)
 */

class EAtomicCounter
{
public:
	/**
	 * Destroys the EAtomicCounter.
	 */
	~EAtomicCounter();

	/**
	 * Creates a new EAtomicCounter and initializes it to zero.
	 */
	EAtomicCounter();

	/**
	 * Creates a new EAtomicCounter and initializes it with
	 * the given value.
	 */
	explicit EAtomicCounter(int initialValue);
	
	/**
	 * Creates the counter by copying another one.
	 */
	EAtomicCounter(const EAtomicCounter& counter);
	
	/**
	 * Assigns the value of another EAtomicCounter.
	 */
	EAtomicCounter& operator = (const EAtomicCounter& counter);

	/**
	 * Assigns a value to the counter.
	 */
	EAtomicCounter& operator = (int value);

	/**
	 * Returns the value of the counter.
	 */
	operator int () const;

	/**
	 * Returns the value of the counter.
	 */
	int value() const;

	/**
	 * Increments the counter and returns the result.
	 */
	int operator ++ (); // prefix

	/**
	 * Increments the counter and returns the previous value.
	 */
	int operator ++ (int); // postfix

	/**
	 * Decrements the counter and returns the result.
	 */
	int operator -- (); // prefix

	/**
	 * Decrements the counter and returns the previous value.
	 */
	int operator -- (int); // postfix

	/**
	 * Returns true if the counter is zero, false otherwise.
	 */
	bool operator ! () const;

private:
	volatile int _counter ES_ALIGN;
};

//=============================================================================

inline EAtomicCounter::operator int () const
{
	return _counter;
}

inline int EAtomicCounter::value() const
{
	return _counter;
}

inline int EAtomicCounter::operator ++ () // prefix
{
	return eso_atomic_add_and_fetch32(&_counter, 1);
}

inline int EAtomicCounter::operator ++ (int) // postfix
{
	return eso_atomic_fetch_and_add32(&_counter, 1);
}

inline int EAtomicCounter::operator -- () // prefix
{
	return eso_atomic_sub_and_fetch32(&_counter, 1);
}

inline int EAtomicCounter::operator -- (int) // postfix
{
	return eso_atomic_fetch_and_sub32(&_counter, 1);
}

inline bool EAtomicCounter::operator ! () const
{
	return _counter == 0;
}

} // namespace efc
#endif // !EATOMIC_COUNTER_HH_
