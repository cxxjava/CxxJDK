/**
 * @file  eso_atomic.h
 * @brief ES Atomic.
 */

#ifndef __ESO_ATOMIC_H__
#define __ESO_ATOMIC_H__

#include "es_comm.h"
#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif

es_int32_t eso_atomic_add_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount);

es_int64_t eso_atomic_add_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount);

es_int32_t eso_atomic_fetch_and_add32(volatile es_int32_t *theValue, es_int32_t theAmount);

es_int64_t eso_atomic_fetch_and_add64(volatile es_int64_t *theValue, es_int64_t theAmount);

es_int32_t eso_atomic_sub_and_fetch32(volatile es_int32_t *theValue, es_int32_t theAmount);

es_int64_t eso_atomic_sub_and_fetch64(volatile es_int64_t *theValue, es_int64_t theAmount);

es_int32_t eso_atomic_fetch_and_sub32(volatile es_int32_t *theValue, es_int32_t theAmount);

es_int64_t eso_atomic_fetch_and_sub64(volatile es_int64_t *theValue, es_int64_t theAmount);

es_int32_t eso_atomic_test_and_set32(volatile es_int32_t *theValue, es_int32_t newVal);

es_int64_t eso_atomic_test_and_set64(volatile es_int64_t *theValue, es_int64_t newVal);

es_bool_t eso_atomic_compare_and_swap32(volatile es_int32_t *theValue, es_int32_t oldVal, es_int32_t newVal);

es_bool_t eso_atomic_compare_and_swap64(volatile es_int64_t *theValue, es_int64_t oldVal, es_int64_t newVal);

es_bool_t eso_atomic_compare_and_swapptr(void * volatile *theValue, void *oldVal, void *newVal);

es_int32_t eso_atomic_or_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask);

es_int32_t eso_atomic_fetch_and_or32(volatile es_int32_t *theValue, es_int32_t theMask);

es_int32_t eso_atomic_xor_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask);

es_int32_t eso_atomic_fetch_and_xor32(volatile es_int32_t *theValue, es_int32_t theMask);

es_int32_t eso_atomic_and_and_fetch32(volatile es_int32_t *theValue, es_int32_t theMask);

es_int32_t eso_atomic_fetch_and_and32(volatile es_int32_t *theValue, es_int32_t theMask);

es_intptr_t* eso_atomic_test_and_setptr(volatile es_intptr_t *theValue, es_intptr_t *newVal);

void eso_atomic_synchronize();

#ifdef __cplusplus
}
#endif

#endif /* __ESO_ATOMIC_H__ */
