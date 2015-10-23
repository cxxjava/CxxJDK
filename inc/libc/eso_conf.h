/**
 * @file  eso_conf.h
 * @brief ES Config Routines
 */


#ifndef __ESO_CONF_H__
#define __ESO_CONF_H__

#include "eso_bon.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * Load config file
 */
es_bool_t eso_conf_load(es_bon_t *bon,
                        const char *fname);


#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_CONF_H__ */
