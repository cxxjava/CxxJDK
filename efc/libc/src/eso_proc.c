/**
 * @file  eso_proc.c
 * @brief OS process Routines
 */

#include "eso_proc.h"
#include "eso_string.h"

#ifdef WIN32

#include <TlHelp32.h>
#include <io.h> //_get_osfhandle

static es_exit_why_e why_from_exit_code(DWORD exit) {
    /* See WinNT.h STATUS_ACCESS_VIOLATION and family for how
     * this class of failures was determined
     */
    if (((exit & 0xC0000000) == 0xC0000000) 
                    && !(exit & 0x3FFF0000))
        return ES_PROC_SIGNAL;
    else
        return ES_PROC_EXIT;

    /* ### No way to tell if Dr Watson grabbed a core, AFAICT. */
}

static const char* has_space(const char *str)
{
	const char *ch;
	for (ch = str; *ch; ++ch) {
		if (eso_isspace(*ch)) {
			return ch;
		}
	}
	return NULL;
}

es_status_t eso_proc_create(es_proc_t *proc,
							const char *progname,
							const char * const *args,
							const char * const *envp,
							const char *currdir,
							es_bool_t detached)
{
	PROCESS_INFORMATION pi;
	DWORD dwCreationFlags = 0;
	char fullpath_progname[ES_PATH_MAX] = {0};
	es_string_t *cmdline = NULL;
	es_status_t rv;
	int i;

	STARTUPINFOA si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);

	if (eso_realpath(progname, fullpath_progname, sizeof(fullpath_progname)) == NULL) {
		return ES_NOTFOUND;
	}

	if (detached) {
		/* 
         * XXX DETACHED_PROCESS won't on Win9x at all; on NT/W2K 
         * 16 bit executables fail (MS KB: Q150956)
         */
		dwCreationFlags |= DETACHED_PROCESS;
	}
	
	/* Handle the args, seperate from argv0 */
	cmdline = eso_mstrdup(fullpath_progname);
	for (i = 1; args && args[i]; ++i) { //ignore program at first.
		if (has_space(args[i]) || !args[i][0]) {
			cmdline = eso_mstrcat(&cmdline, " \"");
			cmdline = eso_mstrcat(&cmdline, args[i]);
			cmdline = eso_mstrcat(&cmdline, "\"");
		}
		else {
			cmdline = eso_mstrcat(&cmdline, " ");
			cmdline = eso_mstrcat(&cmdline, args[i]);
		}
	}

	if (detached) {
		si.dwFlags |= STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;
	}

	if (proc->in || proc->out || proc->err) {
		si.dwFlags |= STARTF_USESTDHANDLES;

		si.hStdInput = proc->in
			? (HANDLE)_get_osfhandle(fileno(proc->in->in))
			: GetStdHandle(STD_INPUT_HANDLE);

		si.hStdOutput = proc->out
			? (HANDLE)_get_osfhandle(fileno(proc->out->out))
			: GetStdHandle(STD_OUTPUT_HANDLE);

		si.hStdError = proc->err
			? (HANDLE)_get_osfhandle(fileno(proc->err->out))
			: GetStdHandle(STD_ERROR_HANDLE);
	}

	rv = CreateProcess(progname, cmdline, /* Command line */
						NULL, NULL,        /* Proc & thread security attributes */
						TRUE,              /* Inherit handles */
						dwCreationFlags,   /* Creation flags */
						envp,         /* Environment block */
						currdir,     /* Current directory name */
						&si, &pi);
	eso_mfree(cmdline);

	/* Check CreateProcess result 
     */
    if (!rv)
        return GetLastError();

	/* XXX Orphaned handle warning - no fix due to broken apr_proc_t api.
     */
    proc->hproc = pi.hProcess;
    proc->pid = pi.dwProcessId;

    CloseHandle(pi.hThread);

    return ES_SUCCESS;
}

es_status_t eso_proc_fork(es_proc_t *proc)
{
	return ES_ENOTIMPL;
}

