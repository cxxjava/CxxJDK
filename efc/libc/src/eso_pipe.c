/**
 * @file  eso_pipe.c
 * @brief Pipe routines
 */

#include "eso_pipe.h"

#ifdef WIN32

#include <windows.h>
#include <io.h>
#include <fcntl.h>

es_pipe_t* eso_pipe_create(void)
{
	es_pipe_t *new_pipe;
	HANDLE filehand_in;
	HANDLE filehand_out;
	SECURITY_ATTRIBUTES sa;
	int fd_in, fd_out;
	
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
	
	if (!CreatePipe(&filehand_in, &filehand_out, &sa, 65536)) {
		return NULL;
	}
	
	fd_in = _open_osfhandle((long)filehand_in, _O_BINARY | _O_RDONLY);
	fd_out = _open_osfhandle((long)filehand_out, _O_BINARY | _O_WRONLY | _O_APPEND);
	
	new_pipe = (es_pipe_t*)eso_calloc(sizeof(es_pipe_t));
	new_pipe->in = fdopen(fd_in, "rb");
	new_pipe->out = fdopen(fd_out, "wb");
	eso_file_setbuffer(new_pipe->in, NULL, 0);
	eso_file_setbuffer(new_pipe->out, NULL, 0);
    
    return new_pipe;
}

#else //linux

#include <unistd.h>
#include <fcntl.h>

#ifndef O_NOATIME
#define O_NOATIME     01000000 /* Do not set atime.  */
#endif

es_pipe_t* eso_pipe_create(void)
{
	es_pipe_t *new_pipe;
	int filedes[2];

    if (pipe(filedes) == -1) {
        return NULL;
    }
    
    //@see: http://rdc.taobao.com/blog/cs/?p=1295
    fcntl(filedes[0], F_SETFL, O_NOATIME);

    new_pipe = (es_pipe_t*)eso_calloc(sizeof(es_pipe_t));
    new_pipe->in = fdopen(filedes[0], "rb");
    new_pipe->out = fdopen(filedes[1], "wb");
    eso_file_setbuffer(new_pipe->in, NULL, 0);
    eso_file_setbuffer(new_pipe->out, NULL, 0);
    
    return new_pipe;
}
#endif //!WIN32

void eso_pipe_destroy(es_pipe_t **pipe)
{
	if (!pipe || !*pipe) {
		return;
	}
	
	if ((*pipe)->in != NULL) {
		eso_fclose((*pipe)->in);
	}
	if ((*pipe)->out != NULL) {
		eso_fclose((*pipe)->out);
	}
	
	ESO_FREE(pipe);
}
