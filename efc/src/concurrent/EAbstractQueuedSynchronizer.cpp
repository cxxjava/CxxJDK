/*
 * EAbstractQueuedSynchronizer.cpp
 *
 *  Created on: 2014-9-23
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EAbstractQueuedSynchronizer.hh"
#include "../../inc/concurrent/ELockSupport.hh"
#include "../../inc/EArrayList.hh"
#include "../../inc/concurrent/EAtomic.hh"
#include "../../inc/ESystem.hh"
#include "../../inc/EDate.hh"
#include "../../inc/EUnsupportedOperationException.hh"

namespace efc {

sp<EAbstractQueuedSynchronizer::Node>  EAbstractQueuedSynchronizer::Node::SHARED = new Node();
sp<EAbstractQueuedSynchronizer::Node> EAbstractQueuedSynchronizer::Node::EXCLUSIVE;

/*
 * For interruptible waits, we need to track whether to throw
 * InterruptedException, if interrupted while blocked on
 * condition, versus reinterrupt current thread, if
 * interrupted while blocked waiting to re-acquire.
 */

/** Mode meaning to reinterrupt on exit from wait */
#define REINTERRUPT   1
/** Mode meaning to throw InterruptedException on exit from wait */
#define THROW_IE     -1

/**
 * The number of nanoseconds for which it is faster to spin
 * rather than to use timed park. A rough estimate suffices
 * to improve responsiveness with very short timeouts.
 */
const llong spinForTimeoutThreshold = 1000L;

EAbstractQueuedSynchronizer::EAbstractQueuedSynchronizer() : state(0)
{
}

efc::EAbstractQueuedSynchronizer::~EAbstractQueuedSynchronizer() {
	//
}

EAbstractQueuedSynchronizer::ConditionObject::~ConditionObject() {
	//
}

EAbstractQueuedSynchronizer::ConditionObject::ConditionObject(
		EAbstractQueuedSynchronizer* aqs) : aqs(aqs) {
}

sp<EAbstractQueuedSynchronizer::Node> EAbstractQueuedSynchronizer::ConditionObject::addConditionWaiter() {
	sp<Node> t = atomic_load(&lastWaiter);
	// If lastWaiter is cancelled, clean out.
	if (t != null && t->waitStatus != Node::CONDITION) {
		unlinkCancelledWaiters();
		t = atomic_load(&lastWaiter);
	}
	sp<Node> node = new Node(EThread::currentThread(), Node::CONDITION);
	if (t == null)
		atomic_store(&firstWaiter, node);
	else
		atomic_store(&t->nextWaiter, node);
	atomic_store(&lastWaiter, node);
	return node;
}

void EAbstractQueuedSynchronizer::ConditionObject::doSignal(sp<Node>& first) {
	do {
		sp<Node> node = atomic_load(&first->nextWaiter);
		atomic_store(&firstWaiter, node);
		if (atomic_load(&firstWaiter) == null)
			atomic_store(&lastWaiter, null);
		atomic_store(&first->nextWaiter, null);
	} while (!aqs->transferForSignal(first) && (first = atomic_load(&firstWaiter)) != null);
}

void EAbstractQueuedSynchronizer::ConditionObject::doSignalAll(sp<Node>& first) {
	atomic_store(&lastWaiter, null);
	atomic_store(&firstWaiter, null);
	do {
		sp<Node> next = atomic_load(&first->nextWaiter);
		atomic_store(&first->nextWaiter, null);
		aqs->transferForSignal(first);
		first = next;
	} while (first != null);
}

void EAbstractQueuedSynchronizer::ConditionObject::unlinkCancelledWaiters() {
	sp<Node> t = atomic_load(&firstWaiter);
	sp<Node> trail = null;
	while (t != null) {
		sp<Node> next = atomic_load(&t->nextWaiter);
		if (t->waitStatus != Node::CONDITION) {
			atomic_store(&t->nextWaiter, null);
			if (trail == null)
				atomic_store(&firstWaiter, next);
			else
				atomic_store(&trail->nextWaiter, next);
			if (next == null)
				atomic_store(&lastWaiter, trail);
		} else {
			trail = t;
		}
		t = next;
	}
}

void EAbstractQueuedSynchronizer::ConditionObject::signal() {
	if (!aqs->isHeldExclusively())
		throw EIllegalStateException(__FILE__, __LINE__);
	sp<Node> first = atomic_load(&firstWaiter);
	if (first != null)
		doSignal(first);
}

void EAbstractQueuedSynchronizer::ConditionObject::signalAll() {
	if (!aqs->isHeldExclusively())
		throw EIllegalStateException(__FILE__, __LINE__);
	sp<Node> first = atomic_load(&firstWaiter);
	if (first != null)
		doSignalAll(first);
}

