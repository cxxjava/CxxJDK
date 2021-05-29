/*
 * EAtomic.cpp
 *
 *  Created on: 2014-10-9
 *      Author: cxxjava@163.com
 */

#include "../../inc/concurrent/EAtomic.hh"
#include "../../inc/EOS.hh"

namespace efc {

DEFINE_STATIC_INITZZ_BEGIN(EAtomic)
EOS::_initzz_();
DEFINE_STATIC_INITZZ_END

byte EAtomic::cmpxchg(byte exchange_value, volatile byte* dest,
		byte compare_value) {
	ES_ASSERT(sizeof(byte) == 1);
	es_uintptr_t dest_addr = (es_uintptr_t) dest;
	es_uintptr_t offset = dest_addr % sizeof(int);
	volatile int* dest_int = (volatile int*) (dest_addr - offset);
	int cur = *dest_int;
	byte* cur_as_bytes = (byte*) (&cur);
	int new_val = cur;
	byte* new_val_as_bytes = (byte*) (&new_val);
	new_val_as_bytes[offset] = exchange_value;
	while (cur_as_bytes[offset] == compare_value) {
		int res = cmpxchg32(new_val, dest_int, cur);
		if (res == cur)
			break;
		cur = res;
		new_val = cur;
		new_val_as_bytes[offset] = exchange_value;
	}
	return cur_as_bytes[offset];
}

#ifdef WIN32

void EAtomic::store    (byte    store_value, byte*    dest) { *dest = store_value; }
void EAtomic::store    (short   store_value, short*   dest) { *dest = store_value; }
void EAtomic::store    (int     store_value, int*     dest) { *dest = store_value; }

void EAtomic::store_ptr(es_intptr_t store_value, es_intptr_t* dest) { *dest = store_value; }
void EAtomic::store_ptr(void*   store_value, void*    dest) { *(void**)dest = store_value; }

void EAtomic::store    (byte    store_value, volatile byte*    dest) { *dest = store_value; }
void EAtomic::store    (short   store_value, volatile short*   dest) { *dest = store_value; }
void EAtomic::store    (int     store_value, volatile int*     dest) { *dest = store_value; }


void EAtomic::store_ptr(es_intptr_t store_value, volatile es_intptr_t* dest) { *dest = store_value; }
void EAtomic::store_ptr(void*    store_value, volatile void*     dest) { *(void* volatile *)dest = store_value; }

// Adding a lock prefix to an instruction on MP machine
// VC++ doesn't like the lock prefix to be on a single line
// so we can't insert a label after the lock prefix.
// By emitting a lock prefix, we can define a label after it.
#define LOCK_IF_MP(mp) __asm cmp mp, 0  \
                       __asm je L0      \
                       __asm _emit 0xF0 \
                       __asm L0:

#ifdef AMD64

void EAtomic::store    (llong    store_value, llong*    dest) { *dest = store_value; }
void EAtomic::store    (llong    store_value, volatile llong* dest) { *dest = store_value; }

int EAtomic::add(int add_value, volatile int* dest) {
	return eso_atomic_add_and_fetch32(dest, add_value);
}

es_intptr_t EAtomic::add_ptr(es_intptr_t add_value, volatile es_intptr_t* dest) {
	return eso_atomic_add_and_fetch64(dest, add_value);
}

void* EAtomic::add_ptr(es_intptr_t add_value, volatile void* dest) {
	return eso_atomic_add_and_fetch64(dest, add_value);
}

void EAtomic::inc(volatile int* dest) {
	(void) EAtomic::add(1, dest);
}

void EAtomic::inc_ptr(volatile es_intptr_t* dest) {
	(void) EAtomic::add_ptr(1, dest);
}

void EAtomic::inc_ptr(volatile void* dest) {
	(void) EAtomic::add_ptr(1, dest);
}

void EAtomic::dec(volatile int* dest) {
	(void) EAtomic::add(-1, dest);
}

void EAtomic::dec_ptr(volatile es_intptr_t* dest) {
	(void) EAtomic::add_ptr(-1, dest);
}

void EAtomic::dec_ptr(volatile void* dest) {
	(void) EAtomic::add_ptr(-1, dest);
}

int EAtomic::xchg32(int exchange_value, volatile int* dest) {
	return eso_atomic_test_and_set32(dest, exchange_value);
}

es_intptr_t EAtomic::xchg_ptr(es_intptr_t exchange_value,
		volatile es_intptr_t* dest) {
	return eso_atomic_test_and_set64(dest, exchange_value);
}

void* EAtomic::xchg_ptr(void* exchange_value, volatile void* dest) {
	return eso_atomic_test_and_set64(dest, exchange_value);
}

int EAtomic::cmpxchg32(int exchange_value, volatile int* dest,
		int compare_value) {
	return eso_atomic_compare_and_swap32(dest, compare_value, exchange_value) ? compare_value : exchange_value;
}

llong EAtomic::cmpxchg64(llong exchange_value, volatile llong* dest,
		llong compare_value) {
	return eso_atomic_compare_and_swap64(dest, compare_value, exchange_value) ? compare_value : exchange_value;
}

es_intptr_t EAtomic::cmpxchg_ptr(es_intptr_t exchange_value, volatile es_intptr_t* dest, es_intptr_t compare_value) {
  return (es_intptr_t)EAtomic::cmpxchg64((llong)exchange_value, (volatile llong*)dest, (llong)compare_value);
}

void* EAtomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)EAtomic::cmpxchg64((llong)exchange_value, (volatile llong*)dest, (llong)compare_value);
}

