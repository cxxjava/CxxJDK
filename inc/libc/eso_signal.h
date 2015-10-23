/**
 * @file  eso_signal.c
 * @brief EZ Signal Handling
 */

#ifndef __ESO_SIGNAL_H__
#define __ESO_SIGNAL_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Call installs a new signal handler for the signal with number signum.
 */
es_status_t eso_signal(int signum, void(*handler)(int sig));

/**
 * Get the description for a specific signal number
 * @param signum The signal number
 * @return The description of the signal
 */
const char* eso_signal_description(int signum);

/**
 * Block the delivery of a particular signal
 * @param signum The signal number
 * @return status
 */
es_status_t eso_signal_block(int signum);

/**
 * Enable the delivery of a particular signal
 * @param signum The signal number
 * @return status
 */
es_status_t eso_signal_unblock(int signum);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESO_SIGNAL_H__ */
