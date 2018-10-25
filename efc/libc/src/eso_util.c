/**
 * @file  eso_util.c
 * @brief ES eso util file.
 */

#include "eso_util.h"
#include "eso_libc.h"

char* eso_resname_from_filename(const char *filename, int global, char res_name[], int buf_size)
{
#ifdef WIN32
	char realpath[ES_PATH_MAX] = {0};
	char *pre = global ? "Global\\" : "Local\\";
	es_size_t r = eso_strlen(pre);
	es_size_t n;
	char *filepath, *ch;

	ES_ASSERT(buf_size > 10);

	if (eso_realpath(filename, realpath, sizeof(realpath)) == NULL) {
		eso_strncpy(realpath, filename, sizeof(realpath));
	}
	filepath = realpath;

	n = eso_strlen(filepath) + 1;
	if (n > 256 - r) {
		filepath += n - (256 - r);
		n = 256;
	}

	eso_memcpy(res_name, pre, r);
	eso_memcpy(res_name + r, filepath, ES_MIN(n, buf_size-r));
	res_name[buf_size] = 0;

	for (ch = res_name + r; *ch; ++ch) {
		if (*ch == ':' || *ch == '/' || *ch == '\\' || *ch == '.' || *ch == '..')
			*ch = '_';
	}

	return res_name;
#else
	return NULL;
#endif
}

char* eso_filepath_name_get(const char *pathname)
{
    const char path_separator = '/';
    const char *s = eso_strrchr(pathname, path_separator);

    const char path_separator_win = '\\';
    const char drive_separator_win = ':';
    const char *s2 = eso_strrchr(pathname, path_separator_win);

    if (s2 > s) s = s2;
    if (!s) s = eso_strrchr(pathname, drive_separator_win);

    return (char*)(s ? ++s : pathname);
}

int eso_file_create(const char *filename,
                          es_bool_t overwrite,
                          const void* data, 
                          es_uint32_t size)
{
	es_file_t *pf;
	int ret = 0;
	
	if (!filename || !*filename) {
		return -1;
	}
	
	if (overwrite || (!overwrite && !eso_fexist(filename))) {
		pf = eso_fopen(filename, "wb");
		if (!pf) return -1;
		if (data && size > 0) {
			ret = eso_fwrite(data, size, pf);
		}
		eso_fclose(pf);
		
		return (pf && ret==(int)size) ? 0 : -1;
	}
	return -1;
}

int eso_file_append_by_filename(const char *filename,
                          const void* data,
                          es_uint32_t size)
{
	es_file_t *pf;
	int ret = 0;

	if (!filename || !*filename) {
		return -1;
	}

	pf = eso_fopen(filename, "ab");
	if (!pf) return -1;
	if (data && size > 0) {
		ret = eso_fwrite(data, size, pf);
	}
	eso_fclose(pf);

	return (pf && ret==(int)size) ? 0 : -1;
}