llong EAtomic::load(volatile llong* src) {
	return *src;
}

#else //

int EAtomic::add(int add_value, volatile int* dest) {
	int mp = EOS::is_MP();
	__asm {
		mov edx, dest;
		mov eax, add_value;
		mov ecx, eax;
		LOCK_IF_MP(mp)
		xadd dword ptr [edx], eax;
		add eax, ecx;
	}
	//return eso_atomic_add_and_fetch32(dest, add_value);
}

es_intptr_t EAtomic::add_ptr(es_intptr_t add_value, volatile es_intptr_t* dest) {
  return (es_intptr_t)EAtomic::add((int)add_value, (volatile int*)dest);
}

void* EAtomic::add_ptr(es_intptr_t add_value, volatile void* dest) {
  return (void*)EAtomic::add((int)add_value, (volatile int*)dest);
}

void EAtomic::inc(volatile int* dest) {
	// alternative for InterlockedIncrement
	int mp = EOS::is_MP();
	__asm {
		mov edx, dest;
		LOCK_IF_MP(mp)
		add dword ptr [edx], 1;
	}
}

void EAtomic::inc_ptr(volatile es_intptr_t* dest) {
  inc((volatile int*)dest);
}

void EAtomic::inc_ptr(volatile void* dest) {
  inc((volatile int*)dest);
}

void EAtomic::dec(volatile int* dest) {
// alternative for InterlockedDecrement
	int mp = EOS::is_MP();
	__asm {
		mov edx, dest;
		LOCK_IF_MP(mp)
		sub dword ptr [edx], 1;
	}
}

void EAtomic::dec_ptr(volatile es_intptr_t* dest) {
	EAtomic::dec((volatile int*)dest);
}

void EAtomic::dec_ptr(volatile void* dest) {
	EAtomic::dec((volatile int*)dest);
}

int EAtomic::xchg32(int exchange_value, volatile int* dest) {
	// alternative for InterlockedExchange
	__asm {
		mov eax, exchange_value;
		mov ecx, dest;
		xchg eax, dword ptr [ecx];
	}
}

es_intptr_t EAtomic::xchg_ptr(es_intptr_t exchange_value, volatile es_intptr_t* dest) {
  return (es_intptr_t)EAtomic::xchg32((int)exchange_value, (volatile int*)dest);
}

void* EAtomic::xchg_ptr(void* exchange_value, volatile void* dest) {
  return (void*)EAtomic::xchg32((int)exchange_value, (volatile int*)dest);
}

