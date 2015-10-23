/*
 * ERuntime.hh
 *
 *  Created on: 2014-9-18
 *      Author: cxxjava@163.com
 */

#ifndef ERUNTIME_HH_
#define ERUNTIME_HH_

#include "EArray.hh"
#include "EString.hh"
#include "ERunnable.hh"
#include "EProcess.hh"
#include "EIOException.hh"

namespace efc {

/**
 * Every Java application has a single instance of class
 * <code>Runtime</code> that allows the application to interface with
 * the environment in which the application is running. The current
 * runtime can be obtained from the <code>getRuntime</code> method.
 * <p>
 * An application cannot create its own instance of this class.
 *
 * @author  unascribed
 * @see     java.lang.Runtime#getRuntime()
 * @since   JDK1.0
 */
class ERuntime {
public:
	DECLARE_STATIC_INITZZ;

public:
	/**
	 * Returns the runtime object associated with the current Java application.
	 * Most of the methods of class <code>Runtime</code> are instance
	 * methods and must be invoked with respect to the current runtime object.
	 *
	 * @return  the <code>Runtime</code> object associated with the current
	 *          Java application.
	 */
	static ERuntime* getRuntime();

	/**
	 * Executes the specified string command in a separate process.
	 *
	 * <p>This is a convenience method.  An invocation of the form
	 * <tt>exec(command)</tt>
	 * behaves in exactly the same way as the invocation
	 * <tt>{@link #exec(String, String[], File) exec}(command, null, null)</tt>.
	 *
	 * @param   command   a specified system command.
	 *
	 * @return  A new {@link Process} object for managing the subprocess
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its
	 *          {@link SecurityManager#checkExec checkExec}
	 *          method doesn't allow creation of the subprocess
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 *
	 * @throws  NullPointerException
	 *          If <code>command</code> is <code>null</code>
	 *
	 * @throws  IllegalArgumentException
	 *          If <code>command</code> is empty
	 *
	 * @see     #exec(String[], String[], File)
	 * @see     ProcessBuilder
	 */
	EProcess* exec(const char* command) THROWS(EIOException);

	/**
	 * Executes the specified string command in a separate process with the
	 * specified environment.
	 *
	 * <p>This is a convenience method.  An invocation of the form
	 * <tt>exec(command, envp)</tt>
	 * behaves in exactly the same way as the invocation
	 * <tt>{@link #exec(String, String[], File) exec}(command, envp, null)</tt>.
	 *
	 * @param   command   a specified system command.
	 *
	 * @param   envp      array of strings, each element of which
	 *                    has environment variable settings in the format
	 *                    <i>name</i>=<i>value</i>, or
	 *                    <tt>null</tt> if the subprocess should inherit
	 *                    the environment of the current process.
	 *
	 * @return  A new {@link Process} object for managing the subprocess
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its
	 *          {@link SecurityManager#checkExec checkExec}
	 *          method doesn't allow creation of the subprocess
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 *
	 * @throws  NullPointerException
	 *          If <code>command</code> is <code>null</code>,
	 *          or one of the elements of <code>envp</code> is <code>null</code>
	 *
	 * @throws  IllegalArgumentException
	 *          If <code>command</code> is empty
	 *
	 * @see     #exec(String[], String[], File)
	 * @see     ProcessBuilder
	 */
	EProcess* exec(const char* command, const char* envp[]) THROWS(EIOException);
	EProcess* exec(const char* command, EArray<EString*>* envp) THROWS(EIOException);

	/**
	 * Executes the specified string command in a separate process with the
	 * specified environment and working directory.
	 *
	 * <p>This is a convenience method.  An invocation of the form
	 * <tt>exec(command, envp, dir)</tt>
	 * behaves in exactly the same way as the invocation
	 * <tt>{@link #exec(String[], String[], File) exec}(cmdarray, envp, dir)</tt>,
	 * where <code>cmdarray</code> is an array of all the tokens in
	 * <code>command</code>.
	 *
	 * <p>More precisely, the <code>command</code> string is broken
	 * into tokens using a {@link StringTokenizer} created by the call
	 * <code>new {@link StringTokenizer}(command)</code> with no
	 * further modification of the character categories.  The tokens
	 * produced by the tokenizer are then placed in the new string
	 * array <code>cmdarray</code>, in the same order.
	 *
	 * @param   command   a specified system command.
	 *
	 * @param   envp      array of strings, each element of which
	 *                    has environment variable settings in the format
	 *                    <i>name</i>=<i>value</i>, or
	 *                    <tt>null</tt> if the subprocess should inherit
	 *                    the environment of the current process.
	 *
	 * @param   dir       the working directory of the subprocess, or
	 *                    <tt>null</tt> if the subprocess should inherit
	 *                    the working directory of the current process.
	 *
	 * @return  A new {@link Process} object for managing the subprocess
	 *
	 * @throws  SecurityException
	 *          If a security manager exists and its
	 *          {@link SecurityManager#checkExec checkExec}
	 *          method doesn't allow creation of the subprocess
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 *
	 * @throws  NullPointerException
	 *          If <code>command</code> is <code>null</code>,
	 *          or one of the elements of <code>envp</code> is <code>null</code>
	 *
	 * @throws  IllegalArgumentException
	 *          If <code>command</code> is empty
	 *
	 * @see     ProcessBuilder
	 * @since 1.3
	 */
	EProcess* exec(const char* command, const char* envp[], const char* dir) THROWS(EIOException);
	EProcess* exec(const char* command, const char* envp[], EFile* dir) THROWS(EIOException);
	EProcess* exec(const char* command, EArray<EString*>* envp, const char* dir) THROWS(EIOException);
	EProcess* exec(const char* command, EArray<EString*>* envp, EFile* dir) THROWS(EIOException);

	/**
	 * Returns the number of processors available to the Java virtual machine.
	 *
	 * <p> This value may change during a particular invocation of the virtual
	 * machine.  Applications that are sensitive to the number of available
	 * processors should therefore occasionally poll this property and adjust
	 * their resource usage appropriately. </p>
	 *
	 * @return  the maximum number of processors available to the virtual
	 *          machine; never smaller than one
	 * @since 1.4
	 */
	int availableProcessors();

	/**
	 * Returns the amount of free memory in the Java Virtual Machine.
	 * Calling the
	 * <code>gc</code> method may result in increasing the value returned
	 * by <code>freeMemory.</code>
	 *
	 * @return  an approximation to the total amount of memory currently
	 *          available for future allocated objects, measured in bytes.
	 */
	ullong freeMemory();

	/**
	 * Returns the total amount of memory in the Java virtual machine.
	 * The value returned by this method may vary over time, depending on
	 * the host environment.
	 * <p>
	 * Note that the amount of memory required to hold an object of any
	 * given type may be implementation-dependent.
	 *
	 * @return  the total amount of memory currently available for current
	 *          and future objects, measured in bytes.
	 */
	ullong totalMemory();

	/**
	 * Returns the maximum amount of memory that the Java virtual machine will
	 * attempt to use.  If there is no inherent limit then the value {@link
	 * java.lang.Long#MAX_VALUE} will be returned. </p>
	 *
	 * @return  the maximum amount of memory that the virtual machine will
	 *          attempt to use, measured in bytes
	 * @since 1.4
	 */
	ullong maxMemory();

private:
	static ERuntime* currentRuntime;

	/** Don't let anyone else instantiate this class */
	ERuntime() {}
};

} /* namespace efc */
#endif /* ERUNTIME_HH_ */