int eso_file_update(es_file_t *pfile,
                           es_int32_t pos, 
                           es_int32_t old_size,
                           es_int32_t new_size,
                           const void *new_data)
{
	es_ssize_t filesize;
	es_uint32_t oldpos;	
	es_int32_t ret;
	
	if (!pfile) return -1;
	
	oldpos = eso_ftell(pfile);  //记录原位置
	filesize = eso_fsize(pfile);
	if (filesize < pos + old_size)
		return -1;
	
	if (new_size == old_size) {
		//等长，直接覆盖
		eso_fseek(pfile, pos, ES_SEEK_SET);
		ret = eso_fwrite(new_data, new_size, pfile);
	}
	else {
		es_int32_t need_move_size;
		
		need_move_size = filesize - (pos + old_size);
		
		if (need_move_size == 0) {
			eso_fseek(pfile, pos, ES_SEEK_SET);
			ret = eso_fwrite(new_data, new_size, pfile);
		}
		else {  //>0
			void *piece_data = eso_malloc(512);
			es_int32_t piece_size = ES_MIN(512, need_move_size);
			es_int32_t shift_size = new_size - old_size;
			es_int32_t total_moved_size;
			
			if (shift_size > 0) {
				total_moved_size = piece_size;
				
				//从后往前复制数据
				while (need_move_size > 0) {
					eso_fseek(pfile, filesize-total_moved_size, ES_SEEK_SET);
					eso_fread(piece_data, piece_size, pfile);
					eso_fseek(pfile, filesize-total_moved_size+shift_size, ES_SEEK_SET);
					eso_fwrite(piece_data, piece_size, pfile);
					
					piece_size = ES_MIN(512, need_move_size);
					
					need_move_size -= piece_size;
					total_moved_size += piece_size;
					
					//eso_fflush(pfile); //立即更新
				}
				
				eso_fseek(pfile, pos, ES_SEEK_SET);
				ret = eso_fwrite(new_data, new_size, pfile);
			}
			else {
				total_moved_size = 0;
				
				//从前往后复制数据
				while (need_move_size > 0) {
					eso_fseek(pfile, pos+old_size+total_moved_size, ES_SEEK_SET);
					eso_fread(piece_data, piece_size, pfile);
					eso_fseek(pfile, pos+new_size+total_moved_size, ES_SEEK_SET);
					eso_fwrite(piece_data, piece_size, pfile);
					
					piece_size = ES_MIN(512, need_move_size);
					
					need_move_size -= piece_size;
					total_moved_size += piece_size;
				}
				
				//文件截短
				ret = eso_ftruncate(pfile, filesize+shift_size);
			}
			
			eso_free(piece_data);
		}
	}
	
	//恢复原位置
	eso_fseek(pfile, oldpos, ES_SEEK_SET);
	return ret;
}

int eso_file_update_by_filename(const char *filename,
                           es_int32_t pos, 
                           es_int32_t old_size,
                           es_int32_t new_size,
                           const void *new_data)
{
	es_file_t *pf;
	es_int32_t ret;
	
	pf = eso_fopen(filename, "r+b");
	if (!pf) return -1;
	
	ret = eso_file_update(pf, pos, old_size, new_size, new_data);
	
	eso_fclose(pf);
	return ret;
}

int eso_file_read(es_file_t *pfile,
                           es_int32_t pos, 
                           es_buffer_t *buffer)
{
	es_ssize_t filesize;
	es_uint32_t oldpos;	
	es_int32_t ret;
	char data[100];
	
	if (!pfile || pos<0 || !buffer) return -1;
	
	oldpos = eso_ftell(pfile);  //记录原位置
	filesize = eso_fsize(pfile);
	if (filesize < pos)
		return -1;
	
	eso_fseek(pfile, pos, ES_SEEK_SET);
	while ((ret = eso_fread(data, sizeof(data), pfile)) > 0) {
		eso_buffer_append(buffer, data, ret);
	}
	
	//恢复原位置
	eso_fseek(pfile, oldpos, ES_SEEK_SET);
	return 0;
}

int eso_file_read_by_filename(const char *filename,
                           es_int32_t pos, 
                           es_buffer_t *buffer)
{
	es_file_t *pf;
	es_int32_t ret;
	
	pf = eso_fopen(filename, "r+b");
	if (!pf) return -1;
	
	ret = eso_file_read(pf, pos, buffer);
	
	eso_fclose(pf);
	return ret;
}

/****************************************************
 * String Check Functions
 ****************************************************/

es_bool_t eso_isdigit_string(char *str)
{
    int ii;
    
    for (ii=0; ii<(int)eso_strlen(str); ++ii)
    {
        if (!eso_isdigit((int)str[ii]))
            return FALSE;
    }
    
    return TRUE;
}

es_bool_t eso_isxdigit_string(char *str)
{
    int ii;
    
    for (ii=0; ii<(int)eso_strlen(str); ++ii)
    {
        if (!eso_isxdigit((int)str[ii]))
            return FALSE;
    }
    
    return TRUE;
}

es_bool_t eso_isalnum_string(char *str)
{
    int ii;
    
    for (ii=0; ii<(int)eso_strlen(str); ++ii)
    {
        if (!eso_isalnum((int)str[ii]))
            return FALSE;
    }
    
    return TRUE;
}

