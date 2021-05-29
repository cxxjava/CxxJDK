/*
 * EThread.cpp
 *
 *  Created on: 2013-3-19
 *      Author: cxxjava@163.com
 */

#include "EThread.hh"
#include "ERuntime.hh"
#include "EOS.hh"
#include "ESystem.hh"
#include "EPark.hh"
#include "ETimeUnit.hh"
#include "EThreadGroup.hh"
#include "ESimpleLock.hh"
#include "EInterruptible.hh"
#include "EThreadService.hh"
#include "ENullPointerException.hh"
#include "EIllegalThreadStateException.hh"
#include "../inc/concurrent/EOrderAccess.hh"

namespace efc {

//=============================================================================

//@see: https://github.com/facebook/rocksdb/blob/master/util/thread_local.cc
//@see: https://github.com/couchbase/gperftools/blob/master/src/windows/port.cc
//@see: http://www.nynaeve.net/?p=183
//@see: http://lallouslab.net/2017/05/30/using-cc-tls-callbacks-in-visual-studio-with-your-32-or-64bits-programs/

// Cleanup function that will be called for a stored thread local
// pointer (if not NULL) when one of the following happens:
// (1) a thread terminates
// (2) a ThreadLocalPtr is destroyed
//
// Warning: this function is called while holding a global mutex. The same mutex
// is used (at least in some cases) by most methods of ThreadLocalPtr, and it's
// shared across all instances of ThreadLocalPtr. Thereforere extra care
// is needed to avoid deadlocks. In particular, the handler shouldn't lock any
// mutexes and shouldn't call any methods of any ThreadLocalPtr instances,
// unless you know what you're doing.
typedef void (*UnrefHandler)(void* ptr);

class ThreadLocalHandler {
public:
	virtual ~ThreadLocalHandler();
	explicit ThreadLocalHandler(UnrefHandler handler);

