/**
 * @file  eso_sys.c
 * @brief ES System Assistant.
 */

#include "eso_sys.h"
#include "eso_string.h"
#include "eso_signal.h"
#include "eso_util.h"

#ifdef WIN32
	#include <windows.h>
	#include <winbase.h>
	#include <direct.h>
#elif defined(__MTK__)
	//
#elif defined(__APPLE__)
	#include <mach-o/dyld.h>
	#include <unistd.h>
    #include <execinfo.h>
#elif defined(__CYGWIN32__)
	#include <unistd.h>
#else
	#include <execinfo.h>
	#include <unistd.h>
#endif

//==============================================================================

es_bool_t ES_BIG_ENDIAN = FALSE;

static int initialized = 0;

#ifdef WIN32

//init socket env
extern int eso_net_startup(void);
//clearup socket env
extern int eso_net_cleanup(void);

extern DWORD g_tls_thread;

es_status_t eso_initialize(void)
{
	if (initialized++) {
        return ES_SUCCESS;
    }
    
	ES_BIG_ENDIAN = eso_is_bigendian();

    g_tls_thread = TlsAlloc();
    
	eso_net_startup();

	return ES_SUCCESS;
}

void eso_terminate(void)
{
	eso_net_cleanup();
	
	TlsFree(g_tls_thread);
}

#else

es_status_t eso_initialize(void)
{
	if (initialized++) {
        return ES_SUCCESS;
    }

	ES_BIG_ENDIAN = eso_is_bigendian();

	/* ignore SIGPIPE */
	eso_signal(SIGPIPE, SIG_IGN);  //20130617

	//

	return ES_SUCCESS;
}

void eso_terminate(void)
{
	//
}

#endif

//==============================================================================

int eso_wordsize_get(void)
{
	return __WORDSIZE;
}

es_bool_t eso_is_bigendian(void)
{
	unsigned short test = 0x1122;
	return (*( (unsigned char*) &test ) == 0x11);
}

//==============================================================================

#define ES_MAX_TRACE_LEVEL 20

#if defined(WIN32)

char* eso_backtrace_get_from(es_string_t **trace, int top)
{
	if (!trace || top < 0 || top > ES_MAX_TRACE_LEVEL) return NULL;
	if (top == 0) top = ES_MAX_TRACE_LEVEL;
	
	//TODO...
	eso_mstrcpy(trace, "~!@#$%^&*(");

	return *trace;
}

#else //linux

char* eso_backtrace_get_from(es_string_t **trace, int top)
{
	void *callers_stack[ES_MAX_TRACE_LEVEL];
	int frames;
	char **symbol;
	int i;
	
	if (!trace || top < 0 || top > ES_MAX_TRACE_LEVEL) return NULL;
	if (top == 0) top = ES_MAX_TRACE_LEVEL;
	
	eso_mmeminit((es_data_t**)trace);
	
	frames = backtrace(callers_stack, top);
	symbol = backtrace_symbols(callers_stack, frames);
	for (i = top; i < frames; i++) {
		eso_mstrcat(trace, symbol[i]);
		eso_mstrcat(trace, "\n");
	}
	free(symbol);
	
	return *trace;
}

#endif //!WIN32

char* eso_backtrace_get(es_string_t **trace)
{
	return eso_backtrace_get_from(trace, 0);
}

//==============================================================================

char* eso_current_workpath(es_string_t **path)
{
	char sbuf[ES_PATH_MAX];
#ifdef WIN32
	if (GetModuleFileNameA(NULL, sbuf, ES_PATH_MAX) != 0) {
		const char *s = eso_strrchr(sbuf, '\\');
		eso_mstrncpy(path, sbuf, s - sbuf);
		return *path;
	}
	return NULL;
#elif defined(__MTK__)
	return NULL;
#else //linux
	if (getcwd(sbuf, ES_PATH_MAX) != NULL) {
		eso_mstrcpy(path, sbuf);
		return *path;
	}
	return NULL;
#endif
}

/**
 * @see: http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
 * Mac OS X: _NSGetExecutablePath() (man 3 dyld)
 * Linux: readlink /proc/self/exe
 * Solaris: getexecname()
 * FreeBSD: sysctl CTL_KERN KERN_PROC KERN_PROC_PATHNAME -1
 * FreeBSD if it has procfs: readlink /proc/curproc/file (FreeBSD doesn't have procfs by default)
 * NetBSD: readlink /proc/curproc/exe
 * DragonFly BSD: readlink /proc/curproc/file
 * Windows: GetModuleFileName() with hModule = NULL
 */
char* eso_execute_filename(es_string_t **path)
{
	char sbuf[ES_PATH_MAX];
#ifdef WIN32
	if (_getcwd(sbuf, ES_PATH_MAX) != NULL) {
		eso_mstrcpy(path, sbuf);
		return *path;
	}
	return NULL;
#elif defined(__MTK__)
	return NULL;
#elif defined(__APPLE__)
	{
		uint32_t size = sizeof(sbuf);
		int res = _NSGetExecutablePath(sbuf, &size);
		char *p = eso_filepath_name_get(sbuf);
		eso_mstrcpy(path, p);
		return *path;
	}
#else //linux
	es_ssize_t len;
	if ((len = readlink("/proc/self/exe", sbuf, ES_PATH_MAX)) != -1) {
		eso_mstrncpy(path, sbuf, len);
		return *path;
	}
	return NULL;
#endif
}
