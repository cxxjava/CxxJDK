/*
 * EThread.hh
 *
 *  Created on: 2013-3-19
 *      Author: cxxjava@163.com
 */

#ifndef ETHREAD_HH_
#define ETHREAD_HH_

#include "EA.hh"
#include "ERunnable.hh"
#include "EString.hh"
#include "EIllegalArgumentException.hh"
#include "EIllegalStateException.hh"
#include "EInterruptedException.hh"
#include "ERuntimeException.hh"
#include "ESharedPtr.hh"

namespace efc {

class EThreadGroup;
class EParker;
class EInterruptible;
class ESimpleLock;
class EParkEvent;
class MainThreadLocal;

/**
 * A <i>thread</i> is a thread of execution in a program. The Java
 * Virtual Machine allows an application to have multiple threads of
 * execution running concurrently.
 * <p>
 * Every thread has a priority. Threads with higher priority are
 * executed in preference to threads with lower priority. Each thread
 * may or may not also be marked as a daemon. When code running in
 * some thread creates a new <code>Thread</code> object, the new
 * thread has its priority initially set equal to the priority of the
 * creating thread, and is a daemon thread if and only if the
 * creating thread is a daemon.
 * <p>
 * When a Java Virtual Machine starts up, there is usually a single
 * non-daemon thread (which typically calls the method named
 * <code>main</code> of some designated class). The Java Virtual
 * Machine continues to execute threads until either of the following
 * occurs:
 * <ul>
 * <li>The <code>exit</code> method of class <code>Runtime</code> has been
 *     called and the security manager has permitted the exit operation
 *     to take place.
 * <li>All threads that are not daemon threads have died, either by
 *     returning from the call to the <code>run</code> method or by
 *     throwing an exception that propagates beyond the <code>run</code>
 *     method.
 * </ul>
 * <p>
 * There are two ways to create a new thread of execution. One is to
 * declare a class to be a subclass of <code>Thread</code>. This
 * subclass should override the <code>run</code> method of class
 * <code>Thread</code>. An instance of the subclass can then be
 * allocated and started. For example, a thread that computes primes
 * larger than a stated value could be written as follows:
 * <p><hr><blockquote><pre>
 *     class PrimeThread extends Thread {
 *         long minPrime;
 *         PrimeThread(long minPrime) {
 *             this.minPrime = minPrime;
 *         }
 *
 *         public void run() {
 *             // compute primes larger than minPrime
 *             &nbsp;.&nbsp;.&nbsp;.
 *         }
 *     }
 * </pre></blockquote><hr>
 * <p>
 * The following code would then create a thread and start it running:
 * <p><blockquote><pre>
 *     PrimeThread p = new PrimeThread(143);
 *     p.start();
 * </pre></blockquote>
 * <p>
 * The other way to create a thread is to declare a class that
 * implements the <code>Runnable</code> interface. That class then
 * implements the <code>run</code> method. An instance of the class can
 * then be allocated, passed as an argument when creating
 * <code>Thread</code>, and started. The same example in this other
 * style looks like the following:
 * <p><hr><blockquote><pre>
 *     class PrimeRun implements Runnable {
 *         long minPrime;
 *         PrimeRun(long minPrime) {
 *             this.minPrime = minPrime;
 *         }
 *
 *         public void run() {
 *             // compute primes larger than minPrime
 *             &nbsp;.&nbsp;.&nbsp;.
 *         }
 *     }
 * </pre></blockquote><hr>
 * <p>
 * The following code would then create a thread and start it running:
 * <p><blockquote><pre>
 *     PrimeRun p = new PrimeRun(143);
 *     new Thread(p).start();
 * </pre></blockquote>
 * <p>
 * Every thread has a name for identification purposes. More than
 * one thread may have the same name. If a name is not specified when
 * a thread is created, a new name is generated for it.
 *
 * @see     Runnable
 * @see     Runtime#exit(int)
 * @see     #run()
 * @see     #stop()
 * @since   JDK1.0
 */

class EThread : public ERunnable {
public:
	DECLARE_STATIC_INITZZ;

public:
	/** Park states */
	enum ParkState {
		/** park state indicating unparked */
		UNPARKED = 1,

		/** park state indicating preemptively unparked */
		PREEMPTIVELY_UNPARKED = 2,

		/** park state indicating parked */
		PARKED = 3
	};

