/*
 * ETimer.cpp
 *
 *  Created on: 2015-2-16
 *      Author: cxxjava@163.com
 */

#include "ETimer.hh"
#include "EThread.hh"
#include "ESystem.hh"
#include "EMath.hh"
#include "ELLong.hh"
#include "EString.hh"
#include "EArrays.hh"
#include "ESynchronizeable.hh"
#include "EInterruptedException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {
namespace timer {

/**
 * This class represents a timer task queue: a priority queue of TimerTasks,
 * ordered on nextExecutionTime.  Each Timer object has one of these, which it
 * shares with its TimerThread.  Internally this class uses a heap, which
 * offers log(n) performance for the add, removeMin and rescheduleMin
 * operations, and constant time performance for the getMin operation.
 */
class TaskQueue : public ESynchronizeable {
public:
    /**
     * Priority queue represented as a balanced binary heap: the two children
     * of queue[n] are queue[2*n] and queue[2*n+1].  The priority queue is
     * ordered on the nextExecutionTime field: The TimerTask with the lowest
     * nextExecutionTime is in queue[1] (assuming the queue is nonempty).  For
     * each node n in the heap, and each descendant of n, d,
     * n.nextExecutionTime <= d.nextExecutionTime.
     */
    EA<sp<ETimerTask> >* queue; // = new TimerTask[128];

    /**
     * The number of tasks in the priority queue.  (The tasks are stored in
     * queue[1] up to queue[size]).
     */
    int size_;// = 0;

    ~TaskQueue() {
    	delete queue;
    }

    TaskQueue() : size_(0) {
    	queue = new EA<sp<ETimerTask> >(128);
    }

    /**
     * Returns the number of tasks currently on the queue.
     */
    int size() {
        return size_;
    }

    /**
     * Adds a new task to the priority queue.
     */
    void add(sp<ETimerTask> task) {
        // Grow backing store if necessary
        if (size_ + 1 == queue->length()) {
        	EA<sp<ETimerTask> >* old = queue;
            queue = EArrays::copyOf(queue, 2*queue->length());
            delete old;
        }

        (*queue)[++size_] = task;
        fixUp(size_);
    }

    /**
     * Return the "head task" of the priority queue.  (The head task is an
     * task with the lowest nextExecutionTime.)
     */
    sp<ETimerTask> getMin() {
        return (*queue)[1];
    }

    /**
     * Return the ith task in the priority queue, where i ranges from 1 (the
     * head task, which is returned by getMin) to the number of tasks on the
     * queue, inclusive.
     */
    sp<ETimerTask> get(int i) {
        return (*queue)[i];
    }

    /**
     * Remove the head task from the priority queue.
     */
    sp<ETimerTask> removeMin() {
    	sp<ETimerTask> task = (*queue)[1];

    	(*queue)[1] = (*queue)[size_];
    	(*queue)[size_--] = null;  // Drop extra reference to prevent memory leak
        fixDown(1);

        return task;
    }

    /**
     * Removes the ith element from queue without regard for maintaining
     * the heap invariant.  Recall that queue is one-based, so
     * 1 <= i <= size.
     */
    void quickRemove(int i) {
        ES_ASSERT(i <= size_);

        (*queue)[i] = (*queue)[size_];
        (*queue)[size_--] = null;  // Drop extra ref to prevent memory leak
    }

    /**
     * Sets the nextExecutionTime associated with the head task to the
     * specified value, and adjusts priority queue accordingly.
     */
    void rescheduleMin(long newTime) {
    	(*queue)[1]->nextExecutionTime = newTime;
        fixDown(1);
    }

    /**
     * Returns true if the priority queue contains no elements.
     */
    boolean isEmpty() {
        return size_==0;
    }

    /**
     * Removes all elements from the priority queue.
     */
    void clear() {
        // Null out task references to prevent memory leak
        //for (int i=1; i<=size_; i++) {
        //	(*queue)[i] = null;
        //}
        queue->clear();

        size_ = 0;
    }

