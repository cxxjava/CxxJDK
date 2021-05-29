/*
 * EOrderAccess.cpp
 *
 *  Created on: 2014-10-7
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EOrderAccess.hh"
#include "../../inc/concurrent/EAtomic.hh"
#include "../../inc/EOS.hh"

namespace efc {

#ifdef WIN32

//@see: hotspot/src/os_cpu/windows_x86/vm/orderAccess_windows_x86.inline.hpp

void EOrderAccess::loadload() {
	acquire();
}

void EOrderAccess::storestore() {
	release();
}

void EOrderAccess::loadstore() {
	acquire();
}

void EOrderAccess::storeload() {
	fence();
}

void EOrderAccess::acquire() {
#ifndef AMD64
  __asm {
    mov eax, dword ptr [esp];
  }
#endif // !AMD64
}

void EOrderAccess::release() {
	// A volatile store has release semantics.
	volatile int local_dummy = 0;
}

void EOrderAccess::fence() {
#ifdef AMD64
  StubRoutines_fence();
#else
  if (EOS::is_MP()) {
    __asm {
      lock add dword ptr [esp], 0;
    }
  }
#endif // AMD64
}

byte    EOrderAccess::load_acquire(volatile byte*   p) { return *p; }
short   EOrderAccess::load_acquire(volatile short*  p) { return *p; }
int     EOrderAccess::load_acquire(volatile int*    p) { return *p; }
llong   EOrderAccess::load_acquire(volatile llong*   p) { return EAtomic::load(p); }
ubyte   EOrderAccess::load_acquire(volatile ubyte*  p) { return *p; }
ushort  EOrderAccess::load_acquire(volatile ushort* p) { return *p; }
uint    EOrderAccess::load_acquire(volatile uint*   p) { return *p; }
ullong  EOrderAccess::load_acquire(volatile ullong*  p) { return EAtomic::load((volatile llong*)p); }
float   EOrderAccess::load_acquire(volatile float*  p) { return *p; }
double  EOrderAccess::load_acquire(volatile double* p) { return *p; }

es_intptr_t EOrderAccess::load_ptr_acquire(volatile es_intptr_t*   p) { return *p; }
void*    EOrderAccess::load_ptr_acquire(volatile void*       p) { return *(void* volatile *)p; }
void*    EOrderAccess::load_ptr_acquire(const volatile void* p) { return *(void* const volatile *)p; }

void     EOrderAccess::release_store(volatile byte*   p, byte   v) { *p = v; }
void     EOrderAccess::release_store(volatile short*  p, short  v) { *p = v; }
void     EOrderAccess::release_store(volatile int*    p, int    v) { *p = v; }
void     EOrderAccess::release_store(volatile llong*   p, llong   v) { EAtomic::store(v, p); }
void     EOrderAccess::release_store(volatile ubyte*  p, ubyte  v) { *p = v; }
void     EOrderAccess::release_store(volatile ushort* p, ushort v) { *p = v; }
void     EOrderAccess::release_store(volatile uint*   p, uint   v) { *p = v; }
void     EOrderAccess::release_store(volatile ullong*  p, ullong  v) { EAtomic::store((llong)v, (volatile llong*)p); }
void     EOrderAccess::release_store(volatile float*  p, float  v) { *p = v; }
void     EOrderAccess::release_store(volatile double* p, double v) { *p = v; }

void     EOrderAccess::release_store_ptr(volatile es_intptr_t* p, es_intptr_t v) { *p = v; }
void     EOrderAccess::release_store_ptr(volatile void*     p, void*    v) { *(void* volatile *)p = v; }

void     EOrderAccess::store_fence(byte*  p, byte  v) {
#ifdef AMD64
	*p = v; fence();
#else
	__asm {
		mov edx, p;
		mov al, v;
		xchg al, BYTE PTR [edx];
	}
#endif // AMD64
}

void     EOrderAccess::store_fence(short* p, short v) {
#ifdef AMD64
	*p = v; fence();
#else
	__asm {
		mov edx, p;
		mov ax, v;
		xchg ax, word ptr [edx];
	}
#endif // AMD64
}

void     EOrderAccess::store_fence(int*   p, int   v) {
#ifdef AMD64
	*p = v; fence();
#else
	__asm {
		mov edx, p;
		mov eax, v;
		xchg eax, dword ptr [edx];
	}
#endif // AMD64
}

void     EOrderAccess::store_fence(llong*   p, llong   v) { *p = v; fence(); }
void     EOrderAccess::store_fence(ubyte*  p, ubyte  v) { store_fence((byte*)p,  (byte)v);  }
void     EOrderAccess::store_fence(ushort* p, ushort v) { store_fence((short*)p, (short)v); }
void     EOrderAccess::store_fence(uint*   p, uint   v) { store_fence((int*)p,   (int)v);   }
void     EOrderAccess::store_fence(ullong*  p, ullong  v) { store_fence((llong*)p,  (llong)v);  }
void     EOrderAccess::store_fence(float*  p, float  v) { *p = v; fence(); }
void     EOrderAccess::store_fence(double* p, double v) { *p = v; fence(); }

void     EOrderAccess::store_ptr_fence(es_intptr_t* p, es_intptr_t v) {
#ifdef AMD64
	*p = v; fence();
#else
	store_fence((int*)p, (int)v);
#endif // AMD64
}

void     EOrderAccess::store_ptr_fence(void**    p, void*    v) {
#ifdef AMD64
	*p = v; fence();
#else
	store_fence((int*)p, (int)v);
#endif // AMD64
}

// Must duplicate definitions instead of calling store_fence because we don't want to cast away volatile.
void     EOrderAccess::release_store_fence(volatile byte*  p, byte  v) {
#ifdef AMD64
	*p = v; fence();
#else
	__asm {
		mov edx, p;
		mov al, v;
		xchg al, BYTE PTR [edx];
	}
#endif // AMD64
}

void     EOrderAccess::release_store_fence(volatile short* p, short v) {
#ifdef AMD64
	*p = v; fence();
#else
	__asm {
		mov edx, p;
		mov ax, v;
		xchg ax, word ptr [edx];
	}
#endif // AMD64
}

void     EOrderAccess::release_store_fence(volatile int*   p, int   v) {
#ifdef AMD64
	*p = v; fence();
#else
	__asm {
		mov edx, p;
		mov eax, v;
		xchg eax, dword ptr [edx];
	}
#endif // AMD64
}

void     EOrderAccess::release_store_fence(volatile llong*   p, llong   v) { release_store(p, v); fence(); }

void     EOrderAccess::release_store_fence(volatile ubyte*  p, ubyte  v) { release_store_fence((volatile byte*)p,  (byte)v);  }
void     EOrderAccess::release_store_fence(volatile ushort* p, ushort v) { release_store_fence((volatile short*)p, (short)v); }
void     EOrderAccess::release_store_fence(volatile uint*   p, uint   v) { release_store_fence((volatile int*)p,   (int)v);   }
void     EOrderAccess::release_store_fence(volatile ullong*  p, ullong  v) { release_store_fence((volatile llong*)p,  (llong)v);  }
void     EOrderAccess::release_store_fence(volatile float*  p, float  v) { *p = v; fence(); }
void     EOrderAccess::release_store_fence(volatile double* p, double v) { *p = v; fence(); }

void     EOrderAccess::release_store_ptr_fence(volatile es_intptr_t* p, es_intptr_t v) {
#ifdef AMD64
	*p = v; fence();
#else
	release_store_fence((volatile int*)p, (int)v);
#endif // AMD64
}

void     EOrderAccess::release_store_ptr_fence(volatile void*     p, void*    v) {
#ifdef AMD64
	*(void* volatile *)p = v; fence();
#else
	release_store_fence((volatile int*)p, (int)v);
#endif // AMD64
}

#else

//@see: hotspot/src/os_cpu/linux_x86/vm/orderAccess_linux_x86.inline.hpp

void EOrderAccess::loadload() {
	acquire();
}

void EOrderAccess::storestore() {
	release();
}

void EOrderAccess::loadstore() {
	acquire();
}

void EOrderAccess::storeload() {
	fence();
}

void EOrderAccess::acquire() {
	volatile es_intptr_t local_dummy;
#ifdef AMD64
	__asm__ volatile ("movq 0(%%rsp),%0" : "=r" (local_dummy) : : "memory");
#else
	__asm__ volatile ("movl 0(%%esp),%0" : "=r" (local_dummy) : : "memory");
#endif // AMD64
}

void EOrderAccess::release() {
	// Avoid hitting the same cache-line from
	// different threads.
	volatile int local_dummy = 0;
}

void EOrderAccess::fence() {
	if (EOS::is_MP()) {
		// always use locked addl since mfence is sometimes expensive
#ifdef AMD64
		__asm__ volatile ("lock; addl $0,0(%%rsp)" : : : "cc", "memory");
#else
		__asm__ volatile ("lock; addl $0,0(%%esp)" : : : "cc", "memory");
#endif
	}
}

byte    EOrderAccess::load_acquire(volatile byte*   p) { return *p; }
short   EOrderAccess::load_acquire(volatile short*  p) { return *p; }
int     EOrderAccess::load_acquire(volatile int*    p) { return *p; }
llong   EOrderAccess::load_acquire(volatile llong*   p) { return EAtomic::load(p); }
ubyte   EOrderAccess::load_acquire(volatile ubyte*  p) { return *p; }
ushort  EOrderAccess::load_acquire(volatile ushort* p) { return *p; }
uint    EOrderAccess::load_acquire(volatile uint*   p) { return *p; }
ullong  EOrderAccess::load_acquire(volatile ullong*  p) { return EAtomic::load((volatile llong*)p); }
float   EOrderAccess::load_acquire(volatile float*  p) { return *p; }
double  EOrderAccess::load_acquire(volatile double* p) { return *p; }

es_intptr_t EOrderAccess::load_ptr_acquire(volatile es_intptr_t*   p) { return *p; }
void*    EOrderAccess::load_ptr_acquire(volatile void*       p) { return *(void* volatile *)p; }
void*    EOrderAccess::load_ptr_acquire(const volatile void* p) { return *(void* const volatile *)p; }

void     EOrderAccess::release_store(volatile byte*   p, byte   v) { *p = v; }
void     EOrderAccess::release_store(volatile short*  p, short  v) { *p = v; }
void     EOrderAccess::release_store(volatile int*    p, int    v) { *p = v; }
void     EOrderAccess::release_store(volatile llong*   p, llong   v) { EAtomic::store(v, p); }
void     EOrderAccess::release_store(volatile ubyte*  p, ubyte  v) { *p = v; }
void     EOrderAccess::release_store(volatile ushort* p, ushort v) { *p = v; }
void     EOrderAccess::release_store(volatile uint*   p, uint   v) { *p = v; }
void     EOrderAccess::release_store(volatile ullong*  p, ullong  v) { EAtomic::store((llong)v, (volatile llong*)p); }
void     EOrderAccess::release_store(volatile float*  p, float  v) { *p = v; }
void     EOrderAccess::release_store(volatile double* p, double v) { *p = v; }

void     EOrderAccess::release_store_ptr(volatile es_intptr_t* p, es_intptr_t v) { *p = v; }
void     EOrderAccess::release_store_ptr(volatile void*     p, void*    v) { *(void* volatile *)p = v; }

void     EOrderAccess::store_fence(byte*  p, byte  v) {
  __asm__ volatile (  "xchgb (%2),%0"
                    : "=r" (v)
                    : "0" (v), "r" (p)
                    : "memory");
}
void     EOrderAccess::store_fence(short* p, short v) {
  __asm__ volatile (  "xchgw (%2),%0"
                    : "=r" (v)
                    : "0" (v), "r" (p)
                    : "memory");
}
void     EOrderAccess::store_fence(int*   p, int   v) {
  __asm__ volatile (  "xchgl (%2),%0"
                    : "=r" (v)
                    : "0" (v), "r" (p)
                    : "memory");
}

void     EOrderAccess::store_fence(llong*   p, llong   v) {
#ifdef AMD64
  __asm__ __volatile__ ("xchgq (%2), %0"
                        : "=r" (v)
                        : "0" (v), "r" (p)
                        : "memory");
#else
  *p = v; fence();
#endif // AMD64
}

// AMD64 copied the bodies for the the signed version. 32bit did this. As long as the
// compiler does the inlining this is simpler.
void     EOrderAccess::store_fence(ubyte*  p, ubyte  v) { store_fence((byte*)p,  (byte)v);  }
void     EOrderAccess::store_fence(ushort* p, ushort v) { store_fence((short*)p, (short)v); }
void     EOrderAccess::store_fence(uint*   p, uint   v) { store_fence((int*)p,   (int)v);   }
void     EOrderAccess::store_fence(ullong*  p, ullong  v) { store_fence((llong*)p,  (llong)v);  }
void     EOrderAccess::store_fence(float*  p, float  v) { *p = v; fence(); }
void     EOrderAccess::store_fence(double* p, double v) { *p = v; fence(); }

void     EOrderAccess::store_ptr_fence(es_intptr_t* p, es_intptr_t v) {
#ifdef AMD64
  __asm__ __volatile__ ("xchgq (%2), %0"
                        : "=r" (v)
                        : "0" (v), "r" (p)
                        : "memory");
#else
  store_fence((int*)p, (int)v);
#endif // AMD64
}

void     EOrderAccess::store_ptr_fence(void**    p, void*    v) {
#ifdef AMD64
  __asm__ __volatile__ ("xchgq (%2), %0"
                        : "=r" (v)
                        : "0" (v), "r" (p)
                        : "memory");
#else
  store_fence((int*)p, (int)v);
#endif // AMD64
}

// Must duplicate definitions instead of calling store_fence because we don't want to cast away volatile.
void     EOrderAccess::release_store_fence(volatile byte*  p, byte  v) {
  __asm__ volatile (  "xchgb (%2),%0"
                    : "=r" (v)
                    : "0" (v), "r" (p)
                    : "memory");
}
void     EOrderAccess::release_store_fence(volatile short* p, short v) {
  __asm__ volatile (  "xchgw (%2),%0"
                    : "=r" (v)
                    : "0" (v), "r" (p)
                    : "memory");
}
void     EOrderAccess::release_store_fence(volatile int*   p, int   v) {
  __asm__ volatile (  "xchgl (%2),%0"
                    : "=r" (v)
                    : "0" (v), "r" (p)
                    : "memory");
}

void     EOrderAccess::release_store_fence(volatile llong*   p, llong   v) {
#ifdef AMD64
  __asm__ __volatile__ (  "xchgq (%2), %0"
                          : "=r" (v)
                          : "0" (v), "r" (p)
                          : "memory");
#else
  release_store(p, v); fence();
#endif // AMD64
}

void     EOrderAccess::release_store_fence(volatile ubyte*  p, ubyte  v) { release_store_fence((volatile byte*)p,  (byte)v);  }
void     EOrderAccess::release_store_fence(volatile ushort* p, ushort v) { release_store_fence((volatile short*)p, (short)v); }
void     EOrderAccess::release_store_fence(volatile uint*   p, uint   v) { release_store_fence((volatile int*)p,   (int)v);   }
void     EOrderAccess::release_store_fence(volatile ullong*  p, ullong  v) { release_store_fence((volatile llong*)p,  (llong)v);  }

void     EOrderAccess::release_store_fence(volatile float*  p, float  v) { *p = v; fence(); }
void     EOrderAccess::release_store_fence(volatile double* p, double v) { *p = v; fence(); }

void     EOrderAccess::release_store_ptr_fence(volatile es_intptr_t* p, es_intptr_t v) {
#ifdef AMD64
  __asm__ __volatile__ (  "xchgq (%2), %0"
                          : "=r" (v)
                          : "0" (v), "r" (p)
                          : "memory");
#else
  release_store_fence((volatile int*)p, (int)v);
#endif // AMD64
}
void     EOrderAccess::release_store_ptr_fence(volatile void*     p, void*    v) {
#ifdef AMD64
  __asm__ __volatile__ (  "xchgq (%2), %0"
                          : "=r" (v)
                          : "0" (v), "r" (p)
                          : "memory");
#else
  release_store_fence((volatile int*)p, (int)v);
#endif // AMD64
}

#endif //!WIN32

} /* namespace efc */