	/**
	 * A thread state.  A thread can be in one of the following states:
	 * <ul>
	 * <li>{@link #NEW}<br>
	 *     A thread that has not yet started is in this state.
	 *     </li>
	 * <li>{@link #RUNNABLE}<br>
	 *     A thread executing in the Java virtual machine is in this state.
	 *     </li>
	 * <li>{@link #BLOCKED}<br>
	 *     A thread that is blocked waiting for a monitor lock
	 *     is in this state.
	 *     </li>
	 * <li>{@link #WAITING}<br>
	 *     A thread that is waiting indefinitely for another thread to
	 *     perform a particular action is in this state.
	 *     </li>
	 * <li>{@link #TIMED_WAITING}<br>
	 *     A thread that is waiting for another thread to perform an action
	 *     for up to a specified waiting time is in this state.
	 *     </li>
	 * <li>{@link #TERMINATED}<br>
	 *     A thread that has exited is in this state.
	 *     </li>
	 * </ul>
	 *
	 * <p>
	 * A thread can be in only one state at a given point in time.
	 * These states are virtual machine states which do not reflect
	 * any operating system thread states.
	 *
	 * @since   1.5
	 * @see #getState
	 */
	enum State {
		/**
		 * Thread state for a thread which has not yet started.
		 */
		NEW,

		/**
		 * Thread state for a runnable thread.  A thread in the runnable
		 * state is executing in the Java virtual machine but it may
		 * be waiting for other resources from the operating system
		 * such as processor.
		 */
		RUNNABLE,

		/**
		 * Thread state for a thread blocked waiting for a monitor lock.
		 * A thread in the blocked state is waiting for a monitor lock
		 * to enter a synchronized block/method or
		 * reenter a synchronized block/method after calling
		 * {@link Object#wait() Object.wait}.
		 */
		BLOCKED,

		/**
		 * Thread state for a waiting thread.
		 * A thread is in the waiting state due to calling one of the
		 * following methods:
		 * <ul>
		 *   <li>{@link Object#wait() Object.wait} with no timeout</li>
		 *   <li>{@link #join() Thread.join} with no timeout</li>
		 *   <li>{@link LockSupport#park() LockSupport.park}</li>
		 * </ul>
		 *
		 * <p>A thread in the waiting state is waiting for another thread to
		 * perform a particular action.
		 *
		 * For example, a thread that has called <tt>Object.wait()</tt>
		 * on an object is waiting for another thread to call
		 * <tt>Object.notify()</tt> or <tt>Object.notifyAll()</tt> on
		 * that object. A thread that has called <tt>Thread.join()</tt>
		 * is waiting for a specified thread to terminate.
		 */
		WAITING,

		/**
		 * Thread state for a waiting thread with a specified waiting time.
		 * A thread is in the timed waiting state due to calling one of
		 * the following methods with a specified positive waiting time:
		 * <ul>
		 *   <li>{@link #sleep Thread.sleep}</li>
		 *   <li>{@link Object#wait(long) Object.wait} with timeout</li>
		 *   <li>{@link #join(long) Thread.join} with timeout</li>
		 *   <li>{@link LockSupport#parkNanos LockSupport.parkNanos}</li>
		 *   <li>{@link LockSupport#parkUntil LockSupport.parkUntil}</li>
		 * </ul>
		 */
		TIMED_WAITING,

		/**
		 * Thread state for a terminated thread.
		 * The thread has completed execution.
		 */
		TERMINATED
	};

	/**
	 * The minimum priority that a thread can have.
	 */
	//static const int MIN_PRIORITY = 1;

	/**
	 * The default priority that is assigned to a thread.
	 */
	//static const int NORM_PRIORITY = 5;

	/**
	 * The maximum priority that a thread can have.
	 */
	//static const int MAX_PRIORITY = 10;

	enum Priority {
		MIN_PRIORITY = 1,
		NORM_PRIORITY = 5,
		MAX_PRIORITY = 10
	};

	// Added in JSR-166

	/**
	 * Interface for handlers invoked when a <tt>Thread</tt> abruptly
	 * terminates due to an uncaught exception.
	 * <p>When a thread is about to terminate due to an uncaught exception
	 * the Java Virtual Machine will query the thread for its
	 * <tt>UncaughtExceptionHandler</tt> using
	 * {@link #getUncaughtExceptionHandler} and will invoke the handler's
	 * <tt>uncaughtException</tt> method, passing the thread and the
	 * exception as arguments.
	 * If a thread has not had its <tt>UncaughtExceptionHandler</tt>
	 * explicitly set, then its <tt>ThreadGroup</tt> object acts as its
	 * <tt>UncaughtExceptionHandler</tt>. If the <tt>ThreadGroup</tt> object
	 * has no
	 * special requirements for dealing with the exception, it can forward
	 * the invocation to the {@linkplain #getDefaultUncaughtExceptionHandler
	 * default uncaught exception handler}.
	 *
	 * @see #setDefaultUncaughtExceptionHandler
	 * @see #setUncaughtExceptionHandler
	 * @see ThreadGroup#uncaughtException
	 * @since 1.5
	 */
	interface UncaughtExceptionHandler : virtual EObject {
		/**
		 * Method invoked when the given thread terminates due to the
		 * given uncaught exception.
		 * <p>Any exception thrown by this method will be ignored by the
		 * Java Virtual Machine.
		 * @param t the thread
		 * @param e the exception
		 */
		virtual void uncaughtException(EThread* t, EThrowable* e) = 0;
	};

public:
	virtual ~EThread();