    /**
     * Establishes the heap invariant (described above) assuming the heap
     * satisfies the invariant except possibly for the leaf-node indexed by k
     * (which may have a nextExecutionTime less than its parent's).
     *
     * This method functions by "promoting" queue[k] up the hierarchy
     * (by swapping it with its parent) repeatedly until queue[k]'s
     * nextExecutionTime is greater than or equal to that of its parent.
     */
    void fixUp(int k) {
        while (k > 1) {
            int j = k >> 1;
            if ((*queue)[j]->nextExecutionTime <= (*queue)[k]->nextExecutionTime)
                break;
            sp<ETimerTask> tmp = (*queue)[j];  (*queue)[j] = (*queue)[k]; (*queue)[k] = tmp;
            k = j;
        }
    }

    /**
     * Establishes the heap invariant (described above) in the subtree
     * rooted at k, which is assumed to satisfy the heap invariant except
     * possibly for node k itself (which may have a nextExecutionTime greater
     * than its children's).
     *
     * This method functions by "demoting" queue[k] down the hierarchy
     * (by swapping it with its smaller child) repeatedly until queue[k]'s
     * nextExecutionTime is less than or equal to those of its children.
     */
    void fixDown(int k) {
        int j;
        while ((j = k << 1) <= size_ && j > 0) {
            if (j < size_ &&
            		(*queue)[j]->nextExecutionTime > (*queue)[j+1]->nextExecutionTime)
                j++; // j indexes smallest kid
            if ((*queue)[k]->nextExecutionTime <= (*queue)[j]->nextExecutionTime)
                break;
            sp<ETimerTask> tmp = (*queue)[j];  (*queue)[j] = (*queue)[k]; (*queue)[k] = tmp;
            k = j;
        }
    }

    /**
     * Establishes the heap invariant (described above) in the entire tree,
     * assuming nothing about the order of the elements prior to the call.
     */
    void heapify() {
        for (int i = size_/2; i >= 1; i--)
            fixDown(i);
    }
};


/**
 * This "helper class" implements the timer's task execution thread, which
 * waits for tasks on the timer queue, executions them when they fire,
 * reschedules repeating tasks, and removes cancelled tasks and spent
 * non-repeating tasks from the queue.
 */
class TimerThread : public EThread {
public:
    /**
     * This flag is set to false by the reaper to inform us that there
     * are no more live references to our Timer object.  Once this flag
     * is true and there are no more tasks in our queue, there is no
     * work left for us to do, so we terminate gracefully.  Note that
     * this field is protected by queue's monitor!
     */
    boolean newTasksMayBeScheduled;// = true;

    /**
     * Our Timer's queue.  We store this reference in preference to
     * a reference to the Timer so the reference graph remains acyclic.
     * Otherwise, the Timer would never be garbage-collected and this
     * thread would never go away.
     */
    TaskQueue* queue;

    TimerThread(TaskQueue* queue) : newTasksMayBeScheduled(true) {
        this->queue = queue;
    }

    virtual void run() {
        try {
            mainLoop();
        } catch (...) {
        	finally {
				SYNCHRONIZED(queue) {
					newTasksMayBeScheduled = false;
					queue->clear();
                }}
        	}
        	throw; //!
        } finally {
            // Someone killed this Thread, behave as if Timer cancelled
            SYNCHRONIZED(queue) {
                newTasksMayBeScheduled = false;
                queue->clear();  // Eliminate obsolete references
            }}
        }
    }

    /**
     * The main timer loop.  (See class comment.)
     */
    void mainLoop() {
        while (true) {
            try {
            	sp<ETimerTask> task;
                boolean taskFired;
                sp<ETimerTask> removed;
                SYNCHRONIZED(queue) {
                    // Wait for queue to become non-empty
                    while (queue->isEmpty() && newTasksMayBeScheduled)
                        queue->wait();
                    if (queue->isEmpty())
                        break; // Queue is empty and will forever remain; die

                    // Queue nonempty; look at first evt and do the right thing
                    llong currentTime, executionTime;
                    task = queue->getMin();
                    SYNCBLOCK(&task->lock) {
                        if (task->state == ETimerTask::CANCELLED) {
                        	removed = queue->removeMin();
                            continue;  // No action required, poll queue again
                        }
                        currentTime = ESystem::currentTimeMillis();
                        executionTime = task->nextExecutionTime;
                        if ((taskFired = (executionTime<=currentTime))) {
                            if (task->period == 0) { // Non-repeating, remove
                            	removed = queue->removeMin();
                                task->state = ETimerTask::EXECUTED;
                            } else { // Repeating task, reschedule
                                queue->rescheduleMin(
                                  task->period<0 ? currentTime   - task->period
                                                : executionTime + task->period);
                            }
                        }
                    }}
                    if (!taskFired) // Task hasn't yet fired; wait
                        queue->wait(executionTime - currentTime);
                }}
                if (taskFired)  // Task fired; run it, holding no locks
                    task->run();
            } catch(EInterruptedException& e) {
            }
        }
    }
};
} /* namespace timer */