void EAbstractQueuedSynchronizer::ConditionObject::awaitUninterruptibly() {
	sp<Node> node = addConditionWaiter();
	int savedState = aqs->fullyRelease(node);
	boolean interrupted = false;
	while (!aqs->isOnSyncQueue(node)) {
		ELockSupport::park(/*this*/);
		if (EThread::interrupted())
			interrupted = true;
	}
	if (aqs->acquireQueued(node, savedState) || interrupted)
		selfInterrupt();
}

int EAbstractQueuedSynchronizer::ConditionObject::checkInterruptWhileWaiting(sp<Node>& node) {
	return EThread::interrupted() ?
			(aqs->transferAfterCancelledWait(node) ? THROW_IE : REINTERRUPT) :
			0;
}

void EAbstractQueuedSynchronizer::ConditionObject::reportInterruptAfterWait(int interruptMode) {
	if (interruptMode == THROW_IE)
		throw EInterruptedException(__FILE__, __LINE__);
	else if (interruptMode == REINTERRUPT)
		selfInterrupt();
}

void EAbstractQueuedSynchronizer::ConditionObject::await() {
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	sp<Node> node = addConditionWaiter();
	int savedState = aqs->fullyRelease(node);
	int interruptMode = 0;
	while (!aqs->isOnSyncQueue(node)) {
		ELockSupport::park(/*this*/);
		if ((interruptMode = checkInterruptWhileWaiting(node)) != 0)
			break;
	}
	if (aqs->acquireQueued(node, savedState) && interruptMode != THROW_IE)
		interruptMode = REINTERRUPT;
	if (atomic_load(&node->nextWaiter) != null) // clean up if cancelled
		unlinkCancelledWaiters();
	if (interruptMode != 0)
		reportInterruptAfterWait(interruptMode);
}

llong EAbstractQueuedSynchronizer::ConditionObject::awaitNanos(
		llong nanosTimeout) {
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	sp<Node> node = addConditionWaiter();
	int savedState = aqs->fullyRelease(node);
	llong deadline = ESystem::nanoTime() + nanosTimeout;
	int interruptMode = 0;
	while (!aqs->isOnSyncQueue(node)) {
		if (nanosTimeout <= 0L) {
			aqs->transferAfterCancelledWait(node);
			break;
		}
		if (nanosTimeout >= spinForTimeoutThreshold)
			ELockSupport::parkNanos(/*this, */nanosTimeout);
		if ((interruptMode = checkInterruptWhileWaiting(node)) != 0)
			break;
		nanosTimeout = deadline - ESystem::nanoTime();
	}
	if (aqs->acquireQueued(node, savedState) && interruptMode != THROW_IE)
		interruptMode = REINTERRUPT;
	if (atomic_load(&node->nextWaiter) != null)
		unlinkCancelledWaiters();
	if (interruptMode != 0)
		reportInterruptAfterWait(interruptMode);
	return deadline - ESystem::nanoTime();
}

boolean EAbstractQueuedSynchronizer::ConditionObject::awaitUntil(
		EDate* deadline) {
	if (deadline == null)
		throw ENullPointerException(__FILE__, __LINE__);
	llong abstime = deadline->getTime();
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	sp<Node> node = addConditionWaiter();
	int savedState = aqs->fullyRelease(node);
	boolean timedout = false;
	int interruptMode = 0;
	while (!aqs->isOnSyncQueue(node)) {
		if (ESystem::currentTimeMillis() > abstime) {
			timedout = aqs->transferAfterCancelledWait(node);
			break;
		}
		ELockSupport::parkUntil(/*this, */abstime);
		if ((interruptMode = checkInterruptWhileWaiting(node)) != 0)
			break;
	}
	if (aqs->acquireQueued(node, savedState) && interruptMode != THROW_IE)
		interruptMode = REINTERRUPT;
	if (atomic_load(&node->nextWaiter) != null)
		unlinkCancelledWaiters();
	if (interruptMode != 0)
		reportInterruptAfterWait(interruptMode);
	return !timedout;
}

boolean EAbstractQueuedSynchronizer::ConditionObject::await(llong time,
		ETimeUnit* unit) {
	if (unit == null)
		throw ENullPointerException(__FILE__, __LINE__);
	llong nanosTimeout = unit->toNanos(time);
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	sp<Node> node = addConditionWaiter();
	int savedState = aqs->fullyRelease(node);
	llong deadline = ESystem::nanoTime() + nanosTimeout;
	boolean timedout = false;
	int interruptMode = 0;
	while (!aqs->isOnSyncQueue(node)) {
		if (nanosTimeout <= 0L) {
			timedout = aqs->transferAfterCancelledWait(node);
			break;
		}
		if (nanosTimeout >= spinForTimeoutThreshold)
			ELockSupport::parkNanos(/*this, */nanosTimeout);
		if ((interruptMode = checkInterruptWhileWaiting(node)) != 0)
			break;
		nanosTimeout = deadline - ESystem::nanoTime();
	}
	if (aqs->acquireQueued(node, savedState) && interruptMode != THROW_IE)
		interruptMode = REINTERRUPT;
	if (atomic_load(&node->nextWaiter) != null)
		unlinkCancelledWaiters();
	if (interruptMode != 0)
		reportInterruptAfterWait(interruptMode);
	return !timedout;
}

