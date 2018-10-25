/**
 * @file  eso_encode.c
 * @brief ES character encode.
 */
#include "eso_encode.h"
#include "eso_libc.h"
#include "eso_mem.h"

//==============================================================================

#define ES_UCS2_ENCODING_LENGTH       2

//==============================================================================

static const unsigned char widget_utf8_bytes_per_character[16] = 
{
  1, 1, 1, 1, 1, 1, 1, 1,
  0, 0, 0, 0, 2, 2, 3, 0 /* we don't support UCS4 */
};

static es_int32_t ucs2_to_utf8_len(es_uint16_t ucs2)
{
	if (ucs2 < 0x80)
		return 1;
	else if (ucs2 < 0x800)
		return 2;
	else
		return 3;
}

static es_uint8_t unicode_to_ucs2(es_uint16_t unicode, es_uint8_t *charLength, es_uint8_t *arrOut)
{
    es_uint8_t status = 1;
    es_uint8_t index = 0;

    if (arrOut != 0) {
        if (unicode < 256) {
            arrOut[index++] = *((es_uint8_t*) (&unicode));
            arrOut[index] = 0;
        }
        else {
            arrOut[index++] = *((es_uint8_t*) (&unicode));
            arrOut[index] = *(((es_uint8_t*) (&unicode)) + 1);
        }
        *charLength = 2;
    }
    else {
        status = 0;
    }

    return status;
}

//==============================================================================

/**
 * 功能：测量utf8字符数
 * @param s	utf8编码的字符串
 * @return utf8字符数
 */
es_int32_t eso_utf8_strlen(const char *str)
{
	int count = 0;
	int rawlen;
	int d;
	es_uint8_t *s = (es_uint8_t*)str;

	if (s == 0) return 0;

	rawlen = eso_strlen(str);

	while (*s != '\0' && rawlen>0) {
		d = widget_utf8_bytes_per_character[*s >> 4];
		if(d==0) 
			return count;
		rawlen -= d;
		s += d;

		count++;
	}
	
	return count;
}

/**
 * 功能：测量utf8字节数
 * @param s	utf8编码的字符串
 * @param n utf8编码的字符数
 * @return utf8有效字节数
 */
es_int32_t eso_utf8_bytes(const char *str, es_int32_t n)
{
	int count = 0;
	int rawlen;
	int d;
	es_uint8_t *s = (es_uint8_t*)str;
	
	if (s == 0 || n <= 0) return 0;
	
	rawlen = eso_strlen(str);
	
	while (*s != '\0' && rawlen>0 && n>0) {
		d = widget_utf8_bytes_per_character[*s >> 4];
		if (d==0) {
			return count;
		}
		rawlen -= d;
		s += d;
		count += d;
		n--;
	}
	
	return count;
}

/**
 * 功能：测量ucs2字符数
 * @param s	ucs2编码的字符串
 * @return ucs2字符数
 */
es_int32_t eso_ucs2_strlen(const char* ucs2str)
{
    es_int32_t nCount = 0;
    es_int32_t nLength = 0;
    char *s = (char*)ucs2str;

    if (s) {
        while (s[nCount] != 0 || s[nCount + 1] != 0) {
            ++nLength;
            nCount += 2;
        }
    }
    return nLength;
}

/**
 * 功能：转换utf8字符到ucs2字符
 * @param ucs2	ucs2编码的字符
 * @param utf8	utf8编码的字符
 * @return utf8字符字节数
 */
es_int32_t eso_utf8_to_ucs2(es_uint16_t *ucs2, es_uint8_t *utf8)
{
	unsigned char c = utf8[0];

	if (c < 0x80) {
		*ucs2 = c;
		return 1;
	} 
	else if (c < 0xe0) {
		*ucs2 = ((es_uint16_t) (c & 0x1f) << 6) | (es_uint16_t) (utf8[1] ^ 0x80);
		return 2;
	} 
	else {
	    *ucs2 =	  ((es_uint16_t) (c & 0x0f) << 12)
				| ((es_uint16_t) (utf8[1] ^ 0x80) << 6)
				|  (es_uint16_t) (utf8[2] ^ 0x80);
		return 3;
	}
}

/**
 * 功能：转换ucs2字符到utf8字符
 * @param utf8	utf8编码的字符
 * @param ucs2	ucs2编码的字符
 * @return utf8字符字节数
 */
