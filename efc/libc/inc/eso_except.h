/**
 * @file  eso_except.h
 * @brief ES Exception Handling in C Language
 */


#ifndef __ESO_EXCEPT_H__
#define __ESO_EXCEPT_H__

#include "es_types.h"
#include "eso_libc.h"
#include <setjmp.h>

/*==============================================================================
Note!!!
this exception module only used at C code.
==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Example:
// try_begin
//     try_begin
//         ...
//         throw(-1, 1001, "1001 error.");
//     catch_one(-1)
//         eso_log("catched error: err_type=%d, err_code=%d, err_msg=%s", 
//                  eso_get_except_errtype(), eso_get_except_errcode(), eso_get_except_errmsg());
//         rethrow;
//     try_end;
//  catch_all()
//     eso_log("catched -1 error");
//  try_end;
*/

//==============================================================================
//Internal struct define.

typedef struct es_except_t es_except_t;
struct es_except_t {
	int  err_type;    /* exception type */
	int  err_code;    /* exception code */
	char err_msg[65]; /* exception message */
};

typedef struct es_except_element_t es_except_element_t;
struct es_except_element_t {
	jmp_buf     status;
	es_except_t exception;
	es_except_element_t* next;
};

extern void ___except_push(es_except_element_t* es_except_element_t);
extern es_except_element_t* ___except_pop(void);
extern es_except_element_t* ___except_top(void);
extern int ___except_is_empty(void);
extern void ___except_thrower(es_except_t* e); 

//==============================================================================
//User's api define.



/**
 * Throw a exception (throw)
 * @param	a	int, exception type
 * @param	b	int, exception code
 * @param	c	string, exception message, max length is 64 byte
 */
#define throw(a, b, c) \
{ \
es_except_t ___ex; \
eso_memset(&___ex, 0, sizeof(___ex)); \
___ex.err_type = a; \
___ex.err_code = b; \
eso_strncpy(___ex.err_msg, c, sizeof(___ex.err_msg)); \
___except_thrower(&___ex); \
}

/**
 * Rethrow a exception (rethrow)
 */
#define rethrow \
___except_thrower(___exception)


/**
 * define try block (monitored c code)
 */
#define try_begin \
{ \
int ___block_catch_found, ___block_occur_exception; \
es_except_element_t ___except_element; \
es_except_t* ___exception; \
eso_memset(&___except_element, 0, sizeof(___except_element)); \
___except_push(&___except_element); \
___exception = &___except_element.exception; \
___block_catch_found = 0; \
___block_occur_exception = setjmp(___except_element.status); \
if (___block_occur_exception == 0) \
{


/**
 * define catch block (exception handling code)
 * catched all exception, this catch block need after all other catched blocks
*/
#define catch_all() \
} \
else \
{ \
___except_pop(); \
___block_catch_found = 1;


/**
 * end try block
 */
#define try_end \
} \
{ \
if(!___block_catch_found) \
{ \
___except_pop(); \
if(___block_occur_exception) ___except_thrower(___exception); \
} \
} \
} 


/**
 * define catch block (exception handling code)
 * catched range(i,j) of exceptions
 */
#define catch_part(i, j) \
} \
else if(___exception->err_type>=i && ___exception->err_type<=j) \
{ \
___except_pop(); \
___block_catch_found = 1;


/**
 * define catch block (exception handling code)
 * catched only one of exception
 */
#define catch_one(i) \
} \
else if(___exception->err_type==i) \
{ \
___except_pop(); \
___block_catch_found = 1;


/**
 * get exception information
 */
#define eso_get_except_errtype() ___exception->err_type
#define eso_get_except_errcode() ___exception->err_code
#define eso_get_except_errmsg() ___exception->err_msg

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_EXCEPT_H__ */
