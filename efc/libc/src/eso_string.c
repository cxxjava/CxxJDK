/**
 * @file  eso_string.c
 * @brief ES es_string_t/es_data_t Operation Functions
 */

#include "eso_string.h"
#include "eso_libc.h"
#include "eso_mem.h"
#include "eso_printf_upper_bound.h"

/**
 * All possible chars for representing a number as a String
 */
static const char digits[] = {
	'0' , '1' , '2' , '3' , '4' , '5' ,
	'6' , '7' , '8' , '9' , 'a' , 'b' ,
	'c' , 'd' , 'e' , 'f' , 'g' , 'h' ,
	'i' , 'j' , 'k' , 'l' , 'm' , 'n' ,
	'o' , 'p' , 'q' , 'r' , 's' , 't' ,
	'u' , 'v' , 'w' , 'x' , 'y' , 'z'
};

/*
 * string dup
*/
es_string_t* eso_mstrdup(const char *str)
{
	es_string_t *to = NULL;
	es_size_t len;
	
	if (!str)
		return NULL;
	
	len = eso_strlen(str);
	
	to = eso_mmalloc(len + 1);
	if (to) {
		eso_strncpy(to, str, eso_mnode_size(to));
	}
	return to;
}


/*
 * string dup n len
*/
es_string_t* eso_mstrndup(const char *str, es_size_t len)
{
	es_string_t *to = NULL;
	
	if (!str)
		return NULL;
	
	to = eso_mmalloc(len + 1);
	if (to) {
		eso_memcpy(to, str, len);
		to[len] = '\0';
	}
	return to;
}

/*
 * string copy
*/
es_string_t* eso_mstrcpy(es_string_t **to, const char *str)
{
	es_size_t str_len, dst_len;
	
	if (!to)
		return NULL;
	
	str_len = eso_strlen(str);
	dst_len = str_len + 1;
	
	if (!*to) {
		*to = eso_mmalloc(dst_len);
	}
	else if (str_len >= eso_mnode_size(*to)) {
		*to = eso_mrealloc(*to, dst_len);
	}

	if (*to) {
		eso_strncpy((*to), str, eso_mnode_size(*to));
		return (*to);
	}
	else
		return NULL;
}

/*
 * string copy n len
*/
es_string_t* eso_mstrncpy(es_string_t **to, 
                          const char *str, es_size_t len)
{
	es_size_t str_len, dst_len;
	
	if (!to)
		return NULL;
	
	str_len = len;
	dst_len = str_len + 1;
	
	if (!*to) {
		*to = eso_mmalloc(dst_len);
	}
	else if (str_len >= eso_mnode_size(*to)) {
		*to = eso_mrealloc(*to, dst_len);
	}

	if (*to) {
		eso_strncpy((*to), str, str_len);
		(*to)[str_len] = 0;
		return (*to);
	}
	else
		return NULL;
}

/*
 * string cat
*/
es_string_t* eso_mstrcat(es_string_t **to, const char *str)
{
	es_size_t oldstr_len = 0;
	es_size_t str_len;
	
	if (!to)
		return NULL;
	
	str_len = eso_strlen(str);

	if (!*to) {
		*to = eso_mmalloc(str_len+1);
	}
	else {
		oldstr_len = eso_strlen((*to));
	
		if ((oldstr_len + str_len) >= eso_mnode_size(*to)) {
			*to = eso_mrealloc(*to, oldstr_len+str_len+1);
		}
	}

	if (*to) {
		/** it's too slow!!!*/
		//eso_strcat((*to), str);
		eso_memcpy((*to) + oldstr_len, str, str_len + 1);
		return (*to);
	}
	else
		return NULL;	
}

/*
 * string cat with n len
*/
es_string_t* eso_mstrncat(es_string_t **to, 
                          const char *str, es_size_t len)
{
	es_size_t oldstr_len, str_len;

	if (!to)
		return NULL;

	str_len = ES_MIN(eso_strlen(str), len);

	if (!*to) {
		*to = eso_mmalloc(str_len+1);
	}
	else {
		oldstr_len = eso_strlen((*to));
	
		if ((oldstr_len + str_len) >= eso_mnode_size(*to)) {
			*to = eso_mrealloc(*to, oldstr_len+str_len+1);
		}
	}

	if (*to) {
		eso_strncat((*to), str, str_len);
		return (*to);
	}
	else
		return NULL;
}

