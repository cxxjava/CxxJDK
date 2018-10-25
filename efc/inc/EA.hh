/*
 * EA.hh
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#ifndef EA_HH_
#define EA_HH_

#include "EBase.hh"
#include "ESharedPtr.hh"
#include "EComparable.hh"
#include "EIterable.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalArgumentException.hh"

#ifdef CPP11_SUPPORT_FULL
#include <initializer_list>
#endif

namespace efc {

// Fixed length array for EObject.

/**
 * Tuning parameter: list size at or below which insertion sort will be
 * used in preference to mergesort or quicksort.
 */
#define INSERTIONSORT_THRESHOLD 7

//=============================================================================
//Primitive Types.

template<typename E>
class EA : public EIterable<E>
{
public:
	virtual ~EA() {
		if (_owned) {
			switch (_type) {
			case MEM_NEW:
				if (_array) delete[] _array; break;
			case MEM_MALLOC:
				if (_array) eso_free(_array); break;
			case MEM_MMALLOC:
				if (_array) eso_mfree(_array); break;
			default:
				ES_ASSERT(false);
				break;
			}
		}
	}

	explicit
	EA(int length, E defval = 0) :
		_length(length), _owned(true), _defval(defval) {
		_array = new E[_length]();
		_type = MEM_NEW;
		if (defval != 0) {
			for (int i = 0; i < _length; i++) {
				_array[i] = defval;
			}
		}
	}

	EA(E* data, int length) :
 		_length(length), _owned(true), _type(MEM_NEW) {
		_array = new E[length];
		eso_memcpy(_array, data, length*sizeof(E));
 	}

	EA(E* data, int length, boolean owned, MEMType type) :
		_array(data), _length(length), _owned(owned), _type(type) {
	}

#ifdef CPP11_SUPPORT_FULL
	EA(std::initializer_list<E> l) :
		_length(l.size()), _owned(true), _type(MEM_NEW) {
		_array = new E[_length];
		int i=0;
		for (auto v : l) {
			_array[i++] = v;
		}
	}
#endif

	EA(const EA<E>& that) {
		EA<E>* t = (EA<E>*)&that;
		_length = t->_length;
		_array = new E[_length];
		_owned = true;
		_type = MEM_NEW;
		eso_memcpy(_array, t->_array, _length*sizeof(E));
	}

	EA<E>& operator= (const EA<E>& that) {
		if (this == &that) return *this;
		EA<E>* t = (EA<E>*)&that;
		if (_owned) {
			delete[] _array;
		}
		_length = t->_length;
		_array = new E[_length];
		_owned = true;
		_type = MEM_NEW;
		eso_memcpy(_array, t->_array, _length*sizeof(E));
		return *this;
	}

	int length() {
		return _length;
	}

	E* address() {
		return _array;
	}

	boolean isOwned() {
		return _owned;
	}

	void setOwned(boolean owned=true) {
		_owned = owned;
	}

	MEMType memoryType() {
		return _type;
	}

