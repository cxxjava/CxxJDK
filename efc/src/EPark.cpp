/*
 * EPark.cpp
 *
 *  Created on: 2014-10-6
 *      Author: cxxjava@163.com
 */

#include "EOS.hh"
#include "EPark.hh"
#include "EThread.hh"
#include "../inc/concurrent/EOrderAccess.hh"
#include "../inc/concurrent/EAtomic.hh"

namespace efc {

/**
 * @see: openjdk-8/hotspot/src/share/vm/runtime/park.cpp
 * @see: openjdk-7/hotspot/src/os/linux/vm/os_linux.cpp
 * @see: openjdk-7/hotspot/src/os/windows/vm/os_windows.cpp
 */

#ifdef WIN32

// An Event wraps a win32 "CreateEvent" kernel handle.
//
// We have a number of choices regarding "CreateEvent" win32 handle leakage:
//
// 1:  When a thread dies return the Event to the EventFreeList, clear the ParkHandle
//     field, and call CloseHandle() on the win32 event handle.  Unpark() would
//     need to be modified to tolerate finding a NULL (invalid) win32 event handle.
//     In addition, an unpark() operation might fetch the handle field, but the
//     event could recycle between the fetch and the SetEvent() operation.
//     SetEvent() would either fail because the handle was invalid, or inadvertently work,
//     as the win32 handle value had been recycled.  In an ideal world calling SetEvent()
//     on an stale but recycled handle would be harmless, but in practice this might
//     confuse other non-Sun code, so it's not a viable approach.
//
// 2:  Once a win32 event handle is associated with an Event, it remains associated
//     with the Event.  The event handle is never closed.  This could be construed
//     as handle leakage, but only up to the maximum # of threads that have been extant
//     at any one time.  This shouldn't be an issue, as windows platforms typically
//     permit a process to have hundreds of thousands of open handles.
//
// 3:  Same as (1), but periodically, at stop-the-world time, rundown the EventFreeList
//     and release unused handles.
//
// 4:  Add a CRITICAL_SECTION to the Event to protect LD+SetEvent from LD;ST(null);CloseHandle.
//     It's not clear, however, that we wouldn't be trading one type of leak for another.
//
// 5.  Use an RCU-like mechanism (Read-Copy Update).
//     Or perhaps something similar to Maged Michael's "Hazard pointers".
//
// We use (2).
//
// TODO-FIXME:
// 1.  Reconcile Doug's JSR166 j.u.c park-unpark with the objectmonitor implementation.
// 2.  Consider wrapping the WaitForSingleObject(Ex) calls in SEH try/finally blocks
//     to recover from (or at least detect) the dreaded Windows 841176 bug.
// 3.  Collapse the interrupt_event, the JSR166 parker event, and the objectmonitor ParkEvent
//     into a single win32 CreateEvent() handle.
//
// _Event transitions in park()
//   -1 => -1 : illegal
//    1 =>  0 : pass - return immediately
//    0 => -1 : block
//
// _Event serves as a restricted-range semaphore :
//    -1 : thread is blocked
//     0 : neutral  - thread is running or ready
//     1 : signaled - thread is running or ready
//
// Another possible encoding of _Event would be
// with explicit "PARKED" and "SIGNALED" bits.

int PlatformEvent::park (llong Millis) {
    ES_ASSERT (_ParkHandle != NULL) ;
    ES_ASSERT (Millis > 0) ;
    int v ;

    // CONSIDER: defer assigning a CreateEvent() handle to the Event until
    // the initial park() operation.

    for (;;) {
        v = _Event ;
        if (EAtomic::cmpxchg32 (v-1, &_Event, v) == v) break ;
    }
    ES_ASSERT ((v == 0) || (v == 1)) ;
    if (v != 0) return OS_OK ;

    // Do this the hard way by blocking ...
    // TODO: consider a brief spin here, gated on the success of recent
    // spin attempts by this thread.
    //
    // We decompose long timeouts into series of shorter timed waits.
    // Evidently large timo values passed in WaitForSingleObject() are problematic on some
    // versions of Windows.  See EventWait() for details.  This may be superstition.  Or not.
    // We trust the WAIT_TIMEOUT indication and don't track the elapsed wait time
    // with os::javaTimeNanos().  Furthermore, we assume that spurious returns from
    // ::WaitForSingleObject() caused by latent ::setEvent() operations will tend
    // to happen early in the wait interval.  Specifically, after a spurious wakeup (rv ==
    // WAIT_OBJECT_0 but _Event is still < 0) we don't bother to recompute Millis to compensate
    // for the already waited time.  This policy does not admit any new outcomes.
    // In the future, however, we might want to track the accumulated wait time and
    // adjust Millis accordingly if we encounter a spurious wakeup.

    const int MAXTIMEOUT = 0x10000000 ;
    DWORD rv = WAIT_TIMEOUT ;
    while (_Event < 0 && Millis > 0) {
       DWORD prd = Millis ;     // set prd = MAX (Millis, MAXTIMEOUT)
       if (Millis > MAXTIMEOUT) {
          prd = MAXTIMEOUT ;
       }
       rv = ::WaitForSingleObject (_ParkHandle, prd) ;
       ES_ASSERT (rv == WAIT_OBJECT_0 || rv == WAIT_TIMEOUT) ;
       if (rv == WAIT_TIMEOUT) {
           Millis -= prd ;
       }
    }
    v = _Event ;
    _Event = 0 ;
    EOrderAccess::fence() ;
    // If we encounter a nearly simultanous timeout expiry and unpark()
    // we return OS_OK indicating we awoke via unpark().
    // Implementor's license -- returning OS_TIMEOUT would be equally valid, however.
    return (v >= 0) ? OS_OK : OS_TIMEOUT ;
}

void PlatformEvent::park() { 
	ES_ASSERT (_ParkHandle != NULL) ;
    // Invariant: Only the thread associated with the Event/PlatformEvent
    // may call park().
    int v ;
    for (;;) {
        v = _Event ;
        if (EAtomic::cmpxchg32 (v-1, &_Event, v) == v) break ;
    }
    ES_ASSERT ((v == 0) || (v == 1)) ;
    if (v != 0) return ;

    // Do this the hard way by blocking ...
    // TODO: consider a brief spin here, gated on the success of recent
    // spin attempts by this thread.
    while (_Event < 0) {
       DWORD rv = ::WaitForSingleObject (_ParkHandle, INFINITE) ;
       ES_ASSERT (rv == WAIT_OBJECT_0) ;
    }

    // Usually we'll find _Event == 0 at this point, but as
    // an optional optimization we clear it, just in case can
    // multiple unpark() operations drove _Event up to 1.
    _Event = 0 ;
    EOrderAccess::fence() ;
    ES_ASSERT (_Event >= 0) ;
}

void PlatformEvent::unpark() {
  ES_ASSERT (_ParkHandle != NULL) ;
  int v ;
  for (;;) {
      v = _Event ;      // Increment _Event if it's < 1.
      if (v > 0) {
         // If it's already signaled just return.
         // The LD of _Event could have reordered or be satisfied
         // by a read-aside from this processor's write buffer.
         // To avoid problems execute a barrier and then
         // ratify the value.  A degenerate CAS() would also work.
         // Viz., CAS (v+0, &_Event, v) == v).
         EOrderAccess::fence() ;
         if (_Event == v) return ;
         continue ;
      }
      if (EAtomic::cmpxchg32(v+1, &_Event, v) == v) break ;
  }
  if (v < 0) {
     ::SetEvent (_ParkHandle) ;
  }
}

// JSR166
// -------------------------------------------------------

/*
 * The Windows implementation of Park is very straightforward: Basic
 * operations on Win32 Events turn out to have the right semantics to
 * use them directly. We opportunistically resuse the event inherited
 * from Monitor.
 */

inline int low (llong value)                    { return int(value); }
inline int high(llong value)                    { return int(value >> 32); }

// the fancy casts are a hopefully portable way
// to do unsigned 32 to 64 bit type conversion
inline void set_low (llong* value, int low )    { *value &= (llong)0xffffffff << 32;
                                                   *value |= (llong)(ullong)(uint)low; }

inline void set_high(llong* value, int high)    { *value &= (llong)(ullong)(uint)0xffffffff;
                                                   *value |= (llong)high       << 32; }

inline llong jlong_from(int h, int l) {
  llong result = 0; // initialization to avoid warning
  set_high(&result, h);
  set_low(&result,  l);
  return result;
}

// Windows format:
//   The FILETIME structure is a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601.
// Java format:
//   Java standards require the number of milliseconds since 1/1/1970

// Constant offset - calculated using offset()
static llong  _offset   = 116444736000000000;
// Fake time counter for reproducible results when debugging
static llong  fake_time = 0;

#ifdef ES_ASSERT
// Just to be safe, recalculate the offset in debug mode
static llong _calculated_offset = 0;
static int   _has_calculated_offset = 0;

llong offset() {
  if (_has_calculated_offset) return _calculated_offset;
  SYSTEMTIME java_origin;
  java_origin.wYear          = 1970;
  java_origin.wMonth         = 1;
  java_origin.wDayOfWeek     = 0; // ignored
  java_origin.wDay           = 1;
  java_origin.wHour          = 0;
  java_origin.wMinute        = 0;
  java_origin.wSecond        = 0;
  java_origin.wMilliseconds  = 0;
  FILETIME jot;
  if (!SystemTimeToFileTime(&java_origin, &jot)) {
    ES_ASSERT(0); //fatal(err_msg("Error = %d\nWindows error", GetLastError()));
  }
  _calculated_offset = jlong_from(jot.dwHighDateTime, jot.dwLowDateTime);
  _has_calculated_offset = 1;
  ES_ASSERT(_calculated_offset == _offset);//"Calculated and constant time offsets must be equal"
  return _calculated_offset;
}
#else
llong offset() {
  return _offset;
}
#endif

static llong windows_to_java_time(FILETIME wt) {
  llong a = jlong_from(wt.dwHighDateTime, wt.dwLowDateTime);
  return (a - offset()) / 10000;
}

static FILETIME java_to_windows_time(llong l) {
  llong a = (l * 10000) + offset();
  FILETIME result;
  result.dwHighDateTime = high(a);
  result.dwLowDateTime  = low(a);
  return result;
}

static llong javaTimeMillis() {
  //if (UseFakeTimers) {
  //  return fake_time++;
  //} else {
    FILETIME wt;
    GetSystemTimeAsFileTime(&wt);
    return windows_to_java_time(wt);
  //}
}

void EParker::park(boolean isAbsolute, llong time) {
  ES_ASSERT (_ParkEvent != NULL) ;
  // First, demultiplex/decode time arguments
  if (time < 0) { // don't wait
    return;
  }
  else if (time == 0 && !isAbsolute) {
    time = INFINITE;
  }
  else if  (isAbsolute) {
    time -= javaTimeMillis(); // convert to relative time
    if (time <= 0) // already elapsed
      return;
  }
  else { // relative
    time /= 1000000; // Must coarsen from nanos to millis
    if (time == 0)   // Wait for the minimal time unit if zero
      time = 1;
  }

  EThread* thread = EThread::currentThread();

  // Don't wait if interrupted or already triggered
  if (thread->isInterrupted(false) ||
    WaitForSingleObject(_ParkEvent, 0) == WAIT_OBJECT_0) {
    ResetEvent(_ParkEvent);
    return;
  }
  else {
    //ThreadBlockInVM tbivm(jt);
    //OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
    //jt->set_suspend_equivalent();

    WaitForSingleObject(_ParkEvent,  time);
    ResetEvent(_ParkEvent);

    // If externally suspended while waiting, re-suspend
    //if (jt->handle_special_suspend_equivalent_condition()) {
    //  jt->java_suspend_self();
    //}
  }
}

void EParker::unpark() {
  ES_ASSERT (_ParkEvent != NULL) ;
  SetEvent(_ParkEvent);
}

#else //

#include <sys/time.h>

// Refer to the comments in os_solaris.cpp park-unpark.
//
// Beware -- Some versions of NPTL embody a flaw where pthread_cond_timedwait() can
// hang indefinitely.  For instance NPTL 0.60 on 2.4.21-4ELsmp is vulnerable.
// For specifics regarding the bug see GLIBC BUGID 261237 :
//    http://www.mail-archive.com/debian-glibc@lists.debian.org/msg10837.html.
// Briefly, pthread_cond_timedwait() calls with an expiry time that's not in the future
// will either hang or corrupt the condvar, resulting in subsequent hangs if the condvar
// is used.  (The simple C test-case provided in the GLIBC bug report manifests the
// hang).  The JVM is vulernable via sleep(), Object.wait(timo), LockSupport.parkNanos()
// and monitorenter when we're using 1-0 locking.  All those operations may result in
// calls to pthread_cond_timedwait().  Using LD_ASSUME_KERNEL to use an older version
// of libpthread avoids the problem, but isn't practical.
//
// Possible remedies:
//
// 1.   Establish a minimum relative wait time.  50 to 100 msecs seems to work.
//      This is palliative and probabilistic, however.  If the thread is preempted
//      between the call to compute_abstime() and pthread_cond_timedwait(), more
//      than the minimum period may have passed, and the abstime may be stale (in the
//      past) resultin in a hang.   Using this technique reduces the odds of a hang
//      but the JVM is still vulnerable, particularly on heavily loaded systems.
//
// 2.   Modify park-unpark to use per-thread (per ParkEvent) pipe-pairs instead
//      of the usual flag-condvar-mutex idiom.  The write side of the pipe is set
//      NDELAY. unpark() reduces to write(), park() reduces to read() and park(timo)
//      reduces to poll()+read().  This works well, but consumes 2 FDs per extant
//      thread.
//
// 3.   Embargo pthread_cond_timedwait() and implement a native "chron" thread
//      that manages timeouts.  We'd emulate pthread_cond_timedwait() by enqueuing
//      a timeout request to the chron thread and then blocking via pthread_cond_wait().
//      This also works well.  In fact it avoids kernel-level scalability impediments
//      on certain platforms that don't handle lots of active pthread_cond_timedwait()
//      timers in a graceful fashion.
//
// 4.   When the abstime value is in the past it appears that control returns
//      correctly from pthread_cond_timedwait(), but the condvar is left corrupt.
//      Subsequent timedwait/wait calls may hang indefinitely.  Given that, we
//      can avoid the problem by reinitializing the condvar -- by cond_destroy()
//      followed by cond_init() -- after all calls to pthread_cond_timedwait().
//      It may be possible to avoid reinitialization by checking the return
//      value from pthread_cond_timedwait().  In addition to reinitializing the
//      condvar we must establish the invariant that cond_signal() is only called
//      within critical sections protected by the adjunct mutex.  This prevents
//      cond_signal() from "seeing" a condvar that's in the midst of being
//      reinitialized or that is corrupt.  Sadly, this invariant obviates the
//      desirable signal-after-unlock optimization that avoids futile context switching.
//
//      I'm also concerned that some versions of NTPL might allocate an auxilliary
//      structure when a condvar is used or initialized.  cond_destroy()  would
//      release the helper structure.  Our reinitialize-after-timedwait fix
//      put excessive stress on malloc/free and locks protecting the c-heap.
//
// We currently use (4).  See the WorkAroundNTPLTimedWaitHang flag.
// It may be possible to refine (4) by checking the kernel and NTPL verisons
// and only enabling the work-around for vulnerable environments.

// utility to compute the abstime argument to timedwait:
// millis is the relative timeout time
// abstime will be the absolute timeout time
// TODO: replace compute_abstime() with unpackTime()

static struct timespec* compute_abstime(timespec* abstime, llong millis) {
	if (millis < 0)
		millis = 0;
	struct timeval now;
	int status = gettimeofday(&now, NULL);
	ES_ASSERT(status == 0);
	llong seconds = millis / 1000;
	millis %= 1000;
	if (seconds > 50000000) { // see man cond_timedwait(3T)
		seconds = 50000000;
	}
	abstime->tv_sec = now.tv_sec + seconds;
	long usec = now.tv_usec + millis * 1000;
	if (usec >= 1000000) {
		abstime->tv_sec += 1;
		usec -= 1000000;
	}
	abstime->tv_nsec = usec * 1000;
	return abstime;
}


// Test-and-clear _Event, always leaves _Event set to 0, returns immediately.
// Conceptually TryPark() should be equivalent to park(0).

int PlatformEvent::TryPark() {
	for (;;) {
		const int v = _Event;
		ES_ASSERT((v == 0) || (v == 1));
		if (EAtomic::cmpxchg32(0, &_Event, v) == v)
			return v;
	}
	//never reach here!
	return OS_ERR;
}

void PlatformEvent::park() {       // AKA "down()"
  // Invariant: Only the thread associated with the Event/PlatformEvent
  // may call park().
  // TODO: assert that _Assoc != NULL or _Assoc == Self
  int v ;
  for (;;) {
      v = _Event ;
      if (EAtomic::cmpxchg32(v-1, &_Event, v) == v) break ;
  }
  ES_ASSERT (v >= 0) ;
  if (v == 0) {
     // Do this the hard way by blocking ...
     int status = pthread_mutex_lock(_mutex);
     ES_ASSERT(status == 0);
     ES_ASSERT (_nParked == 0) ;
     ++ _nParked ;
     while (_Event < 0) {
        status = pthread_cond_wait(_cond, _mutex);
        // for some reason, under 2.7 lwp_cond_wait() may return ETIME ...
        // Treat this the same as if the wait was interrupted
        if (status == ETIME) { status = EINTR; }
        ES_ASSERT(status == 0 || status == EINTR);
     }
     -- _nParked ;

    // In theory we could move the ST of 0 into _Event past the unlock(),
    // but then we'd need a MEMBAR after the ST.
    _Event = 0 ;
     status = pthread_mutex_unlock(_mutex);
     ES_ASSERT(status == 0);
  }
  ES_ASSERT(_Event >= 0) ;
}

int PlatformEvent::park(llong millis) {
	ES_ASSERT(_nParked == 0);

	int v;
	for (;;) {
		v = _Event;
		if (EAtomic::cmpxchg32(v - 1, &_Event, v) == v)
			break;
	}
	ES_ASSERT(v >= 0);
	if (v != 0)
		return OS_OK;

	// We do this the hard way, by blocking the thread.
	// Consider enforcing a minimum timeout value.
	struct timespec abst;
	compute_abstime(&abst, millis);

	int ret = OS_TIMEOUT;
	int status = pthread_mutex_lock(_mutex);
	ES_ASSERT(status == 0);
	ES_ASSERT(_nParked == 0);
	++_nParked;

	// Object.wait(timo) will return because of
	// (a) notification
	// (b) timeout
	// (c) thread.interrupt
	//
	// Thread.interrupt and object.notify{All} both call Event::set.
	// That is, we treat thread.interrupt as a special case of notification.
	// The underlying Solaris implementation, cond_timedwait, admits
	// spurious/premature wakeups, but the JLS/JVM spec prevents the
	// JVM from making those visible to Java code.  As such, we must
	// filter out spurious wakeups.  We assume all ETIME returns are valid.
	//
	// TODO: properly differentiate simultaneous notify+interrupt.
	// In that case, we should propagate the notify to another waiter.

	while (_Event < 0) {
		status = pthread_cond_timedwait(_cond, _mutex, &abst);
		if (status != 0) {// && WorkAroundNPTLTimedWaitHang) {
			pthread_cond_destroy (_cond);
			pthread_cond_init(_cond, NULL);
		}
		ES_ASSERT(status == 0 || status == EINTR || status == ETIME
						|| status == ETIMEDOUT);
		//if (!FilterSpuriousWakeups)
		//	break;                 // previous semantics
		if (status == ETIME || status == ETIMEDOUT)
			break;
		// We consume and ignore EINTR and spurious wakeups.
	}
	--_nParked;
	if (_Event >= 0) {
		ret = OS_OK;
	}
	_Event = 0;
	status = pthread_mutex_unlock(_mutex);
	ES_ASSERT(status == 0);
	ES_ASSERT(_nParked == 0);
	return ret;
}

void PlatformEvent::unpark() {
	int v, AnyWaiters;
	for (;;) {
		v = _Event;
		if (v > 0) {
			// The LD of _Event could have reordered or be satisfied
			// by a read-aside from this processor's write buffer.
			// To avoid problems execute a barrier and then
			// ratify the value.
			EOrderAccess::fence();
			if (_Event == v)
				return;
			continue;
		}
		if (EAtomic::cmpxchg32(v + 1, &_Event, v) == v)
			break;
	}
	if (v < 0) {
		// Wait for the thread associated with the event to vacate
		int status = pthread_mutex_lock(_mutex);
		ES_ASSERT(status == 0);
		AnyWaiters = _nParked;
		ES_ASSERT(AnyWaiters == 0 || AnyWaiters == 1);
		if (AnyWaiters != 0) { // && WorkAroundNPTLTimedWaitHang) {
			AnyWaiters = 0;
			pthread_cond_signal (_cond);
		}
		status = pthread_mutex_unlock(_mutex);
		ES_ASSERT(status == 0);
		if (AnyWaiters != 0) {
			status = pthread_cond_signal(_cond);
			ES_ASSERT(status == 0);
		}
	}

	// Note that we signal() _after dropping the lock for "immortal" Events.
	// This is safe and avoids a common class of  futile wakeups.  In rare
	// circumstances this can cause a thread to return prematurely from
	// cond_{timed}wait() but the spurious wakeup is benign and the victim will
	// simply re-test the condition and re-park itself.
}

#if 1
// JSR166
// -------------------------------------------------------

/*
* The solaris and linux implementations of park/unpark are fairly
* conservative for now, but can be improved. They currently use a
* mutex/condvar pair, plus a a count.
* Park decrements count if > 0, else does a condvar wait.  Unpark
* sets count to 1 and signals condvar.  Only one thread ever waits
* on the condvar. Contention seen when trying to park implies that someone
* is unparking you, so don't wait. And spurious returns are fine, so there
* is no need to track notifications.
*/


#define NANOSECS_PER_SEC 1000000000
#define NANOSECS_PER_MILLISEC 1000000
#define MAX_SECS 100000000
/*
* This code is common to linux and solaris and will be moved to a
* common place in dolphin.
*
* The passed in time value is either a relative time in nanoseconds
* or an absolute time in milliseconds. Either way it has to be unpacked
* into suitable seconds and nanoseconds components and stored in the
* given timespec structure.
* Given time is a 64-bit value and the time_t used in the timespec is only
* a signed-32-bit value (except on 64-bit Linux) we have to watch for
* overflow if times way in the future are given. Further on Solaris versions
* prior to 10 there is a restriction (see cond_timedwait) that the specified
* number of seconds, in abstime, is less than current_time  + 100,000,000.
* As it will be 28 years before "now + 100000000" will overflow we can
* ignore overflow and just impose a hard-limit on seconds using the value
* of "now + 100,000,000". This places a limit on the timeout of about 3.17
* years from "now".
*/

static void unpackTime(timespec* absTime, boolean isAbsolute, llong time) {
	ES_ASSERT(time > 0);

	struct timeval now;
	int status = gettimeofday(&now, NULL);
	ES_ASSERT(status == 0);

	time_t max_secs = now.tv_sec + MAX_SECS;

	if (isAbsolute) {
		llong secs = time / 1000;
		if (secs > max_secs) {
			absTime->tv_sec = max_secs;
		} else {
			absTime->tv_sec = secs;
		}
		absTime->tv_nsec = (time % 1000) * NANOSECS_PER_MILLISEC;
	} else {
		llong secs = time / NANOSECS_PER_SEC;
		if (secs >= MAX_SECS) {
			absTime->tv_sec = max_secs;
			absTime->tv_nsec = 0;
		} else {
			absTime->tv_sec = now.tv_sec + secs;
			absTime->tv_nsec = (time % NANOSECS_PER_SEC) + now.tv_usec * 1000;
			if (absTime->tv_nsec >= NANOSECS_PER_SEC) {
				absTime->tv_nsec -= NANOSECS_PER_SEC;
				++absTime->tv_sec; // note: this must be <= max_secs
			}
		}
	}
	ES_ASSERT(absTime->tv_sec >= 0);
	ES_ASSERT(absTime->tv_sec <= max_secs);
	ES_ASSERT(absTime->tv_nsec >= 0);
	ES_ASSERT(absTime->tv_nsec < NANOSECS_PER_SEC);
}

void EParker::park(boolean isAbsolute, llong time) {
	// Ideally we'd do something useful while spinning, such
	// as calling unpackTime().

	// Optional fast-path check:
	// Return immediately if a permit is available.
	// We depend on Atomic::xchg() having full barrier semantics
	// since we are doing a lock-free update to _counter.
	if (EAtomic::xchg32(0, &_counter) > 0) {
		return;
	}

	EThread* thread = EThread::currentThread();

	// Optional optimization -- avoid state transitions if there's an interrupt pending.
	// Check interrupt before trying to wait
	//@see: if (Thread::is_interrupted(thread, false)) {
	if (thread->isInterrupted(false)) {
		return;
	}

	// Next, demultiplex/decode time arguments
	timespec absTime;
	if (time < 0 || (isAbsolute && time == 0)) { // don't wait at all
		return;
	}
	if (time > 0) {
		unpackTime(&absTime, isAbsolute, time);
	}

	// Enter safepoint region
	// Beware of deadlocks such as 6317397.
	// The per-thread Parker:: mutex is a classic leaf-lock.
	// In particular a thread must never block on the Threads_lock while
	// holding the Parker:: mutex.  If safepoints are pending both the
	// the ThreadBlockInVM() CTOR and DTOR may grab Threads_lock.

	//ThreadBlockInVM tbivm(jt); //TODO?

	// Don't wait if cannot get lock since interference arises from
	// unblocking.  Also. check interrupt before trying wait
	if (thread->isInterrupted(false)
			|| pthread_mutex_trylock(_mutex) != 0) {
		return;
	}

	int status;
	if (_counter > 0) { // no wait needed
		_counter = 0;
		status = pthread_mutex_unlock(_mutex);
		ES_ASSERT(status == 0);
		EOrderAccess::fence();
		return;
	}

	//OSThreadWaitState osts(thread->osthread(), false /* not Object.wait() */);
	//jt->set_suspend_equivalent();
	// cleared by handle_special_suspend_equivalent_condition() or java_suspend_self()

	if (time == 0) {
		status = pthread_cond_wait(_cond, _mutex);
	} else {
		status = pthread_cond_timedwait(_cond, _mutex, &absTime);
		if (status != 0) { // && WorkAroundNPTLTimedWaitHang) {
			pthread_cond_destroy (_cond);
			pthread_cond_init(_cond, NULL);
		}
	}
	ES_ASSERT(status == 0|| status == EINTR ||
			  status == ETIME || status == ETIMEDOUT);

	_counter = 0;
	status = pthread_mutex_unlock(_mutex);
	ES_ASSERT(status == 0);
	// Paranoia to ensure our locked and lock-free paths interact
	// correctly with each other and Java-level accesses.
	EOrderAccess::fence();

	// If externally suspended while waiting, re-suspend
	//if (jt->handle_special_suspend_equivalent_condition()) {
	//	jt->java_suspend_self();
	//}
}

void EParker::unpark() {
	int s, status;
	status = pthread_mutex_lock(_mutex);
	ES_ASSERT(status == 0);
	s = _counter;
	_counter = 1;
	if (s < 1) {
		if (1) { //WorkAroundNPTLTimedWaitHang) {
			status = pthread_cond_signal(_cond);
			ES_ASSERT(status == 0);
			status = pthread_mutex_unlock(_mutex);
			ES_ASSERT(status == 0);
		} else {
			status = pthread_mutex_unlock(_mutex);
			ES_ASSERT(status == 0);
			status = pthread_cond_signal(_cond);
			ES_ASSERT(status == 0);
		}
	} else {
		pthread_mutex_unlock (_mutex);
		ES_ASSERT(status == 0);
	}
}
#endif

#endif //!WIN32


// 6399321 As a temporary measure we copied & modified the ParkEvent::
// allocate() and release() code for use by Parkers.  The Parker:: forms
// will eventually be removed as we consolide and shift over to ParkEvents
// for both builtin synchronization and JSR166 operations.

ESpinLock * EParker::ListLock = NULL;
EParker * volatile EParker::FreeList = NULL;

DEFINE_STATIC_INITZZ_BEGIN(EParker)
EOS::_initzz_();
static ESpinLock lock;
ListLock = &lock;
DEFINE_STATIC_INITZZ_END

EParker * EParker::Allocate(EThread * t) {
	ES_ASSERT (t != NULL);
	EParker * p;

	// Start by trying to recycle an existing but unassociated
	// Parker from the global free list.
	// 8028280: using concurrent free list without memory management can leak
	// pretty badly it turns out.
	SYNCBLOCK(ListLock) {
		p = FreeList;
		if (p != NULL) {
			FreeList = p->FreeNext;
		}
	}}

