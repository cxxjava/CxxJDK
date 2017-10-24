/*
 * EOrderAccess.hh
 *
 *  Created on: 2014-10-7
 *      Author: cxxjava@163.com
 */

#ifndef EORDERACCESS_HH_
#define EORDERACCESS_HH_

#include "../../EBase.hh"

namespace efc {

//                Memory Access Ordering Model
//
// This interface is based on the JSR-133 Cookbook for Compiler Writers
// and on the IA64 memory model.  It is the dynamic equivalent of the
// C/C++ volatile specifier.  I.e., volatility restricts compile-time
// memory access reordering in a way similar to what we want to occur
// at runtime.
//
// In the following, the terms 'previous', 'subsequent', 'before',
// 'after', 'preceding' and 'succeeding' refer to program order.  The
// terms 'down' and 'below' refer to forward load or store motion
// relative to program order, while 'up' and 'above' refer to backward
// motion.
//
//
// We define four primitive memory barrier operations.
//
// LoadLoad:   Load1(s); LoadLoad; Load2
//
// Ensures that Load1 completes (obtains the value it loads from memory)
// before Load2 and any subsequent load operations.  Loads before Load1
// may *not* float below Load2 and any subsequent load operations.
//
// StoreStore: Store1(s); StoreStore; Store2
//
// Ensures that Store1 completes (the effect on memory of Store1 is made
// visible to other processors) before Store2 and any subsequent store
// operations.  Stores before Store1 may *not* float below Store2 and any
// subsequent store operations.
//
// LoadStore:  Load1(s); LoadStore; Store2
//
// Ensures that Load1 completes before Store2 and any subsequent store
// operations.  Loads before Load1 may *not* float below Store2 and any
// subseqeuent store operations.
//
// StoreLoad:  Store1(s); StoreLoad; Load2
//
// Ensures that Store1 completes before Load2 and any subsequent load
// operations.  Stores before Store1 may *not* float below Load2 and any
// subseqeuent load operations.
//
//
// We define two further operations, 'release' and 'acquire'.  They are
// mirror images of each other.
//
// Execution by a processor of release makes the effect of all memory
// accesses issued by it previous to the release visible to all
// processors *before* the release completes.  The effect of subsequent
// memory accesses issued by it *may* be made visible *before* the
// release.  I.e., subsequent memory accesses may float above the
// release, but prior ones may not float below it.
//
// Execution by a processor of acquire makes the effect of all memory
// accesses issued by it subsequent to the acquire visible to all
// processors *after* the acquire completes.  The effect of prior memory
// accesses issued by it *may* be made visible *after* the acquire.
// I.e., prior memory accesses may float below the acquire, but
// subsequent ones may not float above it.
//
// Finally, we define a 'fence' operation, which conceptually is a
// release combined with an acquire.  In the real world these operations
// require one or more machine instructions which can float above and
// below the release or acquire, so we usually can't just issue the
// release-acquire back-to-back.  All machines we know of implement some
// sort of memory fence instruction.
//
//
// The standalone implementations of release and acquire need an associated
// dummy volatile store or load respectively.  To avoid redundant operations,
// we can define the composite operators: 'release_store', 'store_fence' and
// 'load_acquire'.  Here's a summary of the machine instructions corresponding
// to each operation.
//
//               sparc RMO             ia64             x86
// ---------------------------------------------------------------------
// fence         membar #LoadStore |   mf               lock addl 0,(sp)
//                      #StoreStore |
//                      #LoadLoad |
//                      #StoreLoad
//
// release       membar #LoadStore |   st.rel [sp]=r0   movl $0,<dummy>
//                      #StoreStore
//               st %g0,[]
//
// acquire       ld [%sp],%g0          ld.acq <r>=[sp]  movl (sp),<r>
//               membar #LoadLoad |
//                      #LoadStore
//
// release_store membar #LoadStore |   st.rel           <store>
//                      #StoreStore
//               st
//
// store_fence   st                    st               lock xchg
//               fence                 mf
//
// load_acquire  ld                    ld.acq           <load>
//               membar #LoadLoad |
//                      #LoadStore
//
// Using only release_store and load_acquire, we can implement the
// following ordered sequences.
//
// 1. load, load   == load_acquire,  load
//                 or load_acquire,  load_acquire
// 2. load, store  == load,          release_store
//                 or load_acquire,  store
//                 or load_acquire,  release_store
// 3. store, store == store,         release_store
//                 or release_store, release_store
//
// These require no membar instructions for sparc-TSO and no extra
// instructions for ia64.
//
// Ordering a load relative to preceding stores requires a store_fence,
// which implies a membar #StoreLoad between the store and load under
// sparc-TSO.  A fence is required by ia64.  On x86, we use locked xchg.
//
// 4. store, load  == store_fence, load
//
// Use store_fence to make sure all stores done in an 'interesting'
// region are made visible prior to both subsequent loads and stores.
//
// Conventional usage is to issue a load_acquire for ordered loads.  Use
// release_store for ordered stores when you care only that prior stores
// are visible before the release_store, but don't care exactly when the
// store associated with the release_store becomes visible.  Use
// release_store_fence to update values like the thread state, where we
// don't want the current thread to continue until all our prior memory
// accesses (including the new thread state) are visible to other threads.
//
//
//                C++ Volatility
//
// C++ guarantees ordering at operations termed 'sequence points' (defined
// to be volatile accesses and calls to library I/O functions).  'Side
// effects' (defined as volatile accesses, calls to library I/O functions
// and object modification) previous to a sequence point must be visible
// at that sequence point.  See the C++ standard, section 1.9, titled
// "Program Execution".  This means that all barrier implementations,
// including standalone loadload, storestore, loadstore, storeload, acquire
// and release must include a sequence point, usually via a volatile memory
// access.  Other ways to guarantee a sequence point are, e.g., use of
// indirect calls and linux's __asm__ volatile.
// Note: as of 6973570, we have replaced the originally static "dummy" field
// (see above) by a volatile store to the stack. All of the versions of the
// compilers that we currently use (SunStudio, gcc and VC++) respect the
// semantics of volatile here. If you build HotSpot using other
// compilers, you may need to verify that no compiler reordering occurs
// across the sequence point respresented by the volatile access.
//
//
//                os::is_MP Considered Redundant
//
// Callers of this interface do not need to test os::is_MP() before
// issuing an operation. The test is taken care of by the implementation
// of the interface (depending on the vm version and platform, the test
// may or may not be actually done by the implementation).
//
//
//                A Note on Memory Ordering and Cache Coherency
//
// Cache coherency and memory ordering are orthogonal concepts, though they
// interact.  E.g., all existing itanium machines are cache-coherent, but
// the hardware can freely reorder loads wrt other loads unless it sees a
// load-acquire instruction.  All existing sparc machines are cache-coherent
// and, unlike itanium, TSO guarantees that the hardware orders loads wrt
// loads and stores, and stores wrt to each other.
//
// Consider the implementation of loadload.  *If* your platform *isn't*
// cache-coherent, then loadload must not only prevent hardware load
// instruction reordering, but it must *also* ensure that subsequent
// loads from addresses that could be written by other processors (i.e.,
// that are broadcast by other processors) go all the way to the first
// level of memory shared by those processors and the one issuing
// the loadload.
//
// So if we have a MP that has, say, a per-processor D$ that doesn't see
// writes by other processors, and has a shared E$ that does, the loadload
// barrier would have to make sure that either
//
// 1. cache lines in the issuing processor's D$ that contained data from
// addresses that could be written by other processors are invalidated, so
// subsequent loads from those addresses go to the E$, (it could do this
// by tagging such cache lines as 'shared', though how to tell the hardware
// to do the tagging is an interesting problem), or
//
// 2. there never are such cache lines in the issuing processor's D$, which
// means all references to shared data (however identified: see above)
// bypass the D$ (i.e., are satisfied from the E$).
//
// If your machine doesn't have an E$, substitute 'main memory' for 'E$'.
//
// Either of these alternatives is a pain, so no current machine we know of
// has incoherent caches.
//
// If loadload didn't have these properties, the store-release sequence for
// publishing a shared data structure wouldn't work, because a processor
// trying to read data newly published by another processor might go to
// its own incoherent caches to satisfy the read instead of to the newly
// written shared memory.
//
//
//                NOTE WELL!!
//
//                A Note on MutexLocker and Friends
//
// See mutexLocker.hpp.  We assume throughout the VM that MutexLocker's
// and friends' constructors do a fence, a lock and an acquire *in that
// order*.  And that their destructors do a release and unlock, in *that*
// order.  If their implementations change such that these assumptions
// are violated, a whole lot of code will break.

class EOrderAccess {
public:
	static void loadload();
	static void storestore();
	static void loadstore();
	static void storeload();

