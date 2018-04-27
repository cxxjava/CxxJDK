/**
 * @file  es_types.h
 * @brief ES base data types define
 */

#ifndef __ES_TYPES_H__
#define __ES_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//==============================================================================

#if defined(WIN32) || defined(__MTK__)

#ifndef __WORDSIZE
#define __WORDSIZE 32
#endif

/* The number of bits in a char.  */
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* The number of bytes in a double.  */
#ifndef SIZEOF_DOUBLE
#define SIZEOF_DOUBLE 8
#endif

/* The number of bytes in a int.  */
#ifndef SIZEOF_INT
#define SIZEOF_INT 4
#endif

/* The number of bytes in a long.  */
#ifndef SIZEOF_LONG
#define SIZEOF_LONG 4
#endif

/* The number of bytes in a short.  */
#ifndef SIZEOF_SHORT
#define SIZEOF_SHORT 2
#endif

/* The number of bytes in a void.  */
#ifndef SIZEOF_VOID_P
#define SIZEOF_VOID_P 4
#endif

/* The number of bytes in a long long.  */
#ifndef SIZEOF_LLONG
#define SIZEOF_LLONG 8
#endif

#define LLONG(a)  a##I64
#define ULLONG(a) a##UI64

#else //linux

#include <stdint.h>

#if __WORDSIZE == 64

#define AMD64 //64 Bit!

/* The number of bits in a char.  */
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* The number of bytes in a double.  */
#ifndef SIZEOF_DOUBLE
#define SIZEOF_DOUBLE 8
#endif

/* The number of bytes in a int.  */
#ifndef SIZEOF_INT
#define SIZEOF_INT 4
#endif

/* The number of bytes in a long.  */
#ifndef SIZEOF_LONG
#define SIZEOF_LONG 8
#endif

/* The number of bytes in a short.  */
#ifndef SIZEOF_SHORT
#define SIZEOF_SHORT 2
#endif

/* The number of bytes in a void.  */
#ifndef SIZEOF_VOID_P
#define SIZEOF_VOID_P 8
#endif

/* The number of bytes in a long long.  */
#ifndef SIZEOF_LLONG
#define SIZEOF_LLONG 8
#endif

#elif __WORDSIZE == 32

/* The number of bits in a char.  */
#ifndef CHAR_BIT
#define CHAR_BIT 8
#endif

/* The number of bytes in a double.  */
#ifndef SIZEOF_DOUBLE
#define SIZEOF_DOUBLE 8
#endif

/* The number of bytes in a int.  */
#ifndef SIZEOF_INT
#define SIZEOF_INT 4
#endif

/* The number of bytes in a long.  */
#ifndef SIZEOF_LONG
#define SIZEOF_LONG 4
#endif

/* The number of bytes in a short.  */
#ifndef SIZEOF_SHORT
#define SIZEOF_SHORT 2
#endif

/* The number of bytes in a void.  */
#ifndef SIZEOF_VOID_P
#define SIZEOF_VOID_P 4
#endif

/* The number of bytes in a long long.  */
#ifndef SIZEOF_LLONG
#define SIZEOF_LLONG 8
#endif

#else
# error unknown __WORDSIZE
#endif

#define LLONG(a)  a##LL
#define ULLONG(a) a##ULL

#endif

//==============================================================================

/**
 * The smallest value of type <code>longlong</code>.
 */
#define ES_INT64_MIN_VALUE  ULLONG(-9223372036854775807)

/**
 * The largest value of type <code>longlong</code>.
 */
#define ES_INT64_MAX_VALUE  LLONG(9223372036854775807)

/**
 * The smallest value of type <code>int</code>. The constant
 * value of this field is <tt>-2147483648</tt>.
 */
#define ES_INT32_MIN_VALUE  (-2147483647)

/**
 * The largest value of type <code>int</code>. The constant
 * value of this field is <tt>2147483647</tt>.
 */
#define ES_INT32_MAX_VALUE  2147483647

/**
 * The minimum value a Short can have.
 */
