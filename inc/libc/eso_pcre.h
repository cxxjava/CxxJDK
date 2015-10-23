/**
 * @file  eso_pcre.c
 * @brief ES pcre wrapper.
 */

#ifndef __ESO_PCRE_H__
#define __ESO_PCRE_H__

#include "es_types.h"
#include "es_comm.h"
#include "eso_string.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Options. Some are compile-time only, some are run-time only, and some are
both, so we keep them all distinct. However, almost all the bits in the options
word are now used. In the long run, we may have to re-use some of the
compile-time only bits for runtime options, or vice versa. In the comments
below, "compile", "exec", and "DFA exec" mean that the option is permitted to
be set for those functions; "used in" means that an option may be set only for
compile, but is subsequently referenced in exec and/or DFA exec. Any of the
compile-time options may be inspected during studying (and therefore JIT
compiling). */

#define PCRE_CASELESS           0x00000001  /* Compile */
#define PCRE_MULTILINE          0x00000002  /* Compile */
#define PCRE_DOTALL             0x00000004  /* Compile */
#define PCRE_EXTENDED           0x00000008  /* Compile */
#define PCRE_ANCHORED           0x00000010  /* Compile, exec, DFA exec */
#define PCRE_DOLLAR_ENDONLY     0x00000020  /* Compile, used in exec, DFA exec */
#define PCRE_EXTRA              0x00000040  /* Compile */
#define PCRE_NOTBOL             0x00000080  /* Exec, DFA exec */
#define PCRE_NOTEOL             0x00000100  /* Exec, DFA exec */
#define PCRE_UNGREEDY           0x00000200  /* Compile */
#define PCRE_NOTEMPTY           0x00000400  /* Exec, DFA exec */
/* The next two are also used in exec and DFA exec */
#define PCRE_UTF8               0x00000800  /* Compile (same as PCRE_UTF16) */
#define PCRE_UTF16              0x00000800  /* Compile (same as PCRE_UTF8) */
#define PCRE_NO_AUTO_CAPTURE    0x00001000  /* Compile */
/* The next two are also used in exec and DFA exec */
#define PCRE_NO_UTF8_CHECK      0x00002000  /* Compile (same as PCRE_NO_UTF16_CHECK) */
#define PCRE_NO_UTF16_CHECK     0x00002000  /* Compile (same as PCRE_NO_UTF8_CHECK) */
#define PCRE_AUTO_CALLOUT       0x00004000  /* Compile */
#define PCRE_PARTIAL_SOFT       0x00008000  /* Exec, DFA exec */
#define PCRE_PARTIAL            0x00008000  /* Backwards compatible synonym */
#define PCRE_DFA_SHORTEST       0x00010000  /* DFA exec */
#define PCRE_DFA_RESTART        0x00020000  /* DFA exec */
#define PCRE_FIRSTLINE          0x00040000  /* Compile, used in exec, DFA exec */
#define PCRE_DUPNAMES           0x00080000  /* Compile */
#define PCRE_NEWLINE_CR         0x00100000  /* Compile, exec, DFA exec */
#define PCRE_NEWLINE_LF         0x00200000  /* Compile, exec, DFA exec */
#define PCRE_NEWLINE_CRLF       0x00300000  /* Compile, exec, DFA exec */
#define PCRE_NEWLINE_ANY        0x00400000  /* Compile, exec, DFA exec */
#define PCRE_NEWLINE_ANYCRLF    0x00500000  /* Compile, exec, DFA exec */
#define PCRE_BSR_ANYCRLF        0x00800000  /* Compile, exec, DFA exec */
#define PCRE_BSR_UNICODE        0x01000000  /* Compile, exec, DFA exec */
#define PCRE_JAVASCRIPT_COMPAT  0x02000000  /* Compile, used in exec */
#define PCRE_NO_START_OPTIMIZE  0x04000000  /* Compile, exec, DFA exec */
#define PCRE_NO_START_OPTIMISE  0x04000000  /* Synonym */
#define PCRE_PARTIAL_HARD       0x08000000  /* Exec, DFA exec */
#define PCRE_NOTEMPTY_ATSTART   0x10000000  /* Exec, DFA exec */
#define PCRE_UCP                0x20000000  /* Compile, used in exec, DFA exec */