es_status_t eso_proc_wait(es_proc_t *proc,
						  int *exitcode,
						  es_exit_why_e *exitwhy,
						  es_wait_how_e waithow)
{
	DWORD stat;
	DWORD time;

	if (waithow == ES_WAIT)
		time = INFINITE;
	else
		time = 0;

	if ((stat = WaitForSingleObject(proc->hproc, time)) == WAIT_OBJECT_0) {
		if (GetExitCodeProcess(proc->hproc, &stat)) {
			if (exitcode)
				*exitcode = stat;
			if (exitwhy)
				*exitwhy = why_from_exit_code(stat);
			CloseHandle(proc->hproc);
			proc->hproc = NULL;
			return ES_CHILD_DONE;
		}
	}
	else if (stat == WAIT_TIMEOUT) {
		return ES_CHILD_NOTDONE;
	}
	return GetLastError();
}

es_status_t eso_proc_wait_any_proc(es_proc_t *proc,
								   int *exitcode,
								   es_exit_why_e *exitwhy,
								   es_wait_how_e waithow)
{
	DWORD  dwId = GetCurrentProcessId();
	DWORD  i;
	DWORD  nChilds = 0;
	DWORD  nActive = 0;
	HANDLE ps32;
	PROCESSENTRY32W pe32;
	BOOL   bHasMore = FALSE;
	DWORD  dwFlags  = PROCESS_QUERY_INFORMATION;
	es_status_t rv = ES_FAILURE;
	
	if (waithow == ES_WAIT)
	    dwFlags |= SYNCHRONIZE;
	if (!(ps32 = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0))) {
	    return GetLastError();
	}
	pe32.dwSize = sizeof(PROCESSENTRY32W);
	if (!Process32FirstW(ps32, &pe32)) {
	    if (GetLastError() == ERROR_NO_MORE_FILES)
	        return ES_EOF;
	    else
	        return GetLastError();
	}
	do {
	    DWORD  dwRetval = 0;
	    DWORD  nHandles = 0;
	    HANDLE hProcess = NULL;
	    HANDLE pHandles[MAXIMUM_WAIT_OBJECTS];
	    do {
	        if (pe32.th32ParentProcessID == dwId) {
	            nChilds++;
	            if ((hProcess = OpenProcess(dwFlags, FALSE,
	                                        pe32.th32ProcessID)) != NULL) {
	                if (GetExitCodeProcess(hProcess, &dwRetval)) {
	                    if (dwRetval == STILL_ACTIVE) {
	                        nActive++;
	                        if (waithow == ES_WAIT)
	                            pHandles[nHandles++] = hProcess;
	                        else
	                            CloseHandle(hProcess);
	                    }
	                    else {                                
	                        /* Process has exited.
	                         * No need to wait for its termination.
	                         */
	                        CloseHandle(hProcess);
	                        if (exitcode)
	                            *exitcode = dwRetval;
	                        if (exitwhy)
	                            *exitwhy  = why_from_exit_code(dwRetval);
	                        proc->pid = pe32.th32ProcessID;
	                    }
	                }
	                else {
	                    /* Unexpected error code.
	                     * Cleanup and return;
	                     */
	                    rv = GetLastError();
	                    CloseHandle(hProcess);
	                    for (i = 0; i < nHandles; i++)
	                        CloseHandle(pHandles[i]);
	                    return rv;
	                }
	            }
	            else {
	                /* This is our child, so it shouldn't happen
	                 * that we cannot open our child's process handle.
	                 * However if the child process increased the
	                 * security token it might fail.
	                 */
	            }
	        }
	    } while ((bHasMore = Process32NextW(ps32, &pe32)) &&
	             nHandles < MAXIMUM_WAIT_OBJECTS);
	    if (nHandles) {
	        /* Wait for all collected processes to finish */
	        DWORD waitStatus = WaitForMultipleObjects(nHandles, pHandles,
	                                                  TRUE, INFINITE);
	        for (i = 0; i < nHandles; i++)
	            CloseHandle(pHandles[i]);
	        if (waitStatus == WAIT_OBJECT_0) {
	            /* Decrease active count by the number of awaited
	             * processes.
	             */
	            nActive -= nHandles;
	        }
	        else {
	            /* Broken from the infinite loop */
	            break;
	        }
	    }
	} while (bHasMore);
	CloseHandle(ps32);
	if (waithow != ES_WAIT) {
	    if (nChilds && nChilds == nActive) {
	        /* All child processes are running */
	        rv = ES_CHILD_NOTDONE;
	        proc->pid = -1;
	    }
	    else {
	        /* proc->pid contains the pid of the
	         * exited processes
	         */
	        rv = ES_CHILD_DONE;
	    }
	}
	if (nActive == 0) {
	    rv = ES_CHILD_DONE;
	    proc->pid = -1;
	}
	return rv;
}