#define ES_INT16_MIN_VALUE  (-32768)

/**
 * The maximum value a Short can have.
 */
#define ES_INT16_MAX_VALUE  32767

/**
 * The minimum values a `signed char' can hold.
 */
#define ES_SCHAR_MIN_VALUE     (-128)

/**
 * The maximum values a `signed char' can hold.
 */
#define ES_SCHAR_MAX_VALUE     127

#if (SIZEOF_LONG == 8)
#define ES_LONG_MAX_VALUE    ES_INT64_MAX_VALUE
#define ES_LONG_MIN_VALUE    ES_INT64_MIN_VALUE
#else
#define ES_LONG_MAX_VALUE    ES_INT32_MAX_VALUE
#define ES_LONG_MIN_VALUE    ES_INT32_MIN_VALUE
#endif

//==============================================================================

typedef signed char             es_byte_t;
typedef signed char             es_int8_t;
typedef unsigned char           es_uint8_t;

typedef short                   es_int16_t;
typedef unsigned short          es_uint16_t;

typedef int                     es_int32_t;
typedef unsigned int            es_uint32_t;

#ifdef _MSC_VER
typedef __int64                 es_int64_t;
typedef unsigned __int64        es_uint64_t;
#else
typedef long long               es_int64_t;
typedef unsigned long long      es_uint64_t;
#endif

typedef long                    es_long_t;
typedef unsigned long           es_ulong_t;

typedef unsigned long           es_size_t;
typedef long                    es_ssize_t;

typedef float                   es_real32_t;
typedef double                  es_real64_t;

typedef es_ssize_t              es_ptrdiff_t;
typedef es_ssize_t              es_off_t;

typedef unsigned char           es_bool_t;

typedef char                    es_string_t;
typedef unsigned short          es_wstring_t;
typedef void                    es_data_t;

typedef void es_func_t(void);

/* Types for `void *' pointers.  */
#if __WORDSIZE == 64
# ifndef __intptr_t_defined
typedef es_int64_t              es_intptr_t;
#  define __intptr_t_defined
#else
typedef intptr_t                es_intptr_t;
# endif
typedef es_uint64_t             es_uintptr_t;
#else
# ifndef __intptr_t_defined
typedef es_int32_t              es_intptr_t;
#  define __intptr_t_defined
#else
typedef intptr_t                es_intptr_t;
# endif
typedef es_uint32_t             es_uintptr_t;
#endif

typedef struct
{
	es_size_t (*write)(void *p, const void *buf, es_size_t size);
    /* Returns: result - the number of actually written bytes.
       (result < size) means error */
} es_ostream_t;

typedef struct
{
	es_int32_t (*read)(void *p, void *buf, es_size_t *size);
    /* if (input(*size) != 0 && output(*size) == 0) means end_of_stream.
       (output(*size) < input(*size)) is allowed.
       Returns: 0-suceess, other-failure*/
} es_istream_t;

//==============================================================================

#ifndef NULL
#if defined(__cplusplus)
#  define NULL 0
#else
#  define NULL ((void *)0)
#endif
#endif

#ifndef FALSE
#  define FALSE ((es_bool_t)0)
#endif

#ifndef TRUE
#  define TRUE (!FALSE)
#endif

//==============================================================================

extern es_bool_t ES_BIG_ENDIAN;  /*seted at eso_initialize(), default is FALSE.*/

//==============================================================================

#if defined(_MSC_VER)
#define ES_ALIGN       //__declspec(align(SIZEOF_VOID_P))
#define ES_ALIGNN(n)   //__declspec(align((n)))
#elif defined(__GNUC__)
#define ES_ALIGN       __attribute__((aligned(SIZEOF_VOID_P)));
#define ES_ALIGNN(n)   __attribute__((aligned(n)))
#elif defined(__MTK__)
#define ES_ALIGN       __align(SIZEOF_VOID_P)
#define ES_ALIGNN(n)   __align(n)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ES_TYPES_H__ */