/*
 * string vsprintf
*/
es_int32_t eso_mvsprintf(es_string_t **to, 
                         const char *fmt, va_list args)
{
	es_size_t dst_len;
	es_int32_t printf_upper;
	es_int32_t rv = -1;
	va_list args1, args2;
	
#ifdef va_copy
	va_copy(args1, args);
	va_copy(args2, args);
#else
	args1 = args;
	args2 = args;
#endif

	printf_upper = eso_printf_upper_bound(fmt, args1);

	dst_len = printf_upper + 1;

	if (!*to) {
		*to = eso_mmalloc(printf_upper+1);
	}
	else if (dst_len > eso_mnode_size(*to)) {
		*to = eso_mrealloc(*to, dst_len);
	}

	if (*to) {
		/* can't use it!!! maybe args have some bug at arm.
		rv = es_vsnprintf((char *)(*to), dst_len, fmt, args2);
		*/
		rv = eso_vsprintf((char *)(*to), fmt, args2);
	}

#ifdef va_copy
	va_end(args1);
	va_end(args2);
#endif
	
	return rv;
}

/*
 * string sprintf
*/
es_int32_t eso_msprintf(es_string_t **to, const char *fmt, ...)
{
	va_list args;
	es_int32_t rv = -1;

	va_start(args, fmt);
	rv = eso_mvsprintf(to, fmt, args);
	va_end(args);
	
	return rv;
}

/*
 * create a string and zero it
*/
void* eso_mmeminit(es_data_t **to)
{
	if (!to)
		return NULL;
	
	if (!*to) {
		*to = eso_mcalloc(1);
	}
	else {
		eso_mmemfill(*to, '\0');
	}
	
	return *to;
}

/*
 * string fill with a char
*/
void* eso_mmemfill(es_data_t *data, unsigned char c)
{
	if (!data)
		return NULL;
	eso_memset(data, c, eso_mnode_size(data));
	return data;
}

/*
 * string memset
*/
void* eso_mmemset(es_data_t *data, es_size_t offset, 
                  unsigned char c, es_size_t size)
{
	es_size_t set_len;
	
	if (!data)
		return NULL;
	
	if (offset >= eso_mnode_size(data)) {
		return data;
	}

	set_len = ES_MIN(eso_mnode_size(data)-offset, size);

	eso_memset((char *)data+offset, c , set_len);

	return data;
}

/*
 * string memcpy
*/
void* eso_mmemcpy(es_data_t **dest, es_size_t offset, 
                  const void *src, es_size_t size)
{
	if (!dest)
		return NULL;
	
	if (!*dest) {
		*dest = eso_mmalloc(offset+size);
	}
	else if (offset + size > eso_mnode_size(*dest)) {
		*dest = eso_mrealloc(*dest, offset+size);
	}
	if (*dest) {
		eso_memcpy((char *)(*dest)+offset, src, size);
		return (*dest);
	}
	else
		return NULL;
}

/*
 * string memmove
*/
void* eso_mmemmove(es_data_t **dest, es_size_t offset, 
                   const void *src, es_size_t size)
{
	char *p = (char *)src;
	es_ssize_t old_size;
	
	if (!dest || !*dest)
		return NULL;
	
	old_size = eso_mnode_size(*dest); 
	
	if (offset + size > eso_mnode_size(*dest)) {
		es_ssize_t n = (char *)src - (char *)(*dest);
		*dest = eso_mrealloc(*dest, offset+size);
		if (n >= 0 && n <= old_size)
			p = (char *)(*dest) + n;
	}
	if (*dest) {
		eso_memmove((char *)(*dest)+offset, p, size);
		return (*dest);
	}
	else
		return NULL;
}

/*
 * memmory dup
*/
es_data_t* eso_mmemdup(const void *m, es_size_t n)
{
	es_data_t *data = NULL;

	if (m == NULL)
		return NULL;

	data = eso_mmalloc(n);
	if (data) {
		eso_memcpy(data, m, n);
	}
	return data;
}