es_bool_t eso_isascii_string(char *str)
{
    int ii;
    
    for (ii=0; ii<(int)eso_strlen(str); ++ii)
    {
        if (!eso_isascii(str[ii]))
            return FALSE;
    }
    
    return TRUE;
}


/****************************************************
 * String Convert Functions
 ****************************************************/

/** @see tolower */
char eso_tolower(unsigned char c)
{
	if ( (unsigned int)(c - 'A') < 26u )
		c += 'a' - 'A';
	return c;
}

/** @see toupper */
char eso_toupper(unsigned char c)
{
	if ( (unsigned int)(c - 'a') < 26u )
		c += 'A' - 'a';
	return c;
}

void eso_to64(char *s, es_uint32_t v, es_int32_t n)
{
    static unsigned char itoa64[] =         /* 0 ... 63 => ASCII - 64 */
    "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    while (--n >= 0) {
        *s++ = itoa64[v&0x3f];
        v >>= 6;
    }
}

/*
 * @see abs
*/
es_int32_t eso_labs(es_int32_t l)
{
	return l>=0?l:-l;
}

/*
 * Convert a string to lower case
 */
char* eso_strlower(char *str)
{
	if (!str) return NULL;
	
	while (*str) {
		*str = eso_tolower(*str);
	}
	
	return str;
}

/*
 * Convert a string to upper case
 */
char* eso_strupper(char *str)
{
	if (!str) return NULL;
	
	while (*str) {
		*str = eso_toupper(*str);
	}
	
	return str;
}


/****************************************************
 * String Operation Functions
 ****************************************************/

/*
 * strrstr
 */
char* eso_strrstr(const char *src, const char *substr)
{
	char *r = NULL;
	char *s = (char*)src;

	if (!substr || !*substr) {
		return s + eso_strlen(s);
	}

	while (1) {
		char *p = eso_strstr(s, substr);
		if (!p)
			return r;
		r = p;
		s = p + 1;
	}
}

/*
 * strrstr by src whith the index to start the search from
 */
char* eso_strmrstr(const char *src, es_int32_t from_index, const char *substr)
{
	int i, l;

	if (from_index < 0 || !substr || !*substr) {
		return NULL;
	}

	l = eso_strlen(src);
	if (from_index > l) {
		from_index = l;
	}

	i = from_index;
	l = eso_strlen(substr);
	for (; i >= 0; i--) {
		if (eso_strncmp(src + i, substr, l) == 0) {
			return src + i;
		}
	}

	return NULL;
}

/*
 * eso_strstr by src string size limit
 */
char* eso_strnstr(const char *src, es_size_t src_size, const char *substr)
{
	char *p;
	
	if (!src)
		return NULL;
	
	p = eso_strstr(src, substr);
	if (!p)
		return NULL;
	if (p - src + eso_strlen(substr) > src_size)
		return NULL;
	
	return p;
}

/*
 * strrchr by src string size limit
 */
char* eso_strnrstr(const char *src, es_int32_t src_size, const char *substr)
 {
	const char* pend = src + src_size;
	const char* pstr;
	int sl = eso_strlen(substr);
	for (pstr = pend - sl; pstr >= src; pstr--) {
		if (!eso_strncmp(pstr, substr, sl))
			return (char*) pstr;
	}
	return NULL ;
}

/*
 * strcasestr by src string size limit
 */
char* eso_strncasestr(const char *src, es_int32_t src_size, const char *substr)
{
   char *c;
   int substr_len;

   c = (char *)src;
   substr_len = eso_strlen(substr);
   while (1) {
      while (*c && (c-src > src_size)) {
         if (eso_tolower(*c) == eso_tolower(*substr)) break;
         c++;
      }
      if (c == NULL) {
         return NULL;
      }
      if (c-src > src_size) {
         return NULL;
      }
      if ((c-src) - src_size > substr_len) {
         return NULL;
      }
      if (!eso_strncasecmp(c, substr, substr_len)) {
         return c;
      }
      c++;
   }
}

/*
 * case-insensitive string comparisons by src string size limit
 */
