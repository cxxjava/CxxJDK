/**
 * @file  eso_uuid.h
 * @brief ES UUID Library
 */


#ifndef __ESO_UUID_H__
#define __ESO_UUID_H__

#include "es_comm.h"
#include "es_types.h"
#include "es_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * we represent a UUID as a block of 16 bytes.
 */

typedef struct {
    unsigned char data[16]; /**< the actual UUID */
} es_uuid_t;

/** UUIDs are formatted as: 00112233-4455-6677-8899-AABBCCDDEEFF */
#define ES_UUID_FORMATTED_LENGTH 36


/**
 * Generate and return a (new) UUID
 * @param uuid The resulting UUID
 */ 
void eso_uuid_get(es_uuid_t *uuid);

/**
 * Format a UUID into a string, following the standard format
 * @param buffer The buffer to place the formatted UUID string into. It must
 *               be at least EZ_UUID_FORMATTED_LENGTH + 1 bytes long to hold
 *               the formatted UUID and a null terminator
 * @param uuid The UUID to format
 */ 
void eso_uuid_format(char *buffer, const es_uuid_t *uuid);

/**
 * Parse a standard-format string into a UUID
 * @param uuid The resulting UUID
 * @param uuid_str The formatted UUID
 */ 
es_status_t eso_uuid_parse(es_uuid_t *uuid, const char *uuid_str);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_UUID_H__ */