/*
 * string insert
*/
es_string_t* eso_mstrinsert(es_string_t **string, 
                            es_size_t pos, const char *str)
{
	es_size_t old_str_len;
	es_size_t str_len;

	old_str_len = eso_strlen((*string));
	if (pos > old_str_len) {
		return (*string);
	}
	str_len = eso_strlen(str);

	eso_mmemmove((es_data_t**)string, pos+str_len,
						(*string)+pos, old_str_len-pos+1/*'\0'*/);
	if (!*string) {
		return NULL;
	}
	eso_memcpy((*string)+pos, str, str_len);

	return (*string);
}

/*
 * string insert with n len
*/
es_string_t* eso_mstrninsert(es_string_t **string, 
                             es_size_t pos, 
                             const char *str, es_size_t len)
{
	es_size_t old_str_len;
	es_size_t str_len;

	old_str_len = eso_strlen((*string));
	if (pos > old_str_len) {
		return (*string);
	}
	str_len = ES_MIN(eso_strlen(str), len);

	eso_mmemmove((es_data_t**)string, pos+str_len,
						(*string)+pos, old_str_len-pos+1/*'\0'*/);
	if (!*string) {
		return NULL;
	}
	eso_memcpy((*string)+pos, str, str_len);

	return (*string);
}

/*
 * string delete with n len
*/
es_string_t* eso_mstrdelete(es_string_t *string, es_size_t pos, es_size_t len)
{
	es_ssize_t old_str_len;
	es_ssize_t move_str_len;

	old_str_len = eso_strlen(string);
	if (pos >= (es_size_t)old_str_len) {
		return string;
	}
	move_str_len = old_str_len - pos - len;
	if (move_str_len <= 0) {
		string[pos] = 0x00;

		return string;
	}
	eso_memmove(string+pos, string+pos+len, move_str_len);
	string[pos + move_str_len] = 0x00;

	return string;
}

/*
 * string replace
*/
es_string_t* eso_mstrreplace(es_string_t **string, 
                             const char *from_str, const char *to_str)
{
 	return eso_mstrreplace_off(string, 0, eso_strlen((*string)), from_str, to_str);
}

/*
 * string replace by offset
*/
es_string_t* eso_mstrreplace_off(es_string_t **string, 
                                 es_size_t offset, es_size_t size, 
                                 const char *from_str, const char *to_str)
{
	es_size_t src_str_len;
	es_size_t from_str_len;
	es_size_t to_str_len;
	es_size_t shift;
	es_size_t offset_end;
	char *start, *p;
	
	src_str_len = eso_strlen((*string));
	from_str_len = eso_strlen(from_str);
	to_str_len = eso_strlen(to_str);
	
	if (offset > src_str_len - from_str_len
		|| size < from_str_len) {
		return (*string);
	}
	
	if (size > src_str_len - offset) {
		size = src_str_len - offset;
	}
	
	start = (*string) + offset;
	offset_end = offset + size;
	shift = to_str_len - from_str_len;
	while (NULL != (p = eso_strstr(start, from_str))) {
		/**< size area control */
		if (p > (*string) + offset_end - from_str_len)
			break;

		offset_end += shift;
		if (from_str_len == to_str_len) {
			/*from_str len == to_str len*/
			eso_memcpy(p, to_str, to_str_len);
			start = p + to_str_len;
		}
		else {
			es_size_t p_shift = p - (*string);
			
			eso_mmemmove((es_data_t**)string, p - (*string) + to_str_len, 
			                     p + from_str_len, eso_strlen(p + from_str_len) + 1);
			if (!*string)
				return NULL;
			
			/* reset point's based on string point!!! */
			p = (*string) + p_shift;
			
			eso_memcpy(p, to_str, to_str_len);
			start = p + to_str_len;
		}
	}
	
	if (*string)
		return (*string);
	else
		return NULL;
}