void EAbstractQueuedSynchronizer::acquire(int arg) {
	if (!tryAcquire(arg)) {
		sp<Node> node = addWaiter(Node::EXCLUSIVE);
		if (acquireQueued(node, arg)) {
			selfInterrupt();
		}
	}
}

void EAbstractQueuedSynchronizer::acquireInterruptibly(int arg) {
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	if (!tryAcquire(arg))
		doAcquireInterruptibly(arg);
}

boolean EAbstractQueuedSynchronizer::tryAcquireNanos(int arg,
		llong nanosTimeout) {
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	return tryAcquire(arg) || doAcquireNanos(arg, nanosTimeout);
}

boolean EAbstractQueuedSynchronizer::release(int arg) {
	if (tryRelease(arg)) {
		sp<Node> h = atomic_load(&head);
		if (h != null && h->waitStatus != 0) {
			unparkSuccessor(h);
		}
		return true;
	}
	return false;
}

void EAbstractQueuedSynchronizer::acquireShared(int arg) {
	if (tryAcquireShared(arg) < 0)
		doAcquireShared(arg);
}

void EAbstractQueuedSynchronizer::acquireSharedInterruptibly(int arg) {
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	if (tryAcquireShared(arg) < 0)
		doAcquireSharedInterruptibly(arg);
}

boolean EAbstractQueuedSynchronizer::tryAcquireSharedNanos(int arg,
		llong nanosTimeout) {
	if (EThread::interrupted())
		throw EInterruptedException(__FILE__, __LINE__);
	return tryAcquireShared(arg) >= 0 || doAcquireSharedNanos(arg, nanosTimeout);
}

boolean EAbstractQueuedSynchronizer::releaseShared(int arg) {
	if (tryReleaseShared(arg)) {
		doReleaseShared();
		return true ;
	}
	return false ;
}

boolean EAbstractQueuedSynchronizer::hasQueuedThreads() {
	return atomic_load(&head) != atomic_load(&tail);
}

boolean EAbstractQueuedSynchronizer::hasContended() {
	return atomic_load(&head) != null;
}

EThread* EAbstractQueuedSynchronizer::getFirstQueuedThread() {
	// handle only fast path, else relay
	return (atomic_load(&head) == atomic_load(&tail)) ? null : fullGetFirstQueuedThread();
}

boolean EAbstractQueuedSynchronizer::isQueued(EThread* thread) {
	if (thread == null)
		throw ENullPointerException(__FILE__, __LINE__);
	for (sp<Node> p = atomic_load(&tail); p != null; p = atomic_load(&p->prev))
		if (p->thread == thread)
			return true ;
	return false ;
}

int EAbstractQueuedSynchronizer::getQueueLength() {
	int n = 0;
	for (sp<Node> p = atomic_load(&tail); p != null; p = atomic_load(&p->prev)) {
		if (p->thread != null)
			++n;
	}
	return n;
}

ECollection<EThread*>* EAbstractQueuedSynchronizer::getQueuedThreads() {
	EArrayList<EThread*>* list = new EArrayList<EThread*>(32, false);
	for (sp<Node> p = atomic_load(&tail); p != null; p = atomic_load(&p->prev)) {
		EThread* t = p->thread;
		if (t != null)
			list->add(t);
	}
	return list;
}

ECollection<EThread*>* EAbstractQueuedSynchronizer::getExclusiveQueuedThreads() {
	EArrayList<EThread*>* list = new EArrayList<EThread*>(32, false);
	for (sp<Node> p = atomic_load(&tail); p != null; p = atomic_load(&p->prev)) {
		if (!p->isShared()) {
			EThread* t = p->thread;
			if (t != null)
				list->add(t);
		}
	}
	return list;
}

ECollection<EThread*>* EAbstractQueuedSynchronizer::getSharedQueuedThreads() {
	EArrayList<EThread*>* list = new EArrayList<EThread*>(32, false);
	for (sp<Node> p = atomic_load(&tail); p != null; p = atomic_load(&p->prev)) {
		if (p->isShared()) {
			EThread* t = p->thread;
			if (t != null)
				list->add(t);
		}
	}
	return list;
}

EString EAbstractQueuedSynchronizer::toString() {
	int s = getState();
	return EString::formatOf(
			"EAbstractQueuedSynchronizer[State = %d, %s empty queue]", s,
			hasQueuedThreads() ? "non" : "");
}

