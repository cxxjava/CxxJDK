/*
 * eso_alogger.c
 *
 *  Created on: 2015-1-12
 *      Author: cxxjava@163.com
 */

#include "eso_alogger.h"
#include "eso_string.h"
#include "eso_file.h"
#include "eso_libc.h"

#ifdef WIN32
static int java_to_os_priority[10 + 1] = {
  THREAD_PRIORITY_IDLE,                         // 0  Entry should never be used
  THREAD_PRIORITY_LOWEST,                       // 1  MinPriority
  THREAD_PRIORITY_LOWEST,                       // 2
  THREAD_PRIORITY_BELOW_NORMAL,                 // 3
  THREAD_PRIORITY_BELOW_NORMAL,                 // 4
  THREAD_PRIORITY_NORMAL,                       // 5  NormPriority
  THREAD_PRIORITY_NORMAL,                       // 6
  THREAD_PRIORITY_ABOVE_NORMAL,                 // 7
  THREAD_PRIORITY_ABOVE_NORMAL,                 // 8
  THREAD_PRIORITY_HIGHEST,                      // 9  NearMaxPriority
  THREAD_PRIORITY_TIME_CRITICAL                 // 10 MaxPriority
};
#else //!
static int java_to_os_priority[10 + 1] = {
  19,              // 0 Entry should never be used

   4,              // 1 MinPriority
   3,              // 2
   2,              // 3

   1,              // 4
   0,              // 5 NormPriority
  -1,              // 6

  -2,              // 7
  -3,              // 8
  -4,              // 9 NearMaxPriority

  -5               // 10 MaxPriority
};
#endif

#define THREAD_MIN_PRIORITY 1 //

#define THREAD_DAEMON_STAT_INIT 0
#define THREAD_DAEMON_STAT_STOPING 1
#define THREAD_DAEMON_STAT_STOPED 2

#define LOG_WRITE_RETRYS 3 //write retry times

static void* async_logger_loop(es_thread_t* handle) {
	es_alogger_t* logger = (es_alogger_t*)handle->data;

	es_file_t *logf = eso_fopen(logger->logger_name, "w");

	do {
		es_ulong_t length = 0;

		eso_thread_spin_lock(logger->spin_lock);
		length = eso_ring_buffer_get(logger->ring_buffer, logger->read_buffer, logger->buffer_size);
		eso_thread_spin_unlock(logger->spin_lock);

		if (length > 0) {
			eso_fwrite(logger->read_buffer, length, logf);
			eso_fflush(logf);
		}

		eso_thread_yield(); //?
    } while (!(logger->daemon_stat == THREAD_DAEMON_STAT_STOPING));
	logger->daemon_stat = THREAD_DAEMON_STAT_STOPED;

	eso_fclose(logf);

	return NULL;
}

es_alogger_t* eso_alogger_create(const char* name, es_long_t maxmem) {
	es_threadattr_t attr;
	es_alogger_t* logger = (es_alogger_t*)eso_calloc(sizeof(es_alogger_t));

	logger->logger_name = eso_mstrdup(name);
	logger->spin_lock = eso_thread_spin_create();
	logger->buffer_size = ES_ALIGN_UP(maxmem, SIZEOF_VOID_P);
	logger->ring_buffer = eso_ring_buffer_create(logger->buffer_size, NULL, 0L);
	logger->read_buffer = (es_byte_t*)eso_malloc(logger->buffer_size);

	eso_threadattr_init(&attr);
	eso_threadattr_detach_set(&attr, TRUE);
	logger->daemon_thread = eso_thread_create(&attr, async_logger_loop, logger);
	eso_os_thread_priority_set(logger->daemon_thread->td, java_to_os_priority[THREAD_MIN_PRIORITY]);
	logger->daemon_stat = THREAD_DAEMON_STAT_INIT;

	return logger;
}

void eso_alogger_free(es_alogger_t** logger) {
	if (!logger || !*logger) {
		return;
	}
	(*logger)->daemon_stat = THREAD_DAEMON_STAT_STOPING;
	do {
		eso_thread_sleep(1);
	} while((*logger)->daemon_stat != THREAD_DAEMON_STAT_STOPED);
	eso_thread_destroy(&(*logger)->daemon_thread);
	eso_mfree((*logger)->read_buffer);
	eso_ring_buffer_free(&(*logger)->ring_buffer);
	eso_thread_spin_destroy(&(*logger)->spin_lock);
	eso_mfree((*logger)->logger_name);
	ESO_FREE(logger);
}

void eso_alogger_logfmt(es_alogger_t* logger, const char *fmt, ...) {
	es_string_t* str = NULL;
	va_list args;
	es_int32_t rv = -1;

	va_start(args, fmt);
	rv = eso_mvsprintf(&str, fmt, args);
	va_end(args);

	eso_alogger_logstr(logger, str);

	eso_mfree(str);
}

void eso_alogger_logstr(es_alogger_t* logger, const char *str) {
	int i;

	if (!str || !*str) return;

	for (i=0; i<LOG_WRITE_RETRYS; i++) {
		es_ring_buffer_t *rb = NULL;

		eso_thread_spin_lock(logger->spin_lock);
		rb = eso_ring_buffer_put(logger->ring_buffer, str, eso_strlen(str));
		eso_thread_spin_unlock(logger->spin_lock);

		if (rb) break; //success!
		eso_thread_yield();
	}
}