	/**
	 * Allocates a new <code>Thread</code> object. This constructor has
	 * the same effect as <code>Thread(null, null,</code>
	 * <i>gname</i><code>)</code>, where <b><i>gname</i></b> is
	 * a newly generated name. Automatically generated names are of the
	 * form <code>"Thread-"+</code><i>n</i>, where <i>n</i> is an integer.
	 *
	 * @see     #Thread(ThreadGroup, Runnable, String)
	 */
	EThread();

	/**
	 * Allocates a new <code>Thread</code> object. This constructor has
	 * the same effect as <code>Thread(null, target,</code>
	 * <i>gname</i><code>)</code>, where <i>gname</i> is
	 * a newly generated name. Automatically generated names are of the
	 * form <code>"Thread-"+</code><i>n</i>, where <i>n</i> is an integer.
	 *
	 * @param   target   the object whose <code>run</code> method is called.
	 * @see     #Thread(ThreadGroup, Runnable, String)
	 */
	EThread(sp<ERunnable> target);

	/**
	 * Allocates a new <code>Thread</code> object. This constructor has
	 * the same effect as <code>Thread(group, target,</code>
	 * <i>gname</i><code>)</code>, where <i>gname</i> is
	 * a newly generated name. Automatically generated names are of the
	 * form <code>"Thread-"+</code><i>n</i>, where <i>n</i> is an integer.
	 *
	 * @param      group    the thread group.
	 * @param      target   the object whose <code>run</code> method is called.
	 * @exception  SecurityException  if the current thread cannot create a
	 *             thread in the specified thread group.
	 * @see        #Thread(ThreadGroup, Runnable, String)
	 */
	EThread(sp<EThreadGroup> group, sp<ERunnable> target);

	/**
	 * Allocates a new <code>Thread</code> object. This constructor has
	 * the same effect as <code>Thread(null, null, name)</code>.
	 *
	 * @param   name   the name of the new thread.
	 * @see     #Thread(ThreadGroup, Runnable, String)
	 */
	EThread(const char *name);

	/**
	 * Allocates a new <code>Thread</code> object. This constructor has
	 * the same effect as <code>Thread(group, null, name)</code>
	 *
	 * @param      group   the thread group.
	 * @param      name    the name of the new thread.
	 * @exception  SecurityException  if the current thread cannot create a
	 *               thread in the specified thread group.
	 * @see        #Thread(ThreadGroup, Runnable, String)
	 */
	EThread(sp<EThreadGroup> group, const char* name);

	/**
	 * Allocates a new <code>Thread</code> object. This constructor has
	 * the same effect as <code>Thread(null, target, name)</code>.
	 *
	 * @param   target   the object whose <code>run</code> method is called.
	 * @param   name     the name of the new thread.
	 * @see     #Thread(ThreadGroup, Runnable, String)
	 */
	EThread(sp<ERunnable> target, const char *name);

	/**
	 * Allocates a new <code>Thread</code> object so that it has
	 * <code>target</code> as its run object, has the specified
	 * <code>name</code> as its name, and belongs to the thread group
	 * referred to by <code>group</code>.
	 * <p>
	 * If <code>group</code> is <code>null</code> and there is a
	 * security manager, the group is determined by the security manager's
	 * <code>getThreadGroup</code> method. If <code>group</code> is
	 * <code>null</code> and there is not a security manager, or the
	 * security manager's <code>getThreadGroup</code> method returns
	 * <code>null</code>, the group is set to be the same ThreadGroup
	 * as the thread that is creating the new thread.
	 *
	 * <p>If there is a security manager, its <code>checkAccess</code>
	 * method is called with the ThreadGroup as its argument.
	 * <p>In addition, its <code>checkPermission</code>
	 * method is called with the
	 * <code>RuntimePermission("enableContextClassLoaderOverride")</code>
	 * permission when invoked directly or indirectly by the constructor
	 * of a subclass which overrides the <code>getContextClassLoader</code>
	 * or <code>setContextClassLoader</code> methods.
	 * This may result in a SecurityException.

	 * <p>
	 * If the <code>target</code> argument is not <code>null</code>, the
	 * <code>run</code> method of the <code>target</code> is called when
	 * this thread is started. If the target argument is
	 * <code>null</code>, this thread's <code>run</code> method is called
	 * when this thread is started.
	 * <p>
	 * The priority of the newly created thread is set equal to the
	 * priority of the thread creating it, that is, the currently running
	 * thread. The method <code>setPriority</code> may be used to
	 * change the priority to a new value.
	 * <p>
	 * The newly created thread is initially marked as being a daemon
	 * thread if and only if the thread creating it is currently marked
	 * as a daemon thread. The method <code>setDaemon </code> may be used
	 * to change whether or not a thread is a daemon.
	 *
	 * @param      group     the thread group.
	 * @param      target   the object whose <code>run</code> method is called.
	 * @param      name     the name of the new thread.
	 * @exception  SecurityException  if the current thread cannot create a
	 *               thread in the specified thread group or cannot
	 *               override the context class loader methods.
	 * @see        Runnable#run()
	 * @see        #run()
	 * @see        #setDaemon(boolean)
	 * @see        #setPriority(int)
	 * @see        ThreadGroup#checkAccess()
	 * @see        SecurityManager#checkAccess
	 */
	EThread(sp<EThreadGroup> group, sp<ERunnable> target, const char* name,
			ulong stacksize = 0, ulong guardsize = 0) THROWS(EIllegalArgumentException);