/*
 * string split
*/
es_string_t* eso_mstrsplit(char *from, const char *separators,
                           es_size_t pos, es_string_t **to)
{
	char *pstart, *pend, *p;
	es_uint32_t i;

	if (!from) {
		return NULL;
	}

	/**< pos value from 1 is valid */
	if (pos < 1) {
		return NULL;
	}
	
	pstart = from;
	pend = from + eso_strlen(from);

	i = 0;
	while (pstart) {
		i++;
		p = eso_strstr(pstart, separators);
		if (!p) {
			if (i == pos) {
				eso_mstrcpy(to, pstart);

				break;
			}
			else
				return NULL;
		}
		if (i == pos) {
			eso_mstrncpy(to, pstart, ES_MIN(p-pstart, pend-pstart));

			break;
		}
		pstart = p + eso_strlen(separators);
	}
	
	if (*to)
		return (*to);
	else
		return NULL;
}

/*
 * string left trim
*/
es_string_t* eso_mltrim(es_string_t *string, char c)
{
	char *p;

	p = string;
	while (*p == c) {
		p++;
	}
	if (p != string) {
		es_uint32_t str_len;

		str_len = eso_strlen(p);
		eso_memmove(string, p, str_len);
		string[str_len] = 0x00;
	}

	return string;
}

es_string_t* eso_mltrims(es_string_t *string, char c[])
{
	char *p;

	if (!string || !*string || !c || !*c) {
		return string;
	}

	p = string;
	while (*p && eso_strchr(c, *p)) {
		p++;
	}
	if (p != string) {
		es_uint32_t str_len;

		str_len = eso_strlen(p);
		eso_memmove(string, p, str_len);
		string[str_len] = 0x00;
	}

	return string;
}


/*
 * string right trim
*/
es_string_t* eso_mrtrim(es_string_t *string, char c)
{
	es_uint32_t str_len;
	char *p;

	str_len = eso_strlen(string);
	if (str_len > 0) {
		p = string + str_len - 1;
		while (*p == c) {
			*p = 0x00;
			p--;
		}
	}
	
	return string;
}

es_string_t* eso_mrtrims(es_string_t *string, char c[])
{
	es_uint32_t str_len;
	char *p;

	if (!string || !*string || !c || !*c) {
		return string;
	}

	str_len = eso_strlen(string);
	if (str_len > 0) {
		p = string + str_len - 1;
		while (eso_strchr(c, *p)) {
			*p = 0x00;
			p--;
		}
	}

	return string;
}

/*
 * string trim
*/
es_string_t* eso_mtrim(es_string_t *string, char c)
{
	eso_mltrim(string, c);
 	eso_mrtrim(string, c);

	return string;
}

es_string_t* eso_mtrims(es_string_t *string, char c[])
{
	eso_mltrims(string, c);
	eso_mrtrims(string, c);

	return string;
}

/**
 * URL encode
 * if len<=0 then len=strlen(src)
 */
es_string_t* eso_murlencode(es_string_t** encoded, 
                            const char *src, es_int32_t len)
{
	const char *from, *end;
	char *start, *to;
	char hexchars[] = "0123456789ABCDEF";
	unsigned char c;
	es_int32_t dst_len;
	
	ES_ASSERT(encoded);
	
	if (!src) {
		return *encoded;
	}
	
	if (len <= 0) {
		len = eso_strlen(src);
	}
	
	dst_len = 3 * len + 1;
	if (!*encoded) {
		*encoded = (es_string_t*)eso_mmalloc(dst_len);
	}
	else if (dst_len > eso_mnode_size(*encoded)) {
		*encoded = (es_string_t*)eso_mrealloc(*encoded, dst_len);
	}
	
	from = src; end = src+len;
	start = to = (char *)(*encoded);
	
	while (from < end) {
		c = *from++;
		
		if (c == ' ') {
			*to++ = '+';
		}
		else if ((c < '0' && c != '-' && c != '.' && c != '*')
			     ||(c < 'A' && c > '9')
			     ||(c > 'Z' && c < 'a' && c != '_')
			     ||(c > 'z')) 
		{
			to[0] = '%';
			to[1] = hexchars[c >> 4];
			to[2] = hexchars[c & 15];
			to += 3;
		}
		else {
			*to++ = c;
		}
	}
	*to = 0;
	
	return *encoded;
}

/**
 * URL decode
 */
