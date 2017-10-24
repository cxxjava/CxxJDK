/*
 * ESharedPtr.hh
 *
 *  Created on: 2014-04-08
 *      Author: cxxjava@163.com
 */

#ifndef ESHARED_PTR_HH_
#define ESHARED_PTR_HH_

#include "ELockPool.hh"
#include "ESentry.hh"

/**
 *
 */
namespace efc {

extern ELock* gRCLock;

//placement new class with reference count: __NEWRC(classT)(arg1, arg2);
#define NEWRC(T) new (eso_calloc(sizeof(T) + sizeof(int))) T

template<typename T>
inline T* GETRC(T* volatile& p0)
{
	SYNCBLOCK(gRCLock) {
		T* p = p0;
		if (!p) {
			return NULL;
		}
		int* refs = (int*)((char*)p + sizeof(T));
		(*refs)++;
		return p;
    }}
}

//placement delete class with reference count
template<typename T>
inline void DELRC(T* volatile& p0)
{
	T* p;
	int n;
	SYNCBLOCK(gRCLock) {
		p = p0;
		if (!p) {
			return;
		}
		int* refs = (int*)((char*)p + sizeof(T));
		n = (*refs)--;
    }}
	if (n == 0) {
		p->~T();
		eso_free(p);
	}
}

} // namespace efc

//=============================================================================

/**
 *
 */
namespace efc {

template<class T> class sp;
template<class T> class wp;
template<class T> class enable_shared_from_this;
/*
class enable_shared_from_raw;
 */

//
// Enum values are chosen so that code that needs to insert
// a trailing fence for acquire semantics can use a single
// test such as:
//
// if( mo & memory_order_acquire ) { ...fence... }
//
// For leading fences one can use:
//
// if( mo & memory_order_release ) { ...fence... }
//
// Architectures such as Alpha that need a fence on consume
// can use:
//
// if( mo & ( memory_order_acquire | memory_order_consume ) ) { ...fence... }
//

enum memory_order
{
    memory_order_relaxed = 0,
    memory_order_acquire = 1,
    memory_order_release = 2,
    memory_order_acq_rel = 3, // acquire | release
    memory_order_seq_cst = 7, // acq_rel | 4
    memory_order_consume = 8
};

/**
 *  This is one of the @link s20_3_1_base functor base classes@endlink.
 */
template<class _Arg1, class _Arg2, class _Result>
struct binary_function {
	typedef _Arg1 first_argument_type;   ///< the type of the first argument
										 ///  (no surprises here)

	typedef _Arg2 second_argument_type;  ///< the type of the second argument
	typedef _Result result_type;         ///< type of the return type
};

/// One of the @link s20_3_3_comparisons comparison functors@endlink.
template<class _Tp>
struct isless: public binary_function<_Tp, _Tp, bool> {
	bool operator()(const _Tp& __x, const _Tp& __y) const {
		return __x < __y;
	}
};

// verify that types are complete for increased safety

template<class T> inline void checked_delete(T * x)
{
    // intentionally complex - simplification causes regressions
    typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
    (void) sizeof(type_must_be_complete);
    if ((unsigned long)x > 10) { //!!! reserve some points for internal flags.
    	delete x;
    }
}

template<class T> inline void checked_array_delete(T * x)
{
    typedef char type_must_be_complete[ sizeof(T)? 1: -1 ];
    (void) sizeof(type_must_be_complete);
    if ((unsigned long)x > 10) { //!!! reserve some points for internal flags.
    	delete [] x;
    }
}

template<class T> struct checked_deleter
{
    typedef void result_type;
    typedef T * argument_type;

    void operator()(T * x) const
    {
        // disables ADL
        checked_delete(x);
    }
};

template<class T> struct checked_array_deleter
{
    typedef void result_type;
    typedef T * argument_type;

    void operator()(T * x) const
    {
        checked_array_delete(x);
    }
};


namespace detail
{

/**
 *  @brief Swaps two values.
 *  @param  a  A thing of arbitrary type.
 *  @param  b  Another thing of arbitrary type.
 *  @return   Nothing.
 *
 *  This is the simple classic generic implementation.  It will work on
 *  any type which has a copy constructor and an assignment operator.
 */
template<typename _Tp> inline void swap(_Tp& __a, _Tp& __b)
{
  _Tp __tmp = __a;
  __a = __b;
  __b = __tmp;
}

template< class Y, class T > struct sp_convertible
{
    typedef char (&yes) [1];
    typedef char (&no)  [2];

    static yes f( T* );
    static no  f( ... );

    enum _vt { value = sizeof( (f)( static_cast<Y*>(0) ) ) == sizeof(yes) };
};

template< class Y, class T > struct sp_convertible< Y, T[] >
{
    enum _vt { value = false };
};

template< class Y, class T > struct sp_convertible< Y[], T[] >
{
    enum _vt { value = sp_convertible< Y[1], T[1] >::value };
};

template< class Y, std::size_t N, class T > struct sp_convertible< Y[N], T[] >
{
    enum _vt { value = sp_convertible< Y[1], T[1] >::value };
};

struct sp_empty
{
};

template< bool > struct sp_enable_if_convertible_impl;

template<> struct sp_enable_if_convertible_impl<true>
{
    typedef sp_empty type;
};

template<> struct sp_enable_if_convertible_impl<false>
{
};

template< class Y, class T > struct sp_enable_if_convertible: public sp_enable_if_convertible_impl< sp_convertible< Y, T >::value >
{
};

} // namespace detail


namespace detail
{

// atomic

inline int atomic_exchange_and_add( int * pw, int dv )
{
    // int r = *pw;
    // *pw += dv;
    // return r;

#ifdef WIN32
	__asm
    {
        mov esi, [pw]
        mov eax, dv
        lock xadd dword ptr [esi], eax
    }
#else
    int r;

    __asm__ __volatile__
    (
        "lock\n\t"
        "xadd %1, %0":
        "=m"( *pw ), "=r"( r ): // outputs (%0, %1)
        "m"( *pw ), "1"( dv ): // inputs (%2, %3 == %1)
        "memory", "cc" // clobbers
    );

    return r;
#endif
}

inline void atomic_increment( int * pw )
{
    //atomic_exchange_and_add( pw, 1 );

#ifdef WIN32
	__asm
    {
        mov esi, [pw]
        lock inc dword ptr [esi]
    }
#else
    __asm__
    (
        "lock\n\t"
        "incl %0":
        "=m"( *pw ): // output (%0)
        "m"( *pw ): // input (%1)
        "cc" // clobbers
    );
#endif
}

inline int atomic_conditional_increment( int * pw )
{
    // int rv = *pw;
    // if( rv != 0 ) ++*pw;
    // return rv;

#ifdef WIN32
	__asm
    {
        mov esi, [pw]
        mov eax, dword ptr [esi]
    L0:
        test eax, eax
        je L1
        mov ebx, eax
        inc ebx
        lock cmpxchg dword ptr [esi], ebx
        jne L0
    L1:
    }
#else
    int rv, tmp;

    __asm__
    (
        "movl %0, %%eax\n\t"
        "0:\n\t"
        "test %%eax, %%eax\n\t"
        "je 1f\n\t"
        "movl %%eax, %2\n\t"
        "incl %2\n\t"
        "lock\n\t"
        "cmpxchgl %2, %0\n\t"
        "jne 0b\n\t"
        "1:":
        "=m"( *pw ), "=&a"( rv ), "=&r"( tmp ): // outputs (%0, %1, %2)
        "m"( *pw ): // input (%3)
        "cc" // clobbers
    );

    return rv;
#endif
}

} // namespace detail


namespace detail
{

// sp_counted_base

class sp_counted_base
{
private:

