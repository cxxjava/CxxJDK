/*
 * EUnsafe.hh
 *
 *  Created on: 2014-10-9
 *      Author: cxxjava@163.com
 */

#ifndef EUNSAFE_HH_
#define EUNSAFE_HH_

#include "../EThread.hh"
#include "./EOrderAccess.hh"

namespace efc {

class EUnsafe {
public:
	/**
	 * Unblock the given thread blocked on <tt>park</tt>, or, if it is
	 * not blocked, cause the subsequent call to <tt>park</tt> not to
	 * block.  Note: this operation is "unsafe" solely because the
	 * caller must somehow ensure that the thread has not been
	 * destroyed. Nothing special is usually required to ensure this
	 * when called from Java (in which there will ordinarily be a live
	 * reference to the thread) but this is not nearly-automatically
	 * so when calling from native code.
	 * @param thread the thread to unpark.
	 *
	 */
	static void unpark(EThread* thread);

	/**
	 * Block current thread, returning when a balancing
	 * <tt>unpark</tt> occurs, or a balancing <tt>unpark</tt> has
	 * already occurred, or the thread is interrupted, or, if not
	 * absolute and time is not zero, the given time nanoseconds have
	 * elapsed, or if absolute, the given deadline in milliseconds
	 * since Epoch has passed, or spuriously (i.e., returning for no
	 * "reason"). Note: This operation is in the Unsafe class only
	 * because <tt>unpark</tt> is, so it would be strange to place it
	 * elsewhere.
	 */
	static void park(boolean isAbsolute, llong time);

	/**
	 * Atomically update Java variable to <tt>x</tt> if it is currently
	 * holding <tt>expected</tt>.
	 * @return <tt>true</tt> if successful
	 */
	static boolean compareAndSwapObject(volatile void* dest,
										void* expected,
										void* exchange);

	/**
	 * Atomically update Java variable to <tt>x</tt> if it is currently
	 * holding <tt>expected</tt>.
	 * @return <tt>true</tt> if successful
	 */
	static boolean compareAndSwapInt(volatile int* dest,
									 int expected,
									 int exchange);

	/**
	 * Atomically update Java variable to <tt>x</tt> if it is currently
	 * holding <tt>expected</tt>.
	 * @return <tt>true</tt> if successful
	 */
	static boolean compareAndSwapLLong(volatile llong* dest,
									   llong expected,
									   llong exchange);

	/**
	 * Fetches a reference value from a given Java variable, with volatile
	 * load semantics. Otherwise identical to {@link #getObject(Object, long)}
	 */
	static void* getObjectVolatile(volatile void* addr);

	/**
	 * Stores a reference value into a given Java variable, with
	 * volatile store semantics. Otherwise identical to {@link #putObject(Object, long, Object)}
	 */
	static void putObjectVolatile(volatile void* addr, void* x);

	template<typename T>
	static T getVolatile(volatile T* addr) {
		return EOrderAccess::load_acquire((volatile T*)addr);
	}

	template<typename T>
	static void putVolatile(volatile T* addr, T x) {
		EOrderAccess::release_store_fence((volatile T*)addr, x);
	}

	/**
	 * Version of {@link #putObjectVolatile(Object, long, Object)}
	 * that does not guarantee immediate visibility of the store to
	 * other threads. This method is generally only useful if the
	 * underlying field is a Java volatile (or if an array cell, one
	 * that is otherwise only accessed using volatile accesses).
	 */
	static void putOrderedObject(volatile void* addr, void* x);

	template<typename T>
	static void putOrdered(volatile T* addr, T x) {
		EOrderAccess::release_store_fence(addr, x);
	}

	/**
	 * Ensures lack of reordering of loads before the fence
	 * with loads or stores after the fence.
	 * @since 1.8
	 */
	static void loadFence();

	/**
	 * Ensures lack of reordering of stores before the fence
	 * with loads or stores after the fence.
	 * @since 1.8
	 */
	static void storeFence();

	/**
	 * Ensures lack of reordering of loads or stores before the fence
	 * with loads or stores after the fence.
	 * @since 1.8
	 */
	static void fullFence();
};

} /* namespace efc */
#endif /* EUNSAFE_HH_ */