	E& operator[](int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	E& getAt(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		E old = _array[index];
		_array[index] = element;
		return old;
	}

	void reset(E val=0) {
		for (int i = 0; i < _length; i++) {
			_array[i] = val;
		}
	}

	void setLength(int newLength) {
		if (newLength <= _length) {
			_length = newLength;
			return;
		} else {
			E* newArr = new E[newLength]();
			eso_memcpy(newArr, _array, _length*sizeof(E));
			if (_defval != 0) {
				for (int i = _length; i < newLength; i++) {
					newArr[i] = _defval;
				}
			}

			if (_owned) {
				switch (_type) {
				case MEM_NEW:
					if (_array) delete[] _array; break;
				case MEM_MALLOC:
					if (_array) eso_free(_array); break;
				case MEM_MMALLOC:
					if (_array) eso_mfree(_array); break;
				default:
					ES_ASSERT(false);
					break;
				}
			}

			_length = newLength;
			_array = newArr;
			_type = MEM_NEW;
		}
	}

	void sort(int off=0, int len=-1) {
		if (len < 0) {
			len = _length;
		}
		rangeCheck(off, off + len);

		// Insertion sort on smallest arrays
		if (len < INSERTIONSORT_THRESHOLD) {
			for (int i = off; i < len + off; i++)
				for (int j = i; j > off && _array[j - 1] > _array[j]; j--)
					swap(j, j - 1);
			return;
		}

		// Choose a partition element, v
		int m = off + (len >> 1);
		if (len > INSERTIONSORT_THRESHOLD) {
			int l = off;
			int n = off + len - 1;
			if (len > 40) {
				int s = len / 8;
				l = med3(l, l + s, l + 2 * s);
				m = med3(m - s, m, m + s);
				n = med3(n - 2 * s, n - s, n);
			}
			m = med3(l, m, n);
		}
		E v = _array[m];

		int a = off, b = a, c = off + len - 1, d = c;
		while (true) {
			while (b <= c && _array[b] <= v) {
				if (_array[b] == v)
					swap(a++, b);
				b++;
			}
			while (c >= b && _array[c] >= v) {
				if (_array[c] == v)
					swap(c, d--);
				c--;
			}
			if (b > c)
				break;
			swap(b++, c--);
		}

		int s, n = off + len;
		s = ES_MIN(a-off, b-a );
		vecswap(off, b - s, s);
		s = ES_MIN(d-c, n-d-1);
		vecswap(b, n - s, s);

		if ((s = b - a) > 1)
			sort(off, s);
		if ((s = d - c) > 1)
			sort(n - s, s);
	}

	EA<E>* clone(int offset=0, int newLength=-1) {
		if (offset < 0 || offset >= _length) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}

		if (newLength < 0) {
			newLength = _length - offset;
		}

		EA<E>* ea = new EA<E>(newLength);
		eso_memcpy(ea->address(), _array + offset, ES_MIN(_length - offset, newLength)*sizeof(E));
		return ea;
	}

	virtual sp<EIterator<E> > iterator(int index=0) {
		return new Iter(this, index);
	}

private:
	E* _array;
	int _length;
	boolean _owned;
	MEMType _type;
	E _defval;

	class Iter : public EIterator<E> {
	public:
		EA<E>* self;
		int index;
		Iter(EA<E>* a, int i) {
			self = a;
			index = i;
		}
		virtual boolean hasNext() {
			return index < self->length();
		}
		virtual E next() {
			return (*self)[index++];
		}
		virtual void remove() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		virtual E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void rangeCheck(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > _length) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}

	/**
	 * Swaps x[a] with x[b].
	 */
	void swap(int a, int b) {
		if (a == b) return;
		E t = _array[a];
		_array[a] = _array[b];
		_array[b] = t;
	}

	/**
	 * Swaps x[a .. (a+n-1)] with x[b .. (b+n-1)].
	 */
	void vecswap(int a, int b, int n) {
		for (int i = 0; i < n; i++, a++, b++)
			swap(a, b);
	}

	/**
	 * Returns the index of the median of the three indexed integers.
	 */
	int med3(int a, int b, int c) {
		return (_array[a] < _array[b] ?
				(_array[b] < _array[c] ? b : _array[a] < _array[c] ? c : a) :
				(_array[b] > _array[c] ? b : _array[a] > _array[c] ? c : a));
	}
};

//=============================================================================
//Native pointer type.

template<typename T>
class EA<T*> : public EIterable<T*>
{
public:
	typedef T* E;

public:
	virtual ~EA() {
		clear();
        delete[] _array;
	}

	EA(int length, boolean autoFree=true) :
			_length(length), _autoFree(autoFree) {
		_array = new E[_length]();
	}

#ifdef CPP11_SUPPORT_FULL
	EA(std::initializer_list<E> l) :
		_length(l.size()), _autoFree(true) {
		_array = new E[_length];
		int i=0;
		for (auto v : l) {
			_array[i++] = v;
		}
	}
#endif