    sp_counted_base( sp_counted_base const & );
    sp_counted_base & operator= ( sp_counted_base const & );

    int use_count_;        // #shared
    int weak_count_;       // #weak + (#shared != 0)

public:

    sp_counted_base(): use_count_( 1 ), weak_count_( 1 )
    {
    }

    virtual ~sp_counted_base() // nothrow
    {
    }

    // dispose() is called when use_count_ drops to zero, to release
    // the resources managed by *this.

    virtual void dispose() = 0; // nothrow

    virtual void dismiss() = 0; // nothrow

    // destroy() is called when weak_count_ drops to zero.

    virtual void destroy() // nothrow
    {
        delete this;
    }

    void add_ref_copy()
    {
        atomic_increment( &use_count_ );
    }

    bool add_ref_lock() // true on success
    {
        return atomic_conditional_increment( &use_count_ ) != 0;
    }

    void release() // nothrow
    {
        if( atomic_exchange_and_add( &use_count_, -1 ) == 1 )
        {
            dispose();
            weak_release();
        }
    }

    void weak_add_ref() // nothrow
    {
        atomic_increment( &weak_count_ );
    }

    void weak_release() // nothrow
    {
        if( atomic_exchange_and_add( &weak_count_, -1 ) == 1 )
        {
            destroy();
        }
    }

    long use_count() const // nothrow
    {
        return static_cast<int const volatile &>( use_count_ );
    }
};

} //namespace detail


namespace detail
{

// sp_counted_impl_p

template<class X> class sp_counted_impl_p: public sp_counted_base
{
private:

    X * px_;

    sp_counted_impl_p( sp_counted_impl_p const & );
    sp_counted_impl_p & operator= ( sp_counted_impl_p const & );

    typedef sp_counted_impl_p<X> this_type;

public:

    explicit sp_counted_impl_p( X * px ): px_( px )
    {
    }

    virtual void dispose() // nothrow
    {
        checked_delete( px_ );
    }

    virtual void dismiss() // nothrow
    {
    	px_ = null;
    }
};

template<class P, class D> class sp_counted_impl_pd: public sp_counted_base
{
private:

    P ptr; // copy constructor must not throw
    D del; // copy constructor must not throw

    sp_counted_impl_pd( sp_counted_impl_pd const & );
    sp_counted_impl_pd & operator= ( sp_counted_impl_pd const & );

    typedef sp_counted_impl_pd<P, D> this_type;

public:

    // pre: d(p) must not throw

    sp_counted_impl_pd( P p, D & d ): ptr( p ), del( d )
    {
    }

    sp_counted_impl_pd( P p ): ptr( p ), del()
    {
    }

    virtual void dispose() // nothrow
    {
        del( ptr );
    }

    virtual void dismiss() // nothrow
	{
    	ptr = null;
	}
};

template<class P, class D, class A> class sp_counted_impl_pda: public sp_counted_base
{
private:

    P p_; // copy constructor must not throw
    D d_; // copy constructor must not throw
    A a_; // copy constructor must not throw

    sp_counted_impl_pda( sp_counted_impl_pda const & );
    sp_counted_impl_pda & operator= ( sp_counted_impl_pda const & );

    typedef sp_counted_impl_pda<P, D, A> this_type;

public:

    // pre: d( p ) must not throw

    sp_counted_impl_pda( P p, D & d, A a ): p_( p ), d_( d ), a_( a )
    {
    }

    sp_counted_impl_pda( P p, A a ): p_( p ), d_(), a_( a )
    {
    }

    virtual void dispose() // nothrow
    {
        d_( p_ );
    }

    virtual void dismiss() // nothrow
	{
		p_ = null;
	}

    virtual void destroy() // nothrow
    {
        typedef typename A::template rebind< this_type >::other A2;

        A2 a2( a_ );

        this->~this_type();
        a2.deallocate( this, 1 );
    }
};

} // namespace detail


namespace detail
{

// shared_count

struct sp_nothrow_tag {};

template< class D > struct sp_inplace_tag
{
};

class weak_count;

class shared_count
{
private:

