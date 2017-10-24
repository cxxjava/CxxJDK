/**
 * @file  eso_canonicalize.h
 * @brief Pathname canonicalization
 */


#ifndef __ESO_CANONICALIZE_H__
#define __ESO_CANONICALIZE_H__

#include "es_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Convert a pathname to canonical form.  The input path is assumed to contain
 * no duplicate slashes.  On Solaris we can use realpath() to do most of the
 * work, though once that's done we still must collapse any remaining "." and
 * ".." names by hand.
 *
 * @param	path	original path string.
 * @param	out		resolved path string.
 * @param	len		resolved buffer size, must be >= ES_PATH_MAX.
 */
int canonicalize(const char *path, char *out, int len);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_CANONICALIZE_H__ */