es_int32_t eso_strncasecmp(const char *p1, const char *p2, es_size_t len)
{
    while (len != 0) {
		if (eso_toupper(*p1) != eso_toupper(*p2)) {
	    	return eso_toupper(*p1) - eso_toupper(*p2);
		}
		if (*p1 == '\0') {
	    	return 0;
		}
		len--; p1++; p2++;
    }
    return 0;
}

/*
 * eso_strchr by src string size limit
 */
char* eso_strnchr(const char *src, es_int32_t src_size, es_int32_t c)
{
	char *p;
	
	if (src_size <= 0)
		return NULL;
	
	p = eso_strchr(src, c);
	if (!p)
		return NULL;
	if ((p - src + 1) > src_size)
		return NULL;
	
	return p;
}

/*
 * strrchr by src whith the index to start the search from
 */
char* eso_strmrchr(const char *src, es_int32_t from_index, es_int32_t c)
{
	int i;

	if (from_index < 0) {
		return NULL;
	}

	i = ES_MIN(from_index, eso_strlen(src) - 1);
	for (; i >= 0; i--) {
		if (src[i] == c) {
			return src + i;
		}
	}
	return NULL;
}

/*
 * strrchr by src string size limit
 */
char* eso_strnrchr(const char *src, es_int32_t src_size, es_int32_t c)
{
	char *s = (char *)src;
	int count = 0;
	
	if (src_size <= 0)
		return NULL;
	
	while (*s++) {                     /* find end of string */
		count++;
		if (count >= src_size)
			break;
	}
	
	/* search towards front */
	while (--s != src && *s != c)
		;
	if (*s == c)                /* char found ? */
		return((char *)s);
	
	return NULL;
}

/*
 * find string in memory block
 */
char* eso_memstr(const char *src, es_int32_t size, const char *substr)
{
	char *p;
	char len = eso_strlen(substr);

	for (p = (char *)src; p <= (src-len+size); p++)
	{
		if (eso_memcmp(p, substr, len) == 0)
			return p; /* found */
	}
	return NULL;
}

/*
 * string replace
*/
char* eso_strreplace(const char *str, char o, char d)
{
	char *p = (char*)str;
	while (p && *p) {
		if (*p == o) *p = d;
		p++;
	}
	return (char*)str;
}

/*
 * string split
*/
char* eso_strsplit(const char *from, const char *separators, es_int32_t pos, 
                                                     char *to, es_int32_t to_size)
{
	char *pstart, *pend, *p;
	es_int32_t i;

	if (!from || !to || to_size < 1) {
		return NULL;
	}
	eso_memset(to, 0, to_size);
	
	if (pos < 1) {
		return NULL;
	}
	
	pstart = (char *)from;
	pend = pstart + eso_strlen(from);

	i = 0;
	while (pstart) {
		i++;
		p = eso_strstr(pstart, separators);
		if (!p) {
			if (i == pos) {
				eso_memcpy(to, pstart, ES_MIN((es_int32_t)eso_strlen(pstart), to_size-1));

				break;
			}
			else
				return NULL;
		}
		if (i == pos) {
			eso_memcpy(to, pstart, ES_MIN(ES_MIN(p-pstart, pend-pstart), to_size-1));

			break;
		}
		pstart = p + eso_strlen(separators);
	}
	
	return to;
}

es_int32_t eso_strcasecmp(const char *a, const char *b)
{
    const char *p = a;
    const char *q = b;
    for (p = a, q = b; *p && *q; p++, q++) 
	{
        int diff = 0;
		char cp = *p, cq = *q;
		if(*p >= 'A' && *p <= 'Z')
			cp = eso_tolower(*p);
		if(*q >= 'A' && *q <= 'Z')
			cq = eso_tolower(*q);
		diff = cp - cq;
        if (diff)
            return diff;
    }
    if (*p)
        return 1;               /* p was longer than q */
    if (*q)
        return -1;              /* p was shorter than q */
    return 0;                   /* Exact match */
}

/*
 * string reverse
 */
void eso_strreverse(char *str)
{
	int n, i;
	char c = '\0';
	
	if (!str) return;
	n = eso_strlen(str);
	
	for (i=0; i<n/2; i++) {
		c = str[i];
		str[i] = str[n-1-i];
		str[n-1-i] = c;
	}
	
	return;
}