	//TODO:
	EThread(const EThread& that);
	EThread& operator= (const EThread& that);

	/**
	 * Marks this thread as either a {@linkplain #isDaemon daemon} thread
	 * or a user thread. The Java Virtual Machine exits when the only
	 * threads running are all daemon threads.
	 *
	 * <p> This method must be invoked before the thread is started.
	 *
	 * @param  on
	 *         if {@code true}, marks this thread as a daemon thread
	 *
	 * @throws  IllegalThreadStateException
	 *          if this thread is {@linkplain #isAlive alive}
	 *
	 * @throws  SecurityException
	 *          if {@link #checkAccess} determines that the current
	 *          thread cannot modify this thread
	 */
	void setDaemon(boolean on);

	/**
	 * Tests if this thread is a daemon thread.
	 *
	 * @return  <code>true</code> if this thread is a daemon thread;
	 *          <code>false</code> otherwise.
	 * @see     #setDaemon(boolean)
	 */
	boolean isDaemon();

	/**
	 * Inject exit callback to this thread.
	 */
	typedef void CleanupCallback(void* data);

	EThread* injectExitCallback(CleanupCallback* callback, void* arg);

	/**
	 * Causes this thread to begin execution; the Java Virtual Machine
	 * calls the <code>run</code> method of this thread.
	 * <p>
	 * The result is that two threads are running concurrently: the
	 * current thread (which returns from the call to the
	 * <code>start</code> method) and the other thread (which executes its
	 * <code>run</code> method).
	 * <p>
	 * It is never legal to start a thread more than once.
	 * In particular, a thread may not be restarted once it has completed
	 * execution.
	 *
	 * @exception  IllegalThreadStateException  if the thread was already
	 *               started.
	 * @see        #run()
	 * @see        #stop()
	 */
	synchronized void start() THROWS2(EIllegalStateException, ERuntimeException);

	/**
	 * If this thread was constructed using a separate
	 * <code>Runnable</code> run object, then that
	 * <code>Runnable</code> object's <code>run</code> method is called;
	 * otherwise, this method does nothing and returns.
	 * <p>
	 * Subclasses of <code>Thread</code> should override this method.
	 *
	 * @see     #start()
	 * @see     #stop()
	 * @see     #Thread(ThreadGroup, Runnable, String)
	 */
	virtual void run();

    /**
     * Interrupts this thread.
     *
     * <p> Unless the current thread is interrupting itself, which is
     * always permitted, the {@link #checkAccess() checkAccess} method
     * of this thread is invoked, which may cause a {@link
     * SecurityException} to be thrown.
     *
     * <p> If this thread is blocked in an invocation of the {@link
     * Object#wait() wait()}, {@link Object#wait(long) wait(long)}, or {@link
     * Object#wait(long, int) wait(long, int)} methods of the {@link Object}
     * class, or of the {@link #join()}, {@link #join(long)}, {@link
     * #join(long, int)}, {@link #sleep(long)}, or {@link #sleep(long, int)},
     * methods of this class, then its interrupt status will be cleared and it
     * will receive an {@link InterruptedException}.
     *
     * <p> If this thread is blocked in an I/O operation upon an {@link
     * java.nio.channels.InterruptibleChannel </code>interruptible
     * channel<code>} then the channel will be closed, the thread's interrupt
     * status will be set, and the thread will receive a {@link
     * java.nio.channels.ClosedByInterruptException}.
     *
     * <p> If this thread is blocked in a {@link java.nio.channels.Selector}
     * then the thread's interrupt status will be set and it will return
     * immediately from the selection operation, possibly with a non-zero
     * value, just as if the selector's {@link
     * java.nio.channels.Selector#wakeup wakeup} method were invoked.
     *
     * <p> If none of the previous conditions hold then this thread's interrupt
     * status will be set. </p>
     *
     * <p> Interrupting a thread that is not alive need not have any effect.
     *
     * @throws  SecurityException
     *          if the current thread cannot modify this thread
     *
     * @revised 6.0
     * @spec JSR-51
     */
	void interrupt();