	void* get();
	void set(void* value);

private:
	long thread_key;
};

#ifdef WIN32
// Windows doesn't support a per-thread destructor with its
// TLS primitives.  So, we build it manually by inserting a
// function to be called on each thread's exit.
// See http://www.codeproject.com/Articles/8113/Thread-Local-Storage-The-C-Way
// and http://www.nynaeve.net/?p=183
//
// really we do this to have clear conscience since using TLS with thread-pools
// is iffy
// although OK within a request. But otherwise, threads have no identity in its
// modern use.

// This runs on windows only called from the System Loader

// Windows cleanup routine is invoked from a System Loader with a different
// signature so we can not directly hookup the original OnThreadExit which is
// private member
// so we make StaticMeta class share with the us the address of the function so
// we can invoke it.
namespace wintlscleanup {

// This is set to OnThreadExit in StaticMeta singleton constructor
static UnrefHandler thread_local_inclass_routine = null;
static DWORD thread_local_key = DWORD (-1);

// Static callback function to call with each thread termination.
void NTAPI WinOnThreadExit(PVOID module, DWORD reason, PVOID reserved) {
	// We decided to punt on PROCESS_EXIT
	if (DLL_THREAD_DETACH == reason) {
		if (thread_local_key != DWORD(-1) && thread_local_inclass_routine != nullptr) {
			void* tls = TlsGetValue(thread_local_key);
			if (tls != nullptr) {
				thread_local_inclass_routine(tls);
			}
		}
	}
}

}  // !wintlscleanup

// extern "C" suppresses C++ name mangling so we know the symbol name for the
// linker /INCLUDE:symbol pragma above.
extern "C" {
#ifdef _MSC_VER
// The linker must not discard thread_callback_on_exit.  (We force a reference
// to this variable with a linker /include:symbol pragma to ensure that.) If
// this variable is discarded, the OnThreadExit function will never be called.
#ifdef _WIN64

// .CRT section is merged with .rdata on x64 so it must be constant data.
#pragma const_seg(".CRT$XLB")
// When defining a const variable, it must have external linkage to be sure the
// linker doesn't discard it.
extern const PIMAGE_TLS_CALLBACK p_thread_callback_on_exit;
const PIMAGE_TLS_CALLBACK p_thread_callback_on_exit =
	wintlscleanup::WinOnThreadExit;
// Reset the default section.
#pragma const_seg()

#pragma comment(linker, "/include:_tls_used")
#pragma comment(linker, "/include:p_thread_callback_on_exit")

#else  // _WIN64

#pragma data_seg(".CRT$XLB")
PIMAGE_TLS_CALLBACK p_thread_callback_on_exit = wintlscleanup::WinOnThreadExit;
// Reset the default section.
#pragma data_seg()

#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_p_thread_callback_on_exit")

#endif  // _WIN64

#else //!
// https://github.com/couchbase/gperftools/blob/master/src/windows/port.cc
BOOL WINAPI DllMain(HINSTANCE h, DWORD dwReason, PVOID pv) {
	if (dwReason == DLL_THREAD_DETACH)
	wintlscleanup::WinOnThreadExit(h, dwReason, pv);
	return TRUE;
}
#endif
}  // extern "C"

ThreadLocalHandler::ThreadLocalHandler(UnrefHandler handler) {
	thread_key = TlsAlloc();
	wintlscleanup::thread_local_inclass_routine = handler;
	wintlscleanup::thread_local_key = thread_key;
}

ThreadLocalHandler::~ThreadLocalHandler() {
	TlsFree(thread_key);
}

void* ThreadLocalHandler::get() {
	return TlsGetValue(thread_key);
}

void ThreadLocalHandler::set(void* value) {
	TlsSetValue(thread_key, value);
}

#elif defined(__solaris__)

ThreadLocalHandler::ThreadLocalHandler(UnrefHandler handler) {
	thread_key_t key;
	if (thr_keycreate(&key, handler)) {
		throw EException(__FILE__, __LINE__, "pthread_key_create error");
	}
	thread_key = (long)key;
}

ThreadLocalHandler::~ThreadLocalHandler() {
	/* no-op */
}

void* ThreadLocalHandler::get() {
	void* r = NULL;
	thr_getspecific((thread_key_t)thread_key, &r);
	return r;
}

void ThreadLocalHandler::set(void* value) {
	thr_setspecific((thread_key_t)thread_key, value) ;
}

#else //linux||bsd

ThreadLocalHandler::ThreadLocalHandler(UnrefHandler handler) {
	pthread_key_t key;
	if (pthread_key_create(&key, handler)) {
		throw EException(__FILE__, __LINE__, "pthread_key_create error");
	}
	thread_key = (long)key;
}

ThreadLocalHandler::~ThreadLocalHandler() {
	pthread_key_delete((pthread_key_t)thread_key);
}

void* ThreadLocalHandler::get() {
	return pthread_getspecific((pthread_key_t)thread_key);
}

void ThreadLocalHandler::set(void* value) {
	pthread_setspecific((pthread_key_t)thread_key, value) ;
}

#endif //!WIN32

//=============================================================================

#ifdef WIN32
static int java_to_os_priority[EThread::MAX_PRIORITY + 1] = {
  THREAD_PRIORITY_IDLE,                         // 0  Entry should never be used
  THREAD_PRIORITY_LOWEST,                       // 1  MinPriority
  THREAD_PRIORITY_LOWEST,                       // 2
  THREAD_PRIORITY_BELOW_NORMAL,                 // 3
  THREAD_PRIORITY_BELOW_NORMAL,                 // 4
  THREAD_PRIORITY_NORMAL,                       // 5  NormPriority
  THREAD_PRIORITY_NORMAL,                       // 6
  THREAD_PRIORITY_ABOVE_NORMAL,                 // 7
  THREAD_PRIORITY_ABOVE_NORMAL,                 // 8
  THREAD_PRIORITY_HIGHEST,                      // 9  NearMaxPriority
  THREAD_PRIORITY_TIME_CRITICAL                 // 10 MaxPriority
};
#else //!
static int java_to_os_priority[EThread::MAX_PRIORITY + 1] = {
  19,              // 0 Entry should never be used

   4,              // 1 MinPriority
   3,              // 2
   2,              // 3

   1,              // 4
   0,              // 5 NormPriority
  -1,              // 6

  -2,              // 7
  -3,              // 8
  -4,              // 9 NearMaxPriority

  -5               // 10 MaxPriority
};
#endif

static void thread_key_unref_handler(void* arg) {
	EThread* thread = static_cast<EThread*>(arg);
//	printf("unref thread=%p\n", thread);

	delete thread;
}

class MainThreadLocal : public ThreadLocalHandler {
public:
	MainThreadLocal(UnrefHandler handler) : ThreadLocalHandler(handler) {
		EThread* thread = new EThread("main", 0); //the main thread!
		set(thread);
	}
	virtual ~MainThreadLocal() {
		delete (EThread*)get();
	}
};

//=============================================================================

EThread::UncaughtExceptionHandler* volatile EThread::defaultUncaughtExceptionHandler = null;
MainThreadLocal* EThread::threadLocal = null;
volatile int EThread::threadsCount = 0;
volatile int EThread::c_threadsCount = 0;
volatile int EThread::defaultStackSize = -1;

DEFINE_STATIC_INITZZ_BEGIN(EThread)
EOS::_initzz_();
ERuntime::_initzz_();
ETimeUnit::_initzz_();
EParker::_initzz_();
EParkEvent::_initzz_();
threadLocal = new MainThreadLocal(thread_key_unref_handler);
DEFINE_STATIC_INITZZ_END

//=============================================================================

#ifdef WIN32
extern "C" {
/* Chosen for us by eso_initialize */
extern DWORD g_tls_thread;
}
#endif

/**
 * Dispatch an uncaught exception to the handler. This method is
 * intended to be called only by the JVM.
 */
static void dispatchUncaughtException(EThread* thread, EThrowable* e) {
	EThread::UncaughtExceptionHandler* ueh = thread->getUncaughtExceptionHandler();
	if (ueh) ueh->uncaughtException(thread, e);
}

static void *dummy_worker(void *opaque)
{
    es_thread_t *thread = (es_thread_t*)opaque;
#ifdef WIN32
	TlsSetValue(g_tls_thread, thread->td);
#endif
    return thread->func(thread);
}

boolean EThread::thread_create(es_threadattr_t *attr, es_thread_func_t *func, EThread* thread)
{
#ifdef WIN32

	es_thread_t *t;
	HANDLE handle;
	unsigned temp;

	t = (es_thread_t*)eso_calloc(sizeof(es_thread_t));
	if (!t) {
		return false;
	}
	t->data = thread;
	t->func = func;

	thread->thread = t; //!!!
	if ((handle = CreateThread(NULL,
							attr && attr->stacksize > 0 ? attr->stacksize : 0,
							(LPTHREAD_START_ROUTINE)dummy_worker,
							t, 0, (LPDWORD)&temp)) == 0) {
		return false;
	}

	if (attr && attr->detach) {
		CloseHandle(handle);
	}
	else
		t->td = handle;

	return true;

#else //!
	es_thread_t *t;
	pthread_attr_t *temp;

	t = (es_thread_t*) eso_calloc(sizeof(es_thread_t));
	if (!t) {
		return false;
	}
	t->data = thread;
	t->func = func;

	if (attr)
		temp = &attr->attr;
	else
		temp = NULL;

	thread->thread = t; //!!!
	if (::pthread_create(&t->td, temp, dummy_worker, t) == 0) {
		return true;
	} else {
		eso_free(t);
		return false;
	}

#endif
}

void* EThread::thread_dummy_worker(es_thread_t* handle)
{
	EThread *thread = (EThread*)handle->data;

	//set current thread object.
	threadLocal->set(thread);
	thread->tid = eso_os_thread_current_id();

	try {
		try {
			thread->run();
		} catch (EThrowable& e) {
			dispatchUncaughtException(thread, &e);
		}

		// save callback
		CleanupCallback* callback = thread->cleanupCallback;
		void* arg = thread->cleanupArg;

		//@see: openjdk-6/hotspot/src/share/vm/runtime/thread.cpp line 1529
		SYNCBLOCK(thread->parkLock) {
			// It is of profound importance that we set the stillborn bit and reset the thread object,
			// before we do the notify. Since, changing these two variable will make JVM_IsAlive return
			// false. So in case another thread is doing a join on this thread , it will detect that the thread
			// is dead when it gets notified.
			//java_lang_Thread::set_stillborn(threadObj());

			// Thread is exiting. So set thread_status field in  java.lang.Thread class to TERMINATED.
			thread->threadStatus = TERMINATED;
			thread->parkCond->signalAll();
        }}

		// callback
		if (callback) {
			callback(arg);
		}

		// unlink self at last
		thread->self = null;

	} catch (...) {
		//...
	}

