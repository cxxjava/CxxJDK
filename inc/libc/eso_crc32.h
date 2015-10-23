/**
 * @file  eso_crc32.h
 * @brief CRC-32 Calculation
 */

#ifndef __ESO_CRC32_H__
#define __ESO_CRC32_H__


#include "es_comm.h"
#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * CRC-32 routine
 */
es_uint32_t eso_crc32_calc(es_byte_t *p, es_size_t len);


/**
 * CRC-32 advanced routine
 */
#define eso_crc32_init(crc)                                                   \
    crc = 0xffffffff

void eso_crc32_update(es_uint32_t *crc, es_byte_t *p, es_size_t len);

#define eso_crc32_final(crc)                                                  \
    crc ^= 0xffffffff


#ifdef __cplusplus
}
#endif

#endif /* __ESO_CRC32_H__ */