    sp_counted_base * pi_;

    friend class weak_count;

public:

    shared_count(): pi_(0) // nothrow
    {
    }

    template<class Y> explicit shared_count( Y * p ): pi_( 0 )
    {
        try
        {
            pi_ = new sp_counted_impl_p<Y>( p );
        }
        catch(...)
        {
            checked_delete( p );
            throw;
        }
    }

#if defined( _MSC_VER ) && ( _MSC_VER <= 1200 )
    template<class Y, class D> shared_count( Y * p, D d ): pi_(0)
#else
    template<class P, class D> shared_count( P p, D d ): pi_(0)
#endif
    {
#if defined( _MSC_VER ) && ( _MSC_VER <= 1200 )
        typedef Y* P;
#endif
        try
        {
            pi_ = new sp_counted_impl_pd<P, D>(p, d);
        }
        catch(...)
        {
            d(p); // delete p
            throw;
        }
    }

    template< class P, class D > shared_count( P p, sp_inplace_tag<D> ): pi_( 0 )
    {
        try
        {
            pi_ = new sp_counted_impl_pd< P, D >( p );
        }
        catch( ... )
        {
            D::operator_fn( p ); // delete p
            throw;
        }
    }

    template<class P, class D, class A> shared_count( P p, D d, A a ): pi_( 0 )
    {
        typedef sp_counted_impl_pda<P, D, A> impl_type;
        typedef typename A::template rebind< impl_type >::other A2;

        A2 a2( a );

        try
        {
            pi_ = a2.allocate( 1, static_cast< impl_type* >( 0 ) );
            new( static_cast< void* >( pi_ ) ) impl_type( p, d, a );
        }
        catch(...)
        {
            d( p );

            if( pi_ != 0 )
            {
                a2.deallocate( static_cast< impl_type* >( pi_ ), 1 );
            }

            throw;
        }
    }

    template< class P, class D, class A > shared_count( P p, sp_inplace_tag< D >, A a ): pi_( 0 )
    {
        typedef sp_counted_impl_pda< P, D, A > impl_type;
        typedef typename A::template rebind< impl_type >::other A2;

        A2 a2( a );

        try
        {
            pi_ = a2.allocate( 1, static_cast< impl_type* >( 0 ) );
            new( static_cast< void* >( pi_ ) ) impl_type( p, a );
        }
        catch(...)
        {
            D::operator_fn( p );

            if( pi_ != 0 )
            {
                a2.deallocate( static_cast< impl_type* >( pi_ ), 1 );
            }

            throw;
        }
    }

    ~shared_count() // nothrow
    {
        if( pi_ != 0 ) pi_->release();
    }

    shared_count(shared_count const & r): pi_(r.pi_) // nothrow
    {
        if( pi_ != 0 ) pi_->add_ref_copy();
    }

    explicit shared_count(weak_count const & r); // throws bad_weak_ptr when r.use_count() == 0
    shared_count( weak_count const & r, sp_nothrow_tag ); // constructs an empty *this when r.use_count() == 0

    shared_count & operator= (shared_count const & r) // nothrow
    {
        sp_counted_base * tmp = r.pi_;

        if( tmp != pi_ )
        {
            if( tmp != 0 ) tmp->add_ref_copy();
            if( pi_ != 0 ) pi_->release();
            pi_ = tmp;
        }

        return *this;
    }

    void swap(shared_count & r) // nothrow
    {
        sp_counted_base * tmp = r.pi_;
        r.pi_ = pi_;
        pi_ = tmp;
    }

    long use_count() const // nothrow
    {
        return pi_ != 0? pi_->use_count(): 0;
    }

    bool unique() const // nothrow
    {
        return use_count() == 1;
    }

    bool empty() const // nothrow
    {
        return pi_ == 0;
    }

    void dismiss() const
    {
    	if (pi_) pi_->dismiss();
    }

    friend inline bool operator==(shared_count const & a, shared_count const & b)
    {
        return a.pi_ == b.pi_;
    }

    friend inline bool operator<(shared_count const & a, shared_count const & b)
    {
        return isless<sp_counted_base *>()( a.pi_, b.pi_ );
    }
};

class weak_count
{
private:

    sp_counted_base * pi_;

