/*
 * EAtomicReference.hh
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#ifndef EATOMICREFERENCE_HH_
#define EATOMICREFERENCE_HH_

#include "./EUnsafe.hh"
#include "../EString.hh"

namespace efc {

/**
 * An object reference that may be updated atomically. See the {@link
 * java.util.concurrent.atomic} package specification for description
 * of the properties of atomic variables.
 * @since 1.5
 * @param <V> The type of object referred to by this reference
 */

//=============================================================================
//SharedPtr Types.

template<typename E>
class EAtomicReference: public EObject {
public:
	/**
	 * Creates a new AtomicReference with the given initial value.
	 *
	 * @param initialValue the initial value
	 */
	EAtomicReference(sp<E> initialValue) {
		value = initialValue;
	}

	/**
	 * Creates a new AtomicReference with null initial value.
	 */
	EAtomicReference() {
		value = null;
	}

	/**
	 * Gets the current value.
	 *
	 * @return the current value
	 */
	sp<E> get() {
		return atomic_load(&value);
	}

	/**
	 * Sets to the given value.
	 *
	 * @param newValue the new value
	 */
	void set(sp<E> newValue) {
		atomic_store(&value, newValue);
	}

	/**
	 * Eventually sets to the given value.
	 *
	 * @param newValue the new value
	 * @since 1.6
	 */
	void lazySet(sp<E> newValue) {
		value = newValue;
	}

	/**
	 * Atomically sets the value to the given updated value
	 * if the current value {@code ==} the expected value.
	 * @param expect the expected value
	 * @param update the new value
	 * @return true if successful. False return indicates that
	 * the actual value was not equal to the expected value.
	 */
	boolean compareAndSet(sp<E> expect, sp<E> update) {
		return atomic_compare_exchange(&value, &expect, update);
	}

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
	boolean weakCompareAndSet(sp<E> expect, sp<E> update) {
		return atomic_compare_exchange(&value, &expect, update);
	}

	/**
	 * Atomically sets to the given value and returns the old value.
	 *
	 * @param newValue the new value
	 * @return the previous value
	 */
	sp<E> getAndSet(sp<E> newValue) {
		while (true ) {
			sp<E> x = get();
			if (compareAndSet(x, newValue))
				return x;
		}
		//never reach here!!!
		return null;
	}

	/**
	 * Returns the String representation of the current value.
	 * @return the String representation of the current value.
	 */
	virtual EString toString() {
		return EString::valueOf((llong)(get().get()));
	}

public:
	// =
	void operator = (sp<E> newValue) {
		set(newValue);
	}
	void operator = (EAtomicReference<E>& that) {
		set(that.get());
	}

	// ==
	boolean operator == (sp<E> thatValue) {
		return (get() == thatValue);
	}
	boolean operator == (EAtomicReference<E>& that) {
		return (get() == that.get());
	}

	// !=
	boolean operator != (sp<E> thatValue) {
		return (get() != thatValue);
	}
	boolean operator != (EAtomicReference<E>& that) {
		return (get() != that.get());
	}

private:
	sp<E> value;
};

//=============================================================================
//Native pointer type.

template<typename E>
class EAtomicReference<E*>: public EObject {
public:
	virtual ~EAtomicReference() {
		delete value;
	}

	/**
	 * Creates a new AtomicReference with the given initial value.
	 *
	 * @param initialValue the initial value
	 */
	EAtomicReference(E* initialValue) {
		value = initialValue;
	}

	/**
	 * Creates a new AtomicReference with null initial value.
	 */
	EAtomicReference() {
		value = null;
	}

	/**
	 * Gets the current value.
	 *
	 * @return the current value
	 */
	E* get() {
		return value;
	}

	/**
	 * Sets to the given value.
	 *
	 * @param newValue the new value
	 */
	void set(E* newValue) {
		value = newValue;
	}

	/**
	 * Eventually sets to the given value.
	 *
	 * @param newValue the new value
	 * @since 1.6
	 */
	void lazySet(E* newValue) {
		EUnsafe::putOrderedObject(&value, newValue);
	}

	/**
	 * Atomically sets the value to the given updated value
	 * if the current value {@code ==} the expected value.
	 * @param expect the expected value
	 * @param update the new value
	 * @return true if successful. False return indicates that
	 * the actual value was not equal to the expected value.
	 */
	boolean compareAndSet(E* expect, E* update) {
		return eso_atomic_compare_and_swapptr((void * volatile *)&value, expect, update);
//		return EUnsafe::compareAndSwapObject(&value, expect, update);
	}

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
	boolean weakCompareAndSet(E* expect, E* update) {
		return eso_atomic_compare_and_swapptr((void * volatile *)&value, expect, update);
//		return EUnsafe::compareAndSwapObject(&value, expect, update);
	}

	/**
	 * Atomically sets to the given value and returns the old value.
	 *
	 * @param newValue the new value
	 * @return the previous value
	 */
	E* getAndSet(E* newValue) {
		while (true ) {
			E* x = get();
			if (compareAndSet(x, newValue))
				return x;
		}
		//never reach here!!!
		return 0;
	}

	/**
	 * Returns the String representation of the current value.
	 * @return the String representation of the current value.
	 */
	virtual EString toString() {
		return EString::valueOf((llong)get());
	}

public:
	// =
	void operator = (E* newValue) {
		set(newValue);
	}
	void operator = (EAtomicReference<E>& that) {
		set(that.get());
	}

	// ==
	boolean operator == (E* thatValue) {
		return (value == thatValue);
	}
	boolean operator == (EAtomicReference<E>& that) {
		return (value == that.get());
	}

	// !=
	boolean operator != (E* thatValue) {
		return (value != thatValue);
	}
	boolean operator != (EAtomicReference<E>& that) {
		return (value != that.get());
	}

private:
	E* volatile value ES_ALIGN;
};

} /* namespace efc */
#endif /* EATOMICREFERENCE_HH_ */