es_string_t* eso_murldecode(es_string_t** decoded, const char *src)
{
	int len;
	char *dst;
	char a, b;
	
	ES_ASSERT(decoded);
	
	if (!src) {
		return *decoded;
	}
	
	len = eso_strlen(src);
	
	if (!*decoded) {
		*decoded = (es_string_t*) eso_mmalloc(len+1);
	} else if (len > eso_mnode_size(*decoded)) {
		*decoded = (es_string_t*) eso_mrealloc(*decoded, len+1);
	}

	dst = *decoded;
	while (*src) {
		if ((*src == '%') && ((a = src[1]) && (b = src[2]))
				&& (eso_isxdigit(a) && eso_isxdigit(b))) {
			if (a >= 'a')
				a -= 'A' - 'a';
			if (a >= 'A')
				a -= ('A' - 10);
			else
				a -= '0';
			if (b >= 'a')
				b -= 'A' - 'a';
			if (b >= 'A')
				b -= ('A' - 10);
			else
				b -= '0';
			*dst++ = 16 * a + b;
			src += 3;
		}
		else if (*src == '+') {
			*dst++ = ' ';
			src++;
		}
		else {
			*dst++ = *src++;
		}
	}
	*dst++ = '\0';
	
	return *decoded;
}

es_string_t* eso_itostring(es_int32_t i, int radix)
{
	char buf[33] = {0};
	int charPos = 32;
	es_bool_t negative = (i < 0);
	
	if (!negative) {
		i = -i;
	}

	while (i <= -radix) {
		buf[charPos--] = digits[-(i % radix)];
		i = i / radix;
	}
	buf[charPos] = digits[-i];

	if (negative) {
		buf[--charPos] = '-';
	}
	
	return eso_mstrndup(buf + charPos, (33 - charPos));
}

es_string_t* eso_uitostring(es_uint32_t ui, int radix)
{
	switch (radix) {
		case 16:
		case 8:
		case 2: {
			char buf[32];
			int charPos = 32;
			int mask = radix - 1;
			int shift;
			switch(radix) {
			case 16: shift = 4; break;
			case 8: shift = 3; break;
			case 2: shift = 1; break;
			}
			do {
				buf[--charPos] = digits[(int)ui & mask];
				ui = ui >> shift;
			} while (ui != 0);

			return eso_mstrndup(buf + charPos, (32 - charPos));
		}
		default: {
			char buf[33] = {0};
			int charPos = 32;

			while (ui >= radix) {
				buf[charPos--] = digits[(ui % radix)];
				ui = ui / radix;
			}
			buf[charPos] = digits[ui];

			return eso_mstrndup(buf + charPos, (33 - charPos));
		}
	}
}

es_string_t* eso_ltostring(es_int64_t i, int radix)
{
	char buf[65] = {0};
	int charPos = 64;
	es_bool_t negative = (i < 0);

	if (!negative) {
		i = -i;
	}

	while (i <= -radix) {
		buf[charPos--] = digits[(int) (-(i % radix))];
		i = i / radix;
	}
	buf[charPos] = digits[(int) (-i)];

	if (negative) {
		buf[--charPos] = '-';
	}
	
	return eso_mstrndup(buf + charPos, (65 - charPos));
}

es_string_t* eso_ultostring(es_uint64_t ul, int radix)
{
	switch (radix) {
		case 16:
		case 8:
		case 2: {
			char buf[64];
			int charPos = 64;
			int shift;
			es_int64_t mask = radix - 1;
			switch (radix) {
			case 16: shift = 4; break;
			case 8: shift = 3; break;
			case 2: shift = 1; break;
			}
			do {
				buf[--charPos] = digits[(int)((es_int64_t)ul & mask)];
				ul = ul >> shift;
			} while (ul != 0);

			return eso_mstrndup(buf + charPos, (64 - charPos));
		}
		default: {
			char buf[65] = {0};
			int charPos = 64;

			while (ul >= radix) {
				buf[charPos--] = digits[(int) (ul % radix)];
				ul = ul / radix;
			}
			buf[charPos] = digits[(int) ul];

			return eso_mstrndup(buf + charPos, (65 - charPos));
		}
	}
}