    friend class shared_count;

public:

    weak_count(): pi_(0) // nothrow
    {
    }

    weak_count(shared_count const & r): pi_(r.pi_) // nothrow
    {
        if(pi_ != 0) pi_->weak_add_ref();
    }

    weak_count(weak_count const & r): pi_(r.pi_) // nothrow
    {
        if(pi_ != 0) pi_->weak_add_ref();
    }

// Move support

    ~weak_count() // nothrow
    {
        if(pi_ != 0) pi_->weak_release();
    }

    weak_count & operator= (shared_count const & r) // nothrow
    {
        sp_counted_base * tmp = r.pi_;

        if( tmp != pi_ )
        {
            if(tmp != 0) tmp->weak_add_ref();
            if(pi_ != 0) pi_->weak_release();
            pi_ = tmp;
        }

        return *this;
    }

    weak_count & operator= (weak_count const & r) // nothrow
    {
        sp_counted_base * tmp = r.pi_;

        if( tmp != pi_ )
        {
            if(tmp != 0) tmp->weak_add_ref();
            if(pi_ != 0) pi_->weak_release();
            pi_ = tmp;
        }

        return *this;
    }

    void swap(weak_count & r) // nothrow
    {
        sp_counted_base * tmp = r.pi_;
        r.pi_ = pi_;
        pi_ = tmp;
    }

    long use_count() const // nothrow
    {
        return pi_ != 0? pi_->use_count(): 0;
    }

    bool empty() const // nothrow
    {
        return pi_ == 0;
    }

    friend inline bool operator==(weak_count const & a, weak_count const & b)
    {
        return a.pi_ == b.pi_;
    }

    friend inline bool operator<(weak_count const & a, weak_count const & b)
    {
        return isless<sp_counted_base *>()(a.pi_, b.pi_);
    }
};

inline shared_count::shared_count( weak_count const & r ): pi_( r.pi_ )
{
    if( pi_ == 0 || !pi_->add_ref_lock() )
    {
    	throw EException(__FILE__, __LINE__, "bad_weak_ptr");
    }
}

inline shared_count::shared_count( weak_count const & r, sp_nothrow_tag ): pi_( r.pi_ )
{
    if( pi_ != 0 && !pi_->add_ref_lock() )
    {
        pi_ = 0;
    }
}

} // namespace detail


namespace detail {

typedef es_nullptr_t sp_nullptr_t;

// sp_element, element_type

template< class T > struct sp_element
{
    typedef T type;
};

template< class T > struct sp_element< T[] >
{
    typedef T type;
};

template< class T, long N > struct sp_element< T[N] >
{
    typedef T type;
};

// sp_dereference, return type of operator*

template< class T > struct sp_dereference
{
    typedef T & type;
};

template<> struct sp_dereference< void >
{
    typedef void type;
};

template<> struct sp_dereference< void const >
{
    typedef void type;
};

template<> struct sp_dereference< void volatile >
{
    typedef void type;
};

template<> struct sp_dereference< void const volatile >
{
    typedef void type;
};

template< class T > struct sp_dereference< T[] >
{
    typedef void type;
};

template< class T, long N > struct sp_dereference< T[N] >
{
    typedef void type;
};


// sp_member_access, return type of operator->

template< class T > struct sp_member_access
{
    typedef T * type;
};

template< class T > struct sp_member_access< T[] >
{
    typedef void type;
};

template< class T, long N > struct sp_member_access< T[N] >
{
    typedef void type;
};

// sp_array_access, return type of operator[]

template< class T > struct sp_array_access
{
    typedef void type;
};

template< class T > struct sp_array_access< T[] >
{
    typedef T & type;
};

template< class T, long N > struct sp_array_access< T[N] >
{
    typedef T & type;
};

// sp_extent, for operator[] index check

template< class T > struct sp_extent
{
    enum _vt { value = 0 };
};

template< class T, long N > struct sp_extent< T[N] >
{
    enum _vt { value = N };
};

// enable_shared_from_this support

template< class X, class Y, class T > inline void sp_enable_shared_from_this( sp<X> const * ppx, Y const * py, enable_shared_from_this< T > const * pe )
{
    if( pe != 0 )
    {
        pe->_internal_accept_owner( ppx, const_cast< Y* >( py ) );
    }
}

/*
template< class X, class Y > inline void sp_enable_shared_from_this( sp<X> * ppx, Y const * py, enable_shared_from_raw const * pe );
*/

inline void sp_enable_shared_from_this( ... )
{
}

// sp_assert_convertible

template< class Y, class T > inline void sp_assert_convertible()
{
    // static_assert( sp_convertible< Y, T >::value );
    typedef char tmp[ sp_convertible< Y, T >::value? 1: -1 ];
    (void)sizeof( tmp );
}

// pointer constructor helper

template< class T, class Y > inline void sp_pointer_construct( sp< T > * ppx, Y * p, detail::shared_count & pn )
{
    detail::shared_count( p ).swap( pn );
    detail::sp_enable_shared_from_this( ppx, p, p );
}

template< class T, class Y > inline void sp_pointer_construct( sp< T[] > * /*ppx*/, Y * p, detail::shared_count & pn )
{
    sp_assert_convertible< Y[], T[] >();
	detail::shared_count( p, checked_array_deleter< T >() ).swap( pn );
}

template< class T, long N, class Y > inline void sp_pointer_construct( sp< T[N] > * /*ppx*/, Y * p, detail::shared_count & pn )
{
    sp_assert_convertible< Y[N], T[N] >();
    detail::shared_count( p, checked_array_deleter< T >() ).swap( pn );
}

// deleter constructor helper

template< class T, class Y > inline void sp_deleter_construct( sp< T > * ppx, Y * p )
{
    detail::sp_enable_shared_from_this( ppx, p, p );
}

template< class T, class Y > inline void sp_deleter_construct( sp< T[] > * /*ppx*/, Y * /*p*/ )
{
    sp_assert_convertible< Y[], T[] >();
}

template< class T, long N, class Y > inline void sp_deleter_construct( sp< T[N] > * /*ppx*/, Y * /*p*/ )
{
    sp_assert_convertible< Y[N], T[N] >();
}

} //namespace detail

//=============================================================================

//
//  sp
//
//  An enhanced relative of scoped_ptr with reference counted copy semantics.
//  The object pointed to is deleted when the last sp pointing to it
//  is destroyed or reset.
//

template<class T>
class sp : public EObject
{
private:

    // Borland 5.5.1 specific workaround
    typedef sp<T> this_type;

public:
    typedef typename detail::sp_element< T >::type element_type;

    static sp nullPtr() {return sp();}

    sp() throw() : px( 0 ), pn() // never throws in 1.30+
    {
    }

    sp( T* p ) : px( p ), pn() // never throws in 1.30+
	{
    	detail::sp_pointer_construct( this, p, pn );
	}

    sp( detail::sp_nullptr_t ) throw() : px( 0 ), pn() // never throws
	{
	}

    template<class Y>
    explicit sp( Y * p ): px( p ), pn() // Y must be complete
    {
        detail::sp_pointer_construct( this, p, pn );
    }

    //
    // Requirements: D's copy constructor must not throw
    //
    // sp will release p by calling d(p)
    //

    template<class Y, class D>
    sp( Y * p, D d ): px( p ), pn( p, d )
    {
        detail::sp_deleter_construct( this, p );
    }

    template<class D> sp( detail::sp_nullptr_t p, D d ): px( p ), pn( p, d )
	{
	}

    // As above, but with allocator. A's copy constructor shall not throw.

    template<class Y, class D, class A>
    sp( Y * p, D d, A a ): px( p ), pn( p, d, a )
    {
        detail::sp_deleter_construct( this, p );
    }

    template<class D, class A> sp( detail::sp_nullptr_t p, D d, A a ): px( p ), pn( p, d, a )
	{
	}

    //  generated copy constructor, destructor are fine...

    // ... except in C++0x, move disables the implicit copy

    sp( sp const & r ) throw() : px( r.px ), pn( r.pn )
	{
	}

    template<class Y>
    explicit sp( wp<Y> const & r ): pn( r.pn ) // may throw
    {
        detail::sp_assert_convertible< Y, T >();

        // it is now safe to copy r.px, as pn(r.pn) did not throw
        px = r.px;
    }

    template<class Y>
    sp( wp<Y> const & r, detail::sp_nothrow_tag ) throw() : px( 0 ), pn( r.pn, detail::sp_nothrow_tag() )
    {
        if( !pn.empty() )
        {
            px = r.px;
        }
    }

    template<class Y>
    sp( sp<Y> const & r, typename detail::sp_enable_if_convertible<Y,T>::type = detail::sp_empty() )
    throw() : px( r.px ), pn( r.pn )
    {
        detail::sp_assert_convertible< Y, T >();
    }

    // aliasing
    template< class Y >
    sp( sp<Y> const & r, element_type * p ) throw() : px( p ), pn( r.pn )
    {
    }

    // assignment

    sp & operator=( sp const & r ) throw()
    {
        this_type(r).swap(*this);
        return *this;
    }

#if !defined(_MSC_VER) || (_MSC_VER >= 1400)

    template<class Y>
    sp & operator=(sp<Y> const & r) throw()
    {
        this_type(r).swap(*this);
        return *this;
    }

#endif

    sp & operator=( T* p ) throw() // never throws
	{
    	sp t(p);
		t.swap(*this);
		return *this;
	}

    sp & operator=( detail::sp_nullptr_t ) throw() // never throws
	{
		sp t;
		t.swap(*this);
		return *this;
	}

    // Move support

#ifdef CPP11_SUPPORT
    sp( sp && r ) throw() : px( r.px ), pn()
	{
		pn.swap( r.pn );
		r.px = 0;
	}

	template<class Y>
	sp( sp<Y> && r, typename detail::sp_enable_if_convertible<Y,T>::type = detail::sp_empty() )
	throw() : px( r.px ), pn()
	{
		detail::sp_assert_convertible< Y, T >();

		pn.swap( r.pn );
		r.px = 0;
	}

	sp & operator=( sp && r ) throw()
	{
		this_type( static_cast< sp && >( r ) ).swap( *this );
		return *this;
	}

	template<class Y>
	sp & operator=( sp<Y> && r ) throw()
	{
		this_type( static_cast< sp<Y> && >( r ) ).swap( *this );
		return *this;
	}
#endif

    void reset() throw() // never throws in 1.30+
    {
        this_type().swap(*this);
    }