	static void acquire();
	static void release();
	static void fence();

	static byte    load_acquire(volatile byte*   p);
	static short   load_acquire(volatile short*  p);
	static int     load_acquire(volatile int*    p);
	static llong    load_acquire(volatile llong*   p);
	static ubyte   load_acquire(volatile ubyte*  p);
	static ushort  load_acquire(volatile ushort* p);
	static uint    load_acquire(volatile uint*   p);
	static ullong   load_acquire(volatile ullong*  p);
	static float   load_acquire(volatile float*  p);
	static double  load_acquire(volatile double* p);

	static es_intptr_t load_ptr_acquire(volatile es_intptr_t*   p);
	static void*    load_ptr_acquire(volatile void*       p);
	static void*    load_ptr_acquire(const volatile void* p);

	static void     release_store(volatile byte*   p, byte   v);
	static void     release_store(volatile short*  p, short  v);
	static void     release_store(volatile int*    p, int    v);
	static void     release_store(volatile llong*   p, llong   v);
	static void     release_store(volatile ubyte*  p, ubyte  v);
	static void     release_store(volatile ushort* p, ushort v);
	static void     release_store(volatile uint*   p, uint   v);
	static void     release_store(volatile ullong*  p, ullong  v);
	static void     release_store(volatile float*  p, float  v);
	static void     release_store(volatile double* p, double v);