	/**
	 * Tests whether this thread has been interrupted.  The <i>interrupted
	 * status</i> of the thread is unaffected by this method.
	 *
	 * <p>A thread interruption ignored because a thread was not alive
	 * at the time of the interrupt will be reflected by this method
	 * returning false.
	 *
	 * @return  <code>true</code> if this thread has been interrupted;
	 *          <code>false</code> otherwise.
	 * @see     #interrupted()
	 * @revised 6.0
	 */
	boolean isInterrupted();

	/**
	 * Tests if this thread is alive. A thread is alive if it has
	 * been started and has not yet died.
	 *
	 * @return  <code>true</code> if this thread is alive;
	 *          <code>false</code> otherwise.
	 */
	boolean isAlive();

	/**
	 * Changes the priority of this thread.
	 * <p>
	 * First the <code>checkAccess</code> method of this thread is called
	 * with no arguments. This may result in throwing a
	 * <code>SecurityException</code>.
	 * <p>
	 * Otherwise, the priority of this thread is set to the smaller of
	 * the specified <code>newPriority</code> and the maximum permitted
	 * priority of the thread's thread group.
	 *
	 * @param newPriority priority to set this thread to
	 * @exception  IllegalArgumentException  If the priority is not in the
	 *               range <code>MIN_PRIORITY</code> to
	 *               <code>MAX_PRIORITY</code>.
	 * @exception  SecurityException  if the current thread cannot modify
	 *               this thread.
	 * @see        #getPriority
	 * @see        #checkAccess()
	 * @see        #getThreadGroup()
	 * @see        #MAX_PRIORITY
	 * @see        #MIN_PRIORITY
	 * @see        ThreadGroup#getMaxPriority()
	 */
	void setPriority(int newPriority);

	/**
	 * Returns this thread's priority.
	 *
	 * @return  this thread's priority.
	 * @see     #setPriority
	 */
	int getPriority();

	/**
	 * Changes the name of this thread to be equal to the argument
	 * <code>name</code>.
	 * <p>
	 * First the <code>checkAccess</code> method of this thread is called
	 * with no arguments. This may result in throwing a
	 * <code>SecurityException</code>.
	 *
	 * @param      name   the new name for this thread.
	 * @exception  SecurityException  if the current thread cannot modify this
	 *               thread.
	 * @see        #getName
	 * @see        #checkAccess()
	 */
	void setName(const char* name);

	/**
	 * Returns this thread's name.
	 *
	 * @return  this thread's name.
	 * @see     #setName(String)
	 */
	const char* getName();

	/**
	 * Tests if this Thread is main thread.
	 */
	boolean isMainThread();

	/**
	 * Tests if this Thread is c thread.
	 */
	boolean isCThread();

	/**
	 * Returns the thread group to which this thread belongs.
	 * This method returns null if this thread has died
	 * (been stopped).
	 *
	 * @return  this thread's thread group.
	 */
	sp<EThreadGroup> getThreadGroup();

	/**
	 * Waits for this thread to die.
	 *
	 * @exception  InterruptedException if any thread has interrupted
	 *             the current thread.  The <i>interrupted status</i> of the
	 *             current thread is cleared when this exception is thrown.
	 */
	synchronized void join();

	/**
	 * Waits at most <code>millis</code> milliseconds for this thread to
	 * die. A timeout of <code>0</code> means to wait forever.
	 *
	 * @param      millis   the time to wait in milliseconds.
	 * @exception  InterruptedException if any thread has interrupted
	 *             the current thread.  The <i>interrupted status</i> of the
	 *             current thread is cleared when this exception is thrown.
	 */
	synchronized void join(llong millis);

	/**
	 * Waits at most <code>millis</code> milliseconds plus
	 * <code>nanos</code> nanoseconds for this thread to die.
	 *
	 * @param      millis   the time to wait in milliseconds.
	 * @param      nanos    0-999999 additional nanoseconds to wait.
	 * @exception  IllegalArgumentException  if the value of millis is negative
	 *               the value of nanos is not in the range 0-999999.
	 * @exception  InterruptedException if any thread has interrupted
	 *             the current thread.  The <i>interrupted status</i> of the
	 *             current thread is cleared when this exception is thrown.
	 */
	synchronized void join(llong millis, int nanos);

	/**
	 * Determines if the currently running thread has permission to
	 * modify this thread.
	 * <p>
	 * If there is a security manager, its <code>checkAccess</code> method
	 * is called with this thread as its argument. This may result in
	 * throwing a <code>SecurityException</code>.
	 *
	 * @exception  SecurityException  if the current thread is not allowed to
	 *               access this thread.
	 * @see        SecurityManager#checkAccess(Thread)
	 */
	void checkAccess();

