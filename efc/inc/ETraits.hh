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

} /* namespace efc */

#endif //!__ETraits_H__
