/**
 * @@file  eso_bitset.c
 * @@brief ES Bitset library
 */


#include "eso_bitset.h"
#include "eso_mem.h"
#include "eso_libc.h"

#define BITSET_BITS CHAR_BIT

#define BITSET_MASK(pos) \
	( ((es_byte_t)1) << ((pos) - ES_ALIGN_DOWN(pos, BITSET_BITS)) )

#define BITSET_WORD(set, pos) \
	( (set)->bits[(pos) / BITSET_BITS] )

es_bitset_t* eso_bitset_make(es_size_t nbits)
{
	es_bitset_t *set;
	es_data_t *data;
	es_size_t nbytes;
	
	nbytes = ES_ALIGN_UP(nbits, BITSET_BITS) / BITSET_BITS;
	data = eso_mcalloc(sizeof(es_bitset_t) + nbytes);
	if (!data)
		return NULL;
	set = (es_bitset_t *)data;
	set->bits = (es_byte_t *)((char *)data + sizeof(es_bitset_t));
	set->nbits = nbits;
	set->nbytes = nbytes;

	return set;
}

es_bitset_t* eso_bitset_make2(es_size_t nbits, char *str)
{
	es_bitset_t *set = eso_bitset_make(nbits);
	es_size_t bits;
	es_int32_t i;
	
	if (!set)
		return NULL;
	
	bits = ES_MIN(nbits, eso_strlen(str));
	for (i=bits-1; i>=0; i--) {
		if (str[i] != '0') 
			eso_bitset_set_bit(set, i);
		/* unused!
		else
			eso_bitset_reset_bit(set, i);
		*/
	}
	
	return set;
}

void eso_bitset_clear(es_bitset_t *set)
{
	eso_memset(set->bits, 0, set->nbytes);
}

void eso_bitset_free(es_bitset_t **set)
{
	if (!*set)
		return;
	
	ESO_MFREE(set);
}

int eso_bitset_test_bit(es_bitset_t *set, es_size_t pos)
{
	if (pos >= set->nbits) {
	    return -1;
	}

	return (BITSET_WORD(set, pos) & BITSET_MASK(pos)) != 0;
}

es_bool_t eso_bitset_test_any(es_bitset_t *set)
{
	es_uint32_t i;
	
	for (i = 0; i < set->nbytes; i++) {
		if (set->bits[i] > 0)
			return TRUE;
	}
	
	return FALSE;
}

es_bool_t eso_bitset_test_none(es_bitset_t *set)
{
	return !eso_bitset_test_any(set);
}

void eso_bitset_reset_bit(es_bitset_t *set, es_size_t pos)
{
	if (pos >= set->nbits) {
	    return;
	}

	BITSET_WORD(set, pos) &= ~BITSET_MASK(pos);
}

void eso_bitset_set_bit(es_bitset_t *set, es_size_t pos)
{
	if (pos >= set->nbits) {
	    return;
	}

	BITSET_WORD(set, pos) |= BITSET_MASK(pos);
}

void eso_bitset_flip_bit(es_bitset_t *set, es_size_t pos)
{
	if (pos >= set->nbits) {
	    return;
	}

	if (eso_bitset_test_bit(set, pos))
		eso_bitset_reset_bit(set, pos);
	else
		eso_bitset_set_bit(set, pos);
}

/**
 * count 1 bits
 */
es_size_t eso_bitset_count(es_bitset_t *set)
{
	es_size_t count = 0;
	es_size_t i;
	
	for (i = 0; i < set->nbits; i++) {
		if (eso_bitset_test_bit(set, i))
			count++;
	}
	return count;
}

/**
 * get bitset size
 */
es_size_t eso_bitset_size(es_bitset_t *set)
{
	return set->nbits;
}

/**
 * |
 */
void eso_bitset_or(es_bitset_t *set, es_int32_t n)
{
	*(es_uint32_t *)set->bits |= n;
}

/**
 * ^
 */
void eso_bitset_xor(es_bitset_t *set, es_int32_t n)
{
	*(es_uint32_t *)set->bits ^= n;
}

/**
 * &
 */
void eso_bitset_and(es_bitset_t *set, es_int32_t n)
{
	*(es_uint32_t *)set->bits &= n;
}

es_string_t* eso_bitset_to_string(es_bitset_t *set)
{
	es_string_t *str = eso_mmalloc(set->nbits + 1);
	es_uint32_t i, j, pos;
	
	if (!str)
		return NULL;
	
	pos = 0 ;
	for (i = 0; i < set->nbytes; i++) {
		for (j = 0; j < BITSET_BITS; j++) {
			if (pos >= set->nbits)
				break;
			if (set->bits[i] & BITSET_MASK(pos))
				str[pos] = '1';
			else
				str[pos] = '0';
			pos++;
		}
	}
	str[pos] = '\0';
	return str;
}