int EAtomic::cmpxchg32(int exchange_value, volatile int* dest, int compare_value) {
	// alternative for InterlockedCompareExchange
	int mp = EOS::is_MP();
	__asm {
		mov edx, dest
		mov ecx, exchange_value
		mov eax, compare_value
		LOCK_IF_MP(mp)
		cmpxchg dword ptr [edx], ecx
	}
}

llong EAtomic::cmpxchg64(llong exchange_value, volatile llong* dest, llong compare_value) {
	int mp = EOS::is_MP();
	int ex_lo  = (int)exchange_value;
	int ex_hi  = *( ((int*)&exchange_value) + 1 );
	int cmp_lo = (int)compare_value;
	int cmp_hi = *( ((int*)&compare_value) + 1 );
	__asm {
		push ebx
		push edi
		mov eax, cmp_lo
		mov edx, cmp_hi
		mov edi, dest
		mov ebx, ex_lo
		mov ecx, ex_hi
		LOCK_IF_MP(mp)
		cmpxchg8b qword ptr [edi]
		pop edi
		pop ebx
	}
}

es_intptr_t EAtomic::cmpxchg_ptr(es_intptr_t exchange_value, volatile es_intptr_t* dest, es_intptr_t compare_value) {
  return (es_intptr_t)EAtomic::cmpxchg32((int)exchange_value, (volatile int*)dest, (int)compare_value);
}

void* EAtomic::cmpxchg_ptr(void*    exchange_value, volatile void*     dest, void*    compare_value) {
  return (void*)EAtomic::cmpxchg32((int)exchange_value, (volatile int*)dest, (int)compare_value);
}

llong EAtomic::load(volatile llong* src) {
	volatile llong dest;
	volatile llong* pdest = &dest;
	__asm {
		mov eax, src
		fild     qword ptr [eax]
		mov eax, pdest
		fistp    qword ptr [eax]
	}
	return dest;
}

void EAtomic::store(llong store_value, volatile llong* dest) {
	volatile llong* src = &store_value;
	__asm {
		mov eax, src
		fild     qword ptr [eax]
		mov eax, dest
		fistp    qword ptr [eax]
	}
}

void EAtomic::store(llong store_value, llong* dest) {
	EAtomic::store(store_value, (volatile llong*)dest);
}
#endif //!AMD64

#else //WIN32

// Adding a lock prefix to an instruction on MP machine
#define LOCK_IF_MP(mp) "cmp $0, " #mp "; je 1f; lock; 1: "

void EAtomic::store(byte store_value, byte* dest) {
	*dest = store_value;
}

void EAtomic::store(short store_value, short * dest) {
	*dest = store_value;
}

void EAtomic::store(int store_value, int* dest) {
	*dest = store_value;
}

void EAtomic::store_ptr(es_intptr_t store_value, es_intptr_t* dest) {
	*dest = store_value;
}

void EAtomic::store_ptr(void* store_value, void* dest) {
	*(void**)dest = store_value;
}

void EAtomic::store(byte store_value, volatile byte* dest) {
	*dest = store_value;
}

void EAtomic::store(short store_value, volatile short * dest) {
	*dest = store_value;
}

void EAtomic::store(int store_value, volatile int* dest) {
	*dest = store_value;
}

void EAtomic::store_ptr(es_intptr_t store_value, volatile es_intptr_t* dest) {
	*dest = store_value;
}

void EAtomic::store_ptr(void* store_value, volatile void* dest) {
	*(void* volatile *)dest = store_value;
}

void EAtomic::dec(volatile int* dest) {
	int mp = EOS::is_MP();
	__asm__ volatile (LOCK_IF_MP(%1) "subl $1,(%0)" :
	                  : "r" (dest), "r" (mp) : "cc", "memory");
}

int EAtomic::add(int add_value, volatile int* dest) {
	int addend = add_value;
	int mp = EOS::is_MP();
	__asm__ volatile (  LOCK_IF_MP(%3) "xaddl %0,(%2)"
					: "=r" (addend)
					: "0" (addend), "r" (dest), "r" (mp)
					: "cc", "memory");
	return addend + add_value;
}