int EAbstractQueuedSynchronizer::getState() {
	return state;
}

void EAbstractQueuedSynchronizer::setState(int newState) {
	state = newState;
}

boolean EAbstractQueuedSynchronizer::compareAndSetState(int expect,
		int update) {
	// See below for intrinsics setup to support this
	//return unsafe.compareAndSwapInt(this, stateOffset, expect, update);
	return EAtomic::cmpxchg32(update, &state, expect) == expect;
}

boolean EAbstractQueuedSynchronizer::acquireQueued(sp<Node>& node, int arg) {
	boolean failed = true;
	try {
		boolean interrupted = false;
		for (;;) {
			sp<Node> p = node->predecessor();
			sp<Node> h = atomic_load(&head);
			if (h == p && tryAcquire(arg)) {
				setHead(node);
				atomic_store(&p->next, null); // help GC
				failed = false;
				return interrupted;
			}
			if (shouldParkAfterFailedAcquire(p, node)
					&& parkAndCheckInterrupt())
				interrupted = true;
		}
	} catch (...) {
		finally {
			if (failed)
				cancelAcquire(node);
		}
		throw; //!
	}
}

boolean EAbstractQueuedSynchronizer::tryAcquire(int arg) {
	throw EUnsupportedOperationException(__FILE__, __LINE__);
}

boolean EAbstractQueuedSynchronizer::tryRelease(int arg) {
	throw EUnsupportedOperationException(__FILE__, __LINE__);
}

int EAbstractQueuedSynchronizer::tryAcquireShared(int arg) {
	throw EUnsupportedOperationException(__FILE__, __LINE__);
}

boolean EAbstractQueuedSynchronizer::tryReleaseShared(int arg) {
	throw EUnsupportedOperationException(__FILE__, __LINE__);
}

boolean EAbstractQueuedSynchronizer::isHeldExclusively() {
	throw EUnsupportedOperationException(__FILE__, __LINE__);
}

boolean EAbstractQueuedSynchronizer::apparentlyFirstQueuedIsExclusive() {
	sp<Node> s;
	sp<Node> h = atomic_load(&head);
	return h != null && (s = atomic_load(&h->next)) != null && !s->isShared()
			&& s->thread != null;
}

boolean EAbstractQueuedSynchronizer::hasQueuedPredecessors() {
	// The correctness of this depends on head being initialized
	// before tail and on head.next being accurate if the current
	// thread is first in queue.
	sp<Node> t = atomic_load(&tail); // Read fields in reverse initialization order
	sp<Node> h = atomic_load(&head);
	sp<Node> s;
	return h != t &&
		((s = atomic_load(&h->next)) == null || s->thread != EThread::currentThread());
}

boolean EAbstractQueuedSynchronizer::isOnSyncQueue(sp<Node>& node) {
	if (node->waitStatus == Node::CONDITION || atomic_load(&node->prev) == null)
		return false;
	if (atomic_load(&node->next) != null) // If has successor, it must be on queue
		return true;
	/*
	 * node.prev can be non-null, but not yet on queue because
	 * the CAS to place it on queue can fail. So we have to
	 * traverse from tail to make sure it actually made it.  It
	 * will always be near the tail in calls to this method, and
	 * unless the CAS failed (which is unlikely), it will be
	 * there, so we hardly ever traverse much.
	 */
	return findNodeFromTail(node);
}

boolean EAbstractQueuedSynchronizer::transferForSignal(sp<Node>& node) {
	/*
	 * If cannot change waitStatus, the node has been cancelled.
	 */
	if (!compareAndSetWaitStatus(node, Node::CONDITION, 0))
		return false;

	/*
	 * Splice onto queue and try to set waitStatus of predecessor to
	 * indicate that thread is (probably) waiting. If cancelled or
	 * attempt to set waitStatus fails, wake up to resync (in which
	 * case the waitStatus can be transiently and harmlessly wrong).
	 */
	sp<Node> p = enq(node);
	int ws = p->waitStatus;
	if (ws > 0 || !compareAndSetWaitStatus(p, ws, Node::SIGNAL)) {
		ELockSupport::unpark(node->thread);
	}
	return true;
}

boolean EAbstractQueuedSynchronizer::transferAfterCancelledWait(sp<Node>& node) {
	if (compareAndSetWaitStatus(node, Node::CONDITION, 0)) {
		enq(node);
		return true;
	}
	/*
	 * If we lost out to a signal(), then we can't proceed
	 * until it finishes its enq().  Cancelling during an
	 * incomplete transfer is both rare and transient, so just
	 * spin.
	 */
	while (!isOnSyncQueue(node))
		EThread::yield();
	return false;
}