//=============================================================================

EAtomicInteger* ETimer::nextSerialNumber;

DEFINE_STATIC_INITZZ_BEGIN(ETimer)
EThread::_initzz_();
nextSerialNumber = new EAtomicInteger();
DEFINE_STATIC_INITZZ_END

ETimer::~ETimer() {
	this->cancel();
	this->thread->join();
	delete queue;
	delete thread;
}

ETimer::ETimer() {
    queue = new timer::TaskQueue();
    thread = new timer::TimerThread(queue);
	thread->setName(EString::formatOf("Timer-%d", serialNumber()).c_str());
    thread->start();
}

ETimer::ETimer(const char* name) {
	queue = new timer::TaskQueue();
    thread = new timer::TimerThread(queue);
	thread->setName(name);
    thread->start();
}

void ETimer::schedule(sp<ETimerTask> task, llong delay) {
	if (delay < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative delay.");
	sched(task, ESystem::currentTimeMillis()+delay, 0);
}

void ETimer::schedule(sp<ETimerTask> task, EDate* time) {
	sched(task, time->getTime(), 0);
}

void ETimer::schedule(sp<ETimerTask> task, llong delay, llong period) {
	if (delay < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative delay.");
	if (period <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Non-positive period.");
	sched(task, ESystem::currentTimeMillis()+delay, -period);
}

void ETimer::schedule(sp<ETimerTask> task, EDate* firstTime, llong period) {
	if (period <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Non-positive period.");
	sched(task, firstTime->getTime(), -period);
}

void ETimer::scheduleAtFixedRate(sp<ETimerTask> task, llong delay, llong period) {
	if (delay < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative delay.");
	if (period <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Non-positive period.");
	sched(task, ESystem::currentTimeMillis()+delay, period);
}

void ETimer::scheduleAtFixedRate(sp<ETimerTask> task, EDate* firstTime, llong period) {
	if (period <= 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Non-positive period.");
	sched(task, firstTime->getTime(), period);
}

void ETimer::cancel() {
	SYNCHRONIZED(queue) {
		thread->newTasksMayBeScheduled = false;
		queue->clear();
		queue->notify();  // In case queue was already empty.
    }}
}

int ETimer::purge() {
	int result = 0;

	SYNCHRONIZED(queue) {
		 for (int i = queue->size(); i > 0; i--) {
			 if (queue->get(i)->state == ETimerTask::CANCELLED) {
				 queue->quickRemove(i);
				 result++;
			 }
		 }

		 if (result != 0)
			 queue->heapify();
    }}

	 return result;
}

int ETimer::serialNumber() {
	return nextSerialNumber->getAndIncrement();
}

void ETimer::sched(sp<ETimerTask> task, llong time, llong period) {
	if (time < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Illegal execution time.");

	// Constrain value of period sufficiently to prevent numeric
	// overflow while still being effectively infinitely large.
	if (EMath::abs(period) > (ELLong::MAX_VALUE >> 1))
		period >>= 1;

	SYNCHRONIZED(queue) {
		if (!thread->newTasksMayBeScheduled)
			throw EIllegalStateException(__FILE__, __LINE__, "Timer already cancelled.");

		SYNCBLOCK(&task->lock) {
			if (task->state != ETimerTask::VIRGIN)
				throw EIllegalStateException(__FILE__, __LINE__,
					"Task already scheduled or cancelled");
			task->nextExecutionTime = time;
			task->period = period;
			task->state = ETimerTask::SCHEDULED;
        }}

		queue->add(task);
		if (queue->getMin() == task)
			queue->notify();
    }}
}

} /* namespace efc */