void EAtomic::inc(volatile int* dest) {
	int mp = EOS::is_MP();
	__asm__ volatile (LOCK_IF_MP(%1) "addl $1,(%0)" :
	                  : "r" (dest), "r" (mp) : "cc", "memory");
}

void EAtomic::inc_ptr(volatile void* dest) {
	EAtomic::inc_ptr((volatile es_intptr_t*)dest);
}

void EAtomic::dec_ptr(volatile void* dest) {
	EAtomic::dec_ptr((volatile es_intptr_t*)dest);
}

int EAtomic::xchg32(int exchange_value, volatile int* dest) {
	//@see: jdk7/hotspot/src/share/vm/runtime/atomic.cpp
	__asm__ volatile (  "xchgl (%2),%0"
					: "=r" (exchange_value)
					: "0" (exchange_value), "r" (dest)
					: "memory");
	return exchange_value;

//	return eso_atomic_test_and_set32(dest, exchange_value);
}

void* EAtomic::xchg_ptr(void* exchange_value, volatile void* dest) {
	return (void*)EAtomic::xchg_ptr((es_intptr_t)exchange_value, (volatile es_intptr_t*)dest);
}

es_int32_t EAtomic::cmpxchg32(es_int32_t exchange_value, volatile es_int32_t* dest,
		es_int32_t compare_value) {
	//@see: jdk7/hotspot/src/share/vm/runtime/atomic.cpp
	int mp = EOS::is_MP();
	__asm__ volatile (LOCK_IF_MP(%4) "cmpxchgl %1,(%3)"
					: "=a" (exchange_value)
					: "r" (exchange_value), "a" (compare_value), "r" (dest), "r" (mp)
					: "cc", "memory");
	return exchange_value;

//	return eso_atomic_compare_and_swap32(dest, compare_value, exchange_value) ? compare_value : exchange_value;
}

es_int64_t EAtomic::cmpxchg64(es_int64_t exchange_value,
					volatile es_int64_t* dest, es_int64_t compare_value) {
#if (SIZEOF_LONG == 8)
	//@see: jdk7/hotspot/src/share/vm/runtime/atomic.cpp
	bool mp = EOS::is_MP();
	__asm__ __volatile__ (LOCK_IF_MP(%4) "cmpxchgq %1,(%3)"
						: "=a" (exchange_value)
						: "r" (exchange_value), "a" (compare_value), "r" (dest), "r" (mp)
						: "cc", "memory");
	return exchange_value;
#else
	//sync to ubuntu x86
	return eso_atomic_compare_and_swap64(dest, compare_value, exchange_value) ? compare_value : exchange_value;
#endif
}

#ifdef AMD64

void EAtomic::store(llong store_value, llong* dest) {
	*dest = store_value;
}

void EAtomic::store(llong store_value, volatile llong* dest) {
	*dest = store_value;
}

llong EAtomic::load(volatile llong* src) {
	return *src;
}

es_intptr_t EAtomic::add_ptr(es_intptr_t add_value,
		volatile es_intptr_t* dest) {
	es_intptr_t addend = add_value;
	bool mp = EOS::is_MP();
	__asm__ __volatile__ (LOCK_IF_MP(%3) "xaddq %0,(%2)"
						: "=r" (addend)
						: "0" (addend), "r" (dest), "r" (mp)
						: "cc", "memory");
	return addend + add_value;
}

void* EAtomic::add_ptr(es_intptr_t add_value, volatile void* dest) {
	return (void*)EAtomic::add_ptr(add_value, (volatile es_intptr_t*)dest);
}

void EAtomic::inc_ptr(volatile es_intptr_t* dest) {
	bool mp = EOS::is_MP();
	__asm__ __volatile__ (LOCK_IF_MP(%1) "addq $1,(%0)"
	                        :
	                        : "r" (dest), "r" (mp)
	                        : "cc", "memory");
}

