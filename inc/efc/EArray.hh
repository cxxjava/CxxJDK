/*
 * EArray.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef EArray_HH_
#define EArray_HH_

#include "EArrayList.hh"
#include "EComparator.hh"
#include "EIllegalArgumentException.hh"
#include "EIndexOutOfBoundsException.hh"

namespace efc {

// Dynamic length array.

/**
 * Tuning parameter: list size at or below which insertion sort will be
 * used in preference to mergesort or quicksort.
 */
#define INSERTIONSORT_THRESHOLD 7

template<typename E>
class EArray: public EArrayList<E>
{
public:
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

template<>
class EArray<int>: public EArrayList<int>
{
public:
	virtual ~EArray() {
	}

	EArray(int initialCapacity = 32) :
			EArrayList<int>(initialCapacity) {
	}

	EArray(EArray<int>& that) :
			EArrayList<int>(that) {
	}

	EArray(const EArray<int>& that) :
			EArrayList<int>(that) {
	}

	int length() {
		return EArrayList<int>::size();
	}

	int operator[](int index) THROWS(EIndexOutOfBoundsException) {
		return this->getAt(index);
	}
};

//=============================================================================

template<>
class EArray<llong>: public EArrayList<llong>
{
public:
	virtual ~EArray() {
	}

	EArray(int initialCapacity = 32) :
			EArrayList<llong>(initialCapacity) {
	}

	EArray(EArray<llong>& that) :
			EArrayList<llong>(that) {
	}

	EArray(const EArray<llong>& that) :
			EArrayList<llong>(that) {
	}

	int length() {
		return EArrayList<llong>::size();
	}

	llong operator[](int index) THROWS(EIndexOutOfBoundsException) {
		return this->getAt(index);
	}
};

} /* namespace efc */
#endif //!EArray_HH_
