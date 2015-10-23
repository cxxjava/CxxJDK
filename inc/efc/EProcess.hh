/*
 * EProcess.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EProcess_HH_
#define EProcess_HH_

#include "EString.hh"
#include "EArray.hh"
#include "EFileOutputStream.hh"
#include "EFileInputStream.hh"
#include "EIOException.hh"
#include "ERuntimeException.hh"
#include "EIllegalStateException.hh"
#include "EInterruptedException.hh"

namespace efc {

class EProcess: public EObject {
public:
	~EProcess();

	/**
	 * Default constructer.
	 */
	EProcess();

	/**
	 * Constructs a process builder with the specified operating
	 * system program and arguments.  This is a convenience
	 * constructor that sets the process builder's command to a string
	 * list containing the same strings as the <code>command</code>
	 * array, in the same order.  It is not checked whether
	 * <code>command</code> corresponds to a valid operating system
	 * command.</p>
	 *
	 * @param   command  A string array containing the program and its arguments
	 */
	EProcess(const char* command);

	/**
	 * Constructs a process builder with the specified operating
	 * system program and arguments.
	 *
	 * @param   program  A string of the program
	 * @param   args  The arguments to pass to the new program.
	 */
	EProcess(const char* program, const char* args[]);

	/**
	 * Sets this process builder's operating system program and
	 * arguments.  This method does <i>not</i> make a copy of the
	 * <code>command</code> list.  Subsequent updates to the list will
	 * be reflected in the state of the process builder.  It is not
	 * checked whether <code>command</code> corresponds to a valid
	 * operating system command.</p>
	 *
	 * @param   command  The list containing the program and its arguments
	 * @return  This process builder
	 *
	 * @throws  NullPointerException
	 *          If the argument is <code>null</code>
	 */
	EProcess& command(const char* command);
	EProcess& commandFormat(const char* fmt, ...);
	EProcess& commandArguments(const char* command, ...); //must be null end.

	/**
	 * Sets this process builder's operating system program and
	 * arguments.  This method does <i>not</i> make a copy of the
	 * <code>command</code> list.  Subsequent updates to the list will
	 * be reflected in the state of the process builder.  It is not
	 * checked whether <code>command</code> corresponds to a valid
	 * operating system command.</p>
	 *
	 * @param   program  A string of the program
	 * @param   args  The arguments to pass to the new program.
	 *
	 * @throws  NullPointerException
	 *          If the argument is <code>null</code>
	 */
	EProcess& command(const char* program, const char* args[]);
	EProcess& command(const char* program, EArray<EString*>* envp);

	/**
	 * Returns this process builder's operating system program and
	 * arguments.  The returned list is <i>not</i> a copy.  Subsequent
	 * updates to the list will be reflected in the state of this
	 * process builder.
	 *
	 * @return this process builder's program and its arguments
	 */
	EArray<EString*>& command();

	/**
	 * Returns a string map view of this process builder's environment.
	 *
	 * Whenever a process builder is created, the environment is
	 * initialized to a copy of the current process environment (see
	 * {@link System#getenv()}).  Subprocesses subsequently started by
	 * this object's {@link #start()} method will use this map as
	 * their environment.
	 *
	 * <p>The returned object may be modified using ordinary {@link
	 * java.util.Map Map} operations.  These modifications will be
	 * visible to subprocesses started via the {@link #start()}
	 * method.  Two {@code ProcessBuilder} instances always
	 * contain independent process environments, so changes to the
	 * returned map will never be reflected in any other
	 * {@code ProcessBuilder} instance or the values returned by
	 * {@link System#getenv System.getenv}.
	 *
	 * <p>If the system does not support environment variables, an
	 * empty map is returned.
	 *
	 * <p>The returned map does not permit null keys or values.
	 * Attempting to insert or query the presence of a null key or
	 * value will throw a {@link NullPointerException}.
	 * Attempting to query the presence of a key or value which is not
	 * of type {@link String} will throw a {@link ClassCastException}.
	 *
	 * <p>The behavior of the returned map is system-dependent.  A
	 * system may not allow modifications to environment variables or
	 * may forbid certain variable names or values.  For this reason,
	 * attempts to modify the map may fail with
	 * {@link UnsupportedOperationException} or
	 * {@link IllegalArgumentException}
	 * if the modification is not permitted by the operating system.
	 *
	 * <p>Since the external format of environment variable names and
	 * values is system-dependent, there may not be a one-to-one
	 * mapping between them and Java's Unicode strings.  Nevertheless,
	 * the map is implemented in such a way that environment variables
	 * which are not modified by Java code will have an unmodified
	 * native representation in the subprocess.
	 *
	 * <p>The returned map and its collection views may not obey the
	 * general contract of the {@link Object#equals} and
	 * {@link Object#hashCode} methods.
	 *
	 * <p>The returned map is typically case-sensitive on all platforms.
	 *
	 * <p>If a security manager exists, its
	 * {@link SecurityManager#checkPermission checkPermission} method
	 * is called with a
	 * {@link RuntimePermission}{@code ("getenv.*")} permission.
	 * This may result in a {@link SecurityException} being thrown.
	 *
	 * <p>When passing information to a Java subprocess,
	 * <a href=System.html#EnvironmentVSSystemProperties>system properties</a>
	 * are generally preferred over environment variables.
	 *
	 * @return this process builder's environment
	 *
	 * @throws SecurityException
	 *         if a security manager exists and its
	 *         {@link SecurityManager#checkPermission checkPermission}
	 *         method doesn't allow access to the process environment
	 *
	 * @see    Runtime#exec(String[],String[],java.io.File)
	 * @see    System#getenv()
	 */
	EArray<EString*>& environment();

	/**
	 * Sets this process builder's working environment.
	 */
	EProcess& environment(const char* envp[]);
	EProcess& environment(EArray<EString*>* envp);
	EProcess& environmentList(const char* envp0, ...); //must be null end.

	/**
	 * Returns this process builder's working directory.
	 *
	 * Subprocesses subsequently started by this object's {@link
	 * #start()} method will use this as their working directory.
	 * The returned value may be <code>null</code> -- this means to use
	 * the working directory of the current Java process, usually the
	 * directory named by the system property <code>user.dir</code>,
	 * as the working directory of the child process.</p>
	 *
	 * @return  This process builder's working directory
	 */
	const char* directory();

	/**
	 * Sets this process builder's working directory.
	 *
	 * Subprocesses subsequently started by this object's {@link
	 * #start()} method will use this as their working directory.
	 * The argument may be <code>null</code> -- this means to use the
	 * working directory of the current Java process, usually the
	 * directory named by the system property <code>user.dir</code>,
	 * as the working directory of the child process.</p>
	 *
	 * @param   directory  The new working directory
	 * @return  This process builder
	 */
	EProcess& directory(const char* directory);
	EProcess& directory(EFile* directory);

	/**
	 * Marks this process as either a daemon process or a user process.
	 * <p>
	 * This method must be called before the process is started.
	 * @param      on   if <code>true</code>, marks this process as a
	 *                  daemon thread.
	 * @exception  IllegalStateException  if this thread is active.
	 */
	void setDaemon(boolean on);

	/**
	 * Tests if this process is a daemon process.
	 *
	 * @return  <code>true</code> if this process is a daemon process;
	 *          <code>false</code> otherwise.
	 * @see     #setDaemon(boolean)
	 */
	boolean isDaemon();

	/**
	 * Starts a new process using the attributes of this process builder.
	 *
	 * <p>The new process will
	 * invoke the command and arguments given by {@link #command()},
	 * in a working directory as given by {@link #directory()},
	 * with a process environment as given by {@link #environment()}.
	 *
	 * <p>This method checks that the command is a valid operating
	 * system command.  Which commands are valid is system-dependent,
	 * but at the very least the command must be a non-empty list of
	 * non-null strings.
	 *
	 * <p>If there is a security manager, its
	 * {@link SecurityManager#checkExec checkExec}
	 * method is called with the first component of this object's
	 * <code>command</code> array as its argument. This may result in
	 * a {@link SecurityException} being thrown.
	 *
	 * <p>Starting an operating system process is highly system-dependent.
	 * Among the many things that can go wrong are:
	 * <ul>
	 * <li>The operating system program file was not found.
	 * <li>Access to the program file was denied.
	 * <li>The working directory does not exist.
	 * </ul>
	 *
	 * <p>In such cases an exception will be thrown.  The exact nature
	 * of the exception is system-dependent, but it will always be a
	 * subclass of {@link IOException}.
	 *
	 * <p>Subsequent modifications to this process builder will not
	 * affect the returned {@link Process}.</p>
	 *
	 * @return  A new {@link Process} object for managing the subprocess
	 *
	 * @throws  NullPointerException
	 *          If an element of the command list is null
	 *
	 * @throws  IndexOutOfBoundsException
	 *          If the command is an empty list (has size <code>0</code>)
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its
	 *          {@link SecurityManager#checkExec checkExec}
	 *          method doesn't allow creation of the subprocess
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 *
	 * @see     Runtime#exec(String[], String[], java.io.File)
	 * @see     SecurityManager#checkExec(String)
	 */
	void start() THROWS3(EIOException,EIllegalStateException,ERuntimeException);

	/**
	 * Gets the output stream of the subprocess.
	 * Output to the stream is piped into the standard input stream of
	 * the process represented by this <code>Process</code> object.
	 * <p>
	 * Implementation note: It is a good idea for the output stream to
	 * be buffered.
	 *
	 * @return  the output stream connected to the normal input of the
	 *          subprocess.
	 */
	EOutputStream* getOutputStream();

	/**
	 * Gets the input stream of the subprocess.
	 * The stream obtains data piped from the standard output stream
	 * of the process represented by this <code>Process</code> object.
	 * <p>
	 * Implementation note: It is a good idea for the input stream to
	 * be buffered.
	 *
	 * @return  the input stream connected to the normal output of the
	 *          subprocess.
	 */
	EInputStream* getInputStream();

	/**
	 * Gets the error stream of the subprocess.
	 * The stream obtains data piped from the error output stream of the
	 * process represented by this <code>Process</code> object.
	 * <p>
	 * Implementation note: It is a good idea for the input stream to be
	 * buffered.
	 *
	 * @return  the input stream connected to the error stream of the
	 *          subprocess.
	 */
	EInputStream* getErrorStream();

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
	int waitFor();

	/**
	 * Returns the exit value for the subprocess.
	 *
	 * @return  the exit value of the subprocess represented by this
	 *          <code>Process</code> object. by convention, the value
	 *          <code>0</code> indicates normal termination.
	 * @exception  IllegalThreadStateException  if the subprocess represented
	 *             by this <code>Process</code> object has not yet terminated.
	 */
	int exitValue() THROWS(EInterruptedException);

	/**
	 * Kills the subprocess. The subprocess represented by this
	 * <code>Process</code> object is forcibly terminated.
	 */
	void destroy();

private:
	es_proc_t m_Process;
	EString m_Prog;
	EString m_Dir;
	boolean m_Detached;
	EArray<EString*> m_Commands;
	EArray<EString*> m_Environments;

	EFileOutputStream *m_Stdin;
	EFileInputStream *m_Stdout;
	EFileInputStream *m_Stderr;
};

} /* namespace efc */
#endif //!EProcess_HH_
