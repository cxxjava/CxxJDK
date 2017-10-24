/**
 * @@file  eso_base64.h
 * @@brief ES eso standard base64 encode/decode
 */


#ifndef __ESO_BASE64_H__
#define __ESO_BASE64_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Base64 encode (standard implementation)
 * @limit olen>=(ilen+2)/3 * 4
 * @return >=0	the valid encoding data length
 *         -1	failure
 */
int eso_base64_encode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen);

/**
 * Base64 decode (standard implementation)
 * @limit olen>=ilen/4 * 3
 * @return >=0	the valid decoding data length
 *        -1	failure
 */
int eso_base64_decode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen);

/*
 * XBase64 encode (non standard implementation)
 * @limit olen>=(ilen+2)/3 * 4
 * @return >=0	the valid encoding data length
 *        -1	failure
 */
int eso_xbase64_encode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen);

/*
 * XBase64 decode (non standard implementation)
 * @limit olen>=ilen/4 * 3
 * @return >=0	the valid decoding data length
 *        -1	failure
 */
int eso_xbase64_decode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_BASE64_H__ */
