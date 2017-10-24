/*
 * EFork.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EFork_HH_
#define EFork_HH_

#include "EObject.hh"
#include "ERunnable.hh"
#include "EString.hh"
#include "ESharedPtr.hh"
#include "EIllegalStateException.hh"
#include "ERuntimeException.hh"
#include "EInterruptedException.hh"

namespace efc {

class EFork : virtual public ERunnable
{
public:
	virtual ~EFork();
	
	EFork(boolean needLock=false);
	EFork(const char *name, boolean needLock=false);
	EFork(sp<ERunnable> target, boolean needLock=false);
	EFork(sp<ERunnable> target, const char *name, boolean needLock=false);
	
	// unsupported.
	EFork(const EFork& that);
	EFork& operator= (const EFork& that);

	/**
	 * Marks this process as either a daemon process or a user process.
	 * <p>
	 * This method must be called before the process is started.
	 * @param      on   if <code>true</code>, marks this process as a
	 *                  daemon thread.
	 * @exception  IllegalStateException  if this thread is active.
	 */
	synchronized void setDaemon(boolean on) THROWS(ERuntimeException);
	
	/**
	 * Tests if this process is a daemon process.
	 *
	 * @return  <code>true</code> if this process is a daemon process;
	 *          <code>false</code> otherwise.
	 * @see     #setDaemon(boolean)
	 */
	synchronized boolean isDaemon();
	
	/**
	 * Causes this forked process to begin execution and
	 * the <code>run</code> method will be called.
	 * <p>
	 * The result is that two processes are running concurrently: the
	 * current parent (which returns from the call to the
	 * <code>start</code> method) and the other process (which executes its
	 * <code>run</code> method).
	 *
	 * @exception  IllegalStateException  if the process was already
	 *               started.
	 * @exception  RuntimeException  if create the process failed.
	 */
	synchronized void start() THROWS2(EIllegalStateException,ERuntimeException);
	
	/**
	 * If this process was constructed using a separate
	 * <code>Runnable</code> run object, then that
	 * <code>Runnable</code> object's <code>run</code> method is called;
	 * otherwise, this method does nothing and returns.
	 * <p>
	 * Subclasses of <code>EFork</code> should override this method.
	 */
	virtual void run();
	
	/**
	 * Terminates the currently running process. The
	 * argument serves as a status code; by convention, a nonzero
	 * status code indicates abnormal termination.
	 *
	 * @param      status   exit status.
	 * @see _exit(status)
	 */
	void exit(int status);

	/** 
	 * Terminate a process.
	 * @param sig How to kill the process.
	 */
	synchronized void kill(int sig) THROWS(ERuntimeException);
	
	/**
	 * causes the current thread to wait, if necessary, until the
	 * process represented by this <code>Process</code> object has
	 * terminated. This method returns
	 * immediately if the subprocess has already terminated. If the
	 * subprocess has not yet terminated, the calling thread will be
	 * blocked until the subprocess exits.
	 *
	 * @return     the exit value of the process. By convention,
	 *             <code>0</code> indicates normal termination.
	 */
	synchronized int waitFor(es_exit_why_e *exitwhy=null) THROWS(ERuntimeException);
	
	/**
	 * Returns the exit value for the subprocess.
	 *
	 * @return  the exit value of the subprocess represented by this
	 *          <code>Process</code> object. by convention, the value
	 *          <code>0</code> indicates normal termination.
	 * @exception  IllegalThreadStateException  if the subprocess represented
	 *             by this <code>Process</code> object has not yet terminated.
	 */
	synchronized int exitValue(es_exit_why_e *exitwhy=null) THROWS(EInterruptedException);

	/**
	 * Tests if this process is alive. A process is alive if it has
	 * been started and has not yet died.
	 *
	 * @return  <code>true</code> if this process is alive;
	 *          <code>false</code> otherwise.
	 */
	synchronized boolean isAlive();
	
	/**
	 * Returns this process's name.
	 *
	 * @return  this process's name.
	 */
	char* getName();
	
	/**
	 * Returns a string representation of this process, including the
	 * thread's name and priority.
	 *
	 * @return  a string representation of this process.
	 */
	virtual EStringBase toString();
	
	/**
	 * Wait for any current child process to die and return information 
	 * about that child.
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
	static es_os_pid_t waitAny(int *exitcode = null, es_exit_why_e *exitwhy = null,
			                   es_wait_how_e waithow = ES_WAIT) THROWS(ERuntimeException);
	
public:
#ifdef CPP11_SUPPORT
public:
	static sp<EFork> executeX(std::function<void()> func) {
		sp<ERunnable> r(new ERunnableTarget(func));
		sp<EFork> process = new EFork(r);
		process->start();
		return process;
	}
#endif

private:
	es_proc_t    m_Process;

	/* What will be run. */
	sp<ERunnable> m_Target;

	/* Thread name */
	EString      m_Name;

	/* Whether or not the process is a daemon process. */
	boolean	m_Daemon;

	/* Process Lock */
	es_proc_mutex_t *m_Lock;
	
	void init(sp<ERunnable> target, const char *name, boolean needLock);
};

} /* namespace efc */
#endif //!EFork_HH_