/*
 * encode BCD
 * return: >= 0 The Dest Str len.
 *         = -1 Fail.
 * remark: the src_len mast be even
*/
es_int32_t eso_enBCD(char *src, es_int32_t src_len,
                         char *dst, es_int32_t dst_len)
{
	int i;
    int dst_len2;
	
	if (!src || src_len < 0)
		return -1;
	
	dst_len2 = src_len / 2;
	if (dst_len < dst_len2)
		return -1;
	
	eso_memset(dst, 0, dst_len);
    for (i = 0; i < dst_len2; i++) {
        dst[i] = ((src[i * 2] <= 0x39) ? src[i * 2] - 0x30 : src[i * 2] - 0x41 + 10);
        dst[i] = dst[i] << 4;
        dst[i] += ((src[i * 2 + 1] <= 0x39) ? src[i * 2 + 1] - 0x30 : src[i * 2 + 1] - 0x41 + 10);
    }
    
    return dst_len2;
}

/*
 * decode BCD
 * return: >= 0 The Dest Str len.
 *         = -1 Fail.
*/
es_int32_t eso_deBCD(char *src, es_int32_t src_len,
                         char *dst, es_int32_t dst_len)
{
	int i;
    char ch;
    int dst_len2;
	
	if (!src || src_len < 0)
		return -1;
	
	dst_len2 = src_len * 2;
	
	if (dst_len < dst_len2)
		return -1;
	
    for (i = 0; i < src_len; i++) {
        ch = (src[i] & 0xf0) >> 4;
        dst[i * 2] = (ch > 9) ? ch + 0x41 - 10 : ch + 0x30;
        ch = src[i] & 0xf;
        dst[i * 2 + 1] = (ch > 9) ? ch + 0x41 - 10 : ch + 0x30;
    }
    
    return dst_len2;
}

/*
 * 功能: 将10进制整数转换为256进制串
 * 输入: v - 要转换的10进制整数
 *       s - 输出串指针
 *       l - 输出串长度
 * 限制: l 值不能过小，否则转换后的值将超出而被舍弃
 * 输出:
 * 返回: >= 0 成功，返回有效位数， -1 - 失败
 */
int eso_llong2array(es_int64_t v, es_byte_t s[], int l)
{
	int n;

#if defined(DEBUG) || defined(_DEBUG_) || defined(__DEBUG) || defined(DEBUG__) || defined(__DEBUG__)
	{
		es_int64_t max = 1;
		int i;

		n = sizeof(es_int64_t) - l;
		if (n > 0) {
			for (i = 0; i < l; i++) {
				max <<= 8;
			}
			if (v > max) {
				ES_ASSERT(0);
			}
		}
	}
#endif
    
	eso_memset(s, 0, l);
	if (ES_BIG_ENDIAN) {
		v = ES_BSWAP_64(v);
	}
	n = ES_MIN(l, (int)sizeof(es_int64_t));
	eso_memcpy(s, &v, n);

	return n;
}

/*
 * 功能: 将256进制串转换为10进制整数
 * 输入: s - 要转换的源串指针
 *       l - 源串长度
 * 限制: l 值不能过大，否则转换后的值将超出返回值类型的最大值
 * 输出:
 * 返回: >= 0 转换得到的10进制整数，-1 - 失败
 */
es_int64_t eso_array2llong(es_byte_t s[], int l)
{
	es_int64_t r = 0;
	int n, i;
    
	n = sizeof(es_int64_t);
	ES_ASSERT(l <= n);
    
	n = ES_MIN(n, l);
    
	if (!ES_BIG_ENDIAN) {
		eso_memcpy(&r, s, n);
	}
	else {
		for (i = 0; i < n; i++) {
			r <<= 8;
			r |= (es_int64_t) (s[n - i - 1] & 0xFF);
		}
	}
	return r;
}

/**
 * Varint int32 to array.
 * @param value	The int32 value
 * @param target The target array
 * @return The available target data len
 */
