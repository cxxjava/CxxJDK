/**
 * @file  eso_hash.c
 * @brief ES Hash Tables
 */

#include "eso_hash.h"
#include "eso_util.h"
#include "eso_libc.h"

/*
 * The internal form of a hash table.
 *
 * The table is an array indexed by the hash of the key; collisions
 * are resolved by hanging a linked list of hash entries off each
 * element of the array. Although this is a really simple design it
 * isn't too bad given that pools have a low allocation overhead.
 */

typedef struct es_hash_entry_t es_hash_entry_t;

struct es_hash_entry_t {
    es_hash_entry_t *next;
    es_uint32_t     hash;
    es_data_t       *key;
    es_ssize_t       klen;
    const void       *val;
};

/*
 * Data structure for iterating through a hash table.
 *
 * We keep a pointer to the next hash entry here to allow the current
 * hash entry to be freed or otherwise mangled between calls to
 * es_hash_next().
 */
struct es_hash_index_t {
    es_hash_t         *ht;
    es_hash_entry_t   *_this, *next;
    es_uint32_t       index;
};

/*
 * The size of the array is always a power of two. We use the maximum
 * index rather than the size so that we can use bitwise-AND for
 * modular arithmetic.
 * The count of hash entries may be greater depending on the chosen
 * collision rate.
 */
struct es_hash_t {
    es_hash_entry_t   **array;     /* data value is type es_hash_entry_t** */
    es_hash_index_t     iterator;  /* For es_hash_first(NULL, ...) */
    es_uint32_t         count, max;
    es_hashfunc_t      *hash_func;
    es_hash_entry_t    *free;  /* List of recycled entries */
};

/*
 * Hash creation functions.
 */
static es_hash_entry_t **alloc_array(es_hash_t *ht, es_uint32_t max)
{
	return (es_hash_entry_t **)eso_calloc(sizeof(es_hash_entry_t *) * (max + 1));
}

/*
 * Expanding a hash table
 */
static void expand_array(es_hash_t *ht)
{
    es_hash_index_t *hi;
    es_uint32_t new_max;
    es_hash_entry_t **new_array;

    new_max = ht->max * 2 + 1;
    new_array = alloc_array(ht, new_max);
    for (hi = eso_hash_first(ht); hi; hi = eso_hash_next(hi)) {
        es_uint32_t i = hi->_this->hash & new_max;
        hi->_this->next = new_array[i];
        new_array[i] = hi->_this;
    }

    /* free old array */
    ESO_FREE(&ht->array);

    ht->array = new_array;
    ht->max = new_max;
}

static es_uint32_t hashfunc_default(const char *char_key,
                                    es_ssize_t *klen)
{
    es_uint32_t hash = 0;
    const unsigned char *key = (const unsigned char *)char_key;
    const unsigned char *p;
    es_ssize_t i;

    /*
     * This is the popular `times 33' hash algorithm which is used by
     * perl and also appears in Berkeley DB. This is one of the best
     * known hash functions for strings because it is both computed
     * very fast and distributes very well.
     *
     * The originator may be Dan Bernstein but the code in Berkeley DB
     * cites Chris Torek as the source. The best citation I have found
     * is "Chris Torek, Hash function for text in C, Usenet message
     * <27038@mimsy.umd.edu> in comp.lang.c , October, 1990." in Rich
     * Salz's USENIX 1992 paper about INN which can be found at
     * <http://citeseer.nj.nec.com/salz92internetnews.html>.
     *
     * The magic of number 33, i.e. why it works better than many other
     * constants, prime or not, has never been adequately explained by
     * anyone. So I try an explanation: if one experimentally tests all
     * multipliers between 1 and 256 (as I did while writing a low-level
     * data structure library some time ago) one detects that even
     * numbers are not useable at all. The remaining 128 odd numbers
     * (except for the number 1) work more or less all equally well.
     * They all distribute in an acceptable way and this way fill a hash
     * table with an average percent of approx. 86%.
     *
     * If one compares the chi^2 values of the variants (see
     * Bob Jenkins ``Hashing Frequently Asked Questions'' at
     * http://burtleburtle.net/bob/hash/hashfaq.html for a description
     * of chi^2), the number 33 not even has the best value. But the
     * number 33 and a few other equally good numbers like 17, 31, 63,
     * 127 and 129 have nevertheless a great advantage to the remaining
     * numbers in the large set of possible multipliers: their multiply
     * operation can be replaced by a faster operation based on just one
     * shift plus either a single addition or subtraction operation. And
     * because a hash function has to both distribute good _and_ has to
     * be very fast to compute, those few numbers should be preferred.
     *
     *                  -- Ralf S. Engelschall <rse@engelschall.com>
     */

    if (*klen == ES_HASH_KEY_STRING) {
        for (p = key; *p; p++) {
            hash = hash * 33 + *p;
        }
        *klen = p - key;
    }
    else {
        for (p = key, i = *klen; i; i--, p++) {
            hash = hash * 33 + *p;
        }
    }

    return hash;
}

/*
 * This is where we keep the details of the hash function and control
 * the maximum collision rate.
 *
 * If val is non-NULL it creates and initializes a new hash entry if
 * there isn't already one there; it returns an updatable pointer so
 * that hash entries can be removed.
 */

