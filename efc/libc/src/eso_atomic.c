/**
 * @file  eso_atomic.c
 * @brief ES Atomic.
 */

#include "eso_atomic.h"

#if defined(WIN32)

#include <windows.h>
/** -------------------------------------------------------------------------
    Atomic operations using WIN32 kernel functions.

    The 'post' version of each operation applies the operation after fetching
    the value.
 */

es_int32_t eso_atomic_add_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return InterlockedExchangeAdd((LONG volatile *)theValue, (LONG)theAmount) + theAmount;
}

es_int64_t eso_atomic_add_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return InterlockedExchangeAdd64((LONGLONG volatile *)theValue, (LONGLONG)theAmount) + theAmount;
}

es_int32_t eso_atomic_fetch_and_add32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return InterlockedExchangeAdd((LONG volatile *)theValue, (LONG)theAmount);
}

es_int64_t eso_atomic_fetch_and_add64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return InterlockedExchangeAdd64((LONGLONG volatile *)theValue, (LONGLONG)theAmount);
}

es_int32_t eso_atomic_sub_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return InterlockedExchangeAdd((LONG volatile *)theValue, (LONG)-theAmount) - theAmount;
}

es_int64_t eso_atomic_sub_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return InterlockedExchangeAdd64((LONGLONG volatile *)theValue, (LONGLONG)-theAmount) - theAmount;
}

es_int32_t eso_atomic_fetch_and_sub32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return InterlockedExchangeAdd((LONG volatile *)theValue, (LONG)-theAmount);
}

es_int64_t eso_atomic_fetch_and_sub64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return InterlockedExchangeAdd64((LONGLONG volatile *)theValue, (LONGLONG)-theAmount);
}

es_int32_t eso_atomic_test_and_set32(volatile es_int32_t *theValue, es_int32_t newVal)
{
	return InterlockedExchange((LONG volatile *)theValue, (LONG)newVal);
}

es_int64_t eso_atomic_test_and_set64(volatile es_int64_t *theValue, es_int64_t newVal)
{
	return InterlockedExchange64((LONGLONG volatile *)theValue, (LONGLONG)newVal);
}

es_bool_t eso_atomic_compare_and_swap32(volatile es_int32_t *theValue, es_int32_t oldVal, es_int32_t newVal)
{
	return (InterlockedCompareExchange((LONG volatile *)theValue, (LONG)newVal, (LONG)oldVal) == (LONG)oldVal);
}

es_bool_t eso_atomic_compare_and_swap64(volatile es_int64_t *theValue, es_int64_t oldVal, es_int64_t newVal)
{
	return (InterlockedCompareExchange64((LONGLONG volatile *)theValue, (LONGLONG)newVal, (LONGLONG)oldVal) == (LONGLONG)oldVal);
}

es_bool_t eso_atomic_compare_and_swapptr(void * volatile *theValue, void * oldVal, void * newVal)
{
	return (InterlockedCompareExchangePointer((PVOID volatile *)theValue, (PVOID)newVal, (PVOID)oldVal) == (PVOID)oldVal);
}

es_int32_t eso_atomic_or_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	while (TRUE) {
		es_int32_t oldVal = *theValue;
		if (InterlockedCompareExchange((LONG volatile *)theValue, (LONG)(oldVal | theMask), (LONG)oldVal) == (LONG)oldVal) {
			return *theValue;
		}
	}
	//never reach here!!!
	return -1;
}

es_int32_t eso_atomic_fetch_and_or32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	while (TRUE) {
		es_int32_t oldVal = *theValue;
		if (InterlockedCompareExchange((LONG volatile *)theValue, (LONG)(oldVal | theMask), (LONG)oldVal) == (LONG)oldVal) {
			return oldVal;
		}
	}
	//never reach here!!!
	return -1;
}

es_int32_t eso_atomic_xor_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	while (TRUE) {
		es_int32_t oldVal = *theValue;
		if (InterlockedCompareExchange((LONG volatile *)theValue, (LONG)(oldVal ^ theMask), (LONG)oldVal) == (LONG)oldVal) {
			return *theValue;
		}
	}
	//never reach here!!!
	return -1;
}

es_int32_t eso_atomic_fetch_and_xor32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	while (TRUE) {
		es_int32_t oldVal = *theValue;
		if (InterlockedCompareExchange((LONG volatile *)theValue, (LONG)(oldVal ^ theMask), (LONG)oldVal) == (LONG)oldVal) {
			return oldVal;
		}
	}
	//never reach here!!!
	return -1;
}

