/**
 * @file  eso_dso.c
 * @brief Dynamic Object Handling Routines
 */

#include "eso_dso.h"

#ifdef WIN32

es_dso_t* eso_dso_load(const char *path)
{
	return eso_dso_load_ext(path, LOAD_WITH_ALTERED_SEARCH_PATH);
}

es_dso_t* eso_dso_load_ext(const char *path, int flags)
{
	HINSTANCE os_handle = NULL;
	UINT em;

	/* Prevent ugly popups from killing our app */
	em = SetErrorMode(SEM_FAILCRITICALERRORS);
	os_handle = LoadLibraryEx(path, NULL, flags);
	if (!os_handle && flags) {
		os_handle = LoadLibraryEx(path, NULL, 0);
	}
	SetErrorMode(em);

	return os_handle;
}

void eso_dso_unload(es_dso_t **handle)
{
	if (!handle || !*handle) {
		return;
	}

	FreeLibrary(*handle);

	*handle = NULL;
}

void* eso_dso_sym(es_dso_t *handle,
                          const char *symname)
{
	return GetProcAddress(handle, symname);
}

#else //linux

es_dso_t* eso_dso_load(const char *path)
{
	return eso_dso_load_ext(path, RTLD_NOW);
}

es_dso_t* eso_dso_load_ext(const char *path, int flags)
{
#ifdef _AIX
    if (eso_strchr(path + 1, '(') && path[eso_strlen(path) - 1] == ')')
    {
        /* This special archive.a(dso.so) syntax is required for
         * the way libtool likes to build shared libraries on AIX.
         * dlopen() support for such a library requires that the
         * RTLD_MEMBER flag be enabled.
         */
        flags |= RTLD_MEMBER;
    }
#endif
    return dlopen(path, flags);
}

void eso_dso_unload(es_dso_t **handle)
{
	if (!handle || !*handle) {
		return;
	}

	dlclose(*handle);
	
	*handle = NULL;
}

void* eso_dso_sym(es_dso_t *handle,
                          const char *symname)
{
	return dlsym(handle, symname);
}

#endif //!WIN32
