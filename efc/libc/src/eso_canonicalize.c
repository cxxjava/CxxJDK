/**
 * @file  eso_canonicalize.c
 * @brief Pathname canonicalization
 */

#include "eso_canonicalize.h"

#ifdef WIN32
#include "canonicalize_win.inl"
#else
#include "canonicalize_unix.inl"
#endif