	//clear current thread object.
	threadLocal->set(null);

	return NULL;
}

//=============================================================================

EThread::EThread(const char* name, const int c_tid) {
	init(null, null, name, 0L, 0L, true);
	this->c_tid = c_tid;
}

EThread::~EThread() {
	if (group != null) {
		group->remove(this);
		group = null;
	}

	eso_atomic_sub_and_fetch32(&threadsCount, 1);

	// It's possible we can encounter a null _ParkEvent, etc., in stillborn threads.
	// We NULL out the fields for good hygiene.
	EParkEvent::Release (_ParkEvent)   ; _ParkEvent   = NULL ;
	EParkEvent::Release (_SleepEvent)  ; _SleepEvent  = NULL ;
	EParkEvent::Release (_MutexEvent)  ; _MutexEvent  = NULL ;
	EParkEvent::Release (_MuxEvent)    ; _MuxEvent    = NULL ;

	// JSR166 -- return the parker to the free list
	EParker::Release (parker); parker = NULL;

	/* Aggressively null out all reference fields: see bug 4006245 */
	target = null;
	//inheritedAccessControlContext = null;
	blocker = null;
	uncaughtExceptionHandler = null;

	delete blockerLock;
	delete parkCond;
	delete parkLock;
	delete syncLock;

	/* Speed the release of some of these resources */
	/**
	 * FIXME: when thread is detached, freed localValues maybe make AQS crashed at unparkSuccessor!!!
	 * @see test_thread4().
	 */
	delete this->localValues;
	this->localValues = null;
	delete this->inheritableValues;
	this->inheritableValues = null;

	if (self == null && thread) {
		eso_thread_join(NULL, thread);
	}

	eso_threadattr_destroy(&threadAttr);
	eso_thread_destroy(&thread);
}

