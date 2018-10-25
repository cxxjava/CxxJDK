/**
 * @file  eso_printf_upper_bound.c
 * @brief ES calculate printf upper bound
 */

#include "eso_printf_upper_bound.h"
#include "eso_libc.h"

#include <limits.h> //MB_LEN_MAX

typedef union  _GDoubleIEEE754_b  GDoubleIEEE754_b;
typedef union  _GDoubleIEEE754_l  GDoubleIEEE754_l;
#define G_IEEE754_DOUBLE_BIAS   (1023)
/* multiply with base2 exponent to get base10 exponent (nomal numbers) */
#define G_LOG_2_BASE_10         (0.30102999566398119521)

union _GDoubleIEEE754_b
{
	double v_double;
	struct {
		unsigned int sign : 1;
		unsigned int biased_exponent : 11;
		unsigned int mantissa_high : 20;
		unsigned int mantissa_low : 32;
	} mpn;
};

union _GDoubleIEEE754_l
{
	double v_double;
	struct {
		unsigned int mantissa_low : 32;
		unsigned int mantissa_high : 20;
		unsigned int biased_exponent : 11;
		unsigned int sign : 1;
	} mpn;
};

typedef struct
{
	unsigned int min_width;
	unsigned int precision;
	int alternate_format, zero_padding, adjust_left, locale_grouping;
	int add_space, add_sign, possible_sign, seen_precision;
	int mod_long, mod_extra_long;
} PrintfArgSpec;

#if (SIZEOF_LONG > 4) || (SIZEOF_VOID_P > 4)
#	define HONOUR_LONGS 1
#else
#	define HONOUR_LONGS 0
#endif