	EA(const EA<E>& that) : _autoFree(true) {
		EA<E>* t = (EA<E>*)&that;

		_length = t->_length;
		_array = new E[_length];
		eso_memcpy(_array, t->_array, _length*sizeof(E));
		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);
	}

	EA<E>& operator= (const EA<E>& that) {
		if (this == &that) return *this;

		EA<E>* t = (EA<E>*)&that;

		//1.
		if (_autoFree) {
			for (int i=0; i<_length; i++) {
				delete _array[i];
			}
		}
		delete[] _array;

		//2.
		_length = t->_length;
		_array = new E[_length];
		eso_memcpy(_array, t->_array, _length*sizeof(E));
		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);

		return *this;
	}

	int length() {
		return _length;
	}

	E* address() {
		return _array;
	}

	E& operator[](int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param  index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	E& getAt(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		E old = _array[index];
		_array[index] = element;
		return old;
	}

	void clear() {
		if (_autoFree) {
			for (int i=0; i<_length; i++) {
				delete _array[i];
			}
		}
		eso_memset(_array, 0, sizeof(E)*_length);
	}

	void setLength(int newLength) {
		if (newLength <= _length) {
			if (_autoFree) {
				for (int i=newLength; i<_length; i++) {
					delete _array[i];
				}
			}

			_length = newLength;
		} else {
			E* newArr = new E[newLength]();
			eso_memcpy(newArr, _array, _length*sizeof(E));
			delete[] _array;
			_array = newArr;
			_length = newLength;
		}
	}

	void sort(int off=0, int len=-1) {
		if (len < 0) {
			len = _length;
		}
		rangeCheck(off, off + len);

		// Insertion sort on smallest arrays
		if (len < INSERTIONSORT_THRESHOLD) {
			for (int i = off; i < len + off; i++)
				for (int j = i; j > off && _array[j - 1] && _array[j - 1]->compareTo(_array[j]) > 0; j--)
					swap(j, j - 1);
			return;
		}

		// Choose a partition element, v
		int m = off + (len >> 1); // Small arrays, middle element
		if (len > INSERTIONSORT_THRESHOLD) {
			int l = off;
			int n = off + len - 1;
			if (len > 40) { // Big arrays, pseudomedian of 9
				int s = len / 8;
				l = med3(l, l + s, l + 2 * s);
				m = med3(m - s, m, m + s);
				n = med3(n - 2 * s, n - s, n);
			}
			m = med3(l, m, n); // Mid-size, med of 3
		}
		E v = _array[m];

		// Establish Invariant: v* (<v)* (>v)* v*
		int a = off, b = a, c = off + len - 1, d = c;
		int rbv, rcv;
		while (true) {
			while (b <= c && _array[b] && (rbv = _array[b]->compareTo(v)) <= 0) {
				if (rbv == 0)
					swap(a++, b);
				b++;
			}
			while (c >= b && _array[c] && (rcv = _array[c]->compareTo(v)) >= 0) {
				if (rcv == 0)
					swap(c, d--);
				c--;
			}
			if (b > c)
				break;
			swap(b++, c--);
		}

		// Swap partition elements back to middle
		int s, n = off + len;
		s = ES_MIN(a-off, b-a );
		vecswap(off, b - s, s);
		s = ES_MIN(d-c, n-d-1);
		vecswap(b, n - s, s);

		// Recursively sort non-partition-elements
		if ((s = b - a) > 1)
			sort(off, s);
		if ((s = d - c) > 1)
			sort(n - s, s);
	}

	/**
	 * Returns a shallow copy of this list.  (The elements themselves
	 * are not copied.)
	 *
	 * @return a clone of this list
	 */
	EA<E>* clone(int offset=0, int newLength=-1) {
		if (offset < 0 || offset >= _length) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}

		if (newLength < 0) {
			newLength = _length - offset;
		}

		EA<E>* newEA = new EA<E>(newLength, false);
		eso_memcpy(newEA->_array, _array + offset, ES_MIN(_length - offset, newLength)*sizeof(E));
		return newEA;
	}

	/**
	 *
	 */
	void setAutoFree(boolean autoFree = true ) {
		_autoFree = autoFree;
	}

	/**
	 *
	 */
	boolean getAutoFree() {
		return _autoFree;
	}

	virtual sp<EIterator<E> > iterator(int index=0) {
		return new Iter(this, index);
	}