EThread::EThread() {
	init(null, null, EString::formatOf("Thread-0x%x", this).c_str(), false, 0L, 0L);
}

EThread::EThread(sp<ERunnable> target) {
	init(null, target, EString::formatOf("Thread-0x%x", this).c_str(), 0L, 0L);
}

EThread::EThread(sp<EThreadGroup> group, sp<ERunnable> target) {
	init(group, target, EString::formatOf("Thread-0x%x", this).c_str(), 0L, 0L);
}

EThread::EThread(const char* name) {
	init(null, null, name, false, 0L, 0L);
}

EThread::EThread(sp<EThreadGroup> group, const char* name) {
	init(group, null, name, false, 0L, 0L);
}

EThread::EThread(sp<ERunnable> target, const char* name) {
	init(null, target, name, 0L, 0L);
}

EThread::EThread(sp<EThreadGroup> group, sp<ERunnable> target, const char* name,
		unsigned long stacksize, unsigned long guardsize) {
	init(group, target, name, stacksize, guardsize);
}

void EThread::setDaemon(sp<EThread> thread, boolean on) {
	if (thread == null) {
		throw ENullPointerException(__FILE__, __LINE__);
	}
	thread->checkAccess();
	if (thread->isAlive()) {
		throw EIllegalThreadStateException(__FILE__, __LINE__);
	}
	thread->self = on ? thread : null;
}

boolean EThread::isDaemon() {
	return (self != null);
}

void EThread::setDefaultStackSize(int size) {
	defaultStackSize = size;
}

EThread* EThread::injectExitCallback(CleanupCallback* callback, void* arg) {
	cleanupCallback = callback;
	cleanupArg = arg;
	return this;
}

void EThread::start() {
	SYNCBLOCK(parkLock) {
		/**
		 * This method is not invoked for the main method thread or "system"
		 * group threads created/set up by the VM. Any new functionality added
		 * to this method in the future may have to also be added to the VM.
		 *
		 * A zero status value corresponds to state "NEW".
		 */
		//if (threadStatus != 0 || this != me)
		if (thread)
			throw EIllegalThreadStateException(__FILE__, __LINE__, "Already started.");
		group->add(this);

		//TODO: to check max threads???

		if (self != null) {
			eso_threadattr_detach_set(&threadAttr, true);
		}
		threadStatus = RUNNABLE;
		if (!thread_create(&threadAttr, thread_dummy_worker, this)) {
			throw ERuntimeException(__FILE__, __LINE__);
		}
    }}
}

void EThread::run() {
	if (target != null) {
		target->run();
	}
}

void EThread::interrupt0() {
	//@see: openjdk-6/hotspot/src/share/vm/prims/jvm.cpp line 2943
	//MutexLockerEx ml(thread->threadObj() == java_thread ? NULL : Threads_lock);
	//@see: openjdk-6/hotspot/src/os/linux/vm/os_linux.cpp line 3418
	//assert(Thread::current() == thread || Threads_lock->owned_by_self(), "possibility of dangling Thread pointer");

	if (!_interrupted) {
		_interrupted = true;
		// More than one thread can get here with the same value of osthread,
		// resulting in multiple notifications.  We do, however, want the store
		// to interrupted() to be visible to other threads before we execute unpark().
		EOrderAccess::fence();
		EParkEvent * const slp = this->_SleepEvent;
		if (slp != NULL)
			slp->unpark();
	}

	// For JSR166. Unpark even if interrupt status already was set
	this->parker->unpark();
//	this->unpark();

	EParkEvent * ev = this->_ParkEvent;
	if (ev != NULL)
		ev->unpark();
}

