/*
 * EA.hh
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#ifndef EARRAYS_HH_
#define EARRAYS_HH_

#include "EBase.hh"
#include "EComparable.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

/**
 * This class contains various methods for manipulating arrays (such as
 * sorting and searching).  This class also contains a static factory
 * that allows arrays to be viewed as lists.
 *
 * <p>The methods in this class all throw a <tt>NullPointerException</tt> if
 * the specified array reference is null, except where noted.
 *
 * <p>The documentation for the methods contained in this class includes
 * briefs description of the <i>implementations</i>.  Such descriptions should
 * be regarded as <i>implementation notes</i>, rather than parts of the
 * <i>specification</i>.  Implementors should feel free to substitute other
 * algorithms, so long as the specification itself is adhered to.  (For
 * example, the algorithm used by <tt>sort(Object[])</tt> does not have to be
 * a mergesort, but it does have to be <i>stable</i>.)
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since   1.2
 */

class EArrays {
public:

    // Sorting

	/**
	 * Sorts the specified range of the specified array of longs into
	 * ascending numerical order.  The range to be sorted extends from index
	 * <tt>fromIndex</tt>, inclusive, to index <tt>toIndex</tt>, exclusive.
	 * (If <tt>fromIndex==toIndex</tt>, the range to be sorted is empty.)
	 *
	 * <p>The sorting algorithm is a tuned quicksort, adapted from Jon
	 * L. Bentley and M. Douglas McIlroy's "Engineering a Sort Function",
	 * Software-Practice and Experience, Vol. 23(11) P. 1249-1265 (November
	 * 1993).  This algorithm offers n*log(n) performance on many data sets
	 * that cause other quicksorts to degrade to quadratic performance.
	 *
	 * @param a the array to be sorted
	 * @param fromIndex the index of the first element (inclusive) to be
	 *        sorted
	 * @param toIndex the index of the last element (exclusive) to be sorted
	 * @throws IllegalArgumentException if <tt>fromIndex &gt; toIndex</tt>
	 * @throws ArrayIndexOutOfBoundsException if <tt>fromIndex &lt; 0</tt> or
	 * <tt>toIndex &gt; a.length</tt>
	 */
	template<typename T>
	static void sort(EA<T>* a, int fromIndex, int toIndex) {
		if (fromIndex > toIndex) {
			EString msg = EString::formatOf("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}
		a->sort(fromIndex, toIndex-fromIndex);
	}

	// Cloning
	/**
	 * Copies the specified array, truncating or padding with nulls (if necessary)
	 * so the copy has the specified length.  For all indices that are
	 * valid in both the original array and the copy, the two arrays will
	 * contain identical values.  For any indices that are valid in the
	 * copy but not the original, the copy will contain <tt>null</tt>.
	 * Such indices will exist if and only if the specified length
	 * is greater than that of the original array.
	 * The resulting array is of exactly the same class as the original array.
	 *
	 * @param original the array to be copied
	 * @param newLength the length of the copy to be returned
	 * @return a copy of the original array, truncated or padded with nulls
	 *     to obtain the specified length
	 * @throws NegativeArraySizeException if <tt>newLength</tt> is negative
	 * @throws NullPointerException if <tt>original</tt> is null
	 * @since 1.6
	 */
	template<typename T>
	static EA<T>* copyOf(EA<T>* original, int newLength) {
		return (EA<T>*) original->clone(0, newLength);
	}

	/**
	 * Copies the specified range of the specified array into a new array.
	 * The initial index of the range (<tt>from</tt>) must lie between zero
	 * and <tt>original.length</tt>, inclusive.  The value at
	 * <tt>original[from]</tt> is placed into the initial element of the copy
	 * (unless <tt>from == original.length</tt> or <tt>from == to</tt>).
	 * Values from subsequent elements in the original array are placed into
	 * subsequent elements in the copy.  The final index of the range
	 * (<tt>to</tt>), which must be greater than or equal to <tt>from</tt>,
	 * may be greater than <tt>original.length</tt>, in which case
	 * <tt>null</tt> is placed in all elements of the copy whose index is
	 * greater than or equal to <tt>original.length - from</tt>.  The length
	 * of the returned array will be <tt>to - from</tt>.
	 * <p>
	 * The resulting array is of exactly the same class as the original array.
	 *
	 * @param original the array from which a range is to be copied
	 * @param from the initial index of the range to be copied, inclusive
	 * @param to the final index of the range to be copied, exclusive.
	 *     (This index may lie outside the array.)
	 * @return a new array containing the specified range from the original array,
	 *     truncated or padded with nulls to obtain the required length
	 * @throws ArrayIndexOutOfBoundsException if {@code from < 0}
	 *     or {@code from > original.length}
	 * @throws IllegalArgumentException if <tt>from &gt; to</tt>
	 * @throws NullPointerException if <tt>original</tt> is null
	 * @since 1.6
	 */
	template<typename T>
	static EA<T>* copyOfRange(EA<T>* original, int fromIndex, int toIndex) {
		if (fromIndex > toIndex) {
			EString msg = EString::formatOf("fromIndex(%d) > toIndex(%d)", fromIndex, toIndex);
			throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
		}
		return original->clone(fromIndex, toIndex - fromIndex);
	}

private:
	// Suppresses default constructor, ensuring non-instantiability.
	EArrays() {
	}
};

} /* namespace efc */
#endif //!EARRAYS_HH_