private:
	E* _array;
	int _length;
	boolean _autoFree;

	class Iter : public EIterator<E> {
	public:
		EA<E>* self;
		int index;
		Iter(EA<E>* a, int i) {
			self = a;
			index = i;
		}
		virtual boolean hasNext() {
			return index < self->length();
		}
		virtual E next() {
			return (*self)[index++];
		}
		virtual void remove() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		virtual E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void rangeCheck(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > _length) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}

	/**
	 * Swaps x[a] with x[b].
	 */
	void swap(int a, int b) {
		if (a == b) return;

		E t = _array[a];
		_array[a] = _array[b];
		_array[b] = t;
	}

	/**
	 * Swaps x[a .. (a+n-1)] with x[b .. (b+n-1)].
	 */
	void vecswap(int a, int b, int n) {
		for (int i = 0; i < n; i++, a++, b++)
			swap(a, b);
	}

	/**
	 * Returns the index of the median of the three indexed integers.
	 */
	int med3(int a, int b, int c) {
		return (_array[a] == null || _array[a]->compareTo(_array[b]) < 0 ?
				(_array[b] == null || _array[b]->compareTo(_array[c]) < 0 ? b : _array[a] == null || _array[a]->compareTo(_array[c]) < 0 ? c : a) :
				(_array[b] != null && _array[b]->compareTo(_array[c]) > 0 ? b : _array[a] != null && _array[a]->compareTo(_array[c]) > 0 ? c : a));
	}
};

template<>
class EA<EString*> : public EIterable<EString*>
{
public:
	typedef EString* E;

public:
	virtual ~EA() {
		clear();
        delete[] _array;
	}

	EA(int length, boolean autoFree=true) :
			_length(length), _autoFree(autoFree) {
		_array = new E[_length]();
	}

#ifdef CPP11_SUPPORT_FULL
	EA(std::initializer_list<E> l) :
		_length(l.size()), _autoFree(true) {
		_array = new E[_length];
		int i=0;
		for (auto v : l) {
			_array[i++] = v;
		}
	}

	EA(std::initializer_list<const char*> l) :
		_length(l.size()), _autoFree(true) {
		_array = new E[_length];
		int i=0;
		for (auto v : l) {
			_array[i++] = new EString(v);
		}
	}
#endif

