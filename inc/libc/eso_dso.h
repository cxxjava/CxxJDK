/**
 * @file  eso_dso.h
 * @brief Dynamic Object Handling Routines
 */

#ifndef __ES_DSO_H__
#define __ES_DSO_H__

#include "es_comm.h"
#include "es_types.h"

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure for referencing dynamic objects
 */
typedef void      es_dso_t;

/**
 * Load a DSO library.
 * @param path Path to the DSO library
 * @param flags This parameter describes how dlopen()/LoadLibraryEx()
 * will operate upon file with respect to the processing of relocations
 * and the scope of visibility of the symbols provided within file.
 * @return Location to store new handle for the DSO.
 */
es_dso_t* eso_dso_load(const char *path);
es_dso_t* eso_dso_load_ext(const char *path, int flags);

/**
 * Close a DSO library.
 * @param handle handle to close.
 */
void eso_dso_unload(es_dso_t **handle);

/**
 * Load a symbol from a DSO handle.
 * @param ressym Location to store the loaded symbol
 * @param handle handle to load the symbol from.
 * @param symname Name of the symbol to load.
 */
void* eso_dso_sym(es_dso_t *handle,
                          const char *symname);

#ifdef __cplusplus
}
#endif

#endif /* __ES_DSO_H__ */
