/*
 * ETimer.hh
 *
 *  Created on: 2015-2-16
 *      Author: cxxjava@163.com
 */

#ifndef ETIMER_HH_
#define ETIMER_HH_

#include "EObject.hh"
#include "ETimerTask.hh"
#include "EDate.hh"
#include "EAtomicInteger.hh"

namespace efc {

/**
 * A facility for threads to schedule tasks for future execution in a
 * background thread.  Tasks may be scheduled for one-time execution, or for
 * repeated execution at regular intervals.
 *
 * <p>Corresponding to each <tt>Timer</tt> object is a single background
 * thread that is used to execute all of the timer's tasks, sequentially.
 * Timer tasks should complete quickly.  If a timer task takes excessive time
 * to complete, it "hogs" the timer's task execution thread.  This can, in
 * turn, delay the execution of subsequent tasks, which may "bunch up" and
 * execute in rapid succession when (and if) the offending task finally
 * completes.
 *
 * <p>After the last live reference to a <tt>Timer</tt> object goes away
 * <i>and</i> all outstanding tasks have completed execution, the timer's task
 * execution thread terminates gracefully (and becomes subject to garbage
 * collection).  However, this can take arbitrarily long to occur.  By
 * default, the task execution thread does not run as a <i>daemon thread</i>,
 * so it is capable of keeping an application from terminating.  If a caller
 * wants to terminate a timer's task execution thread rapidly, the caller
 * should invoke the timer's <tt>cancel</tt> method.
 *
 * <p>If the timer's task execution thread terminates unexpectedly, for
 * example, because its <tt>stop</tt> method is invoked, any further
 * attempt to schedule a task on the timer will result in an
 * <tt>IllegalStateException</tt>, as if the timer's <tt>cancel</tt>
 * method had been invoked.
 *
 * <p>This class is thread-safe: multiple threads can share a single
 * <tt>Timer</tt> object without the need for external synchronization.
 *
 * <p>This class does <i>not</i> offer real-time guarantees: it schedules
 * tasks using the <tt>Object.wait(long)</tt> method.
 *
 * <p>Java 5.0 introduced the {@code java.util.concurrent} package and
 * one of the concurrency utilities therein is the {@link
 * java.util.concurrent.ScheduledThreadPoolExecutor
 * ScheduledThreadPoolExecutor} which is a thread pool for repeatedly
 * executing tasks at a given rate or delay.  It is effectively a more
 * versatile replacement for the {@code Timer}/{@code TimerTask}
 * combination, as it allows multiple service threads, accepts various
 * time units, and doesn't require subclassing {@code TimerTask} (just
 * implement {@code Runnable}).  Configuring {@code
 * ScheduledThreadPoolExecutor} with one thread makes it equivalent to
 * {@code Timer}.
 *
 * <p>Implementation note: This class scales to large numbers of concurrently
 * scheduled tasks (thousands should present no problem).  Internally,
 * it uses a binary heap to represent its task queue, so the cost to schedule
 * a task is O(log n), where n is the number of concurrently scheduled tasks.
 *
 * <p>Implementation note: All constructors start a timer thread.
 *
 * @see     TimerTask
 * @see     Object#wait(long)
 * @since   1.3
 */

namespace timer {
class TaskQueue;
class TimerThread;
}

class ETimer: public EObject {
public:
	DECLARE_STATIC_INITZZ;

public:
	virtual ~ETimer();

	/**
	 * Creates a new timer.  The associated thread does <i>not</i>
	 * {@linkplain Thread#setDaemon run as a daemon}.
	 */
	ETimer();

	/**
	 * Creates a new timer whose associated thread has the specified name.
	 * The associated thread does <i>not</i>
	 * {@linkplain Thread#setDaemon run as a daemon}.
	 *
	 * @param name the name of the associated thread
	 * @throws NullPointerException if {@code name} is null
	 * @since 1.5
	 */
	ETimer(const char* name);

	//TODO:
	ETimer(const ETimer& that);
	ETimer& operator= (const ETimer& that);

	/**
	 * Schedules the specified task for execution after the specified delay.
	 *
	 * @param task  task to be scheduled.
	 * @param delay delay in milliseconds before task is to be executed.
	 * @throws IllegalArgumentException if <tt>delay</tt> is negative, or
	 *         <tt>delay + System.currentTimeMillis()</tt> is negative.
	 * @throws IllegalStateException if task was already scheduled or
	 *         cancelled, timer was cancelled, or timer thread terminated.
	 * @throws NullPointerException if {@code task} is null
	 */
	void schedule(ETimerTask* task, llong delay);

	/**
	 * Schedules the specified task for execution at the specified time.  If
	 * the time is in the past, the task is scheduled for immediate execution.
	 *
	 * @param task task to be scheduled.
	 * @param time time at which task is to be executed.
	 * @throws IllegalArgumentException if <tt>time.getTime()</tt> is negative.
	 * @throws IllegalStateException if task was already scheduled or
	 *         cancelled, timer was cancelled, or timer thread terminated.
	 * @throws NullPointerException if {@code task} or {@code time} is null
	 */
	void schedule(ETimerTask* task, EDate* time);

