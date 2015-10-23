/*
 * ESharedArr.hh
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#ifndef ESHAREDARR_HH_
#define ESHAREDARR_HH_

#include "ESharedPtr.hh"
#include "EComparable.hh"
#include "EInteger.hh"
#include "EOutOfMemoryError.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalArgumentException.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

/**
 * Tuning parameter: list size at or below which insertion sort will be
 * used in preference to mergesort or quicksort.
 */
#define INSERTIONSORT_THRESHOLD 7

template<typename E>
class ea
{
public:
	virtual ~ea() {
        delete[] _array;
	}

	ea(int length) : _length(length) {
		_array = new sp<E>[_length]();
	}

	ea(ea<E>& that) {
		_length = that.length();
		_array = new sp<E>[_length];
		for (int i = 0; i < _length; i++) {
			_array[i] = that[i];
		}
	}

	ea(const ea<E>& that) {
		throw EUNSUPPORTEDOPERATIONEXCEPTION;
	}

	int length() {
		return _length;
	}

	sp<E>* address() {
		return _array;
	}

	sp<E>& operator[](int index) THROWS(EIndexOutOfBoundsException) {
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
	ea<E>* clone(int offset=0, int newLength=-1) {
		if (offset < 0 || offset >= _length) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}

		if (newLength < 0) {
			newLength = _length - offset;
		}

		ea<E>* newEA = new ea<E>(newLength);
		int n = ES_MIN(_length - offset, newLength);
		for (int i=0; i<n; i++) {
			(*newEA)[i] = _array[i+offset];
		}

		return newEA;
	}

	void sort(int off=0, int len=-1) {
		if (len < 0) {
			len = _length;
		}
		rangeCheck(off, off + len);

		// Insertion sort on smallest arrays
		if (len < INSERTIONSORT_THRESHOLD) {
			for (int i = off; i < len + off; i++)
				for (int j = i; j > off && _array[j - 1] && ((EComparable<E>*) _array[j - 1].get())->compareTo(_array[j].get()) > 0; j--)
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
		sp<E> v = _array[m];

		// Establish Invariant: v* (<v)* (>v)* v*
		int a = off, b = a, c = off + len - 1, d = c;
		int rbv, rcv;
		while (true) {
			while (b <= c && _array[b] && (rbv = ((EComparable<E>*)_array[b].get())->compareTo(v.get())) <= 0) {
				if (rbv == 0)
					swap(a++, b);
				b++;
			}
			while (c >= b && _array[c] && (rcv = ((EComparable<E>*)_array[c].get())->compareTo(v.get())) >= 0) {
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

	sp<E> atomicGet(int index) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		return atomic_load(&_array[index]);
	}

	void atomicSet(int index, sp<E>& e) THROWS(EIndexOutOfBoundsException) {
		if (index < 0 || index >= _length) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		}
		atomic_store(&_array[index], e);
	}

private:
	sp<E>* _array;
	int _length;

	/**
	 * Check that fromIndex and toIndex are in range, and throw an
	 * appropriate exception if they aren't.
	 */
	void rangeCheck(int fromIndex, int toIndex) {
		EString msg;
		if (fromIndex > toIndex) {
			msg.format("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(msg.c_str(), __FILE__, __LINE__);
		}

		if (fromIndex < 0) {
			msg.format("fromIndex(%d)", fromIndex);
			throw EIndexOutOfBoundsException(msg.c_str(), __FILE__, __LINE__);
		}

		if (toIndex > _length) {
			msg.format("toIndex(%d)", toIndex);
			throw EIndexOutOfBoundsException(msg.c_str(), __FILE__, __LINE__);
		}
	}

	/**
	 * Swaps x[a] with x[b].
	 */
	void swap(int a, int b) {
		sp<E> t = _array[a];
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

} /* namespace efc */
#endif //!ESHAREDARR_HH_