void EThread::interrupt() {
	if (this != EThread::currentThread())
		checkAccess();

	SYNCBLOCK(blockerLock) {
		EInterruptible* b = blocker;
		if (b != null) {
			interrupt0();           // Just to set the interrupt flag
			b->interrupt(this);
			return;
		}
    }}
	interrupt0();
}

void EThread::blockedOn(EInterruptible* b) {
	SYNCBLOCK(blockerLock) {
		blocker = b;
    }}
}

EString EThread::toString() {
	return EString::formatOf("Thread[%s,%d,%s]", getName(), getPriority(), (group != null) ? group->getName() : "");
}

void EThread::yield() {
	eso_thread_yield();
}

// TODO-FIXME: reconcile Solaris' os::sleep with the linux variation.
// Solaris uses poll(), linux uses park().
// Poll() is likely a better choice, assuming that Thread.interrupt()
// generates a SIGUSRx signal. Note that SIGUSR1 can interfere with
// SIGSEGV, see 4355769.

//@see: os::sleep(...)
int EThread::sleep0(llong millis, boolean interruptible) {
	EParkEvent * const slp = _SleepEvent;
	slp->reset();
	EOrderAccess::fence();

	if (interruptible) {
		llong prevtime = ESystem::nanoTime();

		for (;;) {
			if (isInterrupted(true )) {
				return OS_INTRPT;
			}

			llong newtime = ESystem::nanoTime();

			if (newtime - prevtime < 0) {
				// time moving backwards, should only happen if no monotonic clock
				// not a guarantee() because JVM should not abort on kernel/glibc bugs
				ES_ASSERT(false); //"time moving backwards"
			} else {
				millis -= (newtime - prevtime) / ES_NANOS_PER_MILLISEC;
			}

			if (millis <= 0) {
				return OS_OK;
			}

			prevtime = newtime;

			{
				//JavaThread *jt = (JavaThread *) thread;
				//ThreadBlockInVM tbivm(jt); //TODO?
				//OSThreadWaitState osts(jt->osthread(), false /* not Object.wait() */);

				//jt->set_suspend_equivalent();
				// cleared by handle_special_suspend_equivalent_condition() or
				// java_suspend_self() via check_and_wait_while_suspended()

				slp->park(millis);

				// were we externally suspended while we were waiting?
				//jt->check_and_wait_while_suspended();
			}
		}
	} else {
		//OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
		llong prevtime = ESystem::nanoTime();

		for (;;) {
			// It'd be nice to avoid the back-to-back javaTimeNanos() calls on
			// the 1st iteration ...
			llong newtime = ESystem::nanoTime();

			if (newtime - prevtime < 0) {
				// time moving backwards, should only happen if no monotonic clock
				// not a guarantee() because JVM should not abort on kernel/glibc bugs
				ES_ASSERT(false); //"time moving backwards"
			} else {
				millis -= (newtime - prevtime) / ES_NANOS_PER_MILLISEC;
			}

			if (millis <= 0)
				break;

			prevtime = newtime;
			slp->park(millis);
		}
		return OS_OK;
	}
}

//@see: openjdk-6/hotspot/src/share/vm/prims/jvm.cpp line 2829
void EThread::sleep(llong millis) {
	if (millis < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "timeout value is negative");
	}

	EThread* thread = EThread::currentThread();

	if (thread->isInterrupted(true)) {
		throw EInterruptedException(__FILE__, __LINE__, "sleep interrupted");
	}

	// Save current thread state and restore it at the end of this block.
	// And set new thread state to SLEEPING.
	EThreadSleepState jtss(thread);

	if (millis == 0) {
		// When ConvertSleepToYield is on, this matches the classic VM implementation of
		// JVM_Sleep. Critical for similar threading behaviour (Win32)
		// It appears that in certain GUI contexts, it may be beneficial to do a short sleep
		// for SOLARIS
		/*
		if (ConvertSleepToYield) {
			os::yield();
		} else {
			ThreadState old_state = thread->osthread()->get_state();
			thread->osthread()->set_state(SLEEPING);
			os::sleep(thread, MinSleepInterval, false );
			thread->osthread()->set_state(old_state);
		}
		*/
		eso_thread_yield();
	} else {
		//ThreadState old_state = thread->osthread()->get_state();
		//thread->osthread()->set_state(SLEEPING);
		if (thread->sleep0(millis, true) == OS_INTRPT) {
			// An asynchronous exception (e.g., ThreadDeathException) could have been thrown on
			// us while we were sleeping. We do not overwrite those.
			//if (!HAS_PENDING_EXCEPTION) {
				// TODO-FIXME: THROW_MSG returns which means we will not call set_state()
				// to properly restore the thread state.  That's likely wrong.
				throw EInterruptedException(__FILE__, __LINE__, "sleep interrupted");
			//}
		}
		//thread->osthread()->set_state(old_state);
	}
}

