/**
 * @file  eso_ini.h
 * @brief ES Inifile Routines
 */


#ifndef __ESO_INI_H__
#define __ESO_INI_H__

#include "es_types.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup es_ini Inifile Routines
 * @ingroup ES 
 * @{
 */

typedef struct es_inifile_t es_inifile_t;

typedef struct es_ini_sec_t es_ini_sec_t;
typedef struct es_ini_key_t es_ini_key_t;

#define ES_INI_RDONLY       0
#define ES_INI_RDWR         1

/**
 * open ini file
 */
es_inifile_t* eso_inifile_open(const char *fname, es_int32_t flag);

/**
 * close ini file
 */
void eso_inifile_close(es_inifile_t **ini);

/**
 * fetch first section
 */
es_ini_sec_t* eso_ini_sec_first(es_inifile_t *ini);

/**
 * fetch next section
 */
es_ini_sec_t* eso_ini_sec_next(es_ini_sec_t *sec);

/**
 * get section name by es_ini_sec_t
 */
char* eso_ini_sec_get_name(es_ini_sec_t *sec);

/**
 * fetch first key by es_ini_sec_t
 */
es_ini_key_t* eso_ini_key_first(es_ini_sec_t *sec);

/**
 * fetch next key by last es_ini_key_t
 */
es_ini_key_t* eso_ini_key_next(es_ini_key_t *key);

/**
 * get key name by es_ini_key_t
 */
char* eso_ini_key_get_name(es_ini_key_t *key);

/**
 * get key value by es_ini_key_t
 */
char* eso_ini_key_get_value(es_ini_key_t *key);

/**
 * get section by section name and section index, index begin 1
 */
es_ini_sec_t* eso_ini_get_section(es_inifile_t *ini,
                                              const char *section,
                                              es_int32_t idx_sec);

/**
 * get key from es_ini_sec_t by key name and key index, index begin 1
 */
es_ini_key_t* eso_ini_get_key(es_ini_sec_t *sec,
                                              const char *key,
                                              es_int32_t idx_key);

/**
 * get key value from first section, first key
 * by section name and key name,
 * return NULL mean none thus key
 */
char* eso_ini_read_string(es_inifile_t *ini,
                                      const char *section,
                                      const char *key,
                                      char *def);

/**
 * get key value from the index's section, the index's key
 * by section name and key name, index begin 1
 * return NULL mean none thus key
 */
char* eso_ini_read_string2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      es_int32_t idx_key,
                                      char *def);

/**
 * get key value from the section
 * by section and key name
 * return NULL mean none thus key
 */
char* eso_ini_read_string3(es_ini_sec_t *sec,
                                      const char *key,
                                      char *def);

/**
 * add new es_ini_sec_t append to inifile
 * return new es_ini_sec_t
 */
es_ini_sec_t* eso_ini_add_sec(es_inifile_t *ini,
                                      const char *section);

/**
 * add new es_ini_key_t append to index section, index begin 1
 * return new es_ini_key_t
 */
es_ini_key_t* eso_ini_add_key(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      const char *value);

/**
 * delete first section in inifile
 */
void eso_ini_del_sec(es_inifile_t *ini,
                                      const char *section);

/**
 * delete es_ini_sec_t in inifile by section name and index,
 * index begin 1
 */
void eso_ini_del_sec2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec);

/**
 * delete first key in first section
 */
void eso_ini_del_key(es_inifile_t *ini,
                                      const char *section,
                                      const char *key);

/**
 * delete key by key index and section index,
 * index begin 1
 */
void eso_ini_del_key2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      es_int32_t idx_key);

/**
 * update first key value in first section
 */
es_bool_t eso_ini_upd_key(es_inifile_t *ini,
                                      const char *section,
                                      const char *key,
                                      const char *value);

/**
 * update key value by key index and section index,
 * index begin 1
 */
es_bool_t eso_ini_upd_key2(es_inifile_t *ini,
                                      const char *section,
                                      es_int32_t idx_sec,
                                      const char *key,
                                      es_int32_t idx_key,
                                      const char *value);

/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_INI_H__ */