es_int32_t eso_atomic_and_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	while (TRUE) {
		es_int32_t oldVal = *theValue;
		if (InterlockedCompareExchange((LONG volatile *)theValue, (LONG)(oldVal & theMask), (LONG)oldVal) == (LONG)oldVal) {
			return *theValue;
		}
	}
	//never reach here!!!
	return -1;
}

es_int32_t eso_atomic_fetch_and_and32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	while (TRUE) {
		es_int32_t oldVal = *theValue;
		if (InterlockedCompareExchange((LONG volatile *)theValue, (LONG)(oldVal & theMask), (LONG)oldVal) == (LONG)oldVal) {
			return oldVal;
		}
	}
	//never reach here!!!
	return -1;
}

#if defined(_AMD64_)
      #define memory_barrier __faststorefence
#elif defined(_IA64_)
      #define memory_barrier __mf
#elif defined(_X86_)
static ES_INLINE void memory_barrier(void) {
    long barrier;
    __asm { xchg barrier, eax }
}
#else
#    error "Missing implementation for the atomic memory barrir operations"
#endif

es_intptr_t* eso_atomic_test_and_setptr(volatile es_intptr_t *theValue, es_intptr_t *newVal)
{
	return (es_intptr_t*)InterlockedExchangePointer((PVOID volatile *)theValue, (PVOID)newVal); //InterlockedExchange64?
}

void eso_atomic_synchronize()
{
	memory_barrier();
}

#elif defined(__APPLE__)

#include <libkern/OSAtomic.h>

/** -------------------------------------------------------------------------
    Atomic operations using OS X kernel functions.

    The 'post' version of each operation applies the operation after fetching
    the value.
 */

es_int32_t eso_atomic_add_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return OSAtomicAdd32Barrier(theAmount, theValue);
}

es_int64_t eso_atomic_add_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return OSAtomicAdd64Barrier(theAmount, theValue);
}

es_int32_t eso_atomic_fetch_and_add32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return OSAtomicAdd32Barrier(theAmount, theValue) - theAmount;
}

es_int64_t eso_atomic_fetch_and_add64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return OSAtomicAdd64Barrier(theAmount, theValue) - theAmount;
}

es_int32_t eso_atomic_sub_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return OSAtomicAdd32Barrier(-theAmount, theValue);
}

es_int64_t eso_atomic_sub_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return OSAtomicAdd64Barrier(-theAmount, theValue);
}

es_int32_t eso_atomic_fetch_and_sub32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return OSAtomicAdd32Barrier(-theAmount, theValue) + theAmount;
}

es_int64_t eso_atomic_fetch_and_sub64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return OSAtomicAdd64Barrier(-theAmount, theValue) + theAmount;
}

es_int32_t eso_atomic_test_and_set32(volatile es_int32_t *theValue, es_int32_t newVal)
{
	//@see: http://stackoverflow.com/questions/78277/how-to-guarantee-64-bit-writes-are-atomic

	while (TRUE ) {
		es_int32_t oldVal = *theValue;
		if (OSAtomicCompareAndSwap32Barrier(oldVal, newVal, theValue)) {
			return oldVal;
		}
	}
	//never reach here!!!
	return -1;
}

es_int64_t eso_atomic_test_and_set64(volatile es_int64_t *theValue, es_int64_t newVal)
{
	//@see: http://stackoverflow.com/questions/78277/how-to-guarantee-64-bit-writes-are-atomic

	while (TRUE) {
		es_int64_t oldVal = *theValue;
		if (OSAtomicCompareAndSwap64Barrier(oldVal, newVal, theValue)) {
			return oldVal;
		}
	}
	//never reach here!!!
	return -1;
}

es_bool_t eso_atomic_compare_and_swap32(volatile es_int32_t *theValue, es_int32_t oldVal, es_int32_t newVal)
{
	return OSAtomicCompareAndSwap32Barrier(oldVal, newVal, theValue);
}

es_bool_t eso_atomic_compare_and_swap64(volatile es_int64_t *theValue, es_int64_t oldVal, es_int64_t newVal)
{
	return OSAtomicCompareAndSwap64Barrier(oldVal, newVal, theValue);
}

es_bool_t eso_atomic_compare_and_swapptr(void * volatile *theValue, void * oldVal, void * newVal)
{
	return OSAtomicCompareAndSwapPtrBarrier(oldVal, newVal, theValue);
}

es_int32_t eso_atomic_or_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return OSAtomicOr32Barrier((es_uint32_t)theMask, (volatile es_uint32_t *)theValue);
}

es_int32_t eso_atomic_fetch_and_or32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return OSAtomicOr32OrigBarrier((es_uint32_t)theMask, (volatile es_uint32_t *)theValue);
}

es_int32_t eso_atomic_xor_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return OSAtomicXor32Barrier((es_uint32_t)theMask, (volatile es_uint32_t *)theValue);
}