	/**
	 * Returns the identifier of this Thread.  The thread ID is a positive
	 * <tt>long</tt> number generated when this thread was created.
	 * The thread ID is unique and remains unchanged during its lifetime.
	 * When a thread is terminated, this thread ID may be reused.
	 *
	 * @return this thread's ID.
	 * @since 1.5
	 */
	long getId();

	/**
	 * Returns the state of this thread.
	 * This method is designed for use in monitoring of the system state,
	 * not for synchronization control.
	 *
	 * @return this thread's state.
	 * @since 1.5
	 */
	State getState();

	/**
	 * Get the binding target object.
	 */
	sp<ERunnable> getTarget();

	/**
	 * Returns a string representation of this thread, including the
	 * thread's name and priority.
	 *
	 * @return  a string representation of this thread.
	 */
	virtual EStringBase toString();

	/**
	 * Returns the handler invoked when this thread abruptly terminates
	 * due to an uncaught exception. If this thread has not had an
	 * uncaught exception handler explicitly set then this thread's
	 * <tt>ThreadGroup</tt> object is returned, unless this thread
	 * has terminated, in which case <tt>null</tt> is returned.
	 * @since 1.5
	 */
	UncaughtExceptionHandler* getUncaughtExceptionHandler();

	/**
	 * Set the handler invoked when this thread abruptly terminates
	 * due to an uncaught exception.
	 * <p>A thread can take full control of how it responds to uncaught
	 * exceptions by having its uncaught exception handler explicitly set.
	 * If no such handler is set then the thread's <tt>ThreadGroup</tt>
	 * object acts as its handler.
	 * @param eh the object to use as this thread's uncaught exception
	 * handler. If <tt>null</tt> then this thread has no explicit handler.
	 * @throws  SecurityException  if the current thread is not allowed to
	 *          modify this thread.
	 * @see #setDefaultUncaughtExceptionHandler
	 * @see ThreadGroup#uncaughtException
	 * @since 1.5
	 */
	void setUncaughtExceptionHandler(UncaughtExceptionHandler* eh);

	/**
	 * Causes the currently executing thread object
	 * to temporarily pause and allow other threads to execute.
	 */
	static void yield();

	/**
	 * Causes the currently executing thread to sleep (temporarily cease
	 * execution) for the specified number of milliseconds. The thread
	 * does not lose ownership of any monitors.
	 *
	 * @param      millis   the length of time to sleep in milliseconds.
	 * @exception  InterruptedException if another thread has interrupted
	 *             the current thread.  The <i>interrupted status</i> of the
	 *             current thread is cleared when this exception is thrown.
	 * @see        java.lang.Object#notify()
	 */
	static void sleep(llong millis) THROWS(EInterruptedException);

	/**
	 * Causes the currently executing thread to sleep (cease execution)
	 * for the specified number of milliseconds plus the specified number
	 * of nanoseconds, subject to the precision and accuracy of system
	 * timers and schedulers. The thread does not lose ownership of any
	 * monitors.
	 *
	 * @param      millis   the length of time to sleep in milliseconds.
	 * @param      nanos    0-999999 additional nanoseconds to sleep.
	 * @exception  IllegalArgumentException  if the value of millis is
	 *             negative or the value of nanos is not in the range
	 *             0-999999.
	 * @exception  InterruptedException if any thread has interrupted
	 *             the current thread.  The <i>interrupted status</i> of the
	 *             current thread is cleared when this exception is thrown.
	 * @see        Object#notify()
	 */
	static void sleep(llong millis, int nanos) THROWS(EInterruptedException);

	/**
	 * Returns a reference to the currently executing thread object.
	 *
	 * @return  the currently executing thread.
	 */
	static EThread* currentThread();

	/**
	 * Tests whether the current thread has been interrupted.  The
	 * <i>interrupted status</i> of the thread is cleared by this method.  In
	 * other words, if this method were to be called twice in succession, the
	 * second call would return false (unless the current thread were
	 * interrupted again, after the first call had cleared its interrupted
	 * status and before the second call had examined it).
	 *
	 * <p>A thread interruption ignored because a thread was not alive
	 * at the time of the interrupt will be reflected by this method
	 * returning false.
	 *
	 * @return  <code>true</code> if the current thread has been interrupted;
	 *          <code>false</code> otherwise.
	 * @see #isInterrupted()
	 * @revised 6.0
	 */
	static boolean interrupted();

	/**
	 * Returns the number of active threads in the current thread's thread
	 * group.
	 *
	 * @return  the number of active threads in the current thread's thread
	 *          group.
	 */
	static int activeCount();