/* Exec-time and get/set-time error codes */

#define PCRE_ERROR_NOMATCH          (-1)
#define PCRE_ERROR_NULL             (-2)
#define PCRE_ERROR_BADOPTION        (-3)
#define PCRE_ERROR_BADMAGIC         (-4)
#define PCRE_ERROR_UNKNOWN_OPCODE   (-5)
#define PCRE_ERROR_UNKNOWN_NODE     (-5)  /* For backward compatibility */
#define PCRE_ERROR_NOMEMORY         (-6)
#define PCRE_ERROR_NOSUBSTRING      (-7)
#define PCRE_ERROR_MATCHLIMIT       (-8)
#define PCRE_ERROR_CALLOUT          (-9)  /* Never used by PCRE itself */
#define PCRE_ERROR_BADUTF8         (-10)  /* Same for 8/16 */
#define PCRE_ERROR_BADUTF16        (-10)  /* Same for 8/16 */
#define PCRE_ERROR_BADUTF8_OFFSET  (-11)  /* Same for 8/16 */
#define PCRE_ERROR_BADUTF16_OFFSET (-11)  /* Same for 8/16 */
#define PCRE_ERROR_PARTIAL         (-12)
#define PCRE_ERROR_BADPARTIAL      (-13)
#define PCRE_ERROR_INTERNAL        (-14)
#define PCRE_ERROR_BADCOUNT        (-15)
#define PCRE_ERROR_DFA_UITEM       (-16)
#define PCRE_ERROR_DFA_UCOND       (-17)
#define PCRE_ERROR_DFA_UMLIMIT     (-18)
#define PCRE_ERROR_DFA_WSSIZE      (-19)
#define PCRE_ERROR_DFA_RECURSE     (-20)
#define PCRE_ERROR_RECURSIONLIMIT  (-21)
#define PCRE_ERROR_NULLWSLIMIT     (-22)  /* No longer actually used */
#define PCRE_ERROR_BADNEWLINE      (-23)
#define PCRE_ERROR_BADOFFSET       (-24)
#define PCRE_ERROR_SHORTUTF8       (-25)
#define PCRE_ERROR_SHORTUTF16      (-25)  /* Same for 8/16 */
#define PCRE_ERROR_RECURSELOOP     (-26)
#define PCRE_ERROR_JIT_STACKLIMIT  (-27)
#define PCRE_ERROR_BADMODE         (-28)
#define PCRE_ERROR_BADENDIANNESS   (-29)
#define PCRE_ERROR_DFA_BADRESTART  (-30)

typedef struct es_pcre_t es_pcre_t;

/**
 * Build a Regular Expression
 * Params: pattern	regular expression
 *         options  parameter options, normal 0
 *         erroffset error location
 * Return: compiled model structure
 */
es_pcre_t* eso_pcre_make(const char *pattern, 
                         int options,
                         int *erroffset);

/**
 * Execute a Regular Expression
 * Params: pcre	compiled model structure
 *         subject the matching string
 *         length  the length of matching string, -1 is strlen(subject)
 *         startoffset	where to start in the subject string
 *         options  parameter options, normal 0
 *         ovector	output, the result of an integer array, the length of 3
           index at 0 - relative to the beginning of the last matched
           index at 1 - relative to the ending of the last matched
           index at 2 - only internal used
 *         ovecsize	output, the array size
 * Return: > 0 => success; value is the number of elements filled in
           = 0 => success, but offsets is not big enough
           -1 => failed to match
           < -1 => some kind of unexpected problem
 */
int eso_pcre_exec(es_pcre_t *pcre, 
                  const char *subject, 
                  int length, 
                  int startoffset, 
                  int options, 
                  int ovector[], /*0-relative to the beginning of the match*/
                  int ovecsize);

/**
 * Free a Regular Expression
 */
void eso_pcre_free(es_pcre_t **pcre);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_PCRE_H__ */