void EThread::sleep(llong millis, int nanos) {
	if (millis < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "timeout value is negative");
	}

	if (nanos < 0 || nanos > 999999) {
		throw EIllegalArgumentException(__FILE__, __LINE__,
				"nanosecond timeout value out of range");
	}

	if (nanos >= 500000 || (nanos != 0 && millis == 0)) {
		millis++;
	}

	sleep(millis);
}

EThread* EThread::currentThread() {
	EThread* thread = (EThread*)threadLocal->get();
	if (!thread) {
		//only happened in c thread!!!
		thread = new EThread("c_main", eso_atomic_add_and_fetch32(&c_threadsCount, 1));
//		printf("new thread=%p\n", thread);

		//set current thread object.
		threadLocal->set(thread);
		thread->tid = eso_os_thread_current_id();
	}
	return thread;
}

boolean EThread::isInterrupted() {
	return isInterrupted(false);
}

boolean EThread::isAlive() {
	State state = threadStatus;
	return (state != NEW && state != TERMINATED);
}

void EThread::setPriority(int newPriority) {
	sp<EThreadGroup> g = group;
	checkAccess();
	if (newPriority > MAX_PRIORITY || newPriority < MIN_PRIORITY) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	if (g != null) {
		if (newPriority > g->getMaxPriority()) {
			newPriority = g->getMaxPriority();
		}
		/** @see:
		setPriority0(priority = newPriority);
		*/
		eso_os_thread_priority_set(tid, java_to_os_priority[newPriority]);
	}
}

int EThread::getPriority() {
	return priority;
}

void EThread::setName(const char* name) {
	checkAccess();
	this->name = name;
}

const char* EThread::getName() {
	return name.c_str();
}

boolean EThread::isMainThread() {
	return c_tid == 0;
}

boolean EThread::isCThread() {
	return c_tid > 0;
}

sp<EThreadGroup> EThread::getThreadGroup() {
	return group;
}

void EThread::join() {
	join(0);
}

void EThread::join(llong millis) {
	llong base = ESystem::currentTimeMillis();
	llong now = 0;

	if (millis < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "timeout value is negative");
	}

	SYNCBLOCK(parkLock) {
		if (millis == 0) {
			while (isAlive()) {
				parkCond->await();
			}
		} else {
			while (isAlive()) {
				llong delay = millis - now;
				if (delay <= 0) {
					break;
				}
				parkCond->awaitNanos(delay * ES_NANOS_PER_MILLISEC);
				now = ESystem::currentTimeMillis() - base;
			}
		}
    }}

	SYNCBLOCK(syncLock) {
		if (thread) {
			eso_thread_join(NULL, thread);
			eso_thread_destroy(&thread);
		}
    }}
}

void EThread::join(llong millis, int nanos) {
	if (millis < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, "timeout value is negative");
	}

	if (nanos < 0 || nanos > 999999) {
		throw EIllegalArgumentException(__FILE__, __LINE__,
				"nanosecond timeout value out of range");
	}

	if (nanos >= 500000 || (nanos != 0 && millis == 0)) {
		millis++;
	}

	join(millis);
}

void EThread::checkAccess() {
	//SecurityManager security = System.getSecurityManager();
	//if (security != null) {
	//	security.checkAccess(this);
	//}
}

boolean EThread::interrupted() {
	return currentThread()->isInterrupted(true);
}

int EThread::activeCount() {
	return currentThread()->getThreadGroup()->activeCount();
}

int EThread::enumerate(EA<EThread*>* tarray) {
	return currentThread()->getThreadGroup()->enumerate(tarray);
}

void EThread::setDefaultUncaughtExceptionHandler(UncaughtExceptionHandler* eh) {
	//SecurityManager sm = System.getSecurityManager();
	//if (sm != null) {
	//	sm.checkPermission(
	//			new RuntimePermission("setDefaultUncaughtExceptionHandler"));
	//}

	defaultUncaughtExceptionHandler = eh;
}

