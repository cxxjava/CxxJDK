#ifndef __ETraits_H__
#define __ETraits_H__

#include "ESharedPtr.hh"

namespace efc {

//@see: STL iterator traits && http://blog.csdn.net/xuqingict/article/details/38453929

//Primitive Types.
template<class T>
struct ETraits
{
    typedef T valueType;
    typedef T* pointer;
    typedef T& reference;
    typedef T indexType;
};

//Native pointer type.
template<class T>
struct ETraits<T*>
{
    typedef T valueType;
    typedef T* pointer;
    typedef T& reference;
    typedef T* indexType;
};

//Native pointer type.
template<class T>
struct ETraits<const T*>
{
    typedef T valueType;
    typedef const T* pointer;
    typedef const T& reference;
    typedef T* indexType;
};

//Shared pointer type.
template<class T>
struct ETraits<sp<T> >
{
    typedef T valueType;
    typedef T* pointer;
    typedef sp<T>& reference;
    typedef T* indexType;
};


//=============================================================================


template<typename T> inline boolean obj_is_null(T o) { return false; }
template<typename T> inline boolean obj_is_null(sp<T> o) { return o==null; }
template<typename T> inline boolean obj_is_null(T* o) { return o==null; }

template<typename T> inline boolean obj_is_equals(T o1, T o2) { return o1==o2; }
template<typename T> inline boolean obj_is_equals(T* o1, sp<T> o2) { return o1->equals(o2.get()); }
template<typename T> inline boolean obj_is_equals(T* o1, T* o2) { return o1->equals(o2); }

template<typename T> inline T obj_get_index(T o) { return o; }
template<typename T> inline T* obj_get_index(sp<T> o) { return o.get(); }

template<typename T> inline EString obj_get_string(T o) { return EString(o); }
template<typename T> inline EString obj_get_string(sp<T> o) { return o->toString(); }
template<typename T> inline EString obj_get_string(T* o) { return o->toString(); }


} /* namespace efc */

#endif //!__ETraits_H__