es_uint8_t eso_varint32ToArray(es_uint32_t value, es_uint8_t* target)
{
	target[0] = (es_uint8_t)(value | 0x80);
	if (value >= (1 << 7)) {
		target[1] = (es_uint8_t)((value >>  7) | 0x80);
		if (value >= (1 << 14)) {
			target[2] = (es_uint8_t)((value >> 14) | 0x80);
			if (value >= (1 << 21)) {
				target[3] = (es_uint8_t)((value >> 21) | 0x80);
				if (value >= (1 << 28)) {
					target[4] = (es_uint8_t)(value >> 28);
					return 5;
				} else {
					target[3] &= 0x7F;
					return 4;
				}
			} else {
				target[2] &= 0x7F;
				return 3;
			}
		} else {
			target[1] &= 0x7F;
			return 2;
		}
	} else {
		target[0] &= 0x7F;
		return 1;
	}
}

/**
 * Varint array to int32.
 * @param buffer The source array
 * @param value	The result of uint32 value
 * @return The end of available buffer
 */
es_uint8_t* eso_arrayToVarint32(const es_uint8_t* buffer, es_uint32_t* value)
{
	// Fast path:  We have enough bytes left in the buffer to guarantee that
	// this read won't cross the end, so we can skip the checks.
	const es_uint8_t* ptr = buffer;
	es_uint32_t b;
	es_uint32_t result;
	int i;
	
	b = *(ptr++); result  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
	b = *(ptr++); result |=  b         << 28; if (!(b & 0x80)) goto done;
	
	// If the input is larger than 32 bits, we still need to read it all
	// and discard the high-order bits.
	for (i = 0; i < MAX_VARINT64_BYTES - MAX_VARINT32_BYTES; i++) {
		b = *(ptr++); if (!(b & 0x80)) goto done;
	}
	
	// We have overrun the maximum size of a varint (10 bytes).  Assume
	// the data is corrupt.
	return NULL;
	
done:
	*value = result;
	return (es_uint8_t*)ptr;
}

/**
 * Varint int64 to array.
 * @param value	The int64 value
 * @param target The target array
 * @return The available target data len
 */
es_uint8_t eso_varint64ToArray(es_uint64_t value, es_uint8_t* target)
{
	// Splitting into 32-bit pieces gives better performance on 32-bit
	// processors.
	es_uint32_t part0 = (es_uint32_t)(value      );
	es_uint32_t part1 = (es_uint32_t)(value >> 28);
	es_uint32_t part2 = (es_uint32_t)(value >> 56);
	
	int size;
	
	// Here we can't really optimize for small numbers, since the value is
	// split into three parts.  Cheking for numbers < 128, for instance,
	// would require three comparisons, since you'd have to make sure part1
	// and part2 are zero.  However, if the caller is using 64-bit integers,
	// it is likely that they expect the numbers to often be very large, so
	// we probably don't want to optimize for small numbers anyway.  Thus,
	// we end up with a hardcoded binary search tree...
	if (part2 == 0) {
		if (part1 == 0) {
			if (part0 < (1 << 14)) {
				if (part0 < (1 << 7)) {
					size = 1; goto size1;
				} else {
					size = 2; goto size2;
				}
			} else {
				if (part0 < (1 << 21)) {
					size = 3; goto size3;
				} else {
					size = 4; goto size4;
				}
			}
		} else {
			if (part1 < (1 << 14)) {
				if (part1 < (1 << 7)) {
					size = 5; goto size5;
				} else {
					size = 6; goto size6;
				}
			} else {
				if (part1 < (1 << 21)) {
					size = 7; goto size7;
				} else {
					size = 8; goto size8;
				}
			}
		}
	} else {
		if (part2 < (1 << 7)) {
			size = 9; goto size9;
		} else {
			size = 10; goto size10;
		}
	}
	
	//"Can't get here.";
	
	size10: target[9] = (es_uint8_t)((part2 >>  7) | 0x80);
	size9 : target[8] = (es_uint8_t)((part2      ) | 0x80);
	size8 : target[7] = (es_uint8_t)((part1 >> 21) | 0x80);
	size7 : target[6] = (es_uint8_t)((part1 >> 14) | 0x80);
	size6 : target[5] = (es_uint8_t)((part1 >>  7) | 0x80);
	size5 : target[4] = (es_uint8_t)((part1      ) | 0x80);
	size4 : target[3] = (es_uint8_t)((part0 >> 21) | 0x80);
	size3 : target[2] = (es_uint8_t)((part0 >> 14) | 0x80);
	size2 : target[1] = (es_uint8_t)((part0 >>  7) | 0x80);
	size1 : target[0] = (es_uint8_t)((part0      ) | 0x80);
	
	target[size-1] &= 0x7F;
	return size;
}