es_int32_t eso_atomic_fetch_and_xor32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return OSAtomicXor32OrigBarrier((es_uint32_t)theMask, (volatile es_uint32_t *)theValue);
}

es_int32_t eso_atomic_and_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return OSAtomicAnd32Barrier((es_uint32_t)theMask, (volatile es_uint32_t *)theValue);
}

es_int32_t eso_atomic_fetch_and_and32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return OSAtomicAnd32OrigBarrier((es_uint32_t)theMask, (volatile es_uint32_t *)theValue);
}

es_intptr_t* eso_atomic_test_and_setptr(volatile es_intptr_t *theValue, es_intptr_t *newVal)
{
#if (SIZEOF_VOID_P == 4)
	return (void*)eso_atomic_test_and_set32((volatile es_int32_t *)theValue, (es_int32_t)newVal);
#elif (SIZEOF_VOID_P == 8)
	return (void*)eso_atomic_test_and_set64((volatile es_int64_t *)theValue, (es_int64_t)newVal);
#else
#   error "Not support (SIZEOF_VOID_P != 4 || SIZEOF_VOID_P != 8!"
	return null;
#endif
}

void eso_atomic_synchronize()
{
	OSMemoryBarrier();
}

#elif defined(__linux__)

/** -------------------------------------------------------------------------
    Atomic operations using GCC builtins.

    The 'post' version of each operation applies the operation after fetching
    the value.
 */
es_int32_t eso_atomic_add_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return __sync_add_and_fetch(theValue, theAmount);
}

es_int64_t eso_atomic_add_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return __sync_add_and_fetch(theValue, theAmount);
}

es_int32_t eso_atomic_fetch_and_add32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return __sync_fetch_and_add(theValue, theAmount);
}

es_int64_t eso_atomic_fetch_and_add64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return __sync_fetch_and_add(theValue, theAmount);
}

es_int32_t eso_atomic_sub_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return __sync_sub_and_fetch(theValue, theAmount);
}

es_int64_t eso_atomic_sub_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return __sync_sub_and_fetch(theValue, theAmount);
}

es_int32_t eso_atomic_fetch_and_sub32(volatile es_int32_t *theValue, es_int32_t theAmount)
{
	return __sync_fetch_and_sub(theValue, theAmount);
}

es_int64_t eso_atomic_fetch_and_sub64(volatile es_int64_t *theValue, es_int64_t theAmount)
{
	return __sync_fetch_and_sub(theValue, theAmount);
}

es_int32_t eso_atomic_test_and_set32(volatile es_int32_t *theValue, es_int32_t newVal)
{
	return __sync_lock_test_and_set(theValue, newVal);
}

es_int64_t eso_atomic_test_and_set64(volatile es_int64_t *theValue, es_int64_t newVal)
{
	return __sync_lock_test_and_set(theValue, newVal);
}

es_bool_t eso_atomic_compare_and_swap32(volatile es_int32_t *theValue, es_int32_t oldVal, es_int32_t newVal)
{
	return __sync_bool_compare_and_swap(theValue, oldVal, newVal);
}

es_bool_t eso_atomic_compare_and_swap64(volatile es_int64_t *theValue, es_int64_t oldVal, es_int64_t newVal)
{
	return __sync_bool_compare_and_swap(theValue, oldVal, newVal);
}

es_bool_t eso_atomic_compare_and_swapptr(void * volatile *theValue, void * oldVal, void * newVal)
{
	return __sync_bool_compare_and_swap(theValue, oldVal, newVal);
}

es_int32_t eso_atomic_or_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return __sync_or_and_fetch(theValue, theMask);
}

es_int32_t eso_atomic_fetch_and_or32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return __sync_fetch_and_or(theValue, theMask);
}

es_int32_t eso_atomic_xor_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return __sync_xor_and_fetch(theValue, theMask);
}

es_int32_t eso_atomic_fetch_and_xor32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return __sync_fetch_and_xor(theValue, theMask);
}

es_int32_t eso_atomic_and_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return __sync_and_and_fetch(theValue, theMask);
}

es_int32_t eso_atomic_fetch_and_and32(volatile es_int32_t *theValue, es_int32_t theMask)
{
	return __sync_fetch_and_and(theValue, theMask);
}

es_intptr_t* eso_atomic_test_and_setptr(volatile es_intptr_t *theValue, es_intptr_t *newVal)
{
	return (es_intptr_t*)__sync_lock_test_and_set(theValue, newVal);
}

void eso_atomic_synchronize()
{
	__sync_synchronize();
}

#else //other

#    error "Missing implementation for the atomic operations"

#endif
