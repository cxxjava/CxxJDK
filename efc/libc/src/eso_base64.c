/**
 * @file  eso_base64.c
 * @brief ES eso standard base64 encode/decode
 */
#include "eso_base64.h"
#include "eso_libc.h"

int eso_base64_encode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen)
{
	es_uint8_t *d, *s;
	const es_uint8_t basis64[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	if (ilen <= 0) {
		return 0;
	}
	
	if (!in || !out || (olen < (ilen+2)/3 * 4)) {
		return -1;
	}

	if (out && olen > 0) {
		eso_memset(out, 0, olen);
	}
	
	s = in;
	d = out;

	while (ilen > 2) {
		*d++ = basis64[(s[0] >> 2) & 0x3f];
		*d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
		*d++ = basis64[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
		*d++ = basis64[s[2] & 0x3f];
		
		s += 3;
		ilen -= 3;
	}
	
	if (ilen) {
		*d++ = basis64[(s[0] >> 2) & 0x3f];

		if (ilen == 1) {
			*d++ = basis64[(s[0] & 3) << 4];
			*d++ = '=';

		} else {
			*d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
			*d++ = basis64[(s[1] & 0x0f) << 2];
		}

		*d++ = '=';
	}

	return d - out;
}


int eso_base64_decode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen)
{
	int  l;
	es_uint8_t *d, *s;
	const es_uint8_t   basis64[] = {
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
		52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
		77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
		15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
		77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
		41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
		77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
	};
	
	if (ilen <= 0) {
		return 0;
	}
	
	if (!in || !out || (olen < ilen/4 * 3)) {
		return -1;
	}
	
	if (out && olen > 0) {
		eso_memset(out, 0, olen);
	}
	
	for (l = 0; l < ilen; l++) {
		if (in[l] == '=') {
			break;
		}

		if (basis64[in[l]] == 77) {
			return -1;
		}
	}

	if (l % 4 == 1) {
		return -1;
	}
	
	s = in;
	d = out;

	while (l > 3) {
		*d++ = (es_uint8_t) (basis64[s[0]] << 2 | basis64[s[1]] >> 4);
		*d++ = (es_uint8_t) (basis64[s[1]] << 4 | basis64[s[2]] >> 2);
		*d++ = (es_uint8_t) (basis64[s[2]] << 6 | basis64[s[3]]);

		s += 4;
		l -= 4;
	}

	if (l > 1) {
		*d++ = (es_uint8_t) (basis64[s[0]] << 2 | basis64[s[1]] >> 4);
	}

	if (l > 2) {
		*d++ = (es_uint8_t) (basis64[s[1]] << 4 | basis64[s[2]] >> 2);
	}

	return d - out;
}

//==============================================================================

/*
 * "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
 *
 * 返回0xFF表示失败
 */
static es_uint8_t en_table ( es_uint8_t in )
{
	es_uint8_t out = 0xFF;

	if (40 == in){
		out = 'z';
	}
	else if (54 == in) {
		out = '=';
	}
	else if (62 == in) {
		out = 'X';
	}
	else if (in >= 5 && in <= 11) {
		out = in - 5 + 'm';
	}
	else if (in >= 0x00 && in <= 27) {
		out = in + '5';
	}
	else if (in >= 28 && in <= 32) {
		out = in - 28 + '0';
	}
	else if (in >= 43 && in <= 48) {
		out = in - 43 + 't';
	}
	else if (in >= 28 && in <= 60) {
		out = in - 28 + 'L';
	}
	else if (61 == in) {
		out = '/';
	}
	else if (63 == in) {
		out = '+';
	}
	else if (64 == in) {
		out = '*';
	}
	return( out );
}

static es_uint8_t de_table ( es_uint8_t in )
{
	es_uint8_t out = 0xFF;
	
	if ( 'z' == in) {
		out = 40;
	}
	else if ( '=' == in) {
		out = 54;
	}
	else if ( 'X' == in) {
		out = 62;
	}
	else if (in >= 'm' && in <= 's') {
		out = in - 'm' + 5;
	}
	else if (in >= '5' && in <= 'P') {
		out = in - '5';
	}
	else if (in >= '0' && in <= '4') {
		out = in - '0' + 28;
	}
	else if (in >= 't' && in <= 'y') {
		out = in - 't' + 43;
	}
	else if (in >= 'L' && in <= 'l') {
		out = in - 'L' + 28;
	}
	else if ('/' == in) {
		out = 61;
	}
	else if ('+' == in) {
		out = 63;
	}
	else if ('*' == in) {
		out = 64;
	}
	return( out );
}

int eso_xbase64_encode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen)
{
	int x, y, z;
	int	i, j;
	char bufa[4];
	char bufb[3];
 
	if (ilen <= 0) {
		return 0;
	}
	
	if (!in || !out || (olen < (ilen+2)/3 * 4)) {
		return -1;
	}
	
	if (out && olen > 0) {
		eso_memset(out, 0, olen);
	}
	
	/*
	 * 由主调函数确保形参有效性
	 */
	x = ilen / 3;
	i = 0;
	j = 0;

	for ( z = 0; z < x; z++ ) {
		bufa[0] = (in[i] & 0xFC) >> 2;
		bufa[1] = (in[i] & 0x03) << 4 | (in[i+1] & 0xF0) >> 4;
		bufa[2] = (in[i+1] & 0x0F) << 2 | (in[i+2] & 0xC0) >> 6;
		bufa[3] = in[i+2] & 0x3F;

		for ( y = 0; y < 4; y++ ) {
			if(( out[j+y] = en_table(bufa[y]) ) == 0xff) {
				return -1;
			}
		}  /* end of for */

		i += 3;
		j += 4;
	}  /* end of for */

	if (i != ilen) {
		for ( z = 0; z < ilen - i; z++ ) {
			bufb[z] = in[i+z];
		}  /* end of for */

		while (z < 3) {
			bufb[z++] = 0;
		}

		bufa[0] = (bufb[0] & 0xFC) >> 2;
		bufa[1] = (bufb[0] & 0x03) << 4 | (bufb[1] & 0xF0) >> 4;
		bufa[2] = (bufb[1] & 0x0F) << 2 | (bufb[2] & 0xC0) >> 6;
		bufa[3] = bufb[2] & 0x3F;
		
		for ( y = 0; y < 4; y++ ) {
			if(( out[j+y] = en_table(bufa[y]) ) == 0xff) {
				return -1;
			}
		}  /* end of for */
		j += 4;

		for (y = 0; y < 3 - (ilen - i); y++ ) {
			out[j-y-1] = '*';
		}
		out[j] = 0x00;
		
	}
	else {
		out[j] = 0x00;
	}

	return (j);
}

