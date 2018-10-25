/**
 * @file  eso_file.c
 * @brief ES file foundation
 */

#include "es_comm.h"
#include "es_types.h"
#include "eso_file.h"

#include <stdio.h>

#if defined(WIN32)
#include <direct.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <io.h>
#include <windows.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#ifdef WIN32
#define FSTAT    _fstat
#define STAT     _stat
#define FILENO   _fileno
#define FDOPEN   _fdopen
#define DUP      _dup

#define DIRDIV   '\\'
#define ACCESS   _access
#define MKDIR(a) _mkdir((a))
#define RMDIR(a) _rmdir((a))
#else
#define FSTAT    fstat
#define STAT     stat
#define FILENO   fileno
#define FDOPEN   fdopen
#define DUP      dup

#define DIRDIV   '/'
#define ACCESS   access
#define MKDIR(a) mkdir((a),0755)
#define RMDIR(a) rmdir((a))
#endif //!WIN32

#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifdef WIN32
es_file_t* eso_fopen(const char *path, const char *mode)
{
	char bmode[32];
	
	ES_ASSERT(path);
	ES_ASSERT(mode);
	
	eso_strncpy(bmode, mode, sizeof(bmode)-2);
	if (eso_strchr(bmode, 'b') == 0) {
		eso_strcat(bmode, "b");
	}
	
	return (es_file_t*)fopen(path, bmode);
}
#endif

/*
 *	读数据
 */
es_size_t eso_fread(void* buffer, es_size_t size, es_file_t* pfile)
{
	return fread(buffer, 1, size, pfile);
}

es_size_t eso_freadn(void* buffer, es_size_t size, es_file_t* pfile)
{
	es_size_t n = 0;
	es_size_t count = 0;
	do {
		n = fread((char*) buffer + count, 1, size - count, pfile);
		count += n;
		if (eso_ferror(pfile) && errno != EINTR) {
			break;
		}
	} while ((count < size) && !eso_feof(pfile));
	return count;
}

/*
 * 写数据
 */
es_size_t eso_fwrite(const void* buffer, es_size_t size, es_file_t* pfile)
{
	return fwrite(buffer, 1, size, pfile);
}

es_size_t eso_fwriten(const void* buffer, es_size_t size, es_file_t* pfile) {
	es_size_t n = 0;
	es_size_t count = 0;
	do {
		n = fwrite((char*)buffer + count, 1, size - count, pfile);
		count += n;
		if (eso_ferror(pfile) && errno != EINTR) {
			break;
		}
	} while (count < size);
	return count;
}

/*
 *	获取文件大小, 文件不存在返回 -1，存在返回文件大小
 */
es_ssize_t eso_fsize(es_file_t* pfile)
{
	struct STAT buf;
	int fd = fileno(pfile);
	if (FSTAT(fd,&buf) != 0) {
		return -1;
	}
	return buf.st_size;
}

/*
 *	获取文件大小, 文件不存在返回 -1，存在返回文件大小
 */
es_ssize_t eso_fsize1(int hfile)
{
	struct STAT buf;
	if (FSTAT(hfile,&buf) != 0) {
		return -1;
	}
	return buf.st_size;
}

/*
 *	获取文件大小, 文件不存在返回 -1，存在返回文件大小
 */
es_ssize_t eso_fsize2(const char* filename)
{
	struct STAT buf;
	if (STAT(filename, &buf) != 0) {
		return -1;
	}
	return buf.st_size;
}

/*
 *	判断文件是否存在, 文件不存在返回0，存在返回1
 */
es_int32_t eso_fexist(const char* filename)
{
	struct STAT buf;
	if (STAT(filename, &buf) != 0)
		return 0;
	if (S_ISDIR(buf.st_mode))
		return 0;
	return 1;
}

/*
 *	删除文件, 成功则返回0，失败返回-1
 */
es_int32_t eso_unlink(const char* filename)
{
	return unlink(filename);
}

/*
 *	文件重命名, 成功则返回0，失败返回-1
 */
es_int32_t eso_frename(const char* oldpath, const char* newpath, es_bool_t overwrite)
{
	if (!overwrite && eso_fexist(newpath)) {
		return -1;
	}
	return rename(oldpath, newpath);
}

/*
 *	文件截短, 成功则返回0，失败返回-1
 *  文件必须是写入模式打开
 */
es_int32_t eso_ftruncate(es_file_t *pfile, es_uint32_t size)
{
#ifdef WIN32
	es_size_t oldpos;
	HANDLE hfile;
	int ret;
	
	oldpos = eso_ftell(pfile);
	hfile = (HANDLE)_get_osfhandle(fileno(pfile));
	SetFilePointer(hfile, size, NULL, FILE_BEGIN);
	ret = SetEndOfFile(hfile);
	eso_fseek(pfile, oldpos, SEEK_SET);
	return ret;
#else
	int fd = fileno(pfile);
	return ftruncate(fd, size);
#endif
}