	EA(const EA<E>& that) : _autoFree(true) {
		EA<E>* t = (EA<E>*)&that;

		_length = t->_length;
		_array = new E[_length];
		eso_memcpy(_array, t->_array, _length*sizeof(E));
		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);
	}

	EA<E>& operator= (const EA<E>& that) {
		if (this == &that) return *this;

		EA<E>* t = (EA<E>*)&that;

		//1.
		if (_autoFree) {
			for (int i=0; i<_length; i++) {
				delete _array[i];
			}
		}
		delete[] _array;

		//2.
		_length = t->_length;
		_array = new E[_length];
		eso_memcpy(_array, t->_array, _length*sizeof(E));
		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);

		return *this;
	}

	int length() {
		return _length;
	}

	E* address() {
		return _array;
	}

	E& operator[](int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param  index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	E& getAt(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		E old = _array[index];
		_array[index] = element;
		return old;
	}

	E setAt(int index, const char* element) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		E old = _array[index];
		_array[index] = new EString(element);
		return old;
	}

	void clear() {
		if (_autoFree) {
			for (int i=0; i<_length; i++) {
				delete _array[i];
			}
		}
		eso_memset(_array, 0, sizeof(E)*_length);
	}

	void setLength(int newLength) {
		if (newLength <= _length) {
			if (_autoFree) {
				for (int i=newLength; i<_length; i++) {
					delete _array[i];
				}
			}

			_length = newLength;
		} else {
			E* newArr = new E[newLength]();
			eso_memcpy(newArr, _array, _length*sizeof(E));
			delete[] _array;
			_array = newArr;
			_length = newLength;
		}
	}

	void sort(int off=0, int len=-1) {
		if (len < 0) {
			len = _length;
		}
		rangeCheck(off, off + len);

		// Insertion sort on smallest arrays
		if (len < INSERTIONSORT_THRESHOLD) {
			for (int i = off; i < len + off; i++)
				for (int j = i; j > off && _array[j - 1] && _array[j - 1]->compareTo(_array[j]) > 0; j--)
					swap(j, j - 1);
			return;
		}

		// Choose a partition element, v
		int m = off + (len >> 1); // Small arrays, middle element
		if (len > INSERTIONSORT_THRESHOLD) {
			int l = off;
			int n = off + len - 1;
			if (len > 40) { // Big arrays, pseudomedian of 9
				int s = len / 8;
				l = med3(l, l + s, l + 2 * s);
				m = med3(m - s, m, m + s);
				n = med3(n - 2 * s, n - s, n);
			}
			m = med3(l, m, n); // Mid-size, med of 3
		}
		E v = _array[m];

		// Establish Invariant: v* (<v)* (>v)* v*
		int a = off, b = a, c = off + len - 1, d = c;
		int rbv, rcv;
		while (true) {
			while (b <= c && _array[b] && (rbv = _array[b]->compareTo(v)) <= 0) {
				if (rbv == 0)
					swap(a++, b);
				b++;
			}
			while (c >= b && _array[c] && (rcv = _array[c]->compareTo(v)) >= 0) {
				if (rcv == 0)
					swap(c, d--);
				c--;
			}
			if (b > c)
				break;
			swap(b++, c--);
		}

		// Swap partition elements back to middle
		int s, n = off + len;
		s = ES_MIN(a-off, b-a );
		vecswap(off, b - s, s);
		s = ES_MIN(d-c, n-d-1);
		vecswap(b, n - s, s);

		// Recursively sort non-partition-elements
		if ((s = b - a) > 1)
			sort(off, s);
		if ((s = d - c) > 1)
			sort(n - s, s);
	}

	/**
	 * Returns a shallow copy of this list.  (The elements themselves
	 * are not copied.)
	 *
	 * @return a clone of this list
	 */
	EA<E>* clone(int offset=0, int newLength=-1) {
		if (offset < 0 || offset >= _length) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}

		if (newLength < 0) {
			newLength = _length - offset;
		}

		EA<E>* newEA = new EA<E>(newLength, false);
		eso_memcpy(newEA->_array, _array + offset, ES_MIN(_length - offset, newLength)*sizeof(E));
		return newEA;
	}

	/**
	 *
	 */
	void setAutoFree(boolean autoFree = true ) {
		_autoFree = autoFree;
	}

	/**
	 *
	 */
	boolean getAutoFree() {
		return _autoFree;
	}

	virtual sp<EIterator<E> > iterator(int index=0) {
		return new Iter(this, index);
	}

private:
	E* _array;
	int _length;
	boolean _autoFree;

	class Iter : public EIterator<E> {
	public:
		EA<E>* self;
		int index;
		Iter(EA<E>* a, int i) {
			self = a;
			index = i;
		}
		virtual boolean hasNext() {
			return index < self->length();
		}
		virtual E next() {
			return (*self)[index++];
		}
		virtual void remove() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		virtual E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void rangeCheck(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > _length) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}

	/**
	 * Swaps x[a] with x[b].
	 */
	void swap(int a, int b) {
		if (a == b) return;

		E t = _array[a];
		_array[a] = _array[b];
		_array[b] = t;
	}

	/**
	 * Swaps x[a .. (a+n-1)] with x[b .. (b+n-1)].
	 */
	void vecswap(int a, int b, int n) {
		for (int i = 0; i < n; i++, a++, b++)
			swap(a, b);
	}

	/**
	 * Returns the index of the median of the three indexed integers.
	 */
	int med3(int a, int b, int c) {
		return (_array[a] == null || _array[a]->compareTo(_array[b]) < 0 ?
				(_array[b] == null || _array[b]->compareTo(_array[c]) < 0 ? b : _array[a] == null || _array[a]->compareTo(_array[c]) < 0 ? c : a) :
				(_array[b] != null && _array[b]->compareTo(_array[c]) > 0 ? b : _array[a] != null && _array[a]->compareTo(_array[c]) > 0 ? c : a));
	}
};

//=============================================================================
//SharedPtr Types.

template<typename T>
class EA<sp<T> > : public EIterable<sp<T> >
{
	typedef sp<T> E;
public:
	virtual ~EA() {
        delete[] _array;
	}