	static void     release_store_ptr(volatile es_intptr_t* p, es_intptr_t v);
	static void     release_store_ptr(volatile void*     p, void*    v);

	static void     store_fence(byte*   p, byte   v);
	static void     store_fence(short*  p, short  v);
	static void     store_fence(int*    p, int    v);
	static void     store_fence(llong*   p, llong   v);
	static void     store_fence(ubyte*  p, ubyte  v);
	static void     store_fence(ushort* p, ushort v);
	static void     store_fence(uint*   p, uint   v);
	static void     store_fence(ullong*  p, ullong  v);
	static void     store_fence(float*  p, float  v);
	static void     store_fence(double* p, double v);

	static void     store_ptr_fence(es_intptr_t* p, es_intptr_t v);
	static void     store_ptr_fence(void**    p, void*    v);

	static void     release_store_fence(volatile byte*   p, byte   v);
	static void     release_store_fence(volatile short*  p, short  v);
	static void     release_store_fence(volatile int*    p, int    v);
	static void     release_store_fence(volatile llong*   p, llong   v);
	static void     release_store_fence(volatile ubyte*  p, ubyte  v);
	static void     release_store_fence(volatile ushort* p, ushort v);
	static void     release_store_fence(volatile uint*   p, uint   v);
	static void     release_store_fence(volatile ullong*  p, ullong  v);
	static void     release_store_fence(volatile float*  p, float  v);
	static void     release_store_fence(volatile double* p, double v);

	static void     release_store_ptr_fence(volatile es_intptr_t* p, es_intptr_t v);
	static void     release_store_ptr_fence(volatile void*     p, void*    v);
};

} /* namespace efc */
#endif /* EORDERACCESS_HH_ */