int EAbstractQueuedSynchronizer::fullyRelease(sp<Node>& node) {
	boolean failed = true;
	try {
		int savedState = getState();

		//@see: release(arg)
		boolean r = false;
		if (tryRelease(savedState)) {
			sp<Node> h = atomic_load(&head);
			if (h != null) { // had fixed bug #2014112801 ?
				unparkSuccessor(h);
			}
			r = true;
		}
		if (r) {
			failed = false;
			return savedState;
		} else {
			throw EIllegalStateException(__FILE__, __LINE__);
		}
	} catch (...) {
		finally {
			if (failed)
				node->waitStatus = Node::CANCELLED;
		}
		throw; //!
	}
}

sp<EAbstractQueuedSynchronizer::Node> EAbstractQueuedSynchronizer::enq(sp<Node>& node) {
	for (;;) {
		sp<Node> t = atomic_load(&tail);
		if (t == null) { // Must initialize
			sp<Node> n(new Node());
			if (compareAndSetHead(n))
				atomic_store(&tail, n);
		} else {
			atomic_store(&node->prev, t);
			if (compareAndSetTail(t, node)) {
				atomic_store(&t->next, node);
				return t;
			}
		}
	}
}

sp<EAbstractQueuedSynchronizer::Node> EAbstractQueuedSynchronizer::addWaiter(sp<Node>& mode) {
	sp<Node> node = new Node(EThread::currentThread(), mode);
	// Try the fast path of enq; backup to full enq on failure
	sp<Node> pred = atomic_load(&tail);
	if (pred != null) {
		atomic_store(&node->prev, pred);
		if (compareAndSetTail(pred, node)) {
			atomic_store(&pred->next, node);
			return node;
		}
	}
	enq(node);
	return node;
}

void EAbstractQueuedSynchronizer::setHead(sp<Node>& node) {
	atomic_store(&head, sp<Node>(node));
	node->thread = null;
	atomic_store(&node->prev, null);
}

void EAbstractQueuedSynchronizer::unparkSuccessor(sp<Node>& node) {

	/*
	 * If status is negative (i.e., possibly needing signal) try
	 * to clear in anticipation of signalling.  It is OK if this
	 * fails or if status is changed by waiting thread.
	 */
	int ws = node->waitStatus;
	if (ws < 0) {
		compareAndSetWaitStatus(node, ws, 0);
	}

	/*
	 * Thread to unpark is held in successor, which is normally
	 * just the next node.  But if cancelled or apparently null,
	 * traverse backwards from tail to find the actual
	 * non-cancelled successor.
	 */
	sp<Node> s = atomic_load(&node->next);
	if (s == null || s->waitStatus > 0) {
		s = null;
		for (sp<Node> t = atomic_load(&tail); t != null && t != node; t = atomic_load(&t->prev))
			if (t->waitStatus <= 0)
				s = t;
	}
	if (s != null)
		ELockSupport::unpark(s->thread);
}

void EAbstractQueuedSynchronizer::doReleaseShared() {
	/*
	 * Ensure that a release propagates, even if there are other
	 * in-progress acquires/releases.  This proceeds in the usual
	 * way of trying to unparkSuccessor of head if it needs
	 * signal. But if it does not, status is set to PROPAGATE to
	 * ensure that upon release, propagation continues.
	 * Additionally, we must loop in case a new node is added
	 * while we are doing this. Also, unlike other uses of
	 * unparkSuccessor, we need to know if CAS to reset status
	 * fails, if so rechecking.
	 */
	for (;;) {
		sp<Node> h = atomic_load(&head);
		if (h != null && h != atomic_load(&tail)) {
			int ws = h->waitStatus;
			if (ws == Node::SIGNAL) {
				if (!compareAndSetWaitStatus(h, Node::SIGNAL, 0))
					continue;            // loop to recheck cases
				unparkSuccessor(h);
			}
			else if (ws == 0 &&
					 !compareAndSetWaitStatus(h, 0, Node::PROPAGATE))
				continue;                // loop on failed CAS
		}
		if (h == atomic_load(&head))                   // loop if head changed
			break;
	}
}

void EAbstractQueuedSynchronizer::setHeadAndPropagate(sp<Node>& node,
		int propagate) {
	sp<Node> h = atomic_load(&head); // Record old head for check below
	setHead(node);
	/*
	 * Try to signal next queued node if:
	 *   Propagation was indicated by caller,
	 *     or was recorded (as h.waitStatus either before
	 *     or after setHead) by a previous operation
	 *     (note: this uses sign-check of waitStatus because
	 *      PROPAGATE status may transition to SIGNAL.)
	 * and
	 *   The next node is waiting in shared mode,
	 *     or we don't know, because it appears null
	 *
	 * The conservatism in both of these checks may cause
	 * unnecessary wake-ups, but only when there are multiple
	 * racing acquires/releases, so most need signals now or soon
	 * anyway.
	 */
	if (propagate > 0 || h == null || h->waitStatus < 0 ||
            (h = atomic_load(&head)) == null || h->waitStatus < 0) {
		//@see: openjdk-8/share/classes/java/util/concurrent/locks/AbstractQueuedSynchronizer.java line 728
		//@see: http://bugs.java.com/view_bug.do?bug_id=7011859
		sp<Node> s = atomic_load(&node->next);
		if (s == null || s->isShared())
			doReleaseShared();
	}
}