	EA(int length) : _length(length) {
		_array = new E[_length]();
	}

#ifdef CPP11_SUPPORT_FULL
	EA(std::initializer_list<E> l) :
		_length(l.size()) {
		_array = new E[_length];
		int i=0;
		for (auto v : l) {
			_array[i++] = v;
		}
	}
#endif

	EA(const EA<E>& that) {
		EA<E>* t = (EA<E>*)&that;

		_length = t->_length;
		_array = new E[_length];
		for (int i = 0; i < _length; i++) {
			_array[i] = (*t)[i];
		}
	}

	EA<E>& operator= (const EA<E>& that) {
		if (this == &that) return *this;

		EA<E>* t = (EA<E>*)&that;

		//1.
		delete[] _array;

		//2.
		_length = t->_length;
		_array = new E[_length];
		for (int i = 0; i < _length; i++) {
			_array[i] = (*t)[i];
		}

		return *this;
	}

	int length() {
		return _length;
	}

	E* address() {
		return _array;
	}

	E& operator[](int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Returns a shallow copy of this list.  (The elements themselves
	 * are not copied.)
	 *
	 * @return a clone of this list
	 */
	EA<E>* clone(int offset=0, int newLength=-1) {
		if (offset < 0 || offset >= _length) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}

		if (newLength < 0) {
			newLength = _length - offset;
		}

		EA<E>* newEA = new EA<E>(newLength);
		int n = ES_MIN(_length - offset, newLength);
		for (int i=0; i<n; i++) {
			(*newEA)[i] = _array[i+offset];
		}

		return newEA;
	}

	E& getAt(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		E old = _array[index];
		_array[index] = element;
		return old;
	}

	void clear() {
		for (int i=0; i<_length; i++) {
			_array[i] = null;
		}
	}

	void setLength(int newLength) {
		if (newLength <= _length) {
			for (int i=newLength; i<_length; i++) {
				_array[i] = null;
			}

			_length = newLength;
		} else {
			E* newArr = new E[newLength];
			for (int i=0; i<_length; i++) {
				newArr[i] = _array[i];
			}
			delete[] _array;
			_array = newArr;
			_length = newLength;
		}
	}

	void sort(int off=0, int len=-1) {
		if (len < 0) {
			len = _length;
		}
		rangeCheck(off, off + len);

		// Insertion sort on smallest arrays
		if (len < INSERTIONSORT_THRESHOLD) {
			for (int i = off; i < len + off; i++)
				for (int j = i; j > off && (_array[j - 1] != null) && _array[j - 1]->compareTo(_array[j].get()) > 0; j--)
					swap(j, j - 1);
			return;
		}

		// Choose a partition element, v
		int m = off + (len >> 1); // Small arrays, middle element
		if (len > INSERTIONSORT_THRESHOLD) {
			int l = off;
			int n = off + len - 1;
			if (len > 40) { // Big arrays, pseudomedian of 9
				int s = len / 8;
				l = med3(l, l + s, l + 2 * s);
				m = med3(m - s, m, m + s);
				n = med3(n - 2 * s, n - s, n);
			}
			m = med3(l, m, n); // Mid-size, med of 3
		}
		E v = _array[m];

		// Establish Invariant: v* (<v)* (>v)* v*
		int a = off, b = a, c = off + len - 1, d = c;
		int rbv, rcv;
		while (true) {
			while (b <= c && (_array[b] != null) && (rbv = _array[b]->compareTo(v.get())) <= 0) {
				if (rbv == 0)
					swap(a++, b);
				b++;
			}
			while (c >= b && (_array[c] != null) && (rcv = _array[c]->compareTo(v.get())) >= 0) {
				if (rcv == 0)
					swap(c, d--);
				c--;
			}
			if (b > c)
				break;
			swap(b++, c--);
		}

		// Swap partition elements back to middle
		int s, n = off + len;
		s = ES_MIN(a-off, b-a );
		vecswap(off, b - s, s);
		s = ES_MIN(d-c, n-d-1);
		vecswap(b, n - s, s);

		// Recursively sort non-partition-elements
		if ((s = b - a) > 1)
			sort(off, s);
		if ((s = d - c) > 1)
			sort(n - s, s);
	}

	E atomicGet(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return atomic_load(&_array[index]);
	}