EThread::UncaughtExceptionHandler* EThread::getDefaultUncaughtExceptionHandler() {
	return defaultUncaughtExceptionHandler;
}

void EThread::setUncaughtExceptionHandler(UncaughtExceptionHandler* eh) {
	checkAccess();
	uncaughtExceptionHandler = eh;
}

EThread::UncaughtExceptionHandler* EThread::getUncaughtExceptionHandler() {
	return uncaughtExceptionHandler != null ?
			uncaughtExceptionHandler :
			(defaultUncaughtExceptionHandler ?
					defaultUncaughtExceptionHandler : group.get());
}

EThread::State EThread::getState() {
	// get current thread state
	return threadStatus;
}

void EThread::setState(State stat) {
	//@see: openjdk-6/jdk/src/share/javavm/export/jvmti.h line 104
	threadStatus = stat;
}

long EThread::getId() {
	return tid;
}

int EThread::getThreadsCount() {
//	return threadsCount.value();
	return threadsCount;
}

int EThread::getCThreadsCount() {
	return c_threadsCount;
}

sp<ERunnable> EThread::getTarget() {
	return target;
}

void EThread::init(sp<EThreadGroup> g, sp<ERunnable> target, const char* name,
		unsigned long stacksize, unsigned long guardsize, boolean ismain/*=false*/) {
//	printf("EThread::EThread %p\n", this);

	c_tid = -1;
	cleanupCallback = null;
	cleanupArg = null;

	eso_atomic_add_and_fetch32(&threadsCount, 1);

	eso_threadattr_init(&threadAttr);

	if (ismain) { //only for main thread.
		this->tid = eso_os_thread_current_id();
		this->group = new EThreadGroup();
		this->priority = NORM_PRIORITY;
		this->threadStatus = RUNNABLE;
	}
	else { //for all other child thread.
		EThread* parent = currentThread();
		if (g == null) {
			g = parent->getThreadGroup();
		}

		/* checkAccess regardless of whether or not threadgroup is
		   explicitly passed in. */
		g->checkAccess();

		/*
		 * Do we have the required permissions?
		 */
		//if (security != null) {
		//	if (isCCLOverridden(getClass())) {
		//		security.checkPermission(SUBCLASS_IMPLEMENTATION_PERMISSION);
		//	}
		//}

		g->addUnstarted();

		this->tid = 0; //need to update after thread run.
		this->group = g;
//		this->daemon = parent->isDaemon();
		this->priority = parent->getPriority();
		setPriority(priority);
		//if (parent.inheritableThreadLocals != null)
		//	this.inheritableThreadLocals =
		//		ThreadLocal.createInheritedMap(parent.inheritableThreadLocals);
		/* Stash the specified stack size in case the VM cares */
//		eso_threadattr_detach_set(&threadAttr, true);
		if (stacksize > 0L || guardsize > 0L) {
			if (stacksize > 0L) {
				es_status_t rv = eso_threadattr_stacksize_set(&threadAttr,
						stacksize);
				if (rv != ES_SUCCESS) {
					throw EIllegalArgumentException(__FILE__, __LINE__);
				}
			}
			if (guardsize > 0L) {
				es_status_t rv = eso_threadattr_guardsize_set(&threadAttr,
						guardsize);
				if (rv != ES_SUCCESS) {
					throw EIllegalArgumentException(__FILE__, __LINE__);
				}
			}
		}
		else if (defaultStackSize > 0) { // set default stack size.
			es_status_t rv = eso_threadattr_stacksize_set(&threadAttr,
					defaultStackSize);
			if (rv != ES_SUCCESS) {
				throw EIllegalArgumentException(__FILE__, __LINE__);
			}
		}
		this->threadStatus = NEW;
	}

	this->name = name;
	this->target = target;
	this->thread = null;
	this->localValues = null;
	this->inheritableValues = null;
	this->uncaughtExceptionHandler = null;
	this->_interrupted = false;
	this->blockerLock = new ESimpleLock();
	this->blocker = null;
	this->parker = EParker::Allocate(this);

	// Many of the following fields are effectively final - immutable
	// Note that nascent threads can't use the Native Monitor-Mutex
	// construct until the _MutexEvent is initialized ...
	// CONSIDER: instead of using a fixed set of purpose-dedicated ParkEvents
	// we might instead use a stack of ParkEvents that we could provision on-demand.
	// The stack would act as a cache to avoid calls to ParkEvent::Allocate()
	// and ::Release()
	_ParkEvent = EParkEvent::Allocate(this);
	_SleepEvent = EParkEvent::Allocate(this);
	_MutexEvent = EParkEvent::Allocate(this);
	_MuxEvent = EParkEvent::Allocate(this);

	parkLock = new ESimpleLock();
	parkCond = parkLock->newCondition();
	syncLock = new ESimpleLock();

	parkState = UNPARKED;

	//TODO: pthread_sigmask()
}