void EAbstractQueuedSynchronizer::cancelAcquire(sp<Node>& node) {
	// Ignore if node doesn't exist
	if (node == null)
		return;

	node->thread = null;

	// Skip cancelled predecessors
	sp<Node> pred = atomic_load(&node->prev);
	while (pred->waitStatus > 0) {
		atomic_store(&node->prev, pred = atomic_load(&pred->prev));
	}

	// predNext is the apparent node to unsplice. CASes below will
	// fail if not, in which case, we lost race vs another cancel
	// or signal, so no further action is necessary.
	sp<Node> predNext = atomic_load(&pred->next);

	// Can use unconditional write instead of CAS here.
	// After this atomic step, other Nodes can skip past us.
	// Before, we are free of interference from other threads.
	node->waitStatus = Node::CANCELLED;

	// If we are the tail, remove ourselves.
	if (node == atomic_load(&tail) && compareAndSetTail(node, pred)) {
		compareAndSetNext(pred, predNext, null);
	} else {
		// If successor needs signal, try to set pred's next-link
		// so it will get one. Otherwise wake it up to propagate.
		int ws;
		sp<Node> h = atomic_load(&head);
		if (h != pred &&
			((ws = pred->waitStatus) == Node::SIGNAL ||
			 (ws <= 0 && compareAndSetWaitStatus(pred, ws, Node::SIGNAL))) &&
			pred->thread != null) {
			sp<Node> next = atomic_load(&node->next);
			if (next != null && next->waitStatus <= 0)
				compareAndSetNext(pred, predNext, next);
		} else {
			unparkSuccessor(node);
		}

		// @see: node->next = node; // help GC
		/* @see: http://blog.csdn.net/aesop_wubo/article/details/7570678
		 *
		 * The current node of the next pointer to their own, one for recycling,
		 * the two is to make the isOnSyncQueue method is simple.
		 */
		atomic_store(&node->next, null); // help GC
	}
}

boolean EAbstractQueuedSynchronizer::shouldParkAfterFailedAcquire(sp<Node>& pred,
		sp<Node>& node) {
	int ws = pred->waitStatus;
	if (ws == Node::SIGNAL)
		/*
		 * This node has already set status asking a release
		 * to signal it, so it can safely park.
		 */
		return true;
	if (ws > 0) {
		/*
		 * Predecessor was cancelled. Skip over predecessors and
		 * indicate retry.
		 */
		do {
			atomic_store(&node->prev, (pred = atomic_load(&pred->prev)));
		} while (pred->waitStatus > 0);
		atomic_store(&pred->next, node);
	} else {
		/*
		 * waitStatus must be 0 or PROPAGATE.  Indicate that we
		 * need a signal, but don't park yet.  Caller will need to
		 * retry to make sure it cannot acquire before parking.
		 */
		compareAndSetWaitStatus(pred, ws, Node::SIGNAL);
	}
	return false;
}

void EAbstractQueuedSynchronizer::selfInterrupt() {
	EThread::currentThread()->interrupt();
}

boolean EAbstractQueuedSynchronizer::parkAndCheckInterrupt() {
	ELockSupport::park(/*this*/);
	return EThread::interrupted();
}

void EAbstractQueuedSynchronizer::doAcquireInterruptibly(int arg) {
	sp<Node> node = addWaiter(Node::EXCLUSIVE);
	boolean failed = true;
	try {
		for (;;) {
			sp<Node> p = node->predecessor();
			sp<Node> h = atomic_load(&head);
			if (h == p && tryAcquire(arg)) {
				setHead(node);
				atomic_store(&p->next, null); // help GC
				failed = false;
				return;
			}
			if (shouldParkAfterFailedAcquire(p, node) &&
				parkAndCheckInterrupt())
				throw EInterruptedException(__FILE__, __LINE__);
		}
	} catch(...) {
		finally {
			if (failed)
				cancelAcquire(node);
		}
		throw; //!
	}
}

