/*
 * EAtomic.hh
 *
 *  Created on: 2014-10-9
 *      Author: cxxjava@163.com
 */

#ifndef EATOMIC_HH_
#define EATOMIC_HH_

#include "../../EBase.hh"

namespace efc {

class EAtomic {
public:
	DECLARE_STATIC_INITZZ;

public:
	// Atomically store to a location
	static void store    (byte    store_value, byte*    dest);
	static void store    (short   store_value, short*   dest);
	static void store    (int     store_value, int*     dest);
	static void store    (llong    store_value, llong*    dest);
	static void store_ptr(es_intptr_t store_value, es_intptr_t* dest);
	static void store_ptr(void*    store_value, void*     dest);

	static void store    (byte    store_value, volatile byte*    dest);
	static void store    (short   store_value, volatile short*   dest);
	static void store    (int     store_value, volatile int*     dest);
	static void store    (llong    store_value, volatile llong*    dest);
	static void store_ptr(es_intptr_t store_value, volatile es_intptr_t* dest);
	static void store_ptr(void*    store_value, volatile void*     dest);

	static llong load(volatile llong* src);

	// Atomically add to a location, return updated value
	static int     add    (int     add_value, volatile int*     dest);
	static es_intptr_t add_ptr(es_intptr_t add_value, volatile es_intptr_t* dest);
	static void*       add_ptr(es_intptr_t add_value, volatile void*     dest);

	// Atomically increment location
	static void inc    (volatile int*     dest);
	static void inc_ptr(volatile es_intptr_t* dest);
	static void inc_ptr(volatile void*     dest);

	// Atomically decrement a location
	static void dec    (volatile int*     dest);
	static void dec_ptr(volatile es_intptr_t* dest);
	static void dec_ptr(volatile void*     dest);

	// Performs atomic exchange of *dest with exchange_value.  Returns old prior value of *dest.
	static int         xchg32(int     exchange_value, volatile int*     dest);
	static es_intptr_t xchg_ptr(es_intptr_t exchange_value, volatile es_intptr_t* dest);
	static void*       xchg_ptr(void*       exchange_value, volatile void*   dest);

	// Performs atomic compare of *dest and compare_value, and exchanges *dest with exchange_value
	// if the comparison succeeded.  Returns prior value of *dest.  Guarantees a two-way memory
	// barrier across the cmpxchg.  I.e., it's really a 'fence_cmpxchg_acquire'.
	static byte        cmpxchg    (byte    exchange_value, volatile byte*    dest, byte    compare_value);
	static es_int32_t  cmpxchg32(es_int32_t exchange_value, volatile es_int32_t* dest, es_int32_t compare_value);
	static es_int64_t  cmpxchg64(es_int64_t exchange_value, volatile es_int64_t* dest, es_int64_t compare_value);
	static es_intptr_t cmpxchg_ptr(es_intptr_t exchange_value, volatile es_intptr_t* dest, es_intptr_t compare_value);
	static void*       cmpxchg_ptr(void*       exchange_value, volatile void*     dest, void*    compare_value);
};

} /* namespace efc */
#endif /* EATOMIC_HH_ */
