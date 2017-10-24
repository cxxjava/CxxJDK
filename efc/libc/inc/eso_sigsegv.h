/**
 * @file  eso_sigsegv.h
 * @brief ES stack-trace for segfaults
 */

#ifndef __ESO_SIGSEGV_H__
#define __ESO_SIGSEGV_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Install signal handler
 *  link add: -rdynamic -ldl
 */
int eso_setup_sigsegv(const char *dumpfile);

#ifdef __cplusplus
}
#endif

#endif	/* __ESO_SIGSEGV_H__ */