//@see: openjdk-6/hotspot/src/share/vm/prims/jvm.cpp line 2953
//@see: openjdk-6/hotspot/src/os/linux/vm/os_linux.cpp line 3442
boolean EThread::isInterrupted(boolean ClearInterrupted) {
	//MutexLockerEx ml(thread->threadObj() == java_thread ? NULL : Threads_lock);
	//assert(Thread::current() == thread || Threads_lock->owned_by_self(), "possibility of dangling Thread pointer");

	//return is_interrupted(ClearInterrupted);
	boolean interrupted = _interrupted;

	if (_interrupted && ClearInterrupted) {
		_interrupted = false;
		// consider thread->_SleepEvent->reset() ... optional optimization
	}

	return interrupted;
}

void EThread::parkFor(llong nanos) {
	//@see: android422/libcore/luni/src/main/java/java/lang/Thread.java
	SYNCBLOCK(parkLock) {
		switch (parkState) {
			case PREEMPTIVELY_UNPARKED: {
				parkState = UNPARKED;
				break;
			}
			case UNPARKED: {
				parkState = PARKED;
				try {
					if (nanos == 0L)
						parkCond->await();
					else
						parkCond->awaitNanos(nanos);
				} catch (EInterruptedException& ex) {
					interrupt();
				} catch (...) {
					finally {
						if (parkState == PARKED) {
							parkState = UNPARKED;
						}
					}
					throw; //!
				} finally {
					/*
					 * Note: If parkState manages to become
					 * PREEMPTIVELY_UNPARKED before hitting this
					 * code, it should left in that state.
					 */
					if (parkState == PARKED) {
						parkState = UNPARKED;
					}
				}
				break;
			}
			default /*parked*/: {
				throw ERuntimeException(__FILE__, __LINE__, 
						"shouldn't happen: attempt to repark");
			}
		}
    }}
	EOrderAccess::fence();
}

void EThread::parkUntil(llong time) {
	//@see: android422/libcore/luni/src/main/java/java/lang/Thread.java
	SYNCBLOCK(parkLock) {
		/*
		 * Note: This conflates the two time bases of "wall clock"
		 * time and "monotonic uptime" time. However, given that
		 * the underlying system can only wait on monotonic time,
		 * it is unclear if there is any way to avoid the
		 * conflation. The downside here is that if, having
		 * calculated the delay, the wall clock gets moved ahead,
		 * this method may not return until well after the wall
		 * clock has reached the originally designated time. The
		 * reverse problem (the wall clock being turned back)
		 * isn't a big deal, since this method is allowed to
		 * spuriously return for any reason, and this situation
		 * can safely be construed as just such a spurious return.
		 */
		llong delayMillis = time - ESystem::currentTimeMillis();

		if (delayMillis <= 0) {
			parkState = UNPARKED;
		} else {
			parkFor(delayMillis * ES_NANOS_PER_MILLISEC);
		}
    }}
}

void EThread::unpark() {
	//@see: android422/libcore/luni/src/main/java/java/lang/Thread.java
	SYNCBLOCK(parkLock) {
		if (threadStatus == NEW) {
			/*
			 * vmThread is null before the thread is start()ed. In
			 * this case, we just go ahead and set the state to
			 * PREEMPTIVELY_UNPARKED. Since this happens before the
			 * thread is started, we don't have to worry about
			 * synchronizing with it.
			 */
			parkState = PREEMPTIVELY_UNPARKED;
			return;
		}

		switch (parkState) {
			case PREEMPTIVELY_UNPARKED: {
				/*
				 * Nothing to do in this case: By definition, a
				 * preemptively unparked thread is to remain in
				 * the preemptively unparked state if it is told
				 * to unpark.
				 */
				break;
			}
			case UNPARKED: {
				parkState = PREEMPTIVELY_UNPARKED;
				break;
			}
			default /*parked*/: {
				parkState = UNPARKED;
				parkCond->signalAll(); //signalAll()?
				break;
			}
		}
    }}
}

} /* namespace efc */
