/**
 * @@file  eso_bitset.h
 * @@brief ES Bitset library
 */


#ifndef __ESO_BITSET_H__
#define __ESO_BITSET_H__

#include "es_types.h"
#include "es_comm.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_bitset_t es_bitset_t;
struct es_bitset_t {
	es_byte_t *bits;
	es_size_t nbits;
	es_size_t nbytes; /*prv*/
};

/**
 * Create
 */
es_bitset_t* eso_bitset_make(es_size_t nbits);
/**
 * Create
 * make from format string like "0101111101"
 */
es_bitset_t* eso_bitset_make2(es_size_t nbits, char *str);

/*Set utils*/

void eso_bitset_reset_bit(es_bitset_t *set, es_size_t pos);
void eso_bitset_set_bit(es_bitset_t *set, es_size_t pos);
void eso_bitset_flip_bit(es_bitset_t *set, es_size_t pos);
void eso_bitset_clear(es_bitset_t *set);

/*Metering utils*/

/**
 * count 1 bits
 */
es_size_t eso_bitset_count(es_bitset_t *set);

/**
 * get bitset size
 */
es_size_t eso_bitset_size(es_bitset_t *set);

/*Operation utils*/

/**
 * |
 */
void eso_bitset_or(es_bitset_t *set, es_int32_t n);
/**
 * ^
 */
void eso_bitset_xor(es_bitset_t *set, es_int32_t n);
/**
 * &
 */
void eso_bitset_and(es_bitset_t *set, es_int32_t n);

/*Test utils*/

/**
 * 1 | 0 | -1(out of nbits)
 */
int eso_bitset_test_bit(es_bitset_t *set, es_size_t pos);
es_bool_t eso_bitset_test_any(es_bitset_t *set);
es_bool_t eso_bitset_test_none(es_bitset_t *set);

/*Convert utils*/

es_string_t* eso_bitset_to_string(es_bitset_t *set);

/**
 * free
 */
void eso_bitset_free(es_bitset_t **set);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_BITSET_H__ */