es_status_t eso_proc_detach(es_bool_t daemonize,
							int in, int out, int err)
{
	return ES_ENOTIMPL;
}

es_status_t eso_proc_kill(es_proc_t *proc, int sig)
{
	if (proc->hproc != NULL) {
        if (TerminateProcess(proc->hproc, sig) == 0) {
            return GetLastError();
        }
        /* On unix, SIGKILL leaves a apr_proc_wait()able pid lying around, 
         * so we will leave hproc alone until the app calls apr_proc_wait().
         */
        return ES_SUCCESS;
    }
    return ES_FAILURE;
}

es_os_pid_t eso_os_process_current(void)
{
	return GetCurrentProcessId();
}

#else //linux

#include <time.h>
#include <sys/wait.h>

#include "eso_signal.h"

es_status_t eso_proc_create(es_proc_t *proc,
							const char *progname,
							const char * const *args,
							const char * const *envp,
							const char *currdir,
							es_bool_t detached)
{
	char fullpath_progname[ES_PATH_MAX] = {0};

	if (eso_realpath(progname, fullpath_progname, sizeof(fullpath_progname)) == NULL) {
		return ES_NOTFOUND;
	}

	if ((proc->pid = fork()) < 0) {
		return errno;
	}
	else if (proc->pid == 0) {
		/* child process */

		eso_signal(SIGCHLD, SIG_DFL); /* not sure if this is needed or not */

		if (currdir != NULL) {
			if (chdir(currdir) == -1) {
				_exit(-1); /* Change of working directory failed. */
			}
		}

		if (detached) {
			eso_proc_detach(TRUE, 0, 0, 0);
		}
		else {
			if (proc->in) {
				if (proc->in->in) {
					dup2(eso_fileno(proc->in->in), STDIN_FILENO);
					eso_fclose(proc->in->in);
					proc->in->in = NULL;
				}
				if (proc->in->out) {
					eso_fclose(proc->in->out);
					proc->in->out = NULL;
				}
			}
			if (proc->out) {
				if (proc->out->out) {
					dup2(eso_fileno(proc->out->out), STDOUT_FILENO);
					eso_fclose(proc->out->out);
					proc->out->out = NULL;
				}
				if (proc->out->in) {
					eso_fclose(proc->out->in);
					proc->out->in = NULL;
				}
			}
			if (proc->err) {
				if (proc->err->out) {
					dup2(eso_fileno(proc->err->out), STDERR_FILENO);
					eso_fclose(proc->err->out);
					proc->err->out = NULL;
				}
				if (proc->err->in) {
					eso_fclose(proc->err->in);
					proc->err->in = NULL;
				}
			}
		}

		execve(fullpath_progname, (char * const *)args, (char * const *)envp);

		_exit(-1);  /* if we get here, there is a problem, so exit with an
		             * error code. */
	}

	/* Parent process */
	return ES_SUCCESS;
}