    template<class Y> void reset( Y * p ) // Y must be complete
    {
        ES_ASSERT( p == 0 || p != px ); // catch self-reset errors
        this_type( p ).swap( *this );
    }

    template<class Y, class D> void reset( Y * p, D d )
    {
        this_type( p, d ).swap( *this );
    }

    template<class Y, class D, class A> void reset( Y * p, D d, A a )
    {
        this_type( p, d, a ).swap( *this );
    }

    template<class Y> void reset( sp<Y> const & r, element_type * p )
    {
        this_type( r, p ).swap( *this );
    }

    // never throws (but has a ES_ASSERT in it, so not marked with BOOST_NOEXCEPT)
    typename detail::sp_dereference< T >::type operator* () const
    {
    	ES_ASSERT( px != 0 );
        return *px;
    }

    // never throws (but has a ES_ASSERT in it, so not marked with BOOST_NOEXCEPT)
    typename detail::sp_member_access< T >::type operator-> () const
    {
    	ES_ASSERT( px != 0 );
        return px;
    }

    // never throws (but has a ES_ASSERT in it, so not marked with BOOST_NOEXCEPT)
    typename detail::sp_array_access< T >::type operator[] ( long i ) const
    {
    	ES_ASSERT( px != 0 );
    	ES_ASSERT( i >= 0 && ( i < detail::sp_extent< T >::value || detail::sp_extent< T >::value == 0 ) );

        return px[ i ];
    }

    element_type * get() const
    {
        return px;
    }

    bool unique() const
    {
        return pn.unique();
    }

    long use_count() const
    {
        return pn.use_count();
    }

    void swap( sp & other )
    {
        detail::swap(px, other.px);
        pn.swap(other.pn);
    }

    template<class Y> bool owner_before( sp<Y> const & rhs ) const throw()
    {
        return pn < rhs.pn;
    }

    template<class Y> bool owner_before( wp<Y> const & rhs ) const throw()
    {
        return pn < rhs.pn;
    }

    inline bool equals( sp & other )
    {
        return (px == other.get()) ? true : (px ? px->equals(other.get()) : false);
    }

    inline bool equals( sp * other )
    {
        return (px == other->get()) ? true : (px ? px->equals(other->get()) : false);
    }

    inline bool operator!() const
	{
		return px == 0;
	}

    bool _internal_equiv( sp const & r ) const
    {
        return px == r.px && pn == r.pn;
    }

    element_type* dismiss() {
    	pn.dismiss();
    	return px;
    }

// Tasteless as this may seem, making all members public allows member templates
// to work in the absence of member template friends. (Matthew Langston)

private:

    template<class Y> friend class sp;
    template<class Y> friend class wp;

    element_type * px;                 // contained pointer
    detail::shared_count pn;    // reference counter
};

template<class T, class U> inline bool operator==(sp<T> const & a, U const * b)
{
    return a.get() == b;
}

template<class T> inline bool operator==(sp<T> const & a, detail::sp_nullptr_t)
{
    return a.get() == null;
}

template<class T, class U> inline bool operator==(sp<T> const & a, sp<U> const & b)
{
    return a.get() == b.get();
}

template<class T, class U> inline bool operator!=(sp<T> const & a, U const * b)
{
    return a.get() != b;
}

template<class T> inline bool operator!=(sp<T> const & a, detail::sp_nullptr_t)
{
    return a.get() != null;
}

template<class T, class U> inline bool operator!=(sp<T> const & a, sp<U> const & b)
{
    return a.get() != b.get();
}

template<class T, class U> inline bool operator<(sp<T> const & a, sp<U> const & b)
{
    return a.owner_before( b );
}

template<class T> inline void swap(sp<T> & a, sp<T> & b)
{
    a.swap(b);
}

template<class T, class U> sp<T> static_pointer_cast( sp<U> const & r )
{
    (void) static_cast< T* >( static_cast< U* >( 0 ) );

    typedef typename sp<T>::element_type E;

    E * p = static_cast< E* >( r.get() );
    return sp<T>( r, p );
}

template<class T, class U> sp<T> const_pointer_cast( sp<U> const & r )
{
    (void) const_cast< T* >( static_cast< U* >( 0 ) );

    typedef typename sp<T>::element_type E;

    E * p = const_cast< E* >( r.get() );
    return sp<T>( r, p );
}

template<class T, class U> sp<T> dynamic_pointer_cast( sp<U> const & r )
{
    (void) dynamic_cast< T* >( static_cast< U* >( 0 ) );

    typedef typename sp<T>::element_type E;

    E * p = dynamic_cast< E* >( r.get() );
    return p? sp<T>( r, p ): sp<T>();
}

template<class T, class U> sp<T> reinterpret_pointer_cast( sp<U> const & r )
{
    (void) reinterpret_cast< T* >( static_cast< U* >( 0 ) );

    typedef typename sp<T>::element_type E;

    E * p = reinterpret_cast< E* >( r.get() );
    return sp<T>( r, p );
}

// get_pointer() enables mem_fn to recognize sp

template<class T> inline typename sp<T>::element_type * get_pointer(sp<T> const & p)
{
    return p.get();
}

// atomic access

template<class T> inline bool atomic_is_lock_free( sp<T> const * /*p*/ )
{
    return false;
}

template<class T> sp<T> atomic_load( sp<T> const * p )
{
	SCOPED_SLOCK0(p) {
    return *p;
	}}
}

