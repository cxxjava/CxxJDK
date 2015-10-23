/*
 * EThreadGroup.hh
 *
 *  Created on: 2014-9-29
 *      Author: cxxjava@163.com
 */

#ifndef ETHREADGROUP_HH_
#define ETHREADGROUP_HH_

#include "EA.hh"
#include "EThread.hh"
#include "ESharedArr.hh"

namespace efc {

/**
 * A thread group represents a set of threads. In addition, a thread
 * group can also include other thread groups. The thread groups form
 * a tree in which every thread group except the initial thread group
 * has a parent.
 * <p>
 * A thread is allowed to access information about its own thread
 * group, but not to access information about its thread group's
 * parent thread group or any other thread groups.
 *
 * @author  unascribed
 * @since   JDK1.0
 */
/* The locking strategy for this code is to try to lock only one level of the
 * tree wherever possible, but otherwise to lock from the bottom up.
 * That is, from child thread groups to parents.
 * This has the advantage of limiting the number of locks that need to be held
 * and in particular avoids having to grab the lock for the root thread group,
 * (or a global lock) which would be a source of contention on a
 * multi-processor system with many thread groups.
 * This policy often leads to taking a snapshot of the state of a thread group
 * and working off of that snapshot, rather than holding the thread group locked
 * while we work on the children.
 */

class EThreadGroup: public EThread::UncaughtExceptionHandler,
		public ESynchronizeable {
public:
	virtual ~EThreadGroup();

	/**
	 * Constructs a new thread group. The parent of this new group is
	 * the thread group of the currently running thread.
	 * <p>
	 * The <code>checkAccess</code> method of the parent thread group is
	 * called with no arguments; this may result in a security exception.
	 *
	 * @param   name   the name of the new thread group.
	 * @exception  SecurityException  if the current thread cannot create a
	 *               thread in the specified thread group.
	 * @see     java.lang.ThreadGroup#checkAccess()
	 * @since   JDK1.0
	 */
	EThreadGroup(const char* name);

	/**
	 * Creates a new thread group. The parent of this new group is the
	 * specified thread group.
	 * <p>
	 * The <code>checkAccess</code> method of the parent thread group is
	 * called with no arguments; this may result in a security exception.
	 *
	 * @param     parent   the parent thread group.
	 * @param     name     the name of the new thread group.
	 * @exception  NullPointerException  if the thread group argument is
	 *               <code>null</code>.
	 * @exception  SecurityException  if the current thread cannot create a
	 *               thread in the specified thread group.
	 * @see     java.lang.SecurityException
	 * @see     java.lang.ThreadGroup#checkAccess()
	 * @since   JDK1.0
	 */
	EThreadGroup(sp<EThreadGroup> parent, const char* name);

	/**
	 * Returns the name of this thread group.
	 *
	 * @return  the name of this thread group.
	 * @since   JDK1.0
	 */
	const char* getName();

	/**
	 * Returns the parent of this thread group.
	 * <p>
	 * First, if the parent is not <code>null</code>, the
	 * <code>checkAccess</code> method of the parent thread group is
	 * called with no arguments; this may result in a security exception.
	 *
	 * @return  the parent of this thread group. The top-level thread group
	 *          is the only thread group whose parent is <code>null</code>.
	 * @exception  SecurityException  if the current thread cannot modify
	 *               this thread group.
	 * @see        java.lang.ThreadGroup#checkAccess()
	 * @see        java.lang.SecurityException
	 * @see        java.lang.RuntimePermission
	 * @since   JDK1.0
	 */
	sp<EThreadGroup> getParent();

	/**
	 * Returns the maximum priority of this thread group. Threads that are
	 * part of this group cannot have a higher priority than the maximum
	 * priority.
	 *
	 * @return  the maximum priority that a thread in this thread group
	 *          can have.
	 * @see     #setMaxPriority
	 * @since   JDK1.0
	 */
	int getMaxPriority();

	/**
	 * Tests if this thread group is a daemon thread group. A
	 * daemon thread group is automatically destroyed when its last
	 * thread is stopped or its last thread group is destroyed.
	 *
	 * @return  <code>true</code> if this thread group is a daemon thread group;
	 *          <code>false</code> otherwise.
	 * @since   JDK1.0
	 */
	boolean isDaemon();

	/**
	 * Changes the daemon status of this thread group.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 * <p>
	 * A daemon thread group is automatically destroyed when its last
	 * thread is stopped or its last thread group is destroyed.
	 *
	 * @param      daemon   if <code>true</code>, marks this thread group as
	 *                      a daemon thread group; otherwise, marks this
	 *                      thread group as normal.
	 * @exception  SecurityException  if the current thread cannot modify
	 *               this thread group.
	 * @see        java.lang.SecurityException
	 * @see        java.lang.ThreadGroup#checkAccess()
	 * @since      JDK1.0
	 */
	void setDaemon(boolean daemon);

	/**
	 * Tests if this thread group has been destroyed.
	 *
	 * @return  true if this object is destroyed
	 * @since   JDK1.1
	 */
	synchronized boolean isDestroyed();

	/**
	 * Sets the maximum priority of the group. Threads in the thread
	 * group that already have a higher priority are not affected.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 * <p>
	 * If the <code>pri</code> argument is less than
	 * {@link Thread#MIN_PRIORITY} or greater than
	 * {@link Thread#MAX_PRIORITY}, the maximum priority of the group
	 * remains unchanged.
	 * <p>
	 * Otherwise, the priority of this ThreadGroup object is set to the
	 * smaller of the specified <code>pri</code> and the maximum permitted
	 * priority of the parent of this thread group. (If this thread group
	 * is the system thread group, which has no parent, then its maximum
	 * priority is simply set to <code>pri</code>.) Then this method is
	 * called recursively, with <code>pri</code> as its argument, for
	 * every thread group that belongs to this thread group.
	 *
	 * @param      pri   the new priority of the thread group.
	 * @exception  SecurityException  if the current thread cannot modify
	 *               this thread group.
	 * @see        #getMaxPriority
	 * @see        java.lang.SecurityException
	 * @see        java.lang.ThreadGroup#checkAccess()
	 * @since      JDK1.0
	 */
	void setMaxPriority(int pri);

	/**
	 * Tests if this thread group is either the thread group
	 * argument or one of its ancestor thread groups.
	 *
	 * @param   g   a thread group.
	 * @return  <code>true</code> if this thread group is the thread group
	 *          argument or one of its ancestor thread groups;
	 *          <code>false</code> otherwise.
	 * @since   JDK1.0
	 */
	boolean parentOf(sp<EThreadGroup> g);

	/**
	 * Determines if the currently running thread has permission to
	 * modify this thread group.
	 * <p>
	 * If there is a security manager, its <code>checkAccess</code> method
	 * is called with this thread group as its argument. This may result
	 * in throwing a <code>SecurityException</code>.
	 *
	 * @exception  SecurityException  if the current thread is not allowed to
	 *               access this thread group.
	 * @see        java.lang.SecurityManager#checkAccess(java.lang.ThreadGroup)
	 * @since      JDK1.0
	 */
	void checkAccess();

	/**
	 * Returns an estimate of the number of active threads in this
	 * thread group.  The result might not reflect concurrent activity,
	 * and might be affected by the presence of certain system threads.
	 * <p>
	 * Due to the inherently imprecise nature of the result, it is
	 * recommended that this method only be used for informational purposes.
	 *
	 * @return  an estimate of the number of active threads in this thread
	 *          group and in any other thread group that has this thread
	 *          group as an ancestor.
	 * @since   JDK1.0
	 */
	int activeCount();

	/**
	 * Copies into the specified array every active thread in this
	 * thread group and its subgroups.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 * <p>
	 * An application might use the <code>activeCount</code> method to
	 * get an estimate of how big the array should be, however <i>if the
	 * array is too short to hold all the threads, the extra threads are
	 * silently ignored.</i>  If it is critical to obtain every active
	 * thread in this thread group and its subgroups, the caller should
	 * verify that the returned int value is strictly less than the length
	 * of <tt>list</tt>.
	 * <p>
	 * Due to the inherent race condition in this method, it is recommended
	 * that the method only be used for informational purposes.
	 *
	 * @param   list   an array into which to place the list of threads.
	 * @return  the number of threads put into the array.
	 * @exception  SecurityException  if the current thread does not
	 *               have permission to enumerate this thread group.
	 * @see     java.lang.ThreadGroup#activeCount()
	 * @see     java.lang.ThreadGroup#checkAccess()
	 * @since   JDK1.0
	 */
	int enumerate(EA<EThread*>* list);

	/**
	 * Copies into the specified array every active thread in this
	 * thread group. If the <code>recurse</code> flag is
	 * <code>true</code>, references to every active thread in this
	 * thread's subgroups are also included. If the array is too short to
	 * hold all the threads, the extra threads are silently ignored.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 * <p>
	 * An application might use the <code>activeCount</code> method to
	 * get an estimate of how big the array should be, however <i>if the
	 * array is too short to hold all the threads, the extra threads are
	 * silently ignored.</i>  If it is critical to obtain every active thread
	 * in this thread group, the caller should verify that the returned int
	 * value is strictly less than the length of <tt>list</tt>.
	 * <p>
	 * Due to the inherent race condition in this method, it is recommended
	 * that the method only be used for informational purposes.
	 *
	 * @param   list      an array into which to place the list of threads.
	 * @param   recurse   a flag indicating whether also to include threads
	 *                    in thread groups that are subgroups of this
	 *                    thread group.
	 * @return  the number of threads placed into the array.
	 * @exception  SecurityException  if the current thread does not
	 *               have permission to enumerate this thread group.
	 * @see     java.lang.ThreadGroup#activeCount()
	 * @see     java.lang.ThreadGroup#checkAccess()
	 * @since   JDK1.0
	 */
	int enumerate(EA<EThread*>* list, boolean recurse);

	/**
	 * Returns an estimate of the number of active groups in this
	 * thread group.  The result might not reflect concurrent activity.
	 * <p>
	 * Due to the inherently imprecise nature of the result, it is
	 * recommended that this method only be used for informational purposes.
	 *
	 * @return  the number of active thread groups with this thread group as
	 *          an ancestor.
	 * @since   JDK1.0
	 */
	int activeGroupCount();

	/**
	 * Copies into the specified array references to every active
	 * subgroup in this thread group.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 * <p>
	 * An application might use the <code>activeGroupCount</code> method to
	 * get an estimate of how big the array should be, however <i>if the
	 * array is too short to hold all the thread groups, the extra thread
	 * groups are silently ignored.</i>  If it is critical to obtain every
	 * active subgroup in this thread group, the caller should verify that
	 * the returned int value is strictly less than the length of
	 * <tt>list</tt>.
	 * <p>
	 * Due to the inherent race condition in this method, it is recommended
	 * that the method only be used for informational purposes.
	 *
	 * @param   list   an array into which to place the list of thread groups.
	 * @return  the number of thread groups put into the array.
	 * @exception  SecurityException  if the current thread does not
	 *               have permission to enumerate this thread group.
	 * @see     java.lang.ThreadGroup#activeGroupCount()
	 * @see     java.lang.ThreadGroup#checkAccess()
	 * @since   JDK1.0
	 */
	int enumerate(EA<EThreadGroup*>* list);

	/**
	 * Copies into the specified array references to every active
	 * subgroup in this thread group. If the <code>recurse</code> flag is
	 * <code>true</code>, references to all active subgroups of the
	 * subgroups and so forth are also included.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 * <p>
	 * An application might use the <code>activeGroupCount</code> method to
	 * get an estimate of how big the array should be, however <i>if the
	 * array is too short to hold all the thread groups, the extra thread
	 * groups are silently ignored.</i>  If it is critical to obtain every
	 * active subgroup in this thread group, the caller should verify that
	 * the returned int value is strictly less than the length of
	 * <tt>list</tt>.
	 * <p>
	 * Due to the inherent race condition in this method, it is recommended
	 * that the method only be used for informational purposes.
	 *
	 * @param   list      an array into which to place the list of threads.
	 * @param   recurse   a flag indicating whether to recursively enumerate
	 *                    all included thread groups.
	 * @return  the number of thread groups put into the array.
	 * @exception  SecurityException  if the current thread does not
	 *               have permission to enumerate this thread group.
	 * @see     java.lang.ThreadGroup#activeGroupCount()
	 * @see     java.lang.ThreadGroup#checkAccess()
	 * @since   JDK1.0
	 */
	int enumerate(EA<EThreadGroup*>* list, boolean recurse);

	/**
	 * Interrupts all threads in this thread group.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 * <p>
	 * This method then calls the <code>interrupt</code> method on all the
	 * threads in this thread group and in all of its subgroups.
	 *
	 * @exception  SecurityException  if the current thread is not allowed
	 *               to access this thread group or any of the threads in
	 *               the thread group.
	 * @see        java.lang.Thread#interrupt()
	 * @see        java.lang.SecurityException
	 * @see        java.lang.ThreadGroup#checkAccess()
	 * @since      1.2
	 */
	void interrupt();

	/**
	 * Destroys this thread group and all of its subgroups. This thread
	 * group must be empty, indicating that all threads that had been in
	 * this thread group have since stopped.
	 * <p>
	 * First, the <code>checkAccess</code> method of this thread group is
	 * called with no arguments; this may result in a security exception.
	 *
	 * @exception  IllegalThreadStateException  if the thread group is not
	 *               empty or if the thread group has already been destroyed.
	 * @exception  SecurityException  if the current thread cannot modify this
	 *               thread group.
	 * @see        java.lang.ThreadGroup#checkAccess()
	 * @since      JDK1.0
	 */
	void destroy();

	/**
	 * Called by the Java Virtual Machine when a thread in this
	 * thread group stops because of an uncaught exception, and the thread
	 * does not have a specific {@link Thread.UncaughtExceptionHandler}
	 * installed.
	 * <p>
	 * The <code>uncaughtException</code> method of
	 * <code>ThreadGroup</code> does the following:
	 * <ul>
	 * <li>If this thread group has a parent thread group, the
	 *     <code>uncaughtException</code> method of that parent is called
	 *     with the same two arguments.
	 * <li>Otherwise, this method checks to see if there is a
	 *     {@linkplain Thread#getDefaultUncaughtExceptionHandler default
	 *     uncaught exception handler} installed, and if so, its
	 *     <code>uncaughtException</code> method is called with the same
	 *     two arguments.
	 * <li>Otherwise, this method determines if the <code>Throwable</code>
	 *     argument is an instance of {@link ThreadDeath}. If so, nothing
	 *     special is done. Otherwise, a message containing the
	 *     thread's name, as returned from the thread's {@link
	 *     Thread#getName getName} method, and a stack backtrace,
	 *     using the <code>Throwable</code>'s {@link
	 *     Throwable#printStackTrace printStackTrace} method, is
	 *     printed to the {@linkplain System#err standard error stream}.
	 * </ul>
	 * <p>
	 * Applications can override this method in subclasses of
	 * <code>ThreadGroup</code> to provide alternative handling of
	 * uncaught exceptions.
	 *
	 * @param   t   the thread that is about to exit.
	 * @param   e   the uncaught exception.
	 * @since   JDK1.0
	 */
	void uncaughtException(EThread* t, EThrowable* e);

	/**
	 * Returns a string representation of this Thread group.
	 *
	 * @return  a string representation of this thread group.
	 * @since   JDK1.0
	 */
	EString toString();

protected:
	friend class EThread;

	EString name;
	int maxPriority;
	boolean destroyed;
	boolean daemon;

	int nUnstartedThreads;// = 0;
	int nthreads;
	EA<EThread*>* threads;

	int ngroups;
	ea<EThreadGroup>* groups;

	/**
	 * Increments the count of unstarted threads in the thread group.
	 * Unstarted threads are not added to the thread group so that they
	 * can be collected if they are never started, but they must be
	 * counted so that daemon thread groups with unstarted threads in
	 * them are not destroyed.
	 */
	void addUnstarted();

	/**
	 * Adds the specified Thread to this group.
	 * @param t the Thread to be added
	 * @exception IllegalThreadStateException If the Thread group has been destroyed.
	 */
	void add(EThread* t);

	/**
	 * Removes the specified Thread from this group.
	 * @param t the Thread to be removed
	 * @return if the Thread has already been destroyed.
	 */
	void remove(EThread* t);

private:
	sp<EThreadGroup> parent;

	int enumerate(EA<EThread*>* list, int n, boolean recurse);
	int enumerate(EA<EThreadGroup*>* list, int n, boolean recurse);

	/**
	 * Adds the specified Thread group to this group.
	 * @param g the specified Thread group to be added
	 * @exception IllegalThreadStateException If the Thread group has been destroyed.
	 */
	void add(sp<EThreadGroup> g);

	/**
	 * Removes the specified Thread group from this group.
	 * @param g the Thread group to be removed
	 * @return if this Thread has already been destroyed.
	 */
	void remove(sp<EThreadGroup> g);

private:
	EThreadGroup();
};

} /* namespace efc */
#endif /* ETHREADGROUP_HH_ */
