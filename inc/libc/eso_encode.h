/**
 * @file  eso_encode.h
 * @brief ES character encode.
 */

#ifndef __ESO_ENCODE_H__
#define __ESO_ENCODE_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Convert utf8 string to ucs2 string.
 * @param ucs2	ucs2 stirng
 * @param utf8	utf8 stirng
 * @return count of utf8 string bytes
 */
es_int32_t eso_utf8_to_ucs2(es_uint16_t *ucs2, es_uint8_t *utf8);

/**
 * Convert ucs2 string to utf8 string.
 * @param utf8	utf8 stirng
 * @param ucs2	ucs2 stirng
 * @return count of utf8 string bytes
 */
es_int32_t eso_ucs2_to_utf8(es_uint8_t *utf8, es_uint16_t ucs2);

/**
 * Measure utf8 string characters.
 * @param s	utf8 stirng
 * @return count of utf8 string characters
 */
es_int32_t eso_utf8_strlen(const char *str);

/**
 * Measure utf8 string bytes.
 * @param s	utf8 stirng
 * @param n count of utf8 string characters
 * @return count of utf8 string bytes
 */
es_int32_t eso_utf8_bytes(const char *str, es_int32_t n);

/**
 * Measure ucs2 string characters.
 * @param s	ucs2 stirng
 * @return count of ucs2 string characters
 */
es_int32_t eso_ucs2_strlen(const char* ucs2str);

/**
 * Convert utf8 string to ucs2 string.
 * @param dest	out->ucs2 string
 * @param src	in->utf8 string
 * @return count of valid ucs2 string bytes(exclude '\0\0')
 */
es_int32_t eso_utf8_to_ucs2_string(es_wstring_t **dest,
                                   const char *utf8str);

/**
 * Convert ucs2 string to utf8 string.
 * @param dest	out->utf8 string
 * @param src	in->ucs2 string
 * @return count of valid utf8 string bytes(exclude '\0')
 */
es_int32_t eso_ucs2_to_utf8_string(es_string_t **dest,
                                   const char *ucs2src);

//ucs2 utils.
es_int32_t eso_ucs2_strcmp(const char *ucs2str1, const char *ucs2str2);
char *eso_ucs2_strcpy(char *ucs2dst, const char *ucs2src);
es_int32_t eso_ucs2_strncmp(const char *ucs2str1, const char *ucs2str2, es_uint32_t size);
char* eso_ucs2_strncpy(char *ucs2dst, const char *ucs2src, es_uint32_t size);
char* eso_ucs2_strcat(char *ucs2dst, const char *ucs2src);
char* eso_ucs2_strncat(char *ucs2dst, const char *ucs2src, es_uint32_t size);
es_uint16_t eso_asc_str_to_ucs2_str(char *ucs2str, char *ascstr);
es_uint16_t eso_asc_str_n_to_ucs2_str(char *ucs2str, char *ascstr, es_uint32_t len);
es_uint16_t eso_ucs2_str_to_asc_str(char *ascstr, char *ucs2str);
es_uint16_t eso_ucs2_str_n_to_asc_str(char *ascstr, char *ucs2str, es_uint32_t len);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_ENCODE_H__ */
