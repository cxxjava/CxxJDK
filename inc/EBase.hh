#ifndef __EBASE_H__
#define __EBASE_H__

#include "es_config.h"
#include "es_types.h"
#include "es_comm.h"
#include "es_status.h"
#include "eso_alogger.h"
#include "eso_array.h"
#include "eso_array_ptr.h"
#include "eso_array_int.h"
#include "eso_atomic.h"
#include "eso_base64.h"
#include "eso_bitset.h"
#include "eso_bson.h"
#include "eso_bson_ext.h"
#include "eso_buffer.h"
#include "eso_canonicalize.h"
#include "eso_conf.h"
#include "eso_crc32.h"
#include "eso_datetime.h"
#include "eso_dso.h"
#include "eso_debug.h"
#include "eso_encode.h"
#include "eso_file.h"
#include "eso_flip_buffer.h"
#include "eso_fmttime.h"
#include "eso_hash.h"
#include "eso_ini.h"
#include "eso_json.h"
#include "eso_libc.h"
#include "eso_locale_str.h"
#include "eso_lz77.h"
#include "eso_lzma.h"
#include "eso_map.h"
#include "eso_mem.h"
#include "eso_mmap.h"
#include "eso_mpool.h"
#include "eso_net.h"
#include "eso_object.h"
#include "eso_pcre.h"
#include "eso_pipe.h"
#include "eso_poll.h"
#include "eso_printf_upper_bound.h"
#include "eso_proc.h"
#include "eso_proc_mutex.h"
#include "eso_queue.h"
#include "eso_ring.h"
#include "eso_ring_buffer.h"
#include "eso_shm.h"
#include "eso_signal.h"
#include "eso_sigsegv.h"
#include "eso_stack.h"
#include "eso_string.h"
#include "eso_sys.h"
#include "eso_thread.h"
#include "eso_thread_cond.h"
#include "eso_thread_mutex.h"
#include "eso_thread_rwlock.h"
#include "eso_thread_spin.h"
#include "eso_util.h"
#include "eso_uuid.h"
#include "eso_vector.h"
#include "eso_zlib.h"

#include "eso_md4.h"
#include "eso_md5.h"
#include "eso_sha1.h"
#include "eso_sha2.h"

#ifdef HAVE_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#endif

#include <new> //need for EObject.hh
#include <typeinfo> //for typeid()
#include <algorithm> // for std::swap()

#if defined(_MSC_VER)
	#define ALWAYS_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
	#define ALWAYS_INLINE __attribute__ ((__visibility__("hidden"), __always_inline__))
#endif

#ifdef WIN32
#if _MSC_VER >= 1600 //__cplusplus >= 199711L
	#define CPP11_SUPPORT
#endif
#else
#if __cplusplus >= 201103L
	#define CPP11_SUPPORT
#endif
#endif

#ifdef CPP11_SUPPORT
#if defined( __clang__ ) && !defined( _LIBCPP_VERSION )
	typedef decltype(nullptr) es_nullptr_t;
#else
	typedef std::nullptr_t    es_nullptr_t;
#endif
#else //!
struct es_nullptr_t
{
    void* __lx;

    struct __nat {int __for_bool_;};

    ALWAYS_INLINE es_nullptr_t() : __lx(0) {}
    ALWAYS_INLINE es_nullptr_t(int __nat::*) : __lx(0) {}

    ALWAYS_INLINE operator int __nat::*() const {return 0;}

    template <class _Tp>
    	ALWAYS_INLINE
        operator _Tp* () const {return 0;}

    template <class _Tp, class _Up>
    	ALWAYS_INLINE
        operator _Tp _Up::* () const {return 0;}

    friend ALWAYS_INLINE bool operator==(es_nullptr_t, es_nullptr_t) {return true;}
    friend ALWAYS_INLINE bool operator!=(es_nullptr_t, es_nullptr_t) {return false;}
    friend ALWAYS_INLINE bool operator<(es_nullptr_t, es_nullptr_t) {return false;}
    friend ALWAYS_INLINE bool operator<=(es_nullptr_t, es_nullptr_t) {return true;}
    friend ALWAYS_INLINE bool operator>(es_nullptr_t, es_nullptr_t) {return false;}
    friend ALWAYS_INLINE bool operator>=(es_nullptr_t, es_nullptr_t) {return true;}

    friend ALWAYS_INLINE bool operator==(es_nullptr_t, long l) {return (l?false:true);}
    friend ALWAYS_INLINE bool operator!=(es_nullptr_t, long l) {return (l?true:false);}
    friend ALWAYS_INLINE bool operator<(es_nullptr_t, long l) {return false;}
    friend ALWAYS_INLINE bool operator<=(es_nullptr_t, long l) {return true;}
    friend ALWAYS_INLINE bool operator>(es_nullptr_t, long l) {return false;}
    friend ALWAYS_INLINE bool operator>=(es_nullptr_t, long l) {return true;}
    
    friend ALWAYS_INLINE bool operator==(long l, es_nullptr_t) {return (l?false:true);}
    friend ALWAYS_INLINE bool operator!=(long l, es_nullptr_t) {return (l?true:false);}
    friend ALWAYS_INLINE bool operator<(long l, es_nullptr_t) {return false;}
    friend ALWAYS_INLINE bool operator<=(long l, es_nullptr_t) {return true;}
    friend ALWAYS_INLINE bool operator>(long l, es_nullptr_t) {return false;}
    friend ALWAYS_INLINE bool operator>=(long l, es_nullptr_t) {return true;}
};