template<class T> inline sp<T> atomic_load_explicit( sp<T> const * p, memory_order /*mo*/ )
{
    return atomic_load( p );
}

template<class T> void atomic_store( sp<T> * p, sp<T> r )
{
	SCOPED_SLOCK0(p) {
    p->swap( r );
	}}
}

template<class T> void atomic_store( sp<T> * p, detail::sp_nullptr_t )
{
	SCOPED_SLOCK0(p) {
	sp<T> r;
    p->swap( r );
	}}
}

template<class T> inline void atomic_store_explicit( sp<T> * p, sp<T> r, memory_order /*mo*/ )
{
    atomic_store( p, r ); // std::move( r )
}

template<class T> sp<T> atomic_exchange( sp<T> * p, sp<T> r )
{
    ELock* l = ESpinLockPool<0>::lockFor(p);

    l->lock();
    p->swap( r );
    l->unlock();

    return r; // return std::move( r )
}

template<class T> sp<T> atomic_exchange_explicit( sp<T> * p, sp<T> r, memory_order /*mo*/ )
{
    return atomic_exchange( p, r ); // std::move( r )
}

template<class T> bool atomic_compare_exchange( sp<T> * p, sp<T> * v, sp<T> w )
{
	ELock* l = ESpinLockPool<0>::lockFor(p);

    l->lock();
    if( p->_internal_equiv( *v ) )
    {
        p->swap( w );
        l->unlock();
        return true;
    }
    else
    {
        sp<T> tmp( *p );
        l->unlock();
        tmp.swap( *v );
        return false;
    }
}

template<class T> inline bool atomic_compare_exchange_explicit( sp<T> * p, sp<T> * v, sp<T> w, memory_order /*success*/, memory_order /*failure*/ )
{
    return atomic_compare_exchange( p, v, w ); // std::move( w )
}

//=============================================================================

// wp

template<class T>
class wp
{
private:

    // Borland 5.5.1 specific workarounds
    typedef wp<T> this_type;

public:

    typedef typename detail::sp_element< T >::type element_type;

    wp() : px(0), pn() // never throws in 1.30+
    {
    }

//  generated copy constructor, assignment, destructor are fine...

// ... except in C++0x, move disables the implicit copy

    wp( wp const & r ) : px( r.px ), pn( r.pn )
    {
    }

    wp & operator=( wp const & r )
    {
        px = r.px;
        pn = r.pn;
        return *this;
    }

    wp & operator=( detail::sp_nullptr_t )
	{
    	wp r;
		px = r.px;
		pn = r.pn;
		return *this;
	}

//
//  The "obvious" converting constructor implementation:
//
//  template<class Y>
//  wp(wp<Y> const & r): px(r.px), pn(r.pn) // never throws
//  {
//  }
//
//  has a serious problem.
//
//  r.px may already have been invalidated. The px(r.px)
//  conversion may require access to *r.px (virtual inheritance).
//
//  It is not possible to avoid spurious access violations since
//  in multithreaded programs r.px may be invalidated at any point.
//

    template<class Y>
    wp( wp<Y> const & r, typename detail::sp_enable_if_convertible<Y,T>::type = detail::sp_empty() )
     : px(r.lock().get()), pn(r.pn)
    {
        detail::sp_assert_convertible< Y, T >();
    }

#ifdef CPP11_SUPPORT
    template<class Y>
    wp( wp<Y> && r, typename detail::sp_enable_if_convertible<Y,T>::type = detail::sp_empty() )
     : px( r.lock().get() ), pn( static_cast< detail::weak_count && >( r.pn ) )
    {
        detail::sp_assert_convertible< Y, T >();
        r.px = 0;
    }

    // for better efficiency in the T == Y case
    wp( wp && r )
     : px( r.px ), pn( static_cast< detail::weak_count && >( r.pn ) )
    {
        r.px = 0;
    }

    // for better efficiency in the T == Y case
    wp & operator=( wp && r )
    {
        this_type( static_cast< wp && >( r ) ).swap( *this );
        return *this;
    }
#endif

    template<class Y>
    wp( sp<Y> const & r, typename detail::sp_enable_if_convertible<Y,T>::type = detail::sp_empty() )
     : px( r.px ), pn( r.pn )
    {
        detail::sp_assert_convertible< Y, T >();
    }

    template<class Y>
    wp & operator=( wp<Y> const & r )
    {
        detail::sp_assert_convertible< Y, T >();

        px = r.lock().get();
        pn = r.pn;

        return *this;
    }

#ifdef CPP11_SUPPORT
    template<class Y>
    wp & operator=( wp<Y> && r )
    {
        this_type( static_cast< wp<Y> && >( r ) ).swap( *this );
        return *this;
    }
#endif

    template<class Y>
    wp & operator=( sp<Y> const & r )
    {
        detail::sp_assert_convertible< Y, T >();

        px = r.px;
        pn = r.pn;

        return *this;
    }

    sp<T> lock() const
    {
        return sp<T>( *this, detail::sp_nothrow_tag() );
    }

    long use_count() const
    {
        return pn.use_count();
    }

    bool expired() const
    {
        return pn.use_count() == 0;
    }

    bool _empty() const // extension, not in std::weak_ptr
    {
        return pn.empty();
    }

    void reset() // never throws in 1.30+
    {
        this_type().swap(*this);
    }

