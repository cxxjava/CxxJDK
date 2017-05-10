/*
 * EArray.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef EArray_HH_
#define EArray_HH_

#include "EArrayList.hh"
#include "EComparator.hh"
#include "EIllegalArgumentException.hh"
#include "EIndexOutOfBoundsException.hh"

namespace efc {

// Dynamic length array.

//=============================================================================
//Primitive Types.

template<typename E>
class EArray: public EArrayList<E>
{
public:
	virtual ~EArray() {
	}

	explicit
	EArray(int initialCapacity = 32) :
			EArrayList<E>(initialCapacity) {
	}

	int length() {
		return EArrayList<E>::size();
	}

	E operator[](int index) THROWS(EIndexOutOfBoundsException) {
		return this->getAt(index);
	}
};

//=============================================================================
//Native pointer type.

template<typename T>
class EArray<T*>: public EArrayList<T*>
{
public:
	typedef T* E;

	virtual ~EArray() {
	}

	explicit
	EArray() : EArrayList<E>() {
	}

	explicit
	EArray(boolean autoFree) : EArrayList<E>(autoFree) {
	}

	explicit
	EArray(boolean autoFree, int initialCapacity) :
			EArrayList<E>(autoFree, initialCapacity) {
	}

	int length() {
		return EArrayList<E>::size();
	}

	E operator[](int index) THROWS(EIndexOutOfBoundsException) {
		return this->getAt(index);
	}
};

//=============================================================================
//Shared pointer type.

template<typename T>
class EArray<sp<T> >: public EArrayList<sp<T> >
{
public:
	typedef sp<T> E;

	virtual ~EArray() {
	}

	explicit
	EArray(int initialCapacity = 32) :
			EArrayList<E>(initialCapacity) {
	}

	int length() {
		return EArrayList<E>::size();
	}

	E operator[](int index) THROWS(EIndexOutOfBoundsException) {
		return this->getAt(index);
	}
};

} /* namespace efc */
#endif //!EArray_HH_