/*
 *	文件截短, 成功则返回0，失败返回-1
 */
es_int32_t eso_ftruncate2(const char *filename, es_uint32_t size)
{
#ifdef WIN32
	es_int32_t ret;
	es_file_t *pf = eso_fopen(filename, "rb");
	if (!pf) {
		return -1;
	}
	ret = eso_ftruncate(pf, size);
	eso_fclose(pf);
	return ret;
#else
	return truncate(filename, size);
#endif
}

/*
 *	文件复制, 成功则返回0，失败返回-1
 */
es_int32_t eso_fcopy(const char* oldpath, const char* newpath, es_bool_t overwrite)
{
	FILE *from_fd, *to_fd;
	es_size_t bytes_read, bytes_write;
	char buffer[512];
	char *ptr;
	
	if (!overwrite && eso_fexist(newpath)) {
		return -1;
	}
	
	/* 打开源文件 */
	if ((from_fd = fopen(oldpath, "rb")) == NULL) {
		return -1;
	}
	
	/* 创建目的文件 */
	if ((to_fd = fopen(newpath, "wb")) == NULL) {
		fclose(from_fd);
		return -1;
	}
	
	while (TRUE) {
		bytes_read = eso_fread(buffer, sizeof(buffer), from_fd);
		
		if (eso_ferror(from_fd) && errno == EINTR) {
			continue;
		}
		
		if (eso_feof(from_fd)) { //read finished!
			break;
		}
		else if (bytes_read > 0) {
			ptr = buffer;
			
			while (TRUE) {
				do {
					bytes_write = eso_fwrite(buffer, bytes_read, to_fd);
				} while (eso_ferror(to_fd) && errno == EINTR);
				
				if (eso_ferror(to_fd)) {
					goto FAILURE;
				}
				
				if (bytes_write == bytes_read) //write finished!
					break;
				else if (bytes_write > 0) {
					ptr += bytes_write;
					bytes_read -= bytes_write;
				}
			} //!while
		}
		else {
			goto FAILURE;
		}
	} //!while
	
	fclose(from_fd);
	fclose(to_fd);
	
	return 0;
	
FAILURE:
	fclose(from_fd);
	fclose(to_fd);
	
	return -1;
}

/*
 * 创建新文件, 成功则返回0，失败返回-1
 */
es_int32_t eso_fcreate(const char* filename, es_bool_t overwrite)
{
	es_file_t *pf;
	
	if (!filename || !*filename) {
		return -1;
	}
	
	if (overwrite || (!overwrite && !eso_fexist(filename))) {
		pf = eso_fopen(filename, "wb");
		eso_fclose(pf);
		
		return pf ? 0 : -1;
	}
	return -1;
}

/*
 * 创建文件路径, 成功则返回0，失败返回-1
 * 支持自动创建多级目录
 */
es_int32_t eso_mkdir(const char *dirname)
{
	int i,len;
	char p[ES_PATH_MAX] = {0};
	
	if (!dirname || !*dirname) {
		return -1;
	}
	
	eso_strncpy(p, dirname, sizeof(p));

	len = strlen(p);
	for(i=0; i<len; i++) {
		if (p[i] == DIRDIV) {
			p[i] = '\0';
			if (*p && ACCESS(p, 0)) {
				if (MKDIR(p) != 0) {
					return -1;
				}
			}
			p[i] = DIRDIV;
		}
	}
	
	//创建最后一级目录
	if (*p && ACCESS(p, 0)) {
		if (MKDIR(p) != 0) {
			return -1;
		}
	}
	
	return 0;
}

/*
 * 删除文件路径, 成功则返回0，失败返回-1
 * 仅支持删除最后一级空目录
 */
es_int32_t eso_rmdir( const char *dirname )
{
	return RMDIR(dirname);
}

char* eso_realpath(const char *path, char *resolved_path, es_size_t max_length)
{
#ifdef WIN32
	return _fullpath(resolved_path, (char*)path, max_length);
#else
	char tmp[ES_PATH_MAX] = {0};
	char *p = realpath(path, tmp);
	if (p) {
		eso_strncpy(resolved_path, p, max_length);
		return resolved_path;
	}
	return NULL;
#endif	
}

es_bool_t eso_pathrooted(const char* path)
{
	if (!path) return FALSE;

#ifdef WIN32
	return (eso_strlen(path) > 1) && (path[1] == ':');
#else
	return path[0] == DIRDIV;
#endif
}

es_os_file_t eso_fileno(es_file_t *pfile)
{
	return FILENO(pfile);
}

es_file_t* eso_fdopen(es_os_file_t hfile, const char* mode)
{
	int newfd = DUP(hfile);
	if (newfd == -1) {
		return NULL;
	}
	return FDOPEN(newfd, mode);
}

void eso_file_setbuffer(es_file_t *pfile, char *buf, es_size_t size)
{
	setvbuf(pfile, buf, (buf || size>0)?_IOFBF:_IONBF, size);
}