boolean EAbstractQueuedSynchronizer::doAcquireNanos(int arg,
		llong nanosTimeout) {
	if (nanosTimeout <= 0L)
		return false;
	llong deadline = ESystem::nanoTime() + nanosTimeout; //@see: openjdk-8/share/classes/java/util/concurrent/locks/AbstractQueuedSynchronizer.java line 917
	sp<Node> node = addWaiter(Node::EXCLUSIVE);
	boolean failed = true;
	boolean result = false;
	try {
		for (;;) {
			sp<Node> p = node->predecessor();
			sp<Node> h = atomic_load(&head);
			if (h == p && tryAcquire(arg)) {
				setHead(node);
				atomic_store(&p->next, null); // help GC
				failed = false;
				result = true;
				goto FINALLY;
			}
			nanosTimeout = deadline - ESystem::nanoTime();
			if (nanosTimeout <= 0L) {
				result = false;
				goto FINALLY;
			}
			if (shouldParkAfterFailedAcquire(p, node) &&
				nanosTimeout > spinForTimeoutThreshold)
				ELockSupport::parkNanos(/*this, */nanosTimeout);
			if (EThread::interrupted())
				throw EInterruptedException(__FILE__, __LINE__);
		}
	} catch(...) {
		finally {
			if (failed)
				cancelAcquire(node);
		}
		throw; //!
	}
	FINALLY:
	finally {
		if (failed)
			cancelAcquire(node);
	}
	return result;
}

void EAbstractQueuedSynchronizer::doAcquireShared(int arg) {
	sp<Node> node = addWaiter(Node::SHARED);
	boolean failed = true;
	try {
		boolean interrupted = false;
		for (;;) {
			sp<Node> p = node->predecessor();
			sp<Node> h = atomic_load(&head);
			if (h == p) {
				int r = tryAcquireShared(arg);
				if (r >= 0) {
					setHeadAndPropagate(node, r);
					atomic_store(&p->next, null); // help GC
					if (interrupted)
						selfInterrupt();
					failed = false;
					return;
				}
			}
			if (shouldParkAfterFailedAcquire(p, node) &&
				parkAndCheckInterrupt())
				interrupted = true;
		}
	} catch(...) {
		finally {
			if (failed)
				cancelAcquire(node);
		}
		throw; //!
	}
}

void EAbstractQueuedSynchronizer::doAcquireSharedInterruptibly(int arg) {
	sp<Node> node = addWaiter(Node::SHARED);
	boolean failed = true;
	try {
		for (;;) {
			sp<Node> p = node->predecessor();
			sp<Node> h = atomic_load(&head);
			if (h == p) {
				int r = tryAcquireShared(arg);
				if (r >= 0) {
					setHeadAndPropagate(node, r);
					atomic_store(&p->next, null); // help GC
					failed = false;
					return;
				}
			}
			if (shouldParkAfterFailedAcquire(p, node) &&
				parkAndCheckInterrupt())
				throw EInterruptedException(__FILE__, __LINE__);
		}
	} catch(...) {
		finally {
			if (failed)
				cancelAcquire(node);
		}
		throw; //!
	}
}

boolean EAbstractQueuedSynchronizer::doAcquireSharedNanos(int arg,
		llong nanosTimeout) {
	if (nanosTimeout <= 0L)
		return false;
	llong deadline = ESystem::nanoTime() + nanosTimeout;
	sp<Node> node = addWaiter(Node::SHARED);
	boolean failed = true;
	boolean result = false;
	try {
		for (;;) {
			sp<Node> p = node->predecessor();
			sp<Node> h = atomic_load(&head);
			if (h == p) {
				int r = tryAcquireShared(arg);
				if (r >= 0) {
					setHeadAndPropagate(node, r);
					atomic_store(&p->next, null); // help GC
					failed = false;
					result = true;
					goto FINALLY;
				}
			}
			nanosTimeout = deadline - ESystem::nanoTime();
			if (nanosTimeout <= 0L) {
				result = false;
				goto FINALLY;
			}
			if (shouldParkAfterFailedAcquire(p, node) &&
				nanosTimeout > spinForTimeoutThreshold)
				ELockSupport::parkNanos(/*this, */nanosTimeout);
			if (EThread::interrupted())
				throw EInterruptedException(__FILE__, __LINE__);
		}
	} catch(...) {
		finally {
			if (failed)
				cancelAcquire(node);
		}
		throw; //!
	}
	FINALLY:
	finally {
		if (failed)
			cancelAcquire(node);
	}
	return result;
}

