/**
 * @file  eso_proc.h
 * @brief OS process Routines
 */

#ifndef __ESO_PROC_H__
#define __ESO_PROC_H__

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <signal.h>

#include "es_types.h"
#include "es_status.h"
#include "eso_pipe.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef WIN32
typedef DWORD             es_os_pid_t;      /**< native process id */
#else
typedef pid_t             es_os_pid_t;      /**< native process id */
#endif

typedef struct es_proc_t {
	/** The process ID */
	es_os_pid_t pid;
#if defined(WIN32)
    /** (Win32 only) Creator's handle granting access to the process
     * @remark This handle is closed and reset to NULL in every case
     * corresponding to a waitpid() on Unix which returns the exit status.
     * Therefore Win32 correspond's to Unix's zombie reaping characteristics
     * and avoids potential handle leaks.
     */
    HANDLE hproc;
#endif
    /** Parent's side of pipe to child's stdin */
    es_pipe_t *in;
    /** Parent's side of pipe to child's stdout */
    es_pipe_t *out;
    /** Parent's side of pipe to child's stdouterr */
    es_pipe_t *err;
} es_proc_t;

/**
 * Init es_proc_t object.
 */
#define ES_PROC_INIT(pproc) do { \
	(pproc)->pid = 0; \
	(pproc)->in = NULL; \
	(pproc)->out = NULL; \
	(pproc)->err = NULL; \
} while(0);


typedef enum {
    ES_WAIT,           /**< wait for the specified process to finish */
    ES_NOWAIT          /**< do not wait -- just see if it has finished */
} es_wait_how_e;

/* I am specifically calling out the values so that the macros below make
 * more sense.  Yes, I know I don't need to, but I am hoping this makes what
 * I am doing more clear.  If you want to add more reasons to exit, continue
 * to use bitmasks.
 */
typedef enum {
    ES_PROC_EXIT = 1,          /**< process exited normally */
    ES_PROC_SIGNAL = 2,        /**< process exited due to a signal */
    ES_PROC_SIGNAL_CORE = 4    /**< process exited and dumped a core file */
} es_exit_why_e;

/** did we exit the process */
#define ES_PROC_CHECK_EXIT(x)        (x & ES_PROC_EXIT)
/** did we get a signal */
#define ES_PROC_CHECK_SIGNALED(x)    (x & ES_PROC_SIGNAL)
/** did we get core */
#define ES_PROC_CHECK_CORE_DUMP(x)   (x & ES_PROC_SIGNAL_CORE)

/**
 * Create a new process and execute a new program within that process.
 * @param proc The resulting process handle.
 * @param progname The program to run 
 * @param args the arguments to pass to the new program.
 * @param envp the environment to pass to the new program.
 * @param currdir Set which directory the child process should start executing in
 * @note This function returns without waiting for the new process to terminate;
 * use eso_proc_wait for that.
 */
es_status_t eso_proc_create(es_proc_t *proc,
							const char *progname,
							const char * const *args,
							const char * const *envp,
							const char *currdir,
							es_bool_t detached);

/**
 * This executes a standard unix fork.
 * @param proc The resulting process handle. 
 * @remark returns ES_INCHILD for the child, and ES_INPARENT for the parent
 * or an error.
 */
es_status_t eso_proc_fork(es_proc_t *proc);

/**
 * Wait for a child process to die
 * @param proc The process handle that corresponds to the desired child process 
 * @param exitcode The returned exit status of the child, if a child process 
 *                 dies, or the signal that caused the child to die.
 *                 On platforms that don't support obtaining this information, 
 *                 the status parameter will be returned as ES_ENOTIMPL.
 * @param exitwhy Why the child died, the bitwise or of:
 * <PRE>
 *            ES_PROC_EXIT         -- process terminated normally
 *            ES_PROC_SIGNAL       -- process was killed by a signal
 *            ES_PROC_SIGNAL_CORE  -- process was killed by a signal, and
 *                                     generated a core dump.
 * </PRE>
 * @param waithow How should we wait.  One of:
 * <PRE>
 *            ES_WAIT   -- block until the child process dies.
 *            ES_NOWAIT -- return immediately regardless of if the 
 *                          child is dead or not.
 * </PRE>
 * @remark The childs status is in the return code to this process.  It is one of:
 * <PRE>
 *            ES_CHILD_DONE     -- child is no longer running.
 *            ES_CHILD_NOTDONE  -- child is still running.
 * </PRE>
 */
es_status_t eso_proc_wait(es_proc_t *proc,
                          int *exitcode,
                          es_exit_why_e *exitwhy,
                          es_wait_how_e waithow);

/**
 * Wait for any current child process to die and return information 
 * about that child.
 * @param proc Pointer to NULL on entry, will be filled out with child's 
 *             information 
 * @param exitcode The returned exit status of the child, if a child process 
 *                 dies, or the signal that caused the child to die.
 *                 On platforms that don't support obtaining this information, 
 *                 the status parameter will be returned as ES_ENOTIMPL.
 * @param exitwhy Why the child died, the bitwise or of:
 * <PRE>
 *            ES_PROC_EXIT         -- process terminated normally
 *            ES_PROC_SIGNAL       -- process was killed by a signal
 *            ES_PROC_SIGNAL_CORE  -- process was killed by a signal, and
 *                                     generated a core dump.
 * </PRE>
 * @param waithow How should we wait.  One of:
 * <PRE>
 *            ES_WAIT   -- block until the child process dies.
 *            ES_NOWAIT -- return immediately regardless of if the 
 *                          child is dead or not.
 * </PRE>
 * @param p Pool to allocate child information out of.
 * @bug Passing proc as a *proc rather than **proc was an odd choice
 * for some platforms... this should be revisited in 1.0
 */
es_status_t eso_proc_wait_any_proc(es_proc_t *proc,
                                   int *exitcode,
                                   es_exit_why_e *exitwhy,
                                   es_wait_how_e waithow);

/**
 * Detach the process from the controlling terminal.
 * @param daemonize set to TRUE if the process should daemonize
 *                  and become a background process, else it will
 *                  stay in the foreground.
 * @param in if 0 then close stdin
 * @param out if 0 then close stdout
 * @param err if 0 then close stderr
 */
es_status_t eso_proc_detach(es_bool_t daemonize,
                            int in, int out, int err);

/** 
 * Terminate a process.
 * @param proc The process to terminate.
 * @param sig How to kill the process.
 */
es_status_t eso_proc_kill(es_proc_t *proc, int sig);

/**
 * native os thread handler
 */
es_os_pid_t eso_os_process_current(void);

#ifdef __cplusplus
}
#endif

#endif /* !__ESO_PROC_H__ */

