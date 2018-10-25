/**
 * @file  eso_signal.c
 * @brief OS signal
 */

#include "eso_signal.h"
#include <string.h>

/*
 * Replace standard signal() with the more reliable sigaction equivalent
 * from W. Richard Stevens' "Advanced Programming in the UNIX Environment"
 * (the version that does not automatically restart system calls).
 */
es_status_t eso_signal(int signum, void(*handler)(int sig)) {
#ifdef WIN32
	return ES_ENOTIMPL;
#else
	struct sigaction act, oact;

	act.sa_handler = handler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
#ifdef SA_INTERRUPT             /* SunOS */
	act.sa_flags |= SA_INTERRUPT;
#endif
#if defined(__osf__) && defined(__alpha)
	/* XXX jeff thinks this should be enabled whenever SA_NOCLDWAIT is defined */

	/* this is required on Tru64 to cause child processes to
	 * disappear gracefully - XPG4 compatible
	 */
	if ((signum == SIGCHLD) && (handler == SIG_IGN)) {
		act.sa_flags |= SA_NOCLDWAIT;
	}
#endif

	if (sigaction(signum, &act, &oact) < 0)
		return ES_FAILURE;
	return ES_SUCCESS;
#endif
}

const char* eso_signal_description(int signum) {
#ifdef WIN32
	return "unknown signal (not supported)";
#else
#if 1
	return strsignal(signum);
#else
	char *desc = "unknown signal (not supported)";

	switch (signum) {
		case 0: desc = "Signal 0"; break;
#ifdef SIGHUP
		case SIGHUP: desc = "Hangup"; break;
#endif
#ifdef SIGINT
		case SIGINT: desc = "Interrupt"; break;
#endif
#ifdef SIGQUIT
		case SIGQUIT: desc = "Quit"; break;
#endif
#ifdef SIGILL
		case SIGILL: desc = "Illegal instruction"; break;
#endif
#ifdef SIGTRAP
		case SIGTRAP: desc = "Trace/BPT trap"; break;
#endif
#ifdef SIGIOT
		case SIGIOT: desc = "IOT instruction"; break;
#endif
#ifdef SIGABRT
		case SIGABRT: desc = "Abort"; break;
#endif
#ifdef SIGEMT
		case SIGEMT: desc = "Emulator trap"; break;
#endif
#ifdef SIGFPE
		case SIGFPE: desc = "Arithmetic exception"; break;
#endif
#ifdef SIGKILL
		case SIGKILL: desc = "Killed"; break;
#endif
#ifdef SIGBUS
		case SIGBUS: desc = "Bus error"; break;
#endif
#ifdef SIGSEGV
		case SIGSEGV: desc = "Segmentation fault"; break;
#endif
#ifdef SIGSYS
		case SIGSYS: desc = "Bad system call"; break;
#endif
#ifdef SIGPIPE
		case SIGPIPE: desc = "Broken pipe"; break;
#endif
#ifdef SIGALRM
		case SIGALRM: desc = "Alarm clock"; break;
#endif
#ifdef SIGTERM
		case SIGTERM: desc = "Terminated"; break;
#endif
#ifdef SIGUSR1
		case SIGUSR1: desc = "User defined signal 1"; break;
#endif
#ifdef SIGUSR2
		case SIGUSR2: desc = "User defined signal 2"; break;
#endif
#ifdef SIGCHLD
		case SIGCHLD: desc = "Child status change"; break;
#endif
#ifdef SIGPWR
		case SIGPWR: desc = "Power-fail restart"; break;
#endif
#ifdef SIGWINCH
		case SIGWINCH: desc = "Window changed"; break;
#endif
#ifdef SIGURG
		case SIGURG: desc = "urgent socket condition"; break;
#endif
#ifdef SIGPOLL
		case SIGPOLL: desc = "Pollable event occurred"; break;
#endif
#if defined(SIGIO) && (SIGIO != SIGPOLL)
		case SIGIO: desc = "socket I/O possible"; break;
#endif
#ifdef SIGSTOP
		case SIGSTOP: desc = "Stopped (signal)"; break;
#endif
#ifdef SIGTSTP
		case SIGTSTP: desc = "Stopped"; break;
#endif
#ifdef SIGCONT
		case SIGCONT: desc = "Continued"; break;
#endif
#ifdef SIGTTIN
		case SIGTTIN: desc = "Stopped (tty input)"; break;
#endif
#ifdef SIGTTOU
		case SIGTTOU: desc = "Stopped (tty output)"; break;
#endif
#ifdef SIGVTALRM
		case SIGVTALRM: desc = "virtual timer expired"; break;
#endif
#ifdef SIGPROF
		case SIGPROF: desc = "profiling timer expired"; break;
#endif
#ifdef SIGXCPU
		case SIGXCPU: desc = "exceeded cpu limit"; break;
#endif
#ifdef SIGXFSZ
		case SIGXFSZ: desc = "exceeded file size limit"; break;
#endif
	}

	return desc;
#endif
#endif //!WIN32
}

es_status_t eso_signal_block(int signum) {
#ifdef WIN32
	return ES_ENOTIMPL;
#else
	sigset_t sig_mask;
	int rv;

	sigemptyset(&sig_mask);

	sigaddset(&sig_mask, signum);

	if ((rv = pthread_sigmask(SIG_BLOCK, &sig_mask, NULL)) != 0) {
#ifdef PTHREAD_SETS_ERRNO
		rv = errno;
#endif
	}

	return rv;
#endif
}

es_status_t eso_signal_unblock(int signum) {
#ifdef WIN32
	return ES_ENOTIMPL;
#else
	sigset_t sig_mask;
	int rv;

	sigemptyset(&sig_mask);

	sigaddset(&sig_mask, signum);

	if ((rv = pthread_sigmask(SIG_UNBLOCK, &sig_mask, NULL)) != 0) {
#ifdef PTHREAD_SETS_ERRNO
		rv = errno;
#endif
	}

	return rv;
#endif
}