void EAtomic::dec_ptr(volatile es_intptr_t* dest) {
	bool mp = EOS::is_MP();
	__asm__ __volatile__ (LOCK_IF_MP(%1) "subq $1,(%0)"
	                        :
	                        : "r" (dest), "r" (mp)
	                        : "cc", "memory");
}

es_intptr_t EAtomic::xchg_ptr(es_intptr_t exchange_value,
		volatile es_intptr_t* dest) {
	__asm__ __volatile__ ("xchgq (%2),%0"
	                        : "=r" (exchange_value)
	                        : "0" (exchange_value), "r" (dest)
	                        : "memory");
	return exchange_value;
}

es_intptr_t EAtomic::cmpxchg_ptr(es_intptr_t exchange_value,
		volatile es_intptr_t* dest, es_intptr_t compare_value) {
	return (es_intptr_t) EAtomic::cmpxchg_ptr((es_intptr_t) exchange_value, (volatile es_intptr_t*) dest, (es_intptr_t) compare_value);
}

void* EAtomic::cmpxchg_ptr(void* exchange_value,
		volatile void* dest, void* compare_value) {
	return (void*) EAtomic::cmpxchg64((es_int64_t) exchange_value, (volatile es_int64_t*) dest, (es_int64_t) compare_value);
}

#else //AMD64

static void _Atomic_move_long(volatile llong* src, volatile llong* dst) {
	/**
	//@see: hotspot/src/os_cpu/linux_x86/vm/linux_x86_32.s
	# Support for jlong Atomic::load and Atomic::store.
	        # void _Atomic_move_long(volatile jlong* src, volatile jlong* dst)
	        .p2align 4,,15
		.type    _Atomic_move_long,@function
	_Atomic_move_long:
	        movl     4(%esp), %eax   # src
	        fildll    (%eax)
	        movl     8(%esp), %eax   # dest
	        fistpll   (%eax)
	        ret
	 */
	eso_atomic_test_and_set64(dst, *src); // is right ???
}

void EAtomic::store(llong store_value, llong* dest) {
	_Atomic_move_long((volatile llong*)&store_value, (volatile llong*)dest);
}

void EAtomic::store(llong store_value, volatile llong* dest) {
	_Atomic_move_long((volatile llong*)&store_value, dest);
}

llong EAtomic::load(volatile llong* src) {
	volatile llong dest;
	_Atomic_move_long(src, &dest);
	return dest;
}

es_intptr_t EAtomic::add_ptr(es_intptr_t add_value,
		volatile es_intptr_t* dest) {
	return (es_intptr_t)EAtomic::add((int)add_value, (volatile int*)dest);
}

void* EAtomic::add_ptr(es_intptr_t add_value, volatile void* dest) {
	return (void*)EAtomic::add((int)add_value, (volatile int*)dest);
}

void EAtomic::inc_ptr(volatile es_intptr_t* dest) {
	EAtomic::inc((volatile int*)dest);
}

void EAtomic::dec_ptr(volatile es_intptr_t* dest) {
	EAtomic::dec((volatile int*)dest);
}

es_intptr_t EAtomic::xchg_ptr(es_intptr_t exchange_value,
		volatile es_intptr_t* dest) {
	return (es_intptr_t)xchg32((int)exchange_value, (volatile int*)dest);
}

es_intptr_t EAtomic::cmpxchg_ptr(es_intptr_t exchange_value,
		volatile es_intptr_t* dest, es_intptr_t compare_value) {
	return (es_intptr_t)EAtomic::cmpxchg_ptr((void*)exchange_value, (volatile void*)dest,
			(void*)compare_value);
}

void* EAtomic::cmpxchg_ptr(void* exchange_value,
		volatile void* dest, void* compare_value) {
#if (SIZEOF_LONG == 8)
	return (void*) cmpxchg64((es_intptr_t) exchange_value, (volatile es_intptr_t*) dest,
					(es_intptr_t) compare_value);
#else
	return (void*) cmpxchg32((es_intptr_t) exchange_value, (volatile es_intptr_t*) dest,
				(es_intptr_t) compare_value);
#endif
}

#endif //!AMD64

#endif //!WIN32

} /* namespace efc */