/**
 * Varint array to int64.
 * @param buffer The source array
 * @param value	The result of uint64 value
 * @return The end of available buffer
 */
es_uint8_t* eso_arrayToVarint64(const es_uint8_t* buffer, es_uint64_t* value)
{
    const es_uint8_t* ptr = buffer;
    es_uint32_t b;

    // Splitting into 32-bit pieces gives better performance on 32-bit
    // processors.
    es_uint32_t part0 = 0, part1 = 0, part2 = 0;

    b = *(ptr++); part0  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
    b = *(ptr++); part0 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
    b = *(ptr++); part0 |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
    b = *(ptr++); part0 |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
    b = *(ptr++); part1  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
    b = *(ptr++); part1 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;
    b = *(ptr++); part1 |= (b & 0x7F) << 14; if (!(b & 0x80)) goto done;
    b = *(ptr++); part1 |= (b & 0x7F) << 21; if (!(b & 0x80)) goto done;
    b = *(ptr++); part2  = (b & 0x7F)      ; if (!(b & 0x80)) goto done;
    b = *(ptr++); part2 |= (b & 0x7F) <<  7; if (!(b & 0x80)) goto done;

    // We have overrun the maximum size of a varint (10 bytes).  The data
    // must be corrupt.
    return NULL;

done:
    *value = ((es_uint64_t)(part0)      ) |
             ((es_uint64_t)(part1) << 28) |
             ((es_uint64_t)(part2) << 56);
    return (es_uint8_t*)ptr;
}

union int_float_bits {
	es_int32_t int_bits;
    float float_bits;
};
union long_double_bits {
	es_int64_t long_bits;
	double double_bits;
};

float eso_intBits2float(es_int32_t x)
{
	union int_float_bits bits;
	bits.int_bits = x;
	return bits.float_bits;
}

es_int32_t eso_float2intBits(float y)
{
	union int_float_bits bits;
	bits.float_bits = y;
	return bits.int_bits;
}

double eso_llongBits2double(es_int64_t x)
{
	union long_double_bits bits;
	bits.long_bits = x;
	return bits.double_bits;
}

es_int64_t eso_double2llongBits(double y)
{
	union long_double_bits bits;
	bits.double_bits = y;
	return bits.long_bits;
}

/* utility function to convert hex character representation to their nibble (4 bit) values */
static ES_INLINE es_uint8_t nibbleFromChar(char c)
{
	if(c >= '0' && c <= '9') return c - '0';
	if(c >= 'a' && c <= 'f') return c - 'a' + 10;
	if(c >= 'A' && c <= 'F') return c - 'A' + 10;
	return 255;
}

/**
 * Convert a string of characters representing a hex buffer into a series of bytes of that real value
 */
es_uint8_t* eso_new_hexstr2bytes(const char *inhex)
{
	es_uint8_t *retval;
	es_uint8_t *p;
	int len, i;

    len = eso_strlen(inhex) / 2;
	retval = eso_malloc(len + 1);
	for (i=0, p = (es_uint8_t*)inhex; i<len; i++) {
		retval[i] = (nibbleFromChar(*p) << 4) | nibbleFromChar(*(p+1));
		p += 2;
	}
    retval[len] = 0;
	return retval;
}

/**
 * Convert a buffer of binary values into a hex string representation
 */
char* eso_new_bytes2hexstr(es_uint8_t *bytes, es_size_t buflen)
{
	static char ByteMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

	char *retval;
	int i;

	retval = eso_malloc(buflen*2 + 1);
	for(i=0; i<buflen; i++) {
		retval[i*2] = ByteMap[bytes[i] >> 4];
		retval[i*2+1] = ByteMap[bytes[i] & 0x0f];
	}
    retval[i*2] = '\0';
	return retval;
}
