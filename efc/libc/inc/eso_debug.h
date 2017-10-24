/**
 * @file  eso_debug.h
 * @brief ES eso debug define
 */

#ifndef __ESO_DEBUG_H__
#define __ESO_DEBUG_H__

#include "eso_libc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __DEBUG

extern es_uint32_t g_tickcount_last;

#define DBG_PRINTF(a) \
		do {		\
			eso_log a;	\
		}while(0)

#define	QUICK_DBG \
		do { \
			es_uint32_t tickcount_curr = eso_get_tick_count(); \
			DBG_PRINTF(("FILE:%s @LINE: %d @TICK: %d", __FILE__,__LINE__, tickcount_curr - g_tickcount_last)); \
			g_tickcount_last = tickcount_curr; \
		}while(0)

#else

#define DBG_PRINTF(a)
#define	QUICK_DBG

#endif //!__DEBUG

#ifdef __cplusplus
}
#endif

#endif /* __ESO_DEBUG_H__ */