	if (p != NULL) {
		ES_ASSERT (p->AssociatedWith == NULL);
	} else {
		// Do this the hard way -- materialize a new Parker..
		p = new EParker();
	}
	p->AssociatedWith = t;          // Associate p with t
	p->FreeNext = NULL;
	return p;
}

void EParker::Release(EParker * p) {
	if (p == NULL)
		return;
	ES_ASSERT (p->AssociatedWith != NULL);
	ES_ASSERT (p->FreeNext == NULL );
	p->AssociatedWith = NULL;

	SYNCBLOCK(ListLock) {
		p->FreeNext = FreeList;
		FreeList = p;
	}}
}

//=============================================================================

ESpinLock * EParkEvent::ListLock = NULL;
EParkEvent * volatile EParkEvent::FreeList = NULL ;

DEFINE_STATIC_INITZZ_BEGIN(EParkEvent)
EOS::_initzz_();
static ESpinLock lock;
ListLock = &lock;
DEFINE_STATIC_INITZZ_END

EParkEvent * EParkEvent::Allocate(EThread * t) {
	// In rare cases -- JVM_RawMonitor* operations -- we can find t == null.
	EParkEvent * ev;

	// Start by trying to recycle an existing but unassociated
	// ParkEvent from the global free list.
	// Using a spin lock since we are part of the mutex impl.
	// 8028280: using concurrent free list without memory management can leak
	// pretty badly it turns out.
	SYNCBLOCK(ListLock) {
		ev = FreeList;
		if (ev != NULL) {
			FreeList = ev->FreeNext;
		}
	}}

	if (ev != NULL) {
		ES_ASSERT(ev->AssociatedWith == NULL);
	} else {
		// Do this the hard way -- materialize a new ParkEvent.
		ev = new EParkEvent();
		ES_ASSERT((intptr_t(ev) & 0xFF) == 0);
	}
	ev->reset();                     // courtesy to caller
	ev->AssociatedWith = t;          // Associate ev with t
	ev->FreeNext = NULL;
	return ev;
}

void EParkEvent::Release(EParkEvent * ev) {
	if (ev == NULL)
		return;
	ES_ASSERT(ev->FreeNext == NULL);
	ev->AssociatedWith = NULL;
	// Note that if we didn't have the TSM/immortal constraint, then
	// when reattaching we could trim the list.
	SYNCBLOCK(ListLock) {
		ev->FreeNext = FreeList;
		FreeList = ev;
	}}
}

// Override operator new and delete so we can ensure that the
// least significant byte of ParkEvent addresses is 0.
// Beware that excessive address alignment is undesirable
// as it can result in D$ index usage imbalance as
// well as bank access imbalance on Niagara-like platforms,
// although Niagara's hash function should help.

void * EParkEvent::operator new(size_t sz) {
	return (void *) ((intptr_t(::operator new(sz + 256)) + 256) & -256);
}

void EParkEvent::operator delete(void * a) {
	// ParkEvents are type-stable and immortal ...
	::operator delete(a);
}

} /* namespace efc */