static es_hash_entry_t **find_entry(es_hash_t *ht,
                                     const void *key,
                                     es_ssize_t klen,
                                     const void *val)
{
    es_hash_entry_t **hep, *he;
    es_uint32_t hash;
    es_hash_entry_t **array;

    hash = ht->hash_func((char*)key, &klen);

    array = (es_hash_entry_t **)ht->array;
    /* scan linked list */
    for (hep = &array[hash & ht->max], he = *hep;
         he; hep = &he->next, he = *hep) {
        if (he->hash == hash
            && he->klen == klen
            && eso_memcmp(he->key, key, klen) == 0)
            break;
    }
    if (he || !val)
        return hep;

    /* add a new entry for non-NULL values */
    if ((he = ht->free) != NULL)
        ht->free = he->next;
    else {
    	he = (es_hash_entry_t*)eso_calloc(sizeof(es_hash_entry_t));
    }
    he->next = NULL;
    he->hash = hash;
    he->key  = eso_calloc(klen+1);
    eso_memcpy(he->key, key, klen);
    he->klen = klen;
    he->val  = val;
    *hep = he;
    ht->count++;
    return hep;
}

es_hash_t* eso_hash_make(es_uint32_t initcapacity, es_hashfunc_t *hashfunc)
{
    es_hash_t *ht;

    ht = (es_hash_t*)eso_malloc(sizeof(es_hash_t));
    ht->free = NULL;
    ht->count = 0;
    ht->max = initcapacity;
    ht->array = alloc_array(ht, ht->max);
    ht->hash_func = hashfunc ? hashfunc : hashfunc_default;

    return ht;
}

/*
 * Hash iteration functions.
 */

es_hash_index_t* eso_hash_next(es_hash_index_t *hi)
{
    es_hash_entry_t **array = (es_hash_entry_t **)hi->ht->array;

 	hi->_this = hi->next;
    while (!hi->_this) {
    	if (hi->index > hi->ht->max)
            return NULL;

    	hi->_this = array[hi->index++];
    }
    hi->next = hi->_this->next;
    return hi;
}

/*
 * non-thread-safe iterator is used.
 */
es_hash_index_t* eso_hash_first(es_hash_t *ht)
{
    es_hash_index_t *hi;

    hi = &ht->iterator;
    hi->ht = ht;
    hi->index = 0;
    hi->_this = NULL;
    hi->next = NULL;

    return eso_hash_next(hi);
}

void eso_hash_this(es_hash_index_t *hi,
                                void **key,
                                es_ssize_t *klen,
                                void **val)
{
	if (key)  *key  = (void *)hi->_this->key;
    if (klen) *klen = hi->_this->klen;
    if (val)  *val  = (void *)hi->_this->val;
}

const void* eso_hash_get(es_hash_t *ht,
                                 const void *key,
                                 es_ssize_t klen)
{
    es_hash_entry_t *he;

    he = *find_entry(ht, key, klen, NULL);

	if (he)
        return (void *)he->val;
    else
        return NULL;
}

const void* eso_hash_set(es_hash_t *ht,
                               const void *key,
                               es_ssize_t klen,
                               const void *val)
{
    es_hash_entry_t **hep;
    const void* oldval = NULL;

    hep = find_entry(ht, key, klen, val);
    if (*hep) {
        es_hash_entry_t *old = *hep;
        if (!val) {
            /* delete entry */
            *hep = old->next;
            old->next = ht->free;
            oldval = old->val;
            ESO_FREE(&old->key);
            ht->free = old;
            --ht->count;
        }
        else {
            /* replace entry */
        	oldval = old->val;
        	old->val = val;
            /* check that the collision rate isn't too high */
            if (ht->count > ht->max) {
                expand_array(ht);
            }
        }
    }
    /* else key not present and val==NULL */

    return oldval;
}

es_uint32_t eso_hash_count(es_hash_t *ht)
{
    return ht->count;
}

void eso_hash_clear(es_hash_t *ht, void (*freeCall)(const void* value))
 {
	es_hash_index_t *hi;
	es_hash_entry_t *free_ref, *free_next;

	if (!ht) return;

	/* free entry */
	for (hi = eso_hash_first(ht); hi; hi = eso_hash_next(hi)) {
		if (freeCall) {
			freeCall(hi->_this->val);
		}
		eso_free(hi->_this->key);
		eso_free(hi->_this);
	}

	/* free free-node */
	free_ref = ht->free;
	while (free_ref) {
		free_next = free_ref->next;
		eso_free(free_ref->key);
		eso_free(free_ref);
		free_ref = free_next;
	}

	ht->free = NULL;
	ht->count = 0;

	/* rest array-node */
	eso_memset(ht->array, 0, sizeof(es_hash_entry_t *) * (ht->max + 1));
}

void eso_hash_free(es_hash_t **ht)
{
	es_hash_index_t *hi;
    es_hash_entry_t *free_ref, *free_next;

	if (!ht || !(*ht))
		return;

    /* free entry */
    for (hi = eso_hash_first(*ht); hi; hi = eso_hash_next(hi)) {
       	eso_free(hi->_this->key);
       	eso_free(hi->_this);
    }

    /* free array-node */
    eso_free((*ht)->array);

    /* free free-node */
    free_ref = (*ht)->free;
	while (free_ref) {
		free_next = free_ref->next;
		eso_free(free_ref->key);
		eso_free(free_ref);
		free_ref = free_next;
	}

    /* free self-node */
    ESO_FREE(ht);
}