	/**
	 * Schedules the specified task for repeated <i>fixed-delay execution</i>,
	 * beginning after the specified delay.  Subsequent executions take place
	 * at approximately regular intervals separated by the specified period.
	 *
	 * <p>In fixed-delay execution, each execution is scheduled relative to
	 * the actual execution time of the previous execution.  If an execution
	 * is delayed for any reason (such as garbage collection or other
	 * background activity), subsequent executions will be delayed as well.
	 * In the long run, the frequency of execution will generally be slightly
	 * lower than the reciprocal of the specified period (assuming the system
	 * clock underlying <tt>Object.wait(long)</tt> is accurate).
	 *
	 * <p>Fixed-delay execution is appropriate for recurring activities
	 * that require "smoothness."  In other words, it is appropriate for
	 * activities where it is more important to keep the frequency accurate
	 * in the short run than in the long run.  This includes most animation
	 * tasks, such as blinking a cursor at regular intervals.  It also includes
	 * tasks wherein regular activity is performed in response to human
	 * input, such as automatically repeating a character as long as a key
	 * is held down.
	 *
	 * @param task   task to be scheduled.
	 * @param delay  delay in milliseconds before task is to be executed.
	 * @param period time in milliseconds between successive task executions.
	 * @throws IllegalArgumentException if {@code delay < 0}, or
	 *         {@code delay + System.currentTimeMillis() < 0}, or
	 *         {@code period <= 0}
	 * @throws IllegalStateException if task was already scheduled or
	 *         cancelled, timer was cancelled, or timer thread terminated.
	 * @throws NullPointerException if {@code task} is null
	 */
	void schedule(ETimerTask* task, llong delay, llong period);

	/**
	 * Schedules the specified task for repeated <i>fixed-delay execution</i>,
	 * beginning at the specified time. Subsequent executions take place at
	 * approximately regular intervals, separated by the specified period.
	 *
	 * <p>In fixed-delay execution, each execution is scheduled relative to
	 * the actual execution time of the previous execution.  If an execution
	 * is delayed for any reason (such as garbage collection or other
	 * background activity), subsequent executions will be delayed as well.
	 * In the long run, the frequency of execution will generally be slightly
	 * lower than the reciprocal of the specified period (assuming the system
	 * clock underlying <tt>Object.wait(long)</tt> is accurate).  As a
	 * consequence of the above, if the scheduled first time is in the past,
	 * it is scheduled for immediate execution.
	 *
	 * <p>Fixed-delay execution is appropriate for recurring activities
	 * that require "smoothness."  In other words, it is appropriate for
	 * activities where it is more important to keep the frequency accurate
	 * in the short run than in the long run.  This includes most animation
	 * tasks, such as blinking a cursor at regular intervals.  It also includes
	 * tasks wherein regular activity is performed in response to human
	 * input, such as automatically repeating a character as long as a key
	 * is held down.
	 *
	 * @param task   task to be scheduled.
	 * @param firstTime First time at which task is to be executed.
	 * @param period time in milliseconds between successive task executions.
	 * @throws IllegalArgumentException if {@code firstTime.getTime() < 0}, or
	 *         {@code period <= 0}
	 * @throws IllegalStateException if task was already scheduled or
	 *         cancelled, timer was cancelled, or timer thread terminated.
	 * @throws NullPointerException if {@code task} or {@code firstTime} is null
	 */
	void schedule(ETimerTask* task, EDate* firstTime, llong period);

	/**
	 * Schedules the specified task for repeated <i>fixed-rate execution</i>,
	 * beginning after the specified delay.  Subsequent executions take place
	 * at approximately regular intervals, separated by the specified period.
	 *
	 * <p>In fixed-rate execution, each execution is scheduled relative to the
	 * scheduled execution time of the initial execution.  If an execution is
	 * delayed for any reason (such as garbage collection or other background
	 * activity), two or more executions will occur in rapid succession to
	 * "catch up."  In the long run, the frequency of execution will be
	 * exactly the reciprocal of the specified period (assuming the system
	 * clock underlying <tt>Object.wait(long)</tt> is accurate).
	 *
	 * <p>Fixed-rate execution is appropriate for recurring activities that
	 * are sensitive to <i>absolute</i> time, such as ringing a chime every
	 * hour on the hour, or running scheduled maintenance every day at a
	 * particular time.  It is also appropriate for recurring activities
	 * where the total time to perform a fixed number of executions is
	 * important, such as a countdown timer that ticks once every second for
	 * ten seconds.  Finally, fixed-rate execution is appropriate for
	 * scheduling multiple repeating timer tasks that must remain synchronized
	 * with respect to one another.
	 *
	 * @param task   task to be scheduled.
	 * @param delay  delay in milliseconds before task is to be executed.
	 * @param period time in milliseconds between successive task executions.
	 * @throws IllegalArgumentException if {@code delay < 0}, or
	 *         {@code delay + System.currentTimeMillis() < 0}, or
	 *         {@code period <= 0}
	 * @throws IllegalStateException if task was already scheduled or
	 *         cancelled, timer was cancelled, or timer thread terminated.
	 * @throws NullPointerException if {@code task} is null
	 */
	void scheduleAtFixedRate(ETimerTask* task, llong delay, llong period);

