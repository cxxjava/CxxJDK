/**
 * @file  eso_libc.h
 * @brief ES libc
 */

#ifndef __ESO_LIBC_H__
#define __ESO_LIBC_H__

#include "es_types.h"
#include "es_comm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 *	LOG
 */
#define eso_log				printf

/*
 *  LIBC
 */
#define eso_malloc			malloc
#define eso_calloc(n)		calloc((n),1)
#define eso_realloc(p,n)	realloc((p),(n))
#define eso_free			free
#define eso_memchr			memchr
#define eso_memcmp			memcmp
#define eso_memcpy			memcpy
#define eso_memmove			memmove
#define eso_memset			memset
#define eso_sscanf          sscanf
#define eso_strdup			strdup
#define eso_strcat			strcat
#define eso_strncat			strncat
#define eso_strchr			strchr
#define eso_strrchr			strrchr
#define eso_strcmp			strcmp
#define eso_strncmp 		strncmp
#define eso_strcpy 			strcpy
#define eso_strncpy 		strncpy
#define eso_strlen 			strlen
#define eso_strstr 			strstr
#define eso_strpbrk 		strpbrk
#define eso_sprintf 		sprintf
#define eso_vsprintf 		vsprintf
#define eso_srand 			srand
#define eso_rand 			rand
#define eso_atol 			atol
#define eso_atof 			atof
#define eso_strtol			strtol
#define eso_strtod			strtod
#define eso_strcoll			strcoll
#define eso_isalnum 		isalnum
#define eso_isalpha 		isalpha
#define eso_iscntrl 		iscntrl
#define eso_isdigit 		isdigit
#define eso_isgraph 		isgraph
#define eso_islower 		islower
#define eso_isprint 		isprint
#define eso_ispunct 		ispunct
#define eso_isspace 		isspace
#define eso_isupper 		isupper
#define eso_isxdigit 		isxdigit
#define eso_isascii(c)    (((c) & ~0x7f)==0)

#ifdef _MSC_VER
#define eso_snprintf 		_snprintf
#define eso_vsnprintf 		_vsnprintf
#else
#define eso_snprintf 		snprintf
#define eso_vsnprintf 		vsnprintf
#endif

#define ESO_FREE(pp) do { \
	if (pp && *pp) { \
		free(*(pp)); \
		*(pp)=NULL; \
	} \
} while(0);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_LIBC_H__ */