	void atomicSet(int index, E& e) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		atomic_store(&_array[index], e);
	}

	virtual sp<EIterator<E> > iterator(int index=0) {
		return new Iter(this, index);
	}

private:
	E* _array;
	int _length;

	class Iter : public EIterator<E> {
	public:
		EA<E>* self;
		int index;
		Iter(EA<E>* a, int i) {
			self = a;
			index = i;
		}
		virtual boolean hasNext() {
			return index < self->length();
		}
		virtual E next() {
			return (*self)[index++];
		}
		virtual void remove() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		virtual E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void rangeCheck(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > _length) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}

	/**
	 * Swaps x[a] with x[b].
	 */
	void swap(int a, int b) {
		if (a == b) return;

		E t = _array[a];
		_array[a] = _array[b];
		_array[b] = t;
	}

	/**
	 * Swaps x[a .. (a+n-1)] with x[b .. (b+n-1)].
	 */
	void vecswap(int a, int b, int n) {
		for (int i = 0; i < n; i++, a++, b++)
			swap(a, b);
	}

	/**
	 * Returns the index of the median of the three indexed integers.
	 */
	int med3(int a, int b, int c) {
		return (_array[a] == null || _array[a]->compareTo(_array[b].get()) < 0 ?
				(_array[b] == null || _array[b]->compareTo(_array[c].get()) < 0 ? b : _array[a] == null || _array[a]->compareTo(_array[c].get()) < 0 ? c : a) :
				(_array[b] != null && _array[b]->compareTo(_array[c].get()) > 0 ? b : _array[a] != null && _array[a]->compareTo(_array[c].get()) > 0 ? c : a));
	}
};

template<>
class EA<sp<EString> > : public EIterable<sp<EString> >
{
	typedef sp<EString> E;
public:
	virtual ~EA() {
        delete[] _array;
	}

	EA(int length) : _length(length) {
		_array = new E[_length]();
	}

#ifdef CPP11_SUPPORT_FULL
	EA(std::initializer_list<E> l) :
		_length(l.size()) {
		_array = new E[_length];
		int i=0;
		for (auto v : l) {
			_array[i++] = v;
		}
	}

	EA(std::initializer_list<const char*> l) :
		_length(l.size()) {
		_array = new E[_length];
		int i=0;
		for (auto v : l) {
			_array[i++] = new EString(v);
		}
	}
#endif

	EA(const EA<E>& that) {
		EA<E>* t = (EA<E>*)&that;

		_length = t->_length;
		_array = new E[_length];
		for (int i = 0; i < _length; i++) {
			_array[i] = (*t)[i];
		}
	}

	EA<E>& operator= (const EA<E>& that) {
		if (this == &that) return *this;

		EA<E>* t = (EA<E>*)&that;

		//1.
		delete[] _array;

		//2.
		_length = t->_length;
		_array = new E[_length];
		for (int i = 0; i < _length; i++) {
			_array[i] = (*t)[i];
		}

		return *this;
	}

	int length() {
		return _length;
	}

	E* address() {
		return _array;
	}

	E& operator[](int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Returns a shallow copy of this list.  (The elements themselves
	 * are not copied.)
	 *
	 * @return a clone of this list
	 */
	EA<E>* clone(int offset=0, int newLength=-1) {
		if (offset < 0 || offset >= _length) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}

		if (newLength < 0) {
			newLength = _length - offset;
		}

		EA<E>* newEA = new EA<E>(newLength);
		int n = ES_MIN(_length - offset, newLength);
		for (int i=0; i<n; i++) {
			(*newEA)[i] = _array[i+offset];
		}

		return newEA;
	}

	E& getAt(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return _array[index];
	}

