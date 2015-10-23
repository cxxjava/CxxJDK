/**
 * @file  eso_sys.h
 * @brief ES System Assistant.
 */

#ifndef __ESO_SYS_H__
#define __ESO_SYS_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Setup any ESO internal data structures.  This MUST be the first function 
 * called for any ESO library. It is safe to call eso_initialize several
 * times as long as eso_terminate is called the same number of times.
 */
es_status_t eso_initialize(void);

/**
 * Tear down any ESO internal data structures which aren't torn down 
 * automatically. eso_terminate must be called once for every call to
 * eso_initialize().
 */
void eso_terminate(void);

/**
 * Get wordsize is 32 or 64.
 */
int eso_wordsize_get(void);

/**
 * Check big endian or little endian.
 */
es_bool_t eso_is_bigendian(void);

/**
 * Get backtrace information.
 */
char* eso_backtrace_get(es_string_t **trace);

/**
 * Get the program current work path.
 */
char* eso_current_workpath(es_string_t **path);

/**
 * Get the program execute's file name.
 */
char* eso_execute_filename(es_string_t **path);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_SYS_H__ */
