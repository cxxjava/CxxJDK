/**
 * @file  eso_pipe.h
 * @brief Pipe routines
 */

#ifndef __ESO_PIPE_H__
#define __ESO_PIPE_H__

#include "es_comm.h"
#include "es_types.h"
#include "eso_file.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct es_pipe_t es_pipe_t;
struct es_pipe_t {
	es_file_t *in;  //in for write.
	es_file_t *out; //out for read.
};

/**
 * Create pipe.
 */
es_pipe_t* eso_pipe_create(void);

/**
 * Destroy pipe.
 */
void eso_pipe_destroy(es_pipe_t **pipe);

#ifdef __cplusplus
}
#endif

#endif  /* ! __ESO_PIPE_H__ */