inline ALWAYS_INLINE es_nullptr_t es_get_nullptr_t() {return es_nullptr_t(0);}

#define nullptr es_get_nullptr_t()
#endif

#ifndef null
#define null nullptr
#endif

#ifndef boolean
#define boolean bool
#endif

#ifndef llong
#define llong   es_int64_t
#endif

#ifndef ullong
#define ullong  es_uint64_t
#endif

#ifndef byte
#define byte    es_int8_t
#endif

#ifndef ubyte
#define ubyte   es_uint8_t
#endif

#ifndef ushort
#define ushort  es_uint16_t
#endif

#ifndef uint
#define uint    es_uint32_t
#endif

#ifndef ulong
#define ulong   unsigned long
#endif

#ifndef uchar
#define uchar   unsigned char
#endif

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#ifdef DELETE
#undef DELETE
#endif

#ifdef CALLBACK
#undef CALLBACK
#endif
#define CALLBACK static

#define UNUSED(n)

#define THROWS(a)
#define THROWS2(a,b)
#define THROWS3(a,b,c)
#define THROWS4(a,b,c,d)
#define THROWS5(a,b,c,d,e)

#ifdef interface
#undef interface
#endif
#define interface struct

#ifdef abstract
#undef abstract
#endif
#define abstract

#ifdef synchronized
#undef synchronized
#endif
#define synchronized

#ifdef finally
#undef finally
#endif
#define finally

#ifdef _MSC_VER
#pragma warning(disable : 4250) //Two or more members have the same name. The one in class2 is inherited because it is a base class for the other classes that contained this member.
#pragma warning(disable : 4521) //multiple copy constructors specified.
#endif

#ifdef WIN32
#define RESTARTABLE(_cmd, _result) \
	_result = _cmd;
#else
#define RESTARTABLE(_cmd, _result) do { \
	do { \
		_result = _cmd; \
	} while ((_result == -1) && (errno == EINTR)); \
} while(0)
#endif


/**
 * helper for variable initialization
 */
#define DECLARE_STATIC_INITZZ \
	static char _initzz_(); \
	static char _initzz___;

#ifdef DEBUG
#define DEFINE_STATIC_INITZZ_BEGIN(classz) \
	char classz::_initzz___ = classz::_initzz_(); \
	char classz::_initzz_() { \
		const char* _hint___ = #classz "::initzz()\n"; \
		if (_initzz___ != 0) return _initzz___; \
		eso_initialize();
#else
#define DEFINE_STATIC_INITZZ_BEGIN(classz) \
	char classz::_initzz___ = classz::_initzz_(); \
	char classz::_initzz_() { \
		if (_initzz___ != 0) return _initzz___; \
		eso_initialize();
#endif

#ifdef DEBUG
#define DEFINE_STATIC_INITZZ_END \
		printf(_hint___); \
		return (_initzz___ = 1); \
	}
#else
#define DEFINE_STATIC_INITZZ_END \
		return (_initzz___ = 1); \
	}
#endif

#define DECLARE_SIMPLE_CLONE(CLASS) \
	CLASS* clone() { \
		return new CLASS(*this); \
	}

#define IPv4_INT2ARR(intip)		((ubyte*)&intip)
#define IPv4_ARR4INT(iparr)		(*(int*)iparr)
#define MAC6_LLONG2ARR(lmac)	((ubyte*)&lmac)
#define MAC6_ARR2LLONG(macarr)	(*(llong*)macarr)
#define MAC6_LEN				6

enum OSReturn {
	OS_OK = 0,			// Operation was successful
	OS_ERR = -1,        // Operation failed
	OS_INTRPT = -2,		// Operation was interrupted
	OS_TIMEOUT = -3,	// Operation timed out
	OS_NOMEM = -5,		// Operation failed for lack of memory
	OS_NORESOURCE = -6	// Operation failed for lack of nonmemory resource
};

enum MEMType {
	MEM_IGNORE = 0,
	MEM_NEW = 1,
	MEM_MALLOC = 2,
	MEM_MMALLOC = 3
};

#if defined(__GNUC__) || (defined(__MWERKS__) && (__MWERKS__ >= 0x3000)) || (defined(__ICC) && (__ICC >= 600)) || defined(__ghs__)
# define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
#elif defined(__DMC__) && (__DMC__ >= 0x810)
# define __CURRENT_FUNCTION__ __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
# define __CURRENT_FUNCTION__ __FUNCSIG__
#elif (defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 600)) || (defined(__IBMCPP__) && (__IBMCPP__ >= 500))
# define __CURRENT_FUNCTION__ __FUNCTION__
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
# define __CURRENT_FUNCTION__ __FUNC__
#elif defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901)
# define __CURRENT_FUNCTION__ __func__
#else
# define __CURRENT_FUNCTION__ "(unknown)"
#endif


/**
 * only for c++11
 */
#ifdef CPP11_SUPPORT
#ifdef __linux__
#define THREAD_TLS thread_local
#elif defined (__APPLE__)
#define THREAD_TLS __thread
#elif defined (_MSC_VER)
#define THREAD_TLS __declspec(thread)
#else // !C++11 && !__GNUC__ && !_MSC_VER
#error "Define a thread local storage qualifier for your compiler/platform!"
#endif
#endif

#endif //!__EBASE_H__
