/**
 * @file  eso_util.h
 * @brief ES eso util file.
 */

#ifndef __ESO_UTIL_H__
#define __ESO_UTIL_H__

#include "es_types.h"
#include "eso_buffer.h"
#include "eso_file.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * parse file name from file path
 */
char* eso_filepath_name_get(const char *pathname);

int eso_file_create(const char *filename,
                          es_bool_t overwrite,
                          const void* data, 
                          es_uint32_t size);

int eso_file_append_by_filename(const char *filename,
                          const void* data,
                          es_uint32_t size);

int eso_file_update(es_file_t *pfile,
                           es_int32_t pos, 
                           es_int32_t old_size,
                           es_int32_t new_size,
                           const void *new_data);

int eso_file_update_by_filename(const char *filename,
                           es_int32_t pos, 
                           es_int32_t old_size,
                           es_int32_t new_size,
                           const void *new_data);

int eso_file_read(es_file_t *pfile,
                           es_int32_t pos, 
                           es_buffer_t *buffer);

int eso_file_read_by_filename(const char *filename,
                           es_int32_t pos, 
                           es_buffer_t *buffer);

char* eso_resname_from_filename(const char *filename, int global, char res_name[], int buf_size);

/****************************************************
 * String Check Functions
 ****************************************************/

es_bool_t eso_isdigit_string(char *str);

es_bool_t eso_isxdigit_string(char *str);

es_bool_t eso_isalnum_string(char *str);

es_bool_t eso_isascii_string(char *str);

/****************************************************
 * String Convert Functions
 ****************************************************/

char eso_tolower(unsigned char c);

char eso_toupper(unsigned char c);

void eso_to64(char *s, es_uint32_t v, es_int32_t n);

es_int32_t eso_labs(es_int32_t l);

char* eso_strlower(char *str);

char* eso_strupper(char *str);

/****************************************************
 * String Operation Functions
 ****************************************************/

/*
 * eso_strstr by src string size limit
 */
char* eso_strnstr(const char *src, es_size_t src_size, const char *substr);

/*
 * strrstr
 */
char* eso_strrstr(const char *src, const char *substr);

/*
 * strrstr by src whith the index to start the search from
 */
char* eso_strmrstr(const char *src, es_int32_t from_index, const char *substr);

/*
 * strrchr by src string size limit
 */
char* eso_strnrstr(const char *src, es_int32_t src_size, const char *substr);

/*
 * strcasestr by src string size limit
 */
char* eso_strncasestr(const char *src, es_int32_t src_size, const char *substr);

/*
 * case-insensitive string comparisons by src string size limit
 */
es_int32_t eso_strncasecmp(const char *p1, const char *p2, es_size_t len);

/*
 * strchr by src string size limit
 */
char* eso_strnchr(const char *src, es_int32_t src_size, es_int32_t c);

/*
 * strrchr by src whith the index to start the search from
 */
char* eso_strmrchr(const char *src, es_int32_t from_index, es_int32_t c);

/*
 * strrchr by src string size limit
 */
char* eso_strnrchr(const char *src, es_int32_t src_size, es_int32_t c);

/*
 * find string in memory block
 */
char* eso_memstr(const char *src, es_int32_t size, const char *substr);

/*
 * string replace
*/
char* eso_strreplace(const char *str, char o, char d);

/*
 * string split
*/
char* eso_strsplit(const char *from, const char *separators, es_int32_t pos, 
                                                     char *to, es_int32_t to_size);

es_int32_t eso_strcasecmp(const char *a, const char *b);

/*
 * string reverse
 */
void eso_strreverse(char *str);

/*
 * encode BCD
 * return: >= 0 The Dest Str len.
 *         = -1 Fail.
 * remark: the src_len mast be even
*/
es_int32_t eso_enBCD(char *src, es_int32_t src_len,
                         char *dst, es_int32_t dst_len);

/*
 * decode BCD
 * return: >= 0 The Dest Str len.
 *         = -1 Fail.
*/
es_int32_t eso_deBCD(char *src, es_int32_t src_len,
                         char *dst, es_int32_t dst_len);

/*
 * convert byte array to int64.
 * @param s - bytes array
 *        l - array size
 * @limit l values can not be too large, otherwise the converted value 
            will exceed the maximum value of the returned value.
 * @return >= 0 int64 value
           -1   failure
 */
es_int64_t eso_array2llong(es_byte_t s[], int l);

/*
 * convert int64 to byte array.
 * @param  v - int64 value
 *         s - to output byte array
 *         l - byte array length
 * @limit  l value cannot be too small, otherwise the converted value
             will exceed and be discarded.
 * @return >= 0 success, return the valid bytes length
           -1   failure
 */
int eso_llong2array(es_int64_t v, es_byte_t s[], int l);

/**
 * float  <--> es_int32_t
 * double <--> es_int64_t
 */
float eso_intBits2float(es_int32_t x);
es_int32_t eso_float2intBits(float y);
double eso_llongBits2double(es_int64_t x);
es_int64_t eso_double2llongBits(double y);

/**
 * var <--> array
 */
#define MAX_VARINT32_BYTES   5
#define MAX_VARINT64_BYTES   10

/**
 * Varint int32 to array.
 * @param value	The int32 value
 * @param target The target array
 * @return The available target data len
 */
es_uint8_t eso_varint32ToArray(es_uint32_t value, es_uint8_t* target);

/**
 * Varint array to int32.
 * @param buffer The source array
 * @param value	The result of uint32 value
 * @return The end of available buffer
 */
es_uint8_t* eso_arrayToVarint32(const es_uint8_t* buffer, es_uint32_t* value);

/**
 * Varint int64 to array.
 * @param value	The int64 value
 * @param target The target array
 * @return The available target data len
 */
es_uint8_t eso_varint64ToArray(es_uint64_t value, es_uint8_t* target);

/**
 * Varint array to int64.
 * @param buffer The source array
 * @param value	The result of uint64 value
 * @return The end of available buffer
 */
es_uint8_t* eso_arrayToVarint64(const es_uint8_t* buffer, es_uint64_t* value);

/**
 * Convert a buffer of binary values into a hex string representation
 */
char* eso_new_bytes2hexstr(es_uint8_t *bytes, es_size_t buflen);

/**
 * Convert a string of characters representing a hex buffer into a series of bytes of that real value
 */
es_uint8_t* eso_new_hexstr2bytes(const char *inhex);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_UTIL_H__ */
