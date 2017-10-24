/**
 * @file  es_comm.h
 * @brief ES Platform Definitions
 */


#ifndef __ES_COMM_H__
#define __ES_COMM_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __ESO_FUNC__
	#define EXTERN 
#else
	#define EXTERN extern
#endif

#if defined(ES_DECLARE_EXPORT)
	#define ES_DECLARE(type)            __declspec(dllexport) type //__stdcall
	#define ES_DECLARE_NONSTD(type)     __declspec(dllexport) type //__cdecl
	#define ES_DECLARE_DATA             __declspec(dllexport)
	#define ES_THREAD_FUNC              __declspec(dllexport)
	#define ES_DECLARE_CLASS            __declspec(dllexport)
#elif defined(ES_DECLARE_IMPORT)
	#define ES_DECLARE(type)            __declspec(dllimport) type //__stdcall
	#define ES_DECLARE_NONSTD(type)     __declspec(dllimport) type //__cdecl
	#define ES_DECLARE_DATA             __declspec(dllimport)
	#define ES_THREAD_FUNC              __declspec(dllimport)
	#define ES_DECLARE_CLASS            __declspec(dllimport)
#else
	#define ES_DECLARE(type)            type// __stdcall
	#define ES_DECLARE_NONSTD(type)     type// __cdecl
	#define ES_DECLARE_DATA
	#define ES_THREAD_FUNC              //__stdcall
	#define ES_DECLARE_CLASS
#endif


/* Don't use simply MIN/MAX, as they're often defined elsewhere in include
   files that are included after this file generating tons of warnings. */
#define ES_MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define ES_MAX(a, b)  (((a) > (b)) ? (a) : (b))

#define ES_ARRAY_LEN(a)  ((int)(sizeof(a) / sizeof(a[0])))

/* ES_ALIGN() is only to be used to align on a power of 2 boundary */
#define ES_ALIGN_UP(size, boundary) \
    (((size) + ((boundary) - 1)) & ~((boundary) - 1))

#define ES_ALIGN_DOWN(size, boundary) \
	((size) & ~((boundary) - 1))

/** Default alignment */
#define ES_ALIGN_DEFAULT(size) ES_ALIGN_UP(size, CHAR_BIT)

/** Check alignment */
#define ES_IS_ALIGNED(size, boundary) \
	(((size) & ((boundary) - 1)) == 0)

/* In case there is no definition of offsetof() provided - though any proper
Standard C system should have one. */
#if !defined(offsetof)
#define offsetof(struct_t,field) ((unsigned long)&(((struct_t *)0)->field))
#endif
#define ES_OFFSETOF(struct_t,field) offsetof(struct_t,field)

#define ES_SIZEOF(struct_t,field) ((unsigned int)sizeof(((struct_t *)0)->field))   

#if defined(_MSC_VER)
	#include <stdarg.h>
	#if !defined(_VA_LIST_DEFINED)
	typedef char *va_list;
	#define _INTSIZEOF(n)   ( (sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1) )
	#define va_start(ap,v)  ( ap = (va_list)&v + _INTSIZEOF(v) )
	#define va_arg(ap,t)    ( *(t *)((ap += _INTSIZEOF(t)) - _INTSIZEOF(t)) )
	#define va_end(ap)      ( ap = (va_list)0 )
	#endif
#elif defined(__arm)
	typedef int *va_list[1];
	#define va_start(ap, parmN) (void)(*(ap) = __va_start(parmN))
	#define va_arg(ap, type) __va_arg(*(ap), type)
	#define va_copy(dest, src) ((void)(*(dest) = *(src)))
	#define va_end(ap) ((void)(*(ap) = 0))
#endif

#ifndef va_copy
	#ifdef __va_copy
	#define va_copy(DEST,SRC) __va_copy((DEST),(SRC))
	#else
	#define va_copy(DEST, SRC) memcpy((&DEST), (&SRC), sizeof(va_list))
	#endif
#endif

#define ES_UNUSED(a) (void)a

#define ES_SWAP(x,y)   ((x)^=(y)^=(x)^=(y)) 

#define ES_BSWAP_16(x) \
	(((x) & 0x00ff) << 8 | ((x) & 0xff00) >> 8)

#define ES_BSWAP_32(x) \
	((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) | \
	 (((x) & 0x0000ff00) <<  8) | (((x) & 0x000000ff) << 24))

#define ES_BSWAP_64(x) \
	((((x) & 0xff00000000000000) >> 56) | (((x) & 0x00ff000000000000) >> 40) | \
	 (((x) & 0x0000ff0000000000) >> 24) | (((x) & 0x000000ff00000000) >>  8) | \
	 (((x) & 0x00000000ff000000) <<  8) | (((x) & 0x0000000000ff0000) << 24) | \
	 (((x) & 0x000000000000ff00) << 40) | (((x) & 0x00000000000000ff) << 56))

#define ES_FAST_MAX(x,y) \
	((x) - ((((x) - (y)) >> 31) & ((x) - (y))))
#define ES_FAST_MIN(x,y) \
	((x) + ((((y) - (x)) >> 31) & ((y) - (x))))

/* So that we can use inline on some critical functions, and use
 * GNUC attributes (such as to get -Wall warnings for printf-like
 * functions).  Only do this in gcc 2.7 or later ... it may work
 * on earlier stuff, but why chance it.
 *
 * We've since discovered that the gcc shipped with NeXT systems
 * as "cc" is completely broken.  It claims to be __GNUC__ and so
 * on, but it doesn't implement half of the things that __GNUC__
 * means.  In particular it's missing inline and the __attribute__
 * stuff.  So we hack around it.  PR#1613. -djg
 */
#if !defined(__GNUC__) || __GNUC__ < 2 || \
    (__GNUC__ == 2 && __GNUC_MINOR__ < 7) ||\
    defined(NEXT)
#ifndef __attribute__
#define __attribute__(__x)
#endif
#define ES_INLINE
#define ES_HAS_INLINE		0
#else
#define ES_INLINE __inline__
#define ES_HAS_INLINE		1
#ifndef _GNU_SOURCE
	#define _GNU_SOURCE         /* define this then can gives us pthread_rwlock_t */
#endif
#endif

#if defined(__arm)
#define ES_PATH_MAX        512
#else
#define ES_PATH_MAX        4096
#endif

#if defined(WIN32) || defined(__arm)
#define ES_FILE_PATH_SEPARATOR  "\\"
#define ES_FILE_PATH_SEPARATOR_CHAR  '\\'
#define ES_LINE_SEPARATOR  "\r\n"
#else
#define ES_FILE_PATH_SEPARATOR  "/"
#define ES_FILE_PATH_SEPARATOR_CHAR  '/'
#define ES_LINE_SEPARATOR  "\n"
#endif

#define ES_NANOS_PER_SECOND      1000000000
#define ES_NANOS_PER_MILLISEC    1000000

#ifdef DEBUG
	#ifndef ES_ASSERT
		#include <assert.h>
		#define ES_ASSERT assert
	#endif
#else
#define ES_ASSERT(p)
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4035) //Disables warnings reporting missing return statement.
#pragma warning(disable : 4996) //This function or variable may be unsafe.
#pragma warning(disable : 4786) //Symbol too long.
#pragma warning(disable : 4099) //type name first seen using 'class' now seen using 'struct'
#pragma warning(disable : 4101) //Unreferenced local variable
#endif

#include "es_config.h"

#ifdef __cplusplus
}
#endif

#endif  /* ! __ES_COMM_H__ */
