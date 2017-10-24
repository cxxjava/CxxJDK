/**
 * @file  eso_alogger.h
 * @brief ES async logger.
 */

#ifndef ESO_ALOGGER_H_
#define ESO_ALOGGER_H_

#include "eso_thread.h"
#include "eso_thread_spin.h"
#include "eso_ring_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_alogger_t es_alogger_t;
struct es_alogger_t {
	es_string_t *logger_name;
	es_thread_t *daemon_thread;
	es_int32_t daemon_stat;

	es_thread_spin_t *spin_lock;
	es_size_t buffer_size;
	es_ring_buffer_t *ring_buffer;
	es_byte_t *read_buffer;
};

es_alogger_t* eso_alogger_create(const char* name, es_long_t maxmem);

void eso_alogger_free(es_alogger_t** logger);

void eso_alogger_logfmt(es_alogger_t* logger, const char *fmt, ...);

void eso_alogger_logstr(es_alogger_t* logger, const char *str);

#ifdef __cplusplus
}
#endif

#endif /* ESO_ALOGGER_H_ */