	/**
	 * Copies into the specified array every active thread in
	 * the current thread's thread group and its subgroups. This method simply
	 * calls the <code>enumerate</code> method of the current thread's thread
	 * group with the array argument.
	 * <p>
	 * First, if there is a security manager, that <code>enumerate</code>
	 * method calls the security
	 * manager's <code>checkAccess</code> method
	 * with the thread group as its argument. This may result
	 * in throwing a <code>SecurityException</code>.
	 *
	 * @param tarray an array of Thread objects to copy to
	 * @return  the number of threads put into the array
	 * @exception  SecurityException  if a security manager exists and its
	 *             <code>checkAccess</code> method doesn't allow the operation.
	 * @see     ThreadGroup#enumerate(Thread[])
	 * @see     SecurityManager#checkAccess(ThreadGroup)
	 */
	static int enumerate(EA<EThread*>* tarray);

	/**
	 * Set the default handler invoked when a thread abruptly terminates
	 * due to an uncaught exception, and no other handler has been defined
	 * for that thread.
	 *
	 * <p>Uncaught exception handling is controlled first by the thread, then
	 * by the thread's {@link ThreadGroup} object and finally by the default
	 * uncaught exception handler. If the thread does not have an explicit
	 * uncaught exception handler set, and the thread's thread group
	 * (including parent thread groups)  does not specialize its
	 * <tt>uncaughtException</tt> method, then the default handler's
	 * <tt>uncaughtException</tt> method will be invoked.
	 * <p>By setting the default uncaught exception handler, an application
	 * can change the way in which uncaught exceptions are handled (such as
	 * logging to a specific device, or file) for those threads that would
	 * already accept whatever &quot;default&quot; behavior the system
	 * provided.
	 *
	 * <p>Note that the default uncaught exception handler should not usually
	 * defer to the thread's <tt>ThreadGroup</tt> object, as that could cause
	 * infinite recursion.
	 *
	 * @param eh the object to use as the default uncaught exception handler.
	 * If <tt>null</tt> then there is no default handler.
	 *
	 * @throws SecurityException if a security manager is present and it
	 *         denies <tt>{@link RuntimePermission}
	 *         (&quot;setDefaultUncaughtExceptionHandler&quot;)</tt>
	 *
	 * @see #setUncaughtExceptionHandler
	 * @see #getUncaughtExceptionHandler
	 * @see ThreadGroup#uncaughtException
	 * @since 1.5
	 */
	static void setDefaultUncaughtExceptionHandler(UncaughtExceptionHandler* eh);

	/**
	 * Returns the default handler invoked when a thread abruptly terminates
	 * due to an uncaught exception. If the returned value is <tt>null</tt>,
	 * there is no default.
	 * @since 1.5
	 * @see #setDefaultUncaughtExceptionHandler
	 */
	static UncaughtExceptionHandler* getDefaultUncaughtExceptionHandler();

	/**
	 * Returns count of alive threads.
	 */
	static int getThreadsCount();

	/**
	 * C thread initialize.
	 */
	static EThread* c_init();

	/**
	 * Returns count of alive c threads.
	 */
	static int getCThreadsCount();

public:
	/**
	 * Parks the current thread for a particular number of nanoseconds, or
	 * indefinitely. If not indefinitely, this method unparks the thread
	 * after the given number of nanoseconds if no other thread unparks it
	 * first. If the thread has been "preemptively unparked," this method
	 * cancels that unparking and returns immediately. This method may
	 * also return spuriously (that is, without the thread being told to
	 * unpark and without the indicated amount of time elapsing).
	 *
	 * <p>See {@link java.util.concurrent.locks.LockSupport} for more
	 * in-depth information of the behavior of this method.</p>
	 *
	 * <p>This method must only be called when <code>this</code> is the current
	 * thread.
	 *
	 * @param nanos number of nanoseconds to park for or <code>0</code>
	 * to park indefinitely
	 * @throws IllegalArgumentException thrown if <code>nanos &lt; 0</code>
	 *
	 * @hide for Unsafe
	 */
	void parkFor(llong nanos);

	/**
	 * Parks the current thread until the specified system time. This
	 * method attempts to unpark the current thread immediately after
	 * <code>System.currentTimeMillis()</code> reaches the specified
	 * value, if no other thread unparks it first. If the thread has
	 * been "preemptively unparked," this method cancels that
	 * unparking and returns immediately. This method may also return
	 * spuriously (that is, without the thread being told to unpark
	 * and without the indicated amount of time elapsing).
	 *
	 * <p>See {@link java.util.concurrent.locks.LockSupport} for more
	 * in-depth information of the behavior of this method.</p>
	 *
	 * <p>This method must only be called when <code>this</code> is the
	 * current thread.
	 *
	 * @param time the time after which the thread should be unparked,
	 * in absolute milliseconds-since-the-epoch
	 *
	 * @hide for Unsafe
	 */
	void parkUntil(llong time);