es_status_t eso_proc_fork(es_proc_t *proc)
{
	if ((proc->pid = fork()) < 0) {
		return errno;
	}
	else if (proc->pid == 0) {
		proc->in = NULL;
		proc->out = NULL;
		proc->err = NULL;

		/* standard way to get rand() return different values. */
		srand((unsigned int) (time(NULL) + getpid()));

		return ES_INCHILD;
	}

	proc->in = NULL;
	proc->out = NULL;
	proc->err = NULL;

	return ES_INPARENT;
}

es_status_t eso_proc_wait(es_proc_t *proc,
                          int *exitcode,
                          es_exit_why_e *exitwhy,
                          es_wait_how_e waithow)
 {
	pid_t pstatus;
	int waitpid_options = WUNTRACED;
	int exit_int;
	int ignore;
	es_exit_why_e ignorewhy;

	if (exitcode == NULL) {
		exitcode = &ignore;
	}

	if (exitwhy == NULL) {
		exitwhy = &ignorewhy;
	}

	if (waithow != ES_WAIT) {
		waitpid_options |= WNOHANG;
	}

	do {
		pstatus = waitpid(proc->pid, &exit_int, waitpid_options);
	} while (pstatus < 0 && errno == EINTR);

	if (pstatus > 0) {
		proc->pid = pstatus;

		if (WIFEXITED(exit_int)) {
			*exitwhy = ES_PROC_EXIT;
			*exitcode = WEXITSTATUS(exit_int);
		} else if (WIFSIGNALED(exit_int)) {
			*exitwhy = ES_PROC_SIGNAL;

#ifdef WCOREDUMP
			if (WCOREDUMP(exit_int)) {
				*exitwhy |= ES_PROC_SIGNAL_CORE;
			}
#endif

			*exitcode = WTERMSIG(exit_int);
		}

		return ES_CHILD_DONE;
	} else if (pstatus == 0) {
		return ES_CHILD_NOTDONE;
	}

	return errno;
}

es_status_t eso_proc_wait_any_proc(es_proc_t *proc,
                                   int *exitcode,
                                   es_exit_why_e *exitwhy,
                                   es_wait_how_e waithow)
{
	proc->pid = -1;
	return eso_proc_wait(proc, exitcode, exitwhy, waithow);
}

es_status_t eso_proc_detach(es_bool_t daemonize,
                            int in, int out, int err)
{
	int x;

	if (daemonize) {
		if ((x = fork()) > 0) {
			exit(0);
		} else if (x == -1) {
			perror("fork");
			fprintf(stderr, "unable to fork new process\n");
			exit(1); /* we can't do anything here, so just exit. */
		}
		/* RAISE_SIGSTOP(DETACH); */
	}

	/* A setsid() failure is not fatal if we didn't just fork().
	 * The calling process may be the process group leader, in
	 * which case setsid() will fail with EPERM.
	 */
	if (setsid() == -1 && daemonize) {
		return errno;
	}

	/* close out the standard file descriptors */
	if (!in) {
		if (freopen("/dev/null", "r", stdin) == NULL) {
			return errno;
			/* continue anyhow -- note we can't close out descriptor 0 because we
			 * have nothing to replace it with, and if we didn't have a descriptor
			 * 0 the next file would be created with that value ... leading to
			 * havoc.
			 */
		}
	}
	if (!out) {
		if (freopen("/dev/null", "w", stdout) == NULL) {
			return errno;
		}
	}
	/* We are going to reopen this again in a little while to the error
	 * log file, but better to do it twice and suffer a small performance
	 * hit for consistancy than not reopen it here.
	 */
	if (!err) {
		if (freopen("/dev/null", "w", stderr) == NULL) {
			return errno;
		}
	}
	return ES_SUCCESS;
}

es_status_t eso_proc_kill(es_proc_t *proc, int sig)
{
	if (kill(proc->pid, sig) == -1) {
		return errno;
	}

	return ES_SUCCESS;
}

es_os_pid_t eso_os_process_current(void)
{
	return getpid();
}

#endif //!WIN32