int eso_xbase64_decode(es_uint8_t *in, int ilen, es_uint8_t *out, int olen)
{
	int  x, y, z;
	int	i, j;
	char bufa[4];
	char bufb[3];
 
	if (ilen <= 0) {
		return 0;
	}
	
	if (!in || !out || (olen < ilen/4 * 3)) {
		return -1;
	}
	
	if (out && olen > 0) {
		eso_memset(out, 0, olen);
	}
	
	/*
	 * 由主调函数确保形参有效性
	 */
	x = ( ilen - 4 ) / 4;
	i = 0;
	j = 0;

	for ( z = 0; z < x; z++ ) {
		for ( y = 0; y < 4; y++ ) {
			if(( bufa[y] = de_table( in[j+y]) ) == 0xff) {
				return -1;
			}
		}  /* end of for */

		out[i]	  = bufa[0] << 2 | ( bufa[1] & 0x30 ) >> 4;
		out[i+1]  = ( bufa[1] & 0x0F ) << 4 | ( bufa[2] & 0x3C ) >> 2;
		out[i+2]  = ( bufa[2] & 0x03 ) << 6 | ( bufa[3] & 0x3F );
		i += 3;
		j += 4;
	}  /* end of for */

	for ( z = 0; z < 4; z++ ) {
		if(( bufa[z]  = de_table(in[j+z]) ) == 0xff) {
			return -1;
		}
	}  /* end of for */

	/*
	 * 编码算法确保了结尾最多有两个'='
	 */
	if ( '*' == in[ilen-2] ) {
		y = 2;
	}
	else if ( '*' == in[ilen-1] ) {
		y = 1;
	}
	else {
		y = 0;
	}

	/*
	 * BASE64算法所需填充字节个数是自识别的
	 */
	for ( z = 0; z < y; z++ ) {
		bufa[4-z-1] = 0x00;
	}  /* end of for */

	bufb[0] = bufa[0] << 2 | ( bufa[1] & 0x30 ) >> 4;
	bufb[1] = ( bufa[1] & 0x0F ) << 4 | ( bufa[2] & 0x3C ) >> 2;
	bufb[2] = ( bufa[2] & 0x03 ) << 6 | ( bufa[3] & 0x3F );

	/*
	 * y必然小于3
	 */
	for ( z = 0; z < 3 - y; z++ ) {
		out[i+z] = bufb[z];
	}  /* end of for */

	/*
	 * 离开for循环的时候已经z++了
	 */
	i += z;
	out[i] = 0x00;
	
	return (i);
}