	/**
	 * Unparks this thread. This unblocks the thread it if it was
	 * previously parked, or indicates that the thread is "preemptively
	 * unparked" if it wasn't already parked. The latter means that the
	 * next time the thread is told to park, it will merely clear its
	 * latent park bit and carry on without blocking.
	 *
	 * <p>See {@link java.util.concurrent.locks.LockSupport} for more
	 * in-depth information of the behavior of this method.</p>
	 *
	 * @hide for Unsafe
	 */
	void unpark();

	/* Set the blocker field; invoked via sun.misc.SharedSecrets from java.nio code
	 */
	void blockedOn(EInterruptible* b);

protected:
	friend class MainThreadLocal;
	friend class EThreadLocal;
	friend class EInheritableThreadLocal;

	/**
	 * Normal thread local values.
	 */
	EObject* localValues; //EThreadLocal::Values

	/**
	 * Inheritable thread local values.
	 */
	EObject* inheritableValues; //EThreadLocal::Values

	// Create a thread from c thread.
	EThread(const char* name, const int c_tid);

private:
	friend class EParker;
	friend class EUnsafe;
	friend class EThreadStatusChanger;

	/* C thread */
	es_thread_t *thread;

	/* What will be run. */
	sp<ERunnable> target;

	/* Thread name */
	EString      name;

	/* Thread Attr */
	es_threadattr_t threadAttr;

	/* Thread priority */
	int priority;

	/** the park state of the thread */
	int parkState;// = ParkState.UNPARKED;

	/* Java thread status for tools,
	 * initialized to indicate thread 'not yet started'
	 */
	volatile State threadStatus;// = 0;

	/* The group of this thread */
	sp<EThreadGroup> group;

	// Thread.isInterrupted state, @see:openjdk/hotspot/src/share/vm/runtime/osThread.hpp line 68
	boolean _interrupted;

	// Lock for thread park/unpark
	ESimpleLock* parkLock;
	ECondition* parkCond;

	//
	ESimpleLock* syncLock;

	/* The object in which this thread is blocked in an interruptible I/O
	 * operation, if any.  The blocker's interrupt method should be invoked
	 * after setting this thread's interrupt status.
	 */
	EInterruptible* volatile blocker;
	ESimpleLock* blockerLock;

	// JSR166 per-thread parker
	EParker* parker;

	EParkEvent * _ParkEvent;  // for synchronized()
	EParkEvent * _SleepEvent;  // for Thread.sleep
	EParkEvent * _MutexEvent;  // for native internal Mutex/Monitor
	EParkEvent * _MuxEvent;  // for low-level muxAcquire-muxRelease

	/* Whether or not the thread is a daemon thread. */
	boolean daemon;

	// C thread id, 0 is main thread, >0 other c thread.
	int c_tid;// = -1;

	// clean up callback at thread exit
	CleanupCallback* cleanupCallback;
	void* cleanupArg;

	// null unless explicitly set
	UncaughtExceptionHandler* uncaughtExceptionHandler;

	// null unless explicitly set
	static UncaughtExceptionHandler* volatile defaultUncaughtExceptionHandler;

	// Store current thread.
	static MainThreadLocal* threadLocal;

	// Count of alive threads.
	static volatile int threadsCount;
	static volatile int c_threadsCount;

	/**
	 * Initializes a Thread.
	 *
	 * @param g the Thread group
	 * @param target the object whose run() method gets called
	 * @param name the name of the new Thread
	 * @param stackSize the desired stack size for the new thread, or
	 *        zero to indicate that this parameter is to be ignored.
	 * @param ismain the main thread flag
	 */
	void init(sp<EThreadGroup> g, sp<ERunnable> target, const char* name,
			 ulong stacksize, ulong guardsize, boolean ismain=false);

	/**
	 * Tests if some Thread has been interrupted.  The interrupted state
	 * is reset or not based on the value of ClearInterrupted that is
	 * passed.
	 */
	boolean isInterrupted(boolean ClearInterrupted);

	void interrupt0();

	/**
	 * Set the state of this thread.
	 */
	void setState(State stat);

	/**
	 * @see: jvm os::sleep
	 */
	int sleep0(llong millis, boolean interruptible);

	/**
	 * Create a native thread.
	 */
	static boolean thread_create(es_threadattr_t *attr, es_thread_func_t *func, EThread* thread);

	/**
	 * Native thread run loop.
	 */
	static void* thread_dummy_worker(es_thread_t* handle);

public:
	/* Thread ID */
	ulong tid;

#ifdef CPP11_SUPPORT
public:
	static sp<EThread> executeX(std::function<void()> func) {
		sp<EThread> thread = new EThread(new ERunnableTarget(func));
		thread->start();
		return thread;
	}
#endif
};

} /* namespace efc */
#endif /* ETHREAD_HH_ */