	/**
	 * Replaces the element at the specified position in this list with
	 * the specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		E old = _array[index];
		_array[index] = element;
		return old;
	}

	E setAt(int index, const char* element) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		E old = _array[index];
		_array[index] = new EString(element);
		return old;
	}

	void clear() {
		for (int i=0; i<_length; i++) {
			_array[i] = null;
		}
	}

	void setLength(int newLength) {
		if (newLength <= _length) {
			for (int i=newLength; i<_length; i++) {
				_array[i] = null;
			}

			_length = newLength;
		} else {
			E* newArr = new E[newLength];
			for (int i=0; i<_length; i++) {
				newArr[i] = _array[i];
			}
			delete[] _array;
			_array = newArr;
			_length = newLength;
		}
	}

	void sort(int off=0, int len=-1) {
		if (len < 0) {
			len = _length;
		}
		rangeCheck(off, off + len);

		// Insertion sort on smallest arrays
		if (len < INSERTIONSORT_THRESHOLD) {
			for (int i = off; i < len + off; i++)
				for (int j = i; j > off && (_array[j - 1] != null) && _array[j - 1]->compareTo(_array[j].get()) > 0; j--)
					swap(j, j - 1);
			return;
		}

		// Choose a partition element, v
		int m = off + (len >> 1); // Small arrays, middle element
		if (len > INSERTIONSORT_THRESHOLD) {
			int l = off;
			int n = off + len - 1;
			if (len > 40) { // Big arrays, pseudomedian of 9
				int s = len / 8;
				l = med3(l, l + s, l + 2 * s);
				m = med3(m - s, m, m + s);
				n = med3(n - 2 * s, n - s, n);
			}
			m = med3(l, m, n); // Mid-size, med of 3
		}
		E v = _array[m];

		// Establish Invariant: v* (<v)* (>v)* v*
		int a = off, b = a, c = off + len - 1, d = c;
		int rbv, rcv;
		while (true) {
			while (b <= c && (_array[b] != null) && (rbv = _array[b]->compareTo(v.get())) <= 0) {
				if (rbv == 0)
					swap(a++, b);
				b++;
			}
			while (c >= b && (_array[c] != null) && (rcv = _array[c]->compareTo(v.get())) >= 0) {
				if (rcv == 0)
					swap(c, d--);
				c--;
			}
			if (b > c)
				break;
			swap(b++, c--);
		}

		// Swap partition elements back to middle
		int s, n = off + len;
		s = ES_MIN(a-off, b-a );
		vecswap(off, b - s, s);
		s = ES_MIN(d-c, n-d-1);
		vecswap(b, n - s, s);

		// Recursively sort non-partition-elements
		if ((s = b - a) > 1)
			sort(off, s);
		if ((s = d - c) > 1)
			sort(n - s, s);
	}

	E atomicGet(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return atomic_load(&_array[index]);
	}

	void atomicSet(int index, E& e) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		atomic_store(&_array[index], e);
	}

	virtual sp<EIterator<E> > iterator(int index=0) {
		return new Iter(this, index);
	}

private:
	E* _array;
	int _length;

	class Iter : public EIterator<E> {
	public:
		EA<E>* self;
		int index;
		Iter(EA<E>* a, int i) {
			self = a;
			index = i;
		}
		virtual boolean hasNext() {
			return index < self->length();
		}
		virtual E next() {
			return (*self)[index++];
		}
		virtual void remove() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
		virtual E moveOut() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}
	};

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void rangeCheck(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}

		if (toIndex > _length) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str());
		}
	}

	/**
	 * Swaps x[a] with x[b].
	 */
	void swap(int a, int b) {
		if (a == b) return;

		E t = _array[a];
		_array[a] = _array[b];
		_array[b] = t;
	}

	/**
	 * Swaps x[a .. (a+n-1)] with x[b .. (b+n-1)].
	 */
	void vecswap(int a, int b, int n) {
		for (int i = 0; i < n; i++, a++, b++)
			swap(a, b);
	}

	/**
	 * Returns the index of the median of the three indexed integers.
	 */
	int med3(int a, int b, int c) {
		return (_array[a] == null || _array[a]->compareTo(_array[b].get()) < 0 ?
				(_array[b] == null || _array[b]->compareTo(_array[c].get()) < 0 ? b : _array[a] == null || _array[a]->compareTo(_array[c].get()) < 0 ? c : a) :
				(_array[b] != null && _array[b]->compareTo(_array[c].get()) > 0 ? b : _array[a] != null && _array[a]->compareTo(_array[c].get()) > 0 ? c : a));
	}
};

} /* namespace efc */
#endif //!EA_HH_
