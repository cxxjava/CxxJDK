/**
 * @file  eso_file.h
 * @brief ES file foundation
 */

#ifndef __ESO_FILE_H__
#define __ESO_FILE_H__

#include "es_comm.h"
#include "es_types.h"
#include "eso_libc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * FILE Handle
 */
typedef FILE es_file_t;

typedef int es_os_file_t;        /**< native file */

/*
 *	WHENCE Define
 */
#define ES_SEEK_SET SEEK_SET
#define ES_SEEK_CUR SEEK_CUR
#define ES_SEEK_END SEEK_END


#ifdef WIN32
es_file_t* eso_fopen(const char *path, const char *mode);
#else
#define eso_fopen		fopen
#endif
#define eso_fgets		fgets
#define eso_fputs		fputs
#define eso_fgetc		fgetc
#define eso_fputc		fputc
#define eso_fflush		fflush
#define eso_fseek		fseek
#define eso_ftell		ftell
#define eso_feof		feof
#define eso_fclose		fclose
#define eso_ferror		ferror

/**
 * Read the data and return readed data length.
 */
es_size_t eso_fread(void* buffer, es_size_t size, es_file_t* pfile);
es_size_t eso_freadn(void* buffer, es_size_t size, es_file_t* pfile);

/**
 * Write the data and return written data length.
 */
es_size_t eso_fwrite(const void* buffer, es_size_t size, es_file_t* pfile);
es_size_t eso_fwriten(const void* buffer, es_size_t size, es_file_t* pfile);

/**
 * Get file size.
 * @return -1 not exist, else file size.
 */
es_ssize_t eso_fsize(es_file_t* pfile);
es_ssize_t eso_fsize1(int hfile);
es_ssize_t eso_fsize2(const char* filename);

/**
 * Determine whether a file exists.
 * @return 0 not exist, 1 exist.
 */
es_int32_t eso_fexist(const char* filename);

/**
 * Remove a file.
 * @return 0 success, -1 failure.
 */
es_int32_t eso_unlink(const char* filename);

/**
 * Rename a file.
 * @return 0 success, -1 failure.
 */
es_int32_t eso_frename(const char* oldpath, const char* newpath, es_bool_t overwrite);

/**
 * Truncate a file.
 * @return 0 success, -1 failure.
 * @mark file must be open to the write mode
 */
es_int32_t eso_ftruncate(es_file_t* pfile, es_uint32_t size);
es_int32_t eso_ftruncate2(const char* filename, es_uint32_t size);

/**
 * Copy a file.
 * @return 0 success, -1 failure.
 */
es_int32_t eso_fcopy(const char* oldpath, const char* newpath, es_bool_t overwrite);

/**
 * Create a file.
 * @return 0 success, -1 failure.
 */
es_int32_t eso_fcreate(const char* filename, es_bool_t overwrite);

/**
 * Make dirs.
 * @return 0 success, -1 failure.
 * @mark automatic creation of multi-level directory
 */
es_int32_t eso_mkdir(const char* dirname);

/**
 * Remove the last dir.
 * @return 0 success, -1 failure.
 * @mark delete the last level of empty directory only
 */
es_int32_t eso_rmdir(const char* dirname);

/**
 * Get a resolved path.
 */
char* eso_realpath(const char* path, char* resolved_path, es_size_t max_length);

/**
 * Determine whether a path is an absolute path.
 */
es_bool_t eso_pathrooted(const char* path);

/**
 * native os file descriptor
 */
es_os_file_t eso_fileno(es_file_t* pfile);
es_file_t* eso_fdopen(es_os_file_t hfile, const char* mode);

/**
 * Set file buffer.
 */
void eso_file_setbuffer(es_file_t* pfile, char * buf, es_size_t size);

#ifdef __cplusplus
}
#endif

#endif /* __ESO_FILE_H__ */
