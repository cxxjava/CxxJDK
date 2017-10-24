/**
 * @file  eso_string.h
 * @brief ES es_string_t/es_data_t Operation Functions
 */

#ifndef __ESO_STRING_H__
#define __ESO_STRING_H__

#include "es_types.h"
#include "es_comm.h"
#include "eso_libc.h"
#include "eso_mem.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * string dup
*/
es_string_t* eso_mstrdup(const char *str);

/*
 * string dup n len
*/
es_string_t* eso_mstrndup(const char *str, es_size_t len);

/*
 * string copy
*/
es_string_t* eso_mstrcpy(es_string_t **to, const char *str);

/*
 * string copy n len
*/
es_string_t* eso_mstrncpy(es_string_t **to, 
                          const char *str, es_size_t len);

/*
 * string cat
*/
es_string_t* eso_mstrcat(es_string_t **to, const char *str);

/*
 * string cat with n len
*/
es_string_t* eso_mstrncat(es_string_t **to, 
                          const char *str, es_size_t len);

/*
 * string vsprintf
*/
es_int32_t eso_mvsprintf(es_string_t **to,
                         const char *fmt, va_list args);

/*
 * string sprintf
*/
es_int32_t eso_msprintf(es_string_t **to, 
                        const char *fmt, ...);

/*
 * create a string and zero it
*/
void* eso_mmeminit(es_data_t **to);

/*
 * string fill with a char
*/
void* eso_mmemfill(es_data_t *data, unsigned char c);

/*
 * string memset
*/
void* eso_mmemset(es_data_t *data, es_size_t offset, 
                  unsigned char c, es_size_t size);

/*
 * string memcpy
*/
void* eso_mmemcpy(es_data_t **dest, es_size_t offset, 
                  const void *src, es_size_t size);

/*
 * string memmove
*/
void* eso_mmemmove(es_data_t **dest, es_size_t offset, 
                   const void *src, es_size_t size);

/*
 * memmory dup
*/
es_data_t* eso_mmemdup(const void *m, es_size_t n);

/*
 * string split
*/
es_string_t* eso_mstrsplit(char *from, const char *separators,
                           es_size_t pos, es_string_t **to);

/*
 * string replace
*/
es_string_t* eso_mstrreplace(es_string_t **string, 
                             const char *from_str, const char *to_str);
/*
 * string replace by offset
*/
es_string_t* eso_mstrreplace_off(es_string_t **string, 
                                 es_size_t offset, es_size_t size, 
                                 const char *from_str, const char *to_str);

/*
 * string insert
*/
es_string_t* eso_mstrinsert(es_string_t **string, 
                            es_size_t pos, const char *str);
/*
 * string insert with n len
*/
es_string_t* eso_mstrninsert(es_string_t **string, 
                             es_size_t pos, 
                             const char *str, es_size_t len);

/*
 * string delete with n len
*/
es_string_t* eso_mstrdelete(es_string_t *string, es_size_t pos, es_size_t len);

/*
 * string left trim
*/
es_string_t* eso_mltrim(es_string_t *string, char c);
es_string_t* eso_mltrims(es_string_t *string, char c[]);

/*
 * string right trim
*/
es_string_t* eso_mrtrim(es_string_t *string, char c);
es_string_t* eso_mrtrims(es_string_t *string, char c[]);

/*
 * string trim
*/
es_string_t* eso_mtrim(es_string_t *string, char c);
es_string_t* eso_mtrims(es_string_t *string, char c[]);

/**
 * URL encode
 * if len<=0 then len=strlen(src)
 */
es_string_t* eso_murlencode(es_string_t** encoded,
                            const char *src, es_int32_t len);

/**
 * URL decode
 */
es_string_t* eso_murldecode(es_string_t** decoded, const char *src);

/**
 * new string for itoa&ltoa
 */
es_string_t* eso_itostring(es_int32_t i, int radix);
es_string_t* eso_ltostring(es_int64_t l, int radix);
es_string_t* eso_uitostring(es_uint32_t ui, int radix);
es_string_t* eso_ultostring(es_uint64_t ul, int radix);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_STRING_H__ */