es_int32_t eso_ucs2_to_utf8(es_uint8_t *utf8, es_uint16_t ucs2)
{
	int count;
	
	if (ucs2 < 0x80)
	  	count = 1;
	else if (ucs2 < 0x800)
		count = 2;
	else
		count = 3;
	
	switch (count) {
		case 3: utf8[2] = 0x80 | (ucs2 & 0x3f); ucs2 = ucs2 >> 6; ucs2 |= 0x800;
		case 2: utf8[1] = 0x80 | (ucs2 & 0x3f); ucs2 = ucs2 >> 6; ucs2 |= 0xc0;
		case 1: utf8[0] = (es_uint8_t)ucs2;
	}
	return count;
}

/**
 * 功能：转换utf8字符串到ucs2字符串
 * @param dest	ucs2输出串
 * @param src	utf8输入串
 * @return ucs2有效字节数，不包括'\0\0'
 */
es_int32_t eso_utf8_to_ucs2_string(es_wstring_t **dest,
                                   const char *utf8str)
{
	int pos = 0;
	int max_ucs2_size;
	es_uint8_t *local_dest;
	char *src = (char*)utf8str;
	
	max_ucs2_size = eso_utf8_strlen(src)*2 + 2;
	*dest = (es_wstring_t *)eso_mmalloc(max_ucs2_size);
	local_dest = (es_uint8_t *)(*dest);
	
	while(*src)	{
		src += eso_utf8_to_ucs2((es_uint16_t*)(local_dest+pos),(es_uint8_t*)src);
		pos += 2;
		if(pos>=max_ucs2_size-2) break;
	}
	local_dest[pos] = 0;
	local_dest[pos+1] = 0;
	return pos;
}

/**
 * 功能：转换ucs2字符串到utf8字符串
 * @param dest	utf8输出串
 * @param src	ucs2输入串
 * @return utf8有效字节数，不包括'\0'
 */
es_int32_t eso_ucs2_to_utf8_string(es_string_t **dest,
                                   const char *ucs2src)
{
	es_int32_t pos = 0;
	int max_utf8_size;
	es_uint8_t *local_dest;
	es_uint16_t *src = (es_uint16_t*)ucs2src;
	
	max_utf8_size = eso_ucs2_strlen(ucs2src)*2*3/2 + 2;
	*dest = (es_string_t*)eso_mmalloc(max_utf8_size);
	local_dest = (es_uint8_t*)*dest;
	
	while (*src) {
		if (pos + ucs2_to_utf8_len(*src) >= max_utf8_size) // leave space of '\0'
		{
			break;
		}
		
		pos += eso_ucs2_to_utf8(local_dest+pos,(*src));
		src += 1;
		if(pos>=max_utf8_size-1) break;
	}
	if (pos >= max_utf8_size) {
		local_dest[max_utf8_size-1] = 0;
	} else
	    local_dest[pos] = 0;
	return pos+1;
}

es_int32_t eso_ucs2_strcmp(const char *ucs2str1, const char *ucs2str2)
{
    es_uint16_t nStr1;
    es_uint16_t nStr2;

    while ((*ucs2str1 == *ucs2str2) && (*(ucs2str1 + 1) == *(ucs2str2 + 1))) {
        if ((*ucs2str1 == 0) && (*(ucs2str1 + 1) == 0)) {
            return 0;
        }
		
        ucs2str1 += 2;
        ucs2str2 += 2;
    }
	
    nStr1 = (ucs2str1[1] << 8) | (es_uint8_t)ucs2str1[0];
    nStr2 = (ucs2str2[1] << 8) | (es_uint8_t)ucs2str2[0];
	
    return (es_int32_t)(nStr1 - nStr2);
}

char *eso_ucs2_strcpy(char *ucs2dst, const char *ucs2src)
{
    int count = 1;
    char *temp = ucs2dst;
	
    if (ucs2src == NULL) {
        if (ucs2dst) {
            *(ucs2dst + count - 1) = '\0';
            *(ucs2dst + count) = '\0';
        }
        return temp;
    }
	
    if (!ucs2dst || !ucs2src) {
        return NULL;
    }
    while (!((*(ucs2src + count) == 0) && (*(ucs2src + count - 1) == 0))) {
        *(ucs2dst + count - 1) = *(ucs2src + count - 1);
        *(ucs2dst + count) = *(ucs2src + count);
        count += 2;
    }
	
    *(ucs2dst + count - 1) = '\0';
    *(ucs2dst + count) = '\0';
	
    return temp;
}

