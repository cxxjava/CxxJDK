/*
 * EPark.hh
 *
 *  Created on: 2014-10-6
 *      Author: cxxjava@163.com
 */

#ifndef EPARK_HH_
#define EPARK_HH_

#include "EThread.hh"
#include "ESpinLock.hh"

namespace efc {

/**
 * @see: openjdk-7/hotspot/src/share/vm/runtime/park.hpp
 * @see: openjdk-7/hotspot/src/os/linux/vm/os_linux.hpp
 * @see: openjdk-7/hotspot/src/os/windows/vm/os_windows.hpp
 */

#ifdef WIN32

class PlatformEvent {
  private:
    double CachePad [4] ;   // increase odds that _Event is sole occupant of cache line
    volatile int _Event ;
    HANDLE _ParkHandle ;

  public:       // TODO-FIXME: make dtor private
    ~PlatformEvent() {}

  public:
    PlatformEvent() {
      _Event   = 0 ;
      _ParkHandle = CreateEvent (NULL, false, false, NULL) ;
      ES_ASSERT(_ParkHandle != NULL) ;
    }

    // Exercise caution using reset() and fired() - they may require MEMBARs
    void reset() { _Event = 0 ; }
    int  fired() { return _Event; }
    void park () ;
    void unpark () ;
    int  park (llong millis) ;
};

class PlatformParker {
  protected:
    HANDLE _ParkEvent ;

  public:
    ~PlatformParker () {}
    PlatformParker  () {
      _ParkEvent = CreateEvent (NULL, true, false, NULL) ;
      ES_ASSERT(_ParkEvent != NULL) ;
    }
};

#else

class PlatformEvent {
private:
	double CachePad[4]; // increase odds that _mutex is sole occupant of cache line
	volatile int _Event;
	volatile int _nParked;
	pthread_mutex_t _mutex[1];
	pthread_cond_t _cond[1];
	double PostPad[2];
	EThread * _Assoc ;

public:
	// TODO-FIXME: make dtor private
	~PlatformEvent() {}

public:
	PlatformEvent() {
		int status;
		status = pthread_cond_init(_cond, NULL);
		ES_ASSERT(status == 0);
		status = pthread_mutex_init(_mutex, NULL);
		ES_ASSERT(status == 0);
		_Event = 0;
		_nParked = 0;
		_Assoc   = NULL ;
	}

	// Use caution with reset() and fired() -- they may require MEMBARs
	void reset() {
		_Event = 0;
	}
	int fired() {
		return _Event;
	}
	void park();
	void unpark();
	int TryPark();
	es_status_t park(llong millis);
	void SetAssociation (EThread * a) { _Assoc = a ; }
};

class PlatformParker {
protected:
	pthread_mutex_t _mutex[1];
	pthread_cond_t _cond[1];

public:
	// TODO-FIXME: make dtor private
	~PlatformParker() {}

public:
	PlatformParker() {
		int status;
		status = pthread_cond_init(_cond, NULL);
		ES_ASSERT(status == 0);
		status = pthread_mutex_init(_mutex, NULL);
		ES_ASSERT(status == 0);
	}
};

#endif //!WIN32


#if 1
class EParker: public PlatformParker {
public:
	DECLARE_STATIC_INITZZ;

private:
	volatile int _counter;
	EParker * FreeNext;
	EThread * AssociatedWith; // Current association

public:
	EParker() :
			PlatformParker() {
		_counter = 0;
		FreeNext = NULL;
		AssociatedWith = NULL;
	}
protected:
	~EParker() {
		//ShouldNotReachHere();
	}
public:
	// For simplicity of interface with Java, all forms of park (indefinite,
	// relative, and absolute) are multiplexed into one call.
	void park(boolean isAbsolute, llong time);
	void unpark();