/* Returns the maximum length of given format string when expanded.
*  If the format is invalid, i_fatal() is called.
*/
es_size_t eso_printf_upper_bound(const char *format, va_list args)
{
	es_size_t len = 1;

	if (!format)
		return len;

	while (*format) {
		register char c = *format++;

		if (c != '%')
			len += 1;
		else /* (c == '%') */
		{
			PrintfArgSpec spec;
			es_bool_t seen_l = FALSE, conv_done = FALSE;
			unsigned int conv_len = 0;

			eso_memset(&spec, 0, sizeof(spec));
			do {
				c = *format++;
				switch (c) {
					GDoubleIEEE754_b u_double_b;
					GDoubleIEEE754_l u_double_l;
					unsigned int v_uint;
					int v_int;
					const char *v_string;

					/* parse flags */
				case '#':
					spec.alternate_format = TRUE;
					break;
				case '0':
					spec.zero_padding = TRUE;
					break;
				case '-':
					spec.adjust_left = TRUE;
					break;
				case ' ':
					spec.add_space = TRUE;
					break;
				case '+':
					spec.add_sign = TRUE;
					break;
				case '\'':
					spec.locale_grouping = TRUE;
					break;

					/* parse output size specifications */
				case '.':
					spec.seen_precision = TRUE;
					break;
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					v_uint = c - '0';
					c = *format;
					while (c >= '0' && c <= '9') {
						format++;
						v_uint = v_uint * 10 + (c - '0');
						c = *format;
					}
					if (spec.seen_precision)
						spec.precision = ES_MAX (spec.precision, v_uint);
					else
						spec.min_width = ES_MAX (spec.min_width, v_uint);
					break;
				case '*':
					v_int = va_arg (args, int);
					if (spec.seen_precision) {
						/* forget about negative precision */
						if (v_int >= 0)
							spec.precision = ES_MAX ((int)spec.precision, v_int);
					}
					else {
						if (v_int < 0)
						{
							v_int = - v_int;
							spec.adjust_left = TRUE;
						}
						spec.min_width = ES_MAX ((int)spec.min_width, v_int);
					}
					break;
	
					/* parse type modifiers */
				case 'h':
					/* ignore */
					break;
				case 'l':
					if (!seen_l) {
						spec.mod_long = TRUE;
						seen_l = TRUE;
						break;
					}
					/* else, fall through */
				case 'L':
					spec.mod_long = TRUE;
					spec.mod_extra_long = TRUE;
					break;

					/* parse output conversions */
				case '%':
					conv_len += 1;
					break;
				case 'o':
					conv_len += 2;
					/* fall through */
				case 'd':
				case 'i':
					conv_len += 1; /* sign */
					/* fall through */
				case 'u':
					conv_len += 4;
					/* fall through */
				case 'x':
				case 'X':
					spec.possible_sign = TRUE;
					conv_len += 10;
					if (spec.mod_long && HONOUR_LONGS)
						conv_len *= 2;
					if (spec.mod_extra_long)
						conv_len *= 2;
					
					if (spec.mod_extra_long) {
#if SIZEOF_LLONG > 0
						(void) va_arg (args, es_int64_t);
#else
						fprintf(stderr, 
							"Error: [%s][%d] mod_extra_long not supported\n",
							__FILE__,
							__LINE__);
#endif
					}
					
					else if (spec.mod_long)
						(void) va_arg (args, long);
					else
						(void) va_arg (args, int);
					break;
				case 'A':
				case 'a':
					/*          0x */
					conv_len += 2;
					/* fall through */
				case 'g':
				case 'G':
				case 'e':
				case 'E':
				case 'f':
					spec.possible_sign = TRUE;
					/*          n   .   dddddddddddddddddddddddd   E   +-  eeee */
					conv_len += 1 + 1 + ES_MAX (24, spec.precision) + 1 + 1 + 4;
					/*
					if (spec.mod_extra_long)
						fprintf(stderr, 
							"Error: [%s][%d] unable to handle long double\n",
							__FILE__,
							__LINE__);
					*/
#ifdef HAVE_LONG_DOUBLE
#error need to implement special handling for long double
#endif
					if (ES_BIG_ENDIAN) {
						u_double_b.v_double = va_arg (args, double);
						/* %f can expand up to all significant digits before '.' (308) */
						if (c == 'f'
							&& u_double_b.mpn.biased_exponent > 0
							&& u_double_b.mpn.biased_exponent < 2047) {
							int exp = u_double_b.mpn.biased_exponent;

							exp -= G_IEEE754_DOUBLE_BIAS;
							exp = (int)(exp * G_LOG_2_BASE_10 + 1);
							conv_len += exp;
						}
					}
					else {
						u_double_l.v_double = va_arg (args, double);
						/* %f can expand up to all significant digits before '.' (308) */
						if (c == 'f'
							&& u_double_l.mpn.biased_exponent > 0
							&& u_double_l.mpn.biased_exponent < 2047) {
							int exp = u_double_l.mpn.biased_exponent;

							exp -= G_IEEE754_DOUBLE_BIAS;
							exp = (int)(exp * G_LOG_2_BASE_10 + 1);
							conv_len += exp;
						}
					}

					/* some printf() implementations require extra padding for rounding */
					conv_len += 2;
					/* we can't really handle locale specific grouping here */
					if (spec.locale_grouping)
						conv_len *= 2;
					break;
				case 'c':
					conv_len += spec.mod_long ? MB_LEN_MAX : 1;
					(void) va_arg (args, int);
					break;
				case 's':
					v_string = va_arg (args, char*);
					if (!v_string)
						conv_len += 8; /* hold "(null)" */
					else if (spec.seen_precision)
						conv_len += spec.precision;
					else
						conv_len += eso_strlen (v_string);
					conv_done = TRUE;
					/*
					if (spec.mod_long)
						fprintf(stderr, 
							"Error: [%s][%d] unable to handle wide char strings\n",
							__FILE__,
							__LINE__);
					*/
					break;
				case 'p':
					spec.alternate_format = TRUE;
					conv_len += 10;
					if (HONOUR_LONGS)
						conv_len *= 2;
					conv_done = TRUE;
					(void) va_arg (args, void*);
					break;
					/* handle invalid cases */
				case 'm':
					/* normally we shouldn't even get here, but we could be just
					 checking the format string is valid before giving the
					 format to vsyslog(). */
					conv_len += strlen(strerror(errno)) + 256;
					break;

					/* handle invalid cases
					 */
				case '\000':
					/* no conversion specification, bad bad */
					/*
					fprintf(stderr, 
							"Error: [%s][%d] Missing conversion specifier\n",
							__FILE__,
							__LINE__);
					*/
					break;
				default:
					/*
					fprintf(stderr, 
							"Error: [%s][%d] unable to handle `%c' while parsing format\n",
							__FILE__,
							__LINE__,
							c);
					*/
					break;
				}
				conv_done |= conv_len > 0;
			} while (!conv_done);

			/* handle width specifications */
			conv_len = ES_MAX (conv_len, ES_MAX (spec.precision, spec.min_width));
			/* handle flags */
			conv_len += spec.alternate_format ? 2 : 0;
			conv_len += (spec.add_space || spec.add_sign || spec.possible_sign);
			/* finally done */
			len += conv_len;
		} /* else (c == '%') */
	} /* while (*format) */

	return len;
}