EThread* EAbstractQueuedSynchronizer::fullGetFirstQueuedThread() {
	/*
	 * The first node is normally head.next. Try to get its
	 * thread field, ensuring consistent reads: If thread
	 * field is nulled out or s.prev is no longer head, then
	 * some other thread(s) concurrently performed setHead in
	 * between some of our reads. We try this twice before
	 * resorting to traversal.
	 */
	sp<Node> h, s;
	EThread* st;
	if (((h = atomic_load(&head)) != null && (s = atomic_load(&h->next)) != null &&
		atomic_load(&head) == atomic_load(&s->prev) && (st = s->thread) != null) ||
		((h = atomic_load(&head)) != null && (s = atomic_load(&h->next)) != null &&
		atomic_load(&head) == atomic_load(&s->prev) && (st = s->thread) != null))
		return st;

	/*
	 * Head's next field might not have been set yet, or may have
	 * been unset after setHead. So we must check to see if tail
	 * is actually first node. If not, we continue on, safely
	 * traversing from tail back to head to find first,
	 * guaranteeing termination.
	 */

	sp<Node> t = atomic_load(&tail);
	EThread* firstThread = null;
	while (t != null && atomic_load(&head) != t) {
		EThread* tt = t->thread;
		if (tt != null)
			firstThread = tt;
		t = atomic_load(&t->prev);
	}
	return firstThread;
}

boolean EAbstractQueuedSynchronizer::findNodeFromTail(sp<Node>& node) {
	sp<Node> t = atomic_load(&tail);
	for (;;) {
		if (t == node)
			return true;
		if (t == null)
			return false;
		t = atomic_load(&t->prev);
	}
}

boolean EAbstractQueuedSynchronizer::compareAndSetHead(sp<Node>& update) {
	//return unsafe.compareAndSwapObject(this, headOffset, null, update);
	sp<Node> np; //null ptr
	return atomic_compare_exchange(&head, &np, update);
}

boolean EAbstractQueuedSynchronizer::compareAndSetTail(sp<Node>& expect,
		sp<Node>& update) {
	//return unsafe.compareAndSwapObject(this, tailOffset, expect, update);
	return atomic_compare_exchange(&tail, &expect, update);
}

boolean EAbstractQueuedSynchronizer::compareAndSetWaitStatus(sp<Node>& node,
		int expect, int update) {
	//return unsafe.compareAndSwapInt(node, waitStatusOffset, expect, update);
	return EAtomic::cmpxchg32(update, &node->waitStatus, expect) == expect;
}

boolean efc::EAbstractQueuedSynchronizer::owns(ConditionObject* condition) {
	if (condition == null)
		throw ENullPointerException(__FILE__, __LINE__);
	return condition->isOwnedBy(this);
}

boolean efc::EAbstractQueuedSynchronizer::hasWaiters(
		ConditionObject* condition) {
	if (!owns(condition))
		throw EIllegalArgumentException(__FILE__, __LINE__, "Not owner");
	return condition->hasWaiters();
}

int efc::EAbstractQueuedSynchronizer::getWaitQueueLength(
		ConditionObject* condition) {
	if (!owns(condition))
		throw EIllegalArgumentException(__FILE__, __LINE__, "Not owner");
	return condition->getWaitQueueLength();
}

ECollection<EThread*>* efc::EAbstractQueuedSynchronizer::getWaitingThreads(
		ConditionObject* condition) {
	if (!owns(condition))
		throw EIllegalArgumentException(__FILE__, __LINE__, "Not owner");
	return condition->getWaitingThreads();
}

boolean EAbstractQueuedSynchronizer::compareAndSetNext(sp<Node>& node, sp<Node>& expect,
		sp<Node> update) {
	//return unsafe.compareAndSwapObject(node, nextOffset, expect, update);
	return atomic_compare_exchange(&node->next, &expect, update);
}

boolean EAbstractQueuedSynchronizer::ConditionObject::isOwnedBy(
		EAbstractQueuedSynchronizer* sync) {
	return sync == aqs;
}

boolean EAbstractQueuedSynchronizer::ConditionObject::hasWaiters() {
	if (!aqs->isHeldExclusively())
		throw EIllegalStateException(__FILE__, __LINE__);
	for (sp<Node> w = atomic_load(&firstWaiter); w != null; w = atomic_load(&w->nextWaiter)) {
		if (w->waitStatus == Node::CONDITION)
			return true;
	}
	return false;
}

int EAbstractQueuedSynchronizer::ConditionObject::getWaitQueueLength() {
	if (!aqs->isHeldExclusively())
		throw EIllegalStateException(__FILE__, __LINE__);
	int n = 0;
	for (sp<Node> w = atomic_load(&firstWaiter); w != null; w = atomic_load(&w->nextWaiter)) {
		if (w->waitStatus == Node::CONDITION)
			++n;
	}
	return n;
}

ECollection<EThread*>* EAbstractQueuedSynchronizer::ConditionObject::getWaitingThreads() {
	if (!aqs->isHeldExclusively())
		throw EIllegalStateException(__FILE__, __LINE__);
	EArrayList<EThread*>* list = new EArrayList<EThread*>(false);
	for (sp<Node> w = atomic_load(&firstWaiter); w != null; w = atomic_load(&w->nextWaiter)) {
		if (w->waitStatus == Node::CONDITION) {
			EThread* t = w->thread;
			if (t != null)
				list->add(t);
		}
	}
	return list;
}

} /* namespace efc */