	// Lifecycle operators
	static EParker * Allocate(EThread * t);
	static void Release(EParker * e);
private:
	static EParker * volatile FreeList;
	static ESpinLock* ListLock;
};
#endif

/////////////////////////////////////////////////////////////
//
// ParkEvents are type-stable and immortal.
//
// Lifecycle: Once a ParkEvent is associated with a thread that ParkEvent remains
// associated with the thread for the thread's entire lifetime - the relationship is
// stable. A thread will be associated at most one ParkEvent.  When the thread
// expires, the ParkEvent moves to the EventFreeList.  New threads attempt to allocate from
// the EventFreeList before creating a new Event.  Type-stability frees us from
// worrying about stale Event or Thread references in the objectMonitor subsystem.
// (A reference to ParkEvent is always valid, even though the event may no longer be associated
// with the desired or expected thread.  A key aspect of this design is that the callers of
// park, unpark, etc must tolerate stale references and spurious wakeups).
//
// Only the "associated" thread can block (park) on the ParkEvent, although
// any other thread can unpark a reachable parkevent.  Park() is allowed to
// return spuriously.  In fact park-unpark a really just an optimization to
// avoid unbounded spinning and surrender the CPU to be a polite system citizen.
// A degenerate albeit "impolite" park-unpark implementation could simply return.
// See http://blogs.sun.com/dave for more details.
//
// Eventually I'd like to eliminate Events and ObjectWaiters, both of which serve as
// thread proxies, and simply make the THREAD structure type-stable and persistent.
// Currently, we unpark events associated with threads, but ideally we'd just
// unpark threads.
//
// The base-class, PlatformEvent, is platform-specific while the ParkEvent is
// platform-independent.  PlatformEvent provides park(), unpark(), etc., and
// is abstract -- that is, a PlatformEvent should never be instantiated except
// as part of a ParkEvent.
// Equivalently we could have defined a platform-independent base-class that
// exported Allocate(), Release(), etc.  The platform-specific class would extend
// that base-class, adding park(), unpark(), etc.
//
// A word of caution: The JVM uses 2 very similar constructs:
// 1. ParkEvent are used for Java-level "monitor" synchronization.
// 2. Parkers are used by JSR166-JUC park-unpark.
//
// We'll want to eventually merge these redundant facilities and use ParkEvent.


class EParkEvent: public PlatformEvent {
public:
	DECLARE_STATIC_INITZZ;

private:
	EParkEvent * FreeNext;

	// Current association
	EThread * AssociatedWith;
	intptr_t RawThreadIdentity;        // LWPID etc
	volatile int Incarnation;

	// diagnostic : keep track of last thread to wake this thread.
	// this is useful for construction of dependency graphs.
	void * LastWaker;

public:
	// MCS-CLH list linkage and Native Mutex/Monitor
	EParkEvent * volatile ListNext;
	EParkEvent * volatile ListPrev;
	volatile intptr_t OnList;
	volatile int TState;
	volatile int Notified;             // for native monitor construct
	volatile int IsWaiting;            // Enqueued on WaitSet

private:
	static EParkEvent * volatile FreeList;
	static ESpinLock* ListLock;

	// It's prudent to mark the dtor as "private"
	// ensuring that it's not visible outside the package.
	// Unfortunately gcc warns about such usage, so
	// we revert to the less desirable "protected" visibility.
	// The other compilers accept private dtors.

protected:
	// Ensure dtor is never invoked
	~EParkEvent() {
	}

	EParkEvent() :
			PlatformEvent() {
		AssociatedWith = NULL;
		FreeNext = NULL;
		ListNext = NULL;
		ListPrev = NULL;
		OnList = 0;
		TState = 0;
		Notified = 0;
		IsWaiting = 0;
	}

	// We use placement-new to force ParkEvent instances to be
	// aligned on 256-byte address boundaries.  This ensures that the least
	// significant byte of a ParkEvent address is always 0.

	void * operator new(size_t sz);
	void operator delete(void * a);

public:
	static EParkEvent * Allocate(EThread * t);
	static void Release(EParkEvent * e);
};

} /* namespace efc */
#endif /* EPARK_HH_ */
