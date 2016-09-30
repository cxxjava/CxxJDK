/*
 * EA.hh
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#ifndef EA_HH_
#define EA_HH_

#include "EBase.hh"
#include "EComparable.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

// Fixed length array for EObject.

/**
 * Tuning parameter: list size at or below which insertion sort will be
 * used in preference to mergesort or quicksort.
 */
#define INSERTIONSORT_THRESHOLD 7

//=============================================================================
//EObject

template<typename E>
class EA : public EObject
{
public:
	virtual ~EA() {
		clear();
        delete[] _array;
	}

	EA(int length, boolean autoFree=true) :
			_length(length), _autoFree(autoFree) {
		_array = new E[_length]();
	}

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

	void sort(int off=0, int len=-1) {
		if (len < 0) {
			len = _length;
		}
		rangeCheck(off, off + len);

		// Insertion sort on smallest arrays
		if (len < INSERTIONSORT_THRESHOLD) {
			for (int i = off; i < len + off; i++)
				for (int j = i; j > off && _array[j - 1] && ((EComparable<E>*) _array[j - 1])->compareTo(_array[j]) > 0; j--)
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
			while (b <= c && _array[b] && (rbv = ((EComparable<E>*)_array[b])->compareTo(v)) <= 0) {
				if (rbv == 0)
					swap(a++, b);
				b++;
			}
			while (c >= b && _array[c] && (rcv = ((EComparable<E>*)_array[c])->compareTo(v)) >= 0) {
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

private:
	E* _array;
	int _length;
	boolean _autoFree;

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
//Primitive Types.

#define EA_DECLARE(E) template<> \
class EA<E> : public EObject \
{ \
public: \
	virtual ~EA() { \
		if (_owned) { \
			switch (_type) { \
			case MEM_NEW: \
				if (_array) delete[] _array; break; \
			case MEM_MALLOC: \
				if (_array) eso_free(_array); break; \
			case MEM_MMALLOC: \
				if (_array) eso_mfree(_array); break; \
			default: \
				ES_ASSERT(false); \
			} \
		} \
	} \
 \
	explicit \
	EA(int length, E defval = 0) : \
		_length(length), _owned(true) { \
		_array = new E[_length](); \
		_type = MEM_NEW; \
		if (defval != 0) { \
			for (int i = 0; i < _length; i++) { \
				_array[i] = defval; \
			} \
		} \
	} \
 \
	EA(E* data, int length) : \
 		_length(length), _owned(true), _type(MEM_NEW) { \
		_array = new E[length]; \
		eso_memcpy(_array, data, length*sizeof(E)); \
 	} \
\
	EA(E* data, int length, boolean owned, MEMType type) : \
		_array(data), _length(length), _owned(owned), _type(type) { \
	} \
 \
	EA(const EA<E>& that) { \
		EA<E>* t = (EA<E>*)&that; \
		_length = t->_length; \
		_array = t->_array; \
		_owned = t->isOwned(); \
		_type = t->_type; \
		t->setOwned(false); \
	} \
 \
	EA<E>& operator= (const EA<E>& that) { \
		if (this == &that) return *this; \
		EA<E>* t = (EA<E>*)&that; \
		if (_owned) { \
			delete[] _array; \
		} \
		_length = t->_length; \
		_array = t->_array; \
		_owned = t->isOwned(); \
		_type = t->_type; \
		t->setOwned(false); \
		return *this; \
	} \
 \
	int length() { \
		return _length; \
	} \
 \
	E* address() { \
		return _array; \
	} \
 \
	boolean isOwned() { \
		return _owned; \
	} \
 \
	void setOwned(boolean owned=true) { \
		_owned = owned; \
	} \
 \
	MEMType memoryType() { \
		return _type; \
	} \
 \
	E& operator[](int index) THROWS(EIndexOutOfBoundsException) { \
		if (index < 0 || index >= _length) { \
			throw EINDEXOUTOFBOUNDSEXCEPTION; \
		} \
		return _array[index]; \
	} \
 \
	E& getAt(int index) THROWS(EIndexOutOfBoundsException) { \
		if (index < 0 || index >= _length) { \
			throw EINDEXOUTOFBOUNDSEXCEPTION; \
		} \
		return _array[index]; \
	} \
 \
	E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) { \
		if (index < 0 || index >= _length) { \
			throw EINDEXOUTOFBOUNDSEXCEPTION; \
		} \
		E old = _array[index]; \
		_array[index] = element; \
		return old; \
	} \
 \
	void reset(E val=0) { \
		for (int i = 0; i < _length; i++) { \
			_array[i] = val; \
		} \
	} \
 \
	void sort(int off=0, int len=-1) { \
		if (len < 0) { \
			len = _length; \
		} \
		rangeCheck(off, off + len); \
 \
		if (len < INSERTIONSORT_THRESHOLD) { \
			for (int i = off; i < len + off; i++) \
				for (int j = i; j > off && _array[j - 1] > _array[j]; j--) \
					swap(j, j - 1); \
			return; \
		} \
 \
		int m = off + (len >> 1); \
		if (len > INSERTIONSORT_THRESHOLD) { \
			int l = off; \
			int n = off + len - 1; \
			if (len > 40) { \
				int s = len / 8; \
				l = med3(l, l + s, l + 2 * s); \
				m = med3(m - s, m, m + s); \
				n = med3(n - 2 * s, n - s, n); \
			} \
			m = med3(l, m, n); \
		} \
		E v = _array[m]; \
 \
		int a = off, b = a, c = off + len - 1, d = c; \
		while (true) { \
			while (b <= c && _array[b] <= v) { \
				if (_array[b] == v) \
					swap(a++, b); \
				b++; \
			} \
			while (c >= b && _array[c] >= v) { \
				if (_array[c] == v) \
					swap(c, d--); \
				c--; \
			} \
			if (b > c) \
				break; \
			swap(b++, c--); \
		} \
 \
		int s, n = off + len; \
		s = ES_MIN(a-off, b-a ); \
		vecswap(off, b - s, s); \
		s = ES_MIN(d-c, n-d-1); \
		vecswap(b, n - s, s); \
 \
		if ((s = b - a) > 1) \
			sort(off, s); \
		if ((s = d - c) > 1) \
			sort(n - s, s); \
	} \
 \
	EA<E>* clone(int offset=0, int newLength=-1) { \
		if (offset < 0 || offset >= _length) { \
			throw EIndexOutOfBoundsException(__FILE__, __LINE__); \
		} \
 \
		if (newLength < 0) { \
			newLength = _length - offset; \
		} \
 \
		EA<E>* ea = new EA<E>(newLength); \
		eso_memcpy(ea->address(), _array + offset, ES_MIN(_length - offset, newLength)*sizeof(E)); \
		return ea; \
	} \
 \
private: \
	E* _array; \
	int _length; \
	boolean _owned; \
	MEMType _type; \
 \
	void rangeCheck(int fromIndex, int toIndex) { \
		EString msg; \
		if (fromIndex > toIndex) { \
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex); \
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str()); \
		} \
 \
		if (fromIndex < 0) { \
			msg.format("fromIndex(%d)", fromIndex); \
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str()); \
		} \
 \
		if (toIndex > _length) { \
			msg.format("toIndex(%d)", toIndex); \
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, msg.c_str()); \
		} \
	} \
 \
	void swap(int a, int b) { \
		E t = _array[a]; \
		_array[a] = _array[b]; \
		_array[b] = t; \
	} \
 \
	void vecswap(int a, int b, int n) { \
		for (int i = 0; i < n; i++, a++, b++) \
			swap(a, b); \
	} \
 \
	int med3(int a, int b, int c) { \
		return (_array[a] < _array[b] ? \
				(_array[b] < _array[c] ? b : _array[a] < _array[c] ? c : a) : \
				(_array[b] > _array[c] ? b : _array[a] > _array[c] ? c : a)); \
	} \
};

EA_DECLARE(byte)
EA_DECLARE(char)
EA_DECLARE(int)
EA_DECLARE(short)
EA_DECLARE(long)
EA_DECLARE(llong)
EA_DECLARE(float)
EA_DECLARE(double)

} /* namespace efc */
#endif //!EA_HH_
