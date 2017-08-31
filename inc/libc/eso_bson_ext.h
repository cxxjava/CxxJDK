/**
 * @file  eso_bson_ext.h
 * @brief Binary Serialized Object Notation.
 */

#ifndef __ESO_BSON_EXT_H__
#define __ESO_BSON_EXT_H__

#include "eso_bson.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Make a bson object load from file.
 * @param bson	The bson object.
 * @param fname	The file name.
 * @return ES_SUCCESS or ES_FAILURE,ES_IOERROR
 */
es_status_t eso_bson_load(es_bson_t *bson,
                               const char *fname);

/**
 * Save data to a file from bson object.
 * @param bson	The bson object.
 * @param fname	The file name.
 * @param node	The node path, if NULL then export all, !NULL export only this node tree.
 * @return ES_SUCCESS or ES_FAILURE,ES_IOERROR
 */
es_status_t eso_bson_save(es_bson_t *bson,
                               const char *fname,
                               const char *node);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_BSON_EXT_H__ */