    void swap(this_type & other)
    {
        std::swap(px, other.px);
        pn.swap(other.pn);
    }

    template<typename Y>
    void _internal_aliasing_assign(wp<Y> const & r, element_type * px2)
    {
        px = px2;
        pn = r.pn;
    }

    template<class Y> bool owner_before( wp<Y> const & rhs ) const
    {
        return pn < rhs.pn;
    }

    template<class Y> bool owner_before( sp<Y> const & rhs ) const
    {
        return pn < rhs.pn;
    }

// Tasteless as this may seem, making all members public allows member templates
// to work in the absence of member template friends. (Matthew Langston)

private:
    template<class Y> friend class wp;
    template<class Y> friend class sp;

    element_type * px;            // contained pointer
    detail::weak_count pn; // reference counter

};  // wp

template<class T, class U> inline bool operator<(wp<T> const & a, wp<U> const & b)
{
    return a.owner_before( b );
}

template<class T> void swap(wp<T> & a, wp<T> & b)
{
    a.swap(b);
}

//=============================================================================

// enable_shared_from_this

template<class T> class enable_shared_from_this
{
protected:

    enable_shared_from_this() throw()
    {
    }

    enable_shared_from_this(enable_shared_from_this const &) throw()
    {
    }

    enable_shared_from_this & operator=(enable_shared_from_this const &) throw()
    {
        return *this;
    }

    ~enable_shared_from_this() throw() // ~weak_ptr<T> newer throws, so this call also must not throw
    {
    }

public:

    sp<T> shared_from_this()
    {
        sp<T> p( weak_this_ );
        ES_ASSERT( p.get() == this );
        return p;
    }

    sp<T const> shared_from_this() const
    {
        sp<T const> p( weak_this_ );
        ES_ASSERT( p.get() == this );
        return p;
    }

public: // actually private, but avoids compiler template friendship issues

    // Note: invoked automatically by shared_ptr; do not call
    template<class X, class Y> void _internal_accept_owner( sp<X> const * ppx, Y * py ) const
    {
        if( weak_this_.expired() )
        {
            weak_this_ = sp<T>( *ppx, py );
        }
    }

protected:

    mutable wp<T> weak_this_;
};

//=============================================================================

// enable_shared_from_raw

/*
template<typename T> sp<T> shared_from_raw(T *);
template<typename T> wp<T> weak_from_raw(T *);

class enable_shared_from_raw
{
protected:

    enable_shared_from_raw()
    {
    }

    enable_shared_from_raw( enable_shared_from_raw const & )
    {
    }

    enable_shared_from_raw & operator=( enable_shared_from_raw const & )
    {
        return *this;
    }

    ~enable_shared_from_raw()
    {
    	ES_ASSERT( shared_this_.use_count() <= 1 ); // make sure no dangling shared_ptr objects exist
    }

private:

    void init_weak_once() const
    {
        if( weak_this_.expired() )
        {
            shared_this_.reset( static_cast<void*>(0), detail::esft2_deleter_wrapper() );
            weak_this_ = shared_this_;
        }
    }

private:
    template<class Y> friend class sp;
    template<typename T> friend sp<T> shared_from_raw(T *);
    template<typename T> friend wp<T> weak_from_raw(T *);
    template< class X, class Y > friend inline void detail::sp_enable_shared_from_this( sp<X> * ppx, Y const * py, enable_shared_from_raw const * pe );

    sp<void> shared_from_this()
    {
        init_weak_once();
        return sp<void>( weak_this_ );
    }

    sp<const void> shared_from_this() const
    {
        init_weak_once();
        return sp<const void>( weak_this_ );
    }

    // Note: invoked automatically by shared_ptr; do not call
    template<class X, class Y> void _internal_accept_owner( sp<X> const * ppx, Y * py ) const
    {
    	ES_ASSERT( ppx != 0 );

        if( weak_this_.expired() )
        {
            weak_this_ = *ppx;
        }
        else if( shared_this_.use_count() != 0 )
        {
        	ES_ASSERT( ppx->unique() ); // no weak_ptrs should exist either, but there's no way to check that

            detail::esft2_deleter_wrapper * pd = get_deleter<detail::esft2_deleter_wrapper>( shared_this_ );
            ES_ASSERT( pd != 0 );

            pd->set_deleter( *ppx );

            ppx->reset( shared_this_, ppx->get() );
            shared_this_.reset();
        }
    }

    mutable wp<void> weak_this_;
private:
    mutable sp<void> shared_this_;
};

template<typename T>
sp<T> shared_from_raw(T *p)
{
    ES_ASSERT(p != 0);
    return sp<T>(p->enable_shared_from_raw::shared_from_this(), p);
}

template<typename T>
wp<T> weak_from_raw(T *p)
{
	ES_ASSERT(p != 0);
    wp<T> result;
    result._internal_aliasing_assign(p->enable_shared_from_raw::weak_this_, p);
    return result;
}

namespace detail
{
	template< class X, class Y > inline void sp_enable_shared_from_this( sp<X> * ppx, Y const * py, enable_shared_from_raw const * pe )
	{
		if( pe != 0 )
		{
			pe->_internal_accept_owner( ppx, const_cast< Y* >( py ) );
		}
	}
} // namepsace detail
*/

} // namespace efc

#endif //!ESHARED_PTR_HH_
