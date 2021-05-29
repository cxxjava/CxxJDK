/*
 * EBase64.cpp
 *
 *  Created on: 2013-7-17
 *      Author: cxxjava@163.com
 */

#include "EBase64.hh"

namespace efc {

EString EBase64::byteArrayToBase64(byte* a, int len) {
	ES_ASSERT(a);

	int olen = (len + 2) / 3 * 4;
	es_uint8_t *out = (es_uint8_t*)eso_calloc(olen);
	eso_base64_encode((es_uint8_t*)a, len, out, olen);
	EString result((char*)out);
	eso_free(out);
	return result;
}

EString EBase64::encodeBase64(byte* a, int len) {
	return byteArrayToBase64(a, len);
}

void EBase64::base64ToByteArray(EByteBuffer* out, const char* s)
{
	ES_ASSERT(out); ES_ASSERT(s);

	int sLen = (int)eso_strlen(s);
	int numGroups = sLen / 4;
	if (4 * numGroups != sLen)
		throw EIllegalArgumentException(__FILE__, __LINE__,
				"String length must be a multiple of four.");
	int oLen = sLen / 4 * 3;
	es_uint8_t *bOut = (es_uint8_t*)eso_calloc(oLen);
	int rv = eso_base64_decode((es_uint8_t*)s, sLen, bOut, oLen);
	if (rv >= 0) {
		out->append(bOut, rv);
		eso_free(bOut);
	} else {
		eso_free(bOut);
		throw EIllegalArgumentException(__FILE__, __LINE__, "String is not in the Base64 Alphabet.");
	}
}

EA<byte> EBase64::decodeBase64(const char* s)
 {
	ES_ASSERT(s);

 	int sLen = (int)eso_strlen(s);
 	int numGroups = sLen / 4;
 	if (4 * numGroups != sLen)
 		throw EIllegalArgumentException(__FILE__, __LINE__,
 				"String length must be a multiple of four.");
 	int oLen = sLen / 4 * 3;
	es_uint8_t *bOut = (es_uint8_t*)eso_malloc(oLen);
 	int rv = eso_base64_decode((es_uint8_t*)s, sLen, bOut, oLen);
 	if (rv >= 0) {
 		return EA<byte>((byte*)bOut, rv, true, MEM_MALLOC);
 	} else {
 		eso_free(bOut);
 		throw EIllegalArgumentException(__FILE__, __LINE__, "String is not in the Base64 Alphabet.");
 	}
 }


} /* namespace efc */