	/**
	 * Schedules the specified task for repeated <i>fixed-rate execution</i>,
	 * beginning at the specified time. Subsequent executions take place at
	 * approximately regular intervals, separated by the specified period.
	 *
	 * <p>In fixed-rate execution, each execution is scheduled relative to the
	 * scheduled execution time of the initial execution.  If an execution is
	 * delayed for any reason (such as garbage collection or other background
	 * activity), two or more executions will occur in rapid succession to
	 * "catch up."  In the long run, the frequency of execution will be
	 * exactly the reciprocal of the specified period (assuming the system
	 * clock underlying <tt>Object.wait(long)</tt> is accurate).  As a
	 * consequence of the above, if the scheduled first time is in the past,
	 * then any "missed" executions will be scheduled for immediate "catch up"
	 * execution.
	 *
	 * <p>Fixed-rate execution is appropriate for recurring activities that
	 * are sensitive to <i>absolute</i> time, such as ringing a chime every
	 * hour on the hour, or running scheduled maintenance every day at a
	 * particular time.  It is also appropriate for recurring activities
	 * where the total time to perform a fixed number of executions is
	 * important, such as a countdown timer that ticks once every second for
	 * ten seconds.  Finally, fixed-rate execution is appropriate for
	 * scheduling multiple repeating timer tasks that must remain synchronized
	 * with respect to one another.
	 *
	 * @param task   task to be scheduled.
	 * @param firstTime First time at which task is to be executed.
	 * @param period time in milliseconds between successive task executions.
	 * @throws IllegalArgumentException if {@code firstTime.getTime() < 0} or
	 *         {@code period <= 0}
	 * @throws IllegalStateException if task was already scheduled or
	 *         cancelled, timer was cancelled, or timer thread terminated.
	 * @throws NullPointerException if {@code task} or {@code firstTime} is null
	 */
	void scheduleAtFixedRate(ETimerTask* task, EDate* firstTime, llong period);

	/**
	 * Terminates this timer, discarding any currently scheduled tasks.
	 * Does not interfere with a currently executing task (if it exists).
	 * Once a timer has been terminated, its execution thread terminates
	 * gracefully, and no more tasks may be scheduled on it.
	 *
	 * <p>Note that calling this method from within the run method of a
	 * timer task that was invoked by this timer absolutely guarantees that
	 * the ongoing task execution is the last task execution that will ever
	 * be performed by this timer.
	 *
	 * <p>This method may be called repeatedly; the second and subsequent
	 * calls have no effect.
	 */
	void cancel();

	/**
	 * Removes all cancelled tasks from this timer's task queue.  <i>Calling
	 * this method has no effect on the behavior of the timer</i>, but
	 * eliminates the references to the cancelled tasks from the queue.
	 * If there are no external references to these tasks, they become
	 * eligible for garbage collection.
	 *
	 * <p>Most programs will have no need to call this method.
	 * It is designed for use by the rare application that cancels a large
	 * number of tasks.  Calling this method trades time for space: the
	 * runtime of the method may be proportional to n + c log n, where n
	 * is the number of tasks in the queue and c is the number of cancelled
	 * tasks.
	 *
	 * <p>Note that it is permissible to call this method from within a
	 * a task scheduled on this timer.
	 *
	 * @return the number of tasks removed from the queue.
	 * @since 1.5
	 */
	int purge();

private:
	/**
	 * The timer task queue.  This data structure is shared with the timer
	 * thread.  The timer produces tasks, via its various schedule calls,
	 * and the timer thread consumes, executing timer tasks as appropriate,
	 * and removing them from the queue when they're obsolete.
	 */
	timer::TaskQueue* queue;// = new TaskQueue();

	/**
	 * The timer thread.
	 */
	timer::TimerThread* thread;// = new TimerThread(queue);

	/**
	 * This ID is used to generate thread names.
	 */
	static EAtomicInteger* nextSerialNumber;
	static int serialNumber();

	/**
	 * Schedule the specified timer task for execution at the specified
	 * time with the specified period, in milliseconds.  If period is
	 * positive, the task is scheduled for repeated execution; if period is
	 * zero, the task is scheduled for one-time execution. Time is specified
	 * in Date.getTime() format.  This method checks timer state, task state,
	 * and initial execution time, but not period.
	 *
	 * @throws IllegalArgumentException if <tt>time</tt> is negative.
	 * @throws IllegalStateException if task was already scheduled or
	 *         cancelled, timer was cancelled, or timer thread terminated.
	 * @throws NullPointerException if {@code task} is null
	 */
	void sched(ETimerTask* task, llong time, llong period);
};

} /* namespace efc */
#endif /* ETIMER_HH_ */
