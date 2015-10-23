/**
 * @file  eso_printf_upper_bound.h
 * @brief ES calculate printf upper bound
 */

#ifndef __ESO_PRINTF_UPPER_BOUND_H__
#define __ESO_PRINTF_UPPER_BOUND_H__

#include "es_types.h"
#include "es_comm.h"
#include "eso_libc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MAX_INT2STR_LEN ((sizeof(es_uint32_t) * CHAR_BIT + 2) / 3 + 1)

/* Returns the maximum length of given format string when expanded.
*  If the format is invalid, i_fatal() is called.
*/
es_size_t eso_printf_upper_bound(const char *format, va_list args);


#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_PRINTF_UPPER_BOUND_H__ */