es_int32_t eso_ucs2_strncmp(const char *ucs2str1, const char *ucs2str2, es_uint32_t size)
{
    es_uint16_t count = 0;
    es_uint16_t nStr1;
    es_uint16_t nStr2;
	
    size = size << 1;   /* User is passing no of charcters not bytes */
	
    while (count < size) {
        nStr1 = (ucs2str1[1] << 8) | (es_uint8_t)ucs2str1[0];
        nStr2 = (ucs2str2[1] << 8) | (es_uint8_t)ucs2str2[0];
        if (nStr1 == 0 || nStr2 == 0 || nStr1 != nStr2) {
            return nStr1 - nStr2;
        }
        ucs2str1 += 2;
        ucs2str2 += 2;
        count += 2;
    }
    return 0;
}

char* eso_ucs2_strncpy(char *ucs2dst, const char *ucs2src, es_uint32_t size)
{
    es_uint16_t count = 1;
    es_uint32_t count1 = 0;
    char *temp = ucs2dst;
	
    size = size * 2;
	
    while (!((*(ucs2src + count) == 0) && (*(ucs2src + count - 1) == 0)) && (count1 < size))
    {
        *(ucs2dst + count - 1) = *(ucs2src + count - 1);
        *(ucs2dst + count) = *(ucs2src + count);
        count += 2;
        count1 += 2;
    }
	
    *(ucs2dst + count - 1) = '\0';
    *(ucs2dst + count) = '\0';
	
    return temp;
}

char* eso_ucs2_strcat(char *ucs2dst, const char *ucs2src)
{
    char *dest = ucs2dst;
    dest = dest + eso_ucs2_strlen(ucs2dst) * 2;
    eso_ucs2_strcpy(dest, ucs2src);
    return ucs2dst;
}

char* eso_ucs2_strncat(char *ucs2dst, const char *ucs2src, es_uint32_t size)
{
    char *dest = ucs2dst;
    dest = dest + eso_ucs2_strlen(ucs2dst) * 2;
    eso_ucs2_strncpy(dest, ucs2src, size);
    return ucs2dst;
}

es_uint16_t eso_asc_str_to_ucs2_str(char *ucs2str, char *ascstr)
{
    es_int16_t count = -1;
    es_uint8_t charLen = 0;
    es_uint8_t arrOut[2];

    while (*ascstr != '\0') {
        unicode_to_ucs2((es_uint16_t) (*((es_uint8_t *)ascstr)), &charLen, arrOut);

        ucs2str[++count] = arrOut[0];
        ucs2str[++count] = arrOut[1];
        ascstr++;
    }

    ucs2str[++count] = '\0';
    ucs2str[++count] = '\0';
    return count + 1;
}

es_uint16_t eso_asc_str_n_to_ucs2_str(char *ucs2str, char *ascstr, es_uint32_t len)
{
    es_int16_t count = -1;
    es_uint8_t charLen = 0;
    es_uint8_t arrOut[2];

    while (len) {
        unicode_to_ucs2((es_uint16_t) (*((es_uint8_t *)ascstr)), &charLen, arrOut);
        
        ucs2str[++count] = arrOut[0];
        ucs2str[++count] = arrOut[1];
		
        if (*ascstr == '\0') {
            break;
        }
        else {
            len--;
            ascstr++;
        }
    }
	
    return count + 1;
}

es_uint16_t eso_ucs2_str_to_asc_str(char *ascstr, char *ucs2str)
{
	es_uint16_t count = 0;
	
	while (!((*ucs2str == 0) && (*(ucs2str + 1) == 0))) {
        *ascstr = *(ucs2str);
		
        ucs2str += 2;
        ascstr++;
        count++;
    }
	
    *ascstr = 0;
    return count;
}

es_uint16_t eso_ucs2_str_n_to_asc_str(char *ascstr, char *ucs2str, es_uint32_t len)
{
    es_uint16_t count = 0;

    while ((len) && (!((*ucs2str == 0) && (*(ucs2str + 1) == 0)))) {
        *ascstr = *(ucs2str);
		
        ucs2str += 2;
        ascstr++;
        count++;
        len -= 2;
    }
	
    return count;
}
