/**
 * @file  eso_bon_ext.h
 * @brief Binary Object Notation.
 */

#ifndef __ESO_BON_EXT_H__
#define __ESO_BON_EXT_H__

#include "eso_bon.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Make a bon object load from file.
 * @param bon	The bon object.
 * @param fname	The file name.
 * @return ES_SUCCESS or ES_FAILURE,ES_IOERROR
 */
es_status_t eso_bon_load(es_bon_t *bon,
                               const char *fname);

/**
 * Save data to a file from bon object. 
 * @param bon	The bon object.
 * @param fname	The file name.
 * @param node	The node path, if NULL then export all, !NULL export only this node tree.
 * @return ES_SUCCESS or ES_FAILURE,ES_IOERROR
 */
es_status_t eso_bon_save(es_bon_t *bon,
                               const char *fname,
                               const char *node);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_BON_EXT_H__ */
