/*
 * EA.hh
 *
 *  Created on: 2013-8-5
 *      Author: cxxjava@163.com
 */

#ifndef EARRAYS_HH_
#define EARRAYS_HH_

#include "EBase.hh"
#include "EFloat.hh"
#include "EDouble.hh"
#include "EComparable.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalArgumentException.hh"
#include "EIndexOutOfBoundsException.hh"

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
	 * Sorts the specified array into ascending numerical order.
	 *
	 * <p>Implementation note: The sorting algorithm is a Dual-Pivot Quicksort
	 * by Vladimir Yaroslavskiy, Jon Bentley, and Joshua Bloch. This algorithm
	 * offers O(n log(n)) performance on many data sets that cause other
	 * quicksorts to degrade to quadratic performance, and is typically
	 * faster than traditional (one-pivot) Quicksort implementations.
	 *
	 * @param a the array to be sorted
	 */
	template<typename T>
	static void sort(EA<T>* a) {
		if (!a) return;
		EArrays::sort(a, 0, a->length());
	}

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
		if (!a) return;
		rangeCheck(a->length(), fromIndex, toIndex);
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

	template<typename T>
	static EA<T> copyOf(EA<T>& original, int newLength) {
		sp<EA<T> > o = original.clone(0, newLength);
		return (*o);
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
		if (!original) return null;
		rangeCheck(original->length(), fromIndex, toIndex);
		return original->clone(fromIndex, toIndex - fromIndex);
	}

	template<typename T>
	static EA<T> copyOfRange(EA<T>& original, int fromIndex, int toIndex) {
		rangeCheck(original.length(), fromIndex, toIndex);
		sp<EA<T> > o = original.clone(fromIndex, toIndex - fromIndex);
		return (*o);
	}

	/**
	 * Returns a string representation of the contents of the specified array.
	 * The string representation consists of a list of the array's elements,
	 * enclosed in square brackets (<tt>"[]"</tt>).  Adjacent elements are
	 * separated by the characters <tt>", "</tt> (a comma followed by a
	 * space).  Elements are converted to strings as by
	 * <tt>String.valueOf(long)</tt>.  Returns <tt>"null"</tt> if <tt>a</tt>
	 * is <tt>null</tt>.
	 *
	 * @param a the array whose string representation to return
	 * @return a string representation of <tt>a</tt>
	 * @since 1.5
	 */
	template<typename T>
	static EString toString(EA<T>* a) {
		if (a == null)
			return "null";
		int iMax = a->length() - 1;
		if (iMax == -1)
			return "[]";

		EString b;
		b.append('[');
		for (int i = 0; ; i++) {
			b.append((*a)[i]);
			if (i == iMax)
				return b.append(']');
			b.append(", ");
		}
	}

	/**
	 * Returns a string representation of the contents of the specified array.
	 * If the array contains other arrays as elements, they are converted to
	 * strings by the {@link Object#toString} method inherited from
	 * <tt>Object</tt>, which describes their <i>identities</i> rather than
	 * their contents.
	 *
	 * <p>The value returned by this method is equal to the value that would
	 * be returned by <tt>Arrays.asList(a).toString()</tt>, unless <tt>a</tt>
	 * is <tt>null</tt>, in which case <tt>"null"</tt> is returned.
	 *
	 * @param a the array whose string representation to return
	 * @return a string representation of <tt>a</tt>
	 * @see #deepToString(Object[])
	 * @since 1.5
	 */
	static EString toString(EA<EObject*>* a) {
		if (a == null)
			return "null";

		int iMax = a->length() - 1;
		if (iMax == -1)
			return "[]";

		EString b;
		b.append('[');
		for (int i = 0; ; i++) {
			b.append(a ? (*a)[i]->toString() : "null");
			if (i == iMax)
				return b.append(']');
			b.append(", ");
		}
	}

	// Searching

	/**
	 * Searches the specified array of longs for the specified value using the
	 * binary search algorithm.  The array must be sorted (as
	 * by the {@link #sort(long[])} method) prior to making this call.  If it
	 * is not sorted, the results are undefined.  If the array contains
	 * multiple elements with the specified value, there is no guarantee which
	 * one will be found.
	 *
	 * @param a the array to be searched
	 * @param key the value to be searched for
	 * @return index of the search key, if it is contained in the array;
	 *         otherwise, <tt>(-(<i>insertion point</i>) - 1)</tt>.  The
	 *         <i>insertion point</i> is defined as the point at which the
	 *         key would be inserted into the array: the index of the first
	 *         element greater than the key, or <tt>a.length</tt> if all
	 *         elements in the array are less than the specified key.  Note
	 *         that this guarantees that the return value will be &gt;= 0 if
	 *         and only if the key is found.
	 */
	template<typename T>
	static int binarySearch(EA<T>* a, T key) {
		if (!a) return -1;
		return EArrays::binarySearch(a, 0, a->length(), key);
	}

	/**
	 * Searches a range of
	 * the specified array of longs for the specified value using the
	 * binary search algorithm.
	 * The range must be sorted (as
	 * by the {@link #sort(long[], int, int)} method)
	 * prior to making this call.  If it
	 * is not sorted, the results are undefined.  If the range contains
	 * multiple elements with the specified value, there is no guarantee which
	 * one will be found.
	 *
	 * @param a the array to be searched
	 * @param fromIndex the index of the first element (inclusive) to be
	 *          searched
	 * @param toIndex the index of the last element (exclusive) to be searched
	 * @param key the value to be searched for
	 * @return index of the search key, if it is contained in the array
	 *         within the specified range;
	 *         otherwise, <tt>(-(<i>insertion point</i>) - 1)</tt>.  The
	 *         <i>insertion point</i> is defined as the point at which the
	 *         key would be inserted into the array: the index of the first
	 *         element in the range greater than the key,
	 *         or <tt>toIndex</tt> if all
	 *         elements in the range are less than the specified key.  Note
	 *         that this guarantees that the return value will be &gt;= 0 if
	 *         and only if the key is found.
	 * @throws IllegalArgumentException
	 *         if {@code fromIndex > toIndex}
	 * @throws ArrayIndexOutOfBoundsException
	 *         if {@code fromIndex < 0 or toIndex > a.length}
	 * @since 1.6
	 */
	template<typename T>
	static int binarySearch(EA<T>* a, int fromIndex, int toIndex, T key) {
		if (!a) return -1;
		rangeCheck(a->length(), fromIndex, toIndex);

		int low = fromIndex;
		int high = toIndex - 1;

		while (low <= high) {
			int mid = ((unsigned)(low + high)) >> 1;
			llong midVal = (*a)[mid];

			if (midVal < key)
				low = mid + 1;
			else if (midVal > key)
				high = mid - 1;
			else
				return mid; // key found
		}
		return -(low + 1);  // key not found.
	}

	/**
	 * Searches the specified array of doubles for the specified value using
	 * the binary search algorithm.  The array must be sorted
	 * (as by the {@link #sort(double[])} method) prior to making this call.
	 * If it is not sorted, the results are undefined.  If the array contains
	 * multiple elements with the specified value, there is no guarantee which
	 * one will be found.  This method considers all NaN values to be
	 * equivalent and equal.
	 *
	 * @param a the array to be searched
	 * @param key the value to be searched for
	 * @return index of the search key, if it is contained in the array;
	 *         otherwise, <tt>(-(<i>insertion point</i>) - 1)</tt>.  The
	 *         <i>insertion point</i> is defined as the point at which the
	 *         key would be inserted into the array: the index of the first
	 *         element greater than the key, or <tt>a.length</tt> if all
	 *         elements in the array are less than the specified key.  Note
	 *         that this guarantees that the return value will be &gt;= 0 if
	 *         and only if the key is found.
	 */
	static int binarySearch(EA<double>* a, double key) {
		if (!a) return -1;
		return EArrays::binarySearch(a, 0, a->length(), key);
	}

	/**
	 * Searches a range of
	 * the specified array of doubles for the specified value using
	 * the binary search algorithm.
	 * The range must be sorted
	 * (as by the {@link #sort(double[], int, int)} method)
	 * prior to making this call.
	 * If it is not sorted, the results are undefined.  If the range contains
	 * multiple elements with the specified value, there is no guarantee which
	 * one will be found.  This method considers all NaN values to be
	 * equivalent and equal.
	 *
	 * @param a the array to be searched
	 * @param fromIndex the index of the first element (inclusive) to be
	 *          searched
	 * @param toIndex the index of the last element (exclusive) to be searched
	 * @param key the value to be searched for
	 * @return index of the search key, if it is contained in the array
	 *         within the specified range;
	 *         otherwise, <tt>(-(<i>insertion point</i>) - 1)</tt>.  The
	 *         <i>insertion point</i> is defined as the point at which the
	 *         key would be inserted into the array: the index of the first
	 *         element in the range greater than the key,
	 *         or <tt>toIndex</tt> if all
	 *         elements in the range are less than the specified key.  Note
	 *         that this guarantees that the return value will be &gt;= 0 if
	 *         and only if the key is found.
	 * @throws IllegalArgumentException
	 *         if {@code fromIndex > toIndex}
	 * @throws ArrayIndexOutOfBoundsException
	 *         if {@code fromIndex < 0 or toIndex > a.length}
	 * @since 1.6
	 */
	static int binarySearch(EA<double>* a, int fromIndex, int toIndex, double key) {
		if (!a) return -1;
		rangeCheck(a->length(), fromIndex, toIndex);

		int low = fromIndex;
		int high = toIndex - 1;

		while (low <= high) {
			int mid = ((unsigned)(low + high)) >> 1;
			double midVal = (*a)[mid];

			if (midVal < key)
				low = mid + 1;  // Neither val is NaN, thisVal is smaller
			else if (midVal > key)
				high = mid - 1; // Neither val is NaN, thisVal is larger
			else {
				llong midBits = EDouble::doubleToLLongBits(midVal);
				llong keyBits = EDouble::doubleToLLongBits(key);
				if (midBits == keyBits)     // Values are equal
					return mid;             // Key found
				else if (midBits < keyBits) // (-0.0, 0.0) or (!NaN, NaN)
					low = mid + 1;
				else                        // (0.0, -0.0) or (NaN, !NaN)
					high = mid - 1;
			}
		}
		return -(low + 1);  // key not found.
	}

	/**
	 * Searches the specified array of floats for the specified value using
	 * the binary search algorithm. The array must be sorted
	 * (as by the {@link #sort(float[])} method) prior to making this call. If
	 * it is not sorted, the results are undefined. If the array contains
	 * multiple elements with the specified value, there is no guarantee which
	 * one will be found. This method considers all NaN values to be
	 * equivalent and equal.
	 *
	 * @param a the array to be searched
	 * @param key the value to be searched for
	 * @return index of the search key, if it is contained in the array;
	 *         otherwise, <tt>(-(<i>insertion point</i>) - 1)</tt>. The
	 *         <i>insertion point</i> is defined as the point at which the
	 *         key would be inserted into the array: the index of the first
	 *         element greater than the key, or <tt>a.length</tt> if all
	 *         elements in the array are less than the specified key. Note
	 *         that this guarantees that the return value will be &gt;= 0 if
	 *         and only if the key is found.
	 */
	static int binarySearch(EA<float>* a, float key) {
		if (!a) return -1;
		return EArrays::binarySearch(a, 0, a->length(), key);
	}

	/**
	 * Searches a range of
	 * the specified array of floats for the specified value using
	 * the binary search algorithm.
	 * The range must be sorted
	 * (as by the {@link #sort(float[], int, int)} method)
	 * prior to making this call. If
	 * it is not sorted, the results are undefined. If the range contains
	 * multiple elements with the specified value, there is no guarantee which
	 * one will be found. This method considers all NaN values to be
	 * equivalent and equal.
	 *
	 * @param a the array to be searched
	 * @param fromIndex the index of the first element (inclusive) to be
	 *          searched
	 * @param toIndex the index of the last element (exclusive) to be searched
	 * @param key the value to be searched for
	 * @return index of the search key, if it is contained in the array
	 *         within the specified range;
	 *         otherwise, <tt>(-(<i>insertion point</i>) - 1)</tt>. The
	 *         <i>insertion point</i> is defined as the point at which the
	 *         key would be inserted into the array: the index of the first
	 *         element in the range greater than the key,
	 *         or <tt>toIndex</tt> if all
	 *         elements in the range are less than the specified key. Note
	 *         that this guarantees that the return value will be &gt;= 0 if
	 *         and only if the key is found.
	 * @throws IllegalArgumentException
	 *         if {@code fromIndex > toIndex}
	 * @throws ArrayIndexOutOfBoundsException
	 *         if {@code fromIndex < 0 or toIndex > a.length}
	 * @since 1.6
	 */
	static int binarySearch(EA<float>* a, int fromIndex, int toIndex, float key) {
		if (!a) return -1;
		rangeCheck(a->length(), fromIndex, toIndex);

		int low = fromIndex;
		int high = toIndex - 1;

		while (low <= high) {
			int mid = ((unsigned)(low + high)) >> 1;
			float midVal = (*a)[mid];

			if (midVal < key)
				low = mid + 1;  // Neither val is NaN, thisVal is smaller
			else if (midVal > key)
				high = mid - 1; // Neither val is NaN, thisVal is larger
			else {
				int midBits = EFloat::floatToIntBits(midVal);
				int keyBits = EFloat::floatToIntBits(key);
				if (midBits == keyBits)     // Values are equal
					return mid;             // Key found
				else if (midBits < keyBits) // (-0.0, 0.0) or (!NaN, NaN)
					low = mid + 1;
				else                        // (0.0, -0.0) or (NaN, !NaN)
					high = mid - 1;
			}
		}
		return -(low + 1);  // key not found.
	}

	// Equality Testing

	/**
	 * Returns <tt>true</tt> if the two specified arrays of longs are
	 * <i>equal</i> to one another.  Two arrays are considered equal if both
	 * arrays contain the same number of elements, and all corresponding pairs
	 * of elements in the two arrays are equal.  In other words, two arrays
	 * are equal if they contain the same elements in the same order.  Also,
	 * two array references are considered equal if both are <tt>null</tt>.<p>
	 *
	 * @param a one array to be tested for equality
	 * @param a2 the other array to be tested for equality
	 * @return <tt>true</tt> if the two arrays are equal
	 */
	template<typename T>
	static boolean equals(EA<T>* a, EA<T>* a2) {
		if (a==a2)
			return true;
		if (a==null || a2==null)
			return false;

		int length = a->length();
		if (a2->length() != length)
			return false;

		for (int i=0; i<length; i++)
			if ((*a)[i] != (*a2)[i])
				return false;

		return true;
	}

	/**
	 * Returns <tt>true</tt> if the two specified arrays of doubles are
	 * <i>equal</i> to one another.  Two arrays are considered equal if both
	 * arrays contain the same number of elements, and all corresponding pairs
	 * of elements in the two arrays are equal.  In other words, two arrays
	 * are equal if they contain the same elements in the same order.  Also,
	 * two array references are considered equal if both are <tt>null</tt>.<p>
	 *
	 * Two doubles <tt>d1</tt> and <tt>d2</tt> are considered equal if:
	 * <pre>    <tt>new Double(d1).equals(new Double(d2))</tt></pre>
	 * (Unlike the <tt>==</tt> operator, this method considers
	 * <tt>NaN</tt> equals to itself, and 0.0d unequal to -0.0d.)
	 *
	 * @param a one array to be tested for equality
	 * @param a2 the other array to be tested for equality
	 * @return <tt>true</tt> if the two arrays are equal
	 * @see Double#equals(Object)
	 */
	static boolean equals(EA<double>* a, EA<double>* a2) {
		if (a==a2)
			return true;
		if (a==null || a2==null)
			return false;

		int length = a->length();
		if (a2->length() != length)
			return false;

		for (int i=0; i<length; i++)
			if (EDouble::doubleToLLongBits((*a)[i])!=EDouble::doubleToLLongBits((*a2)[i]))
				return false;

		return true;
	}

	/**
	 * Returns <tt>true</tt> if the two specified arrays of floats are
	 * <i>equal</i> to one another.  Two arrays are considered equal if both
	 * arrays contain the same number of elements, and all corresponding pairs
	 * of elements in the two arrays are equal.  In other words, two arrays
	 * are equal if they contain the same elements in the same order.  Also,
	 * two array references are considered equal if both are <tt>null</tt>.<p>
	 *
	 * Two floats <tt>f1</tt> and <tt>f2</tt> are considered equal if:
	 * <pre>    <tt>new Float(f1).equals(new Float(f2))</tt></pre>
	 * (Unlike the <tt>==</tt> operator, this method considers
	 * <tt>NaN</tt> equals to itself, and 0.0f unequal to -0.0f.)
	 *
	 * @param a one array to be tested for equality
	 * @param a2 the other array to be tested for equality
	 * @return <tt>true</tt> if the two arrays are equal
	 * @see Float#equals(Object)
	 */
	static boolean equals(EA<float>* a, EA<float>* a2) {
		if (a==a2)
			return true;
		if (a==null || a2==null)
			return false;

		int length = a->length();
		if (a2->length() != length)
			return false;

		for (int i=0; i<length; i++)
			if (EFloat::floatToIntBits((*a)[i])!=EFloat::floatToIntBits((*a2)[i]))
				return false;

		return true;
	}

	/**
	 * Returns <tt>true</tt> if the two specified arrays of Objects are
	 * <i>equal</i> to one another.  The two arrays are considered equal if
	 * both arrays contain the same number of elements, and all corresponding
	 * pairs of elements in the two arrays are equal.  Two objects <tt>e1</tt>
	 * and <tt>e2</tt> are considered <i>equal</i> if <tt>(e1==null ? e2==null
	 * : e1.equals(e2))</tt>.  In other words, the two arrays are equal if
	 * they contain the same elements in the same order.  Also, two array
	 * references are considered equal if both are <tt>null</tt>.<p>
	 *
	 * @param a one array to be tested for equality
	 * @param a2 the other array to be tested for equality
	 * @return <tt>true</tt> if the two arrays are equal
	 */
	static boolean equals(EA<EObject*>* a, EA<EObject*>* a2) {
		if (a==a2)
			return true;
		if (a==null || a2==null)
			return false;

		int length = a->length();
		if (a2->length() != length)
			return false;

		for (int i=0; i<length; i++) {
			EObject* o1 = (*a)[i];
			EObject* o2 = (*a2)[i];
			if (!(o1==null ? o2==null : o1->equals(o2)))
				return false;
		}

		return true;
	}

	// Filling

	/**
	 * Assigns the specified long value to each element of the specified array
	 * of longs.
	 *
	 * @param a the array to be filled
	 * @param val the value to be stored in all elements of the array
	 */
	template<typename T>
	static void fill(EA<T>* a, T val) {
		if (!a) return;
		for (int i = 0, len = a->length(); i < len; i++)
			(*a)[i] = val;
	}

	/**
	 * Assigns the specified long value to each element of the specified
	 * range of the specified array of longs.  The range to be filled
	 * extends from index <tt>fromIndex</tt>, inclusive, to index
	 * <tt>toIndex</tt>, exclusive.  (If <tt>fromIndex==toIndex</tt>, the
	 * range to be filled is empty.)
	 *
	 * @param a the array to be filled
	 * @param fromIndex the index of the first element (inclusive) to be
	 *        filled with the specified value
	 * @param toIndex the index of the last element (exclusive) to be
	 *        filled with the specified value
	 * @param val the value to be stored in all elements of the array
	 * @throws IllegalArgumentException if <tt>fromIndex &gt; toIndex</tt>
	 * @throws ArrayIndexOutOfBoundsException if <tt>fromIndex &lt; 0</tt> or
	 *         <tt>toIndex &gt; a.length</tt>
	 */
	template<typename T>
	static void fill(EA<T>* a, int fromIndex, int toIndex, T val) {
		if (!a) return;
		rangeCheck(a->length(), fromIndex, toIndex);
		for (int i = fromIndex; i < toIndex; i++)
			(*a)[i] = val;
	}

	// HashCode

	/**
	 * Returns a hash code based on the contents of the specified array.
	 * For any two non-null <tt>int</tt> arrays <tt>a</tt> and <tt>b</tt>
	 * such that <tt>Arrays.equals(a, b)</tt>, it is also the case that
	 * <tt>Arrays.hashCode(a) == Arrays.hashCode(b)</tt>.
	 *
	 * <p>The value returned by this method is the same value that would be
	 * obtained by invoking the {@link List#hashCode() <tt>hashCode</tt>}
	 * method on a {@link List} containing a sequence of {@link Integer}
	 * instances representing the elements of <tt>a</tt> in the same order.
	 * If <tt>a</tt> is <tt>null</tt>, this method returns 0.
	 *
	 * @param a the array whose hash value to compute
	 * @return a content-based hash code for <tt>a</tt>
	 * @since 1.5
	 */
	template<typename T>
	static int hashCode(EA<T>* a) {
		if (a == null)
			return 0;

		int result = 1;
		for (int i=0; i<a->length(); i++) {
			T element = (*a)[i];
			result = 31 * result + element;
		}

		return result;
	}

	/**
	 * Returns a hash code based on the contents of the specified array.
	 * For any two <tt>long</tt> arrays <tt>a</tt> and <tt>b</tt>
	 * such that <tt>Arrays.equals(a, b)</tt>, it is also the case that
	 * <tt>Arrays.hashCode(a) == Arrays.hashCode(b)</tt>.
	 *
	 * <p>The value returned by this method is the same value that would be
	 * obtained by invoking the {@link List#hashCode() <tt>hashCode</tt>}
	 * method on a {@link List} containing a sequence of {@link Long}
	 * instances representing the elements of <tt>a</tt> in the same order.
	 * If <tt>a</tt> is <tt>null</tt>, this method returns 0.
	 *
	 * @param a the array whose hash value to compute
	 * @return a content-based hash code for <tt>a</tt>
	 * @since 1.5
	 */
	static int hashCode(EA<llong>* a) {
		if (a == null)
			return 0;

		int result = 1;
		for (int i=0; i<a->length(); i++) {
			llong element = (*a)[i];
			int elementHash = (int)(element ^ (((ullong)element) >> 32));
			result = 31 * result + elementHash;
		}

		return result;
	}

	/**
	 * Returns a hash code based on the contents of the specified array.
	 * For any two <tt>boolean</tt> arrays <tt>a</tt> and <tt>b</tt>
	 * such that <tt>Arrays.equals(a, b)</tt>, it is also the case that
	 * <tt>Arrays.hashCode(a) == Arrays.hashCode(b)</tt>.
	 *
	 * <p>The value returned by this method is the same value that would be
	 * obtained by invoking the {@link List#hashCode() <tt>hashCode</tt>}
	 * method on a {@link List} containing a sequence of {@link Boolean}
	 * instances representing the elements of <tt>a</tt> in the same order.
	 * If <tt>a</tt> is <tt>null</tt>, this method returns 0.
	 *
	 * @param a the array whose hash value to compute
	 * @return a content-based hash code for <tt>a</tt>
	 * @since 1.5
	 */
	static int hashCode(EA<boolean>* a) {
		if (a == null)
			return 0;

		int result = 1;
		for (int i=0; i<a->length(); i++) {
			boolean element = (*a)[i];
			result = 31 * result + (element ? 1231 : 1237);
		}

		return result;
	}

	/**
	 * Returns a hash code based on the contents of the specified array.
	 * For any two <tt>float</tt> arrays <tt>a</tt> and <tt>b</tt>
	 * such that <tt>Arrays.equals(a, b)</tt>, it is also the case that
	 * <tt>Arrays.hashCode(a) == Arrays.hashCode(b)</tt>.
	 *
	 * <p>The value returned by this method is the same value that would be
	 * obtained by invoking the {@link List#hashCode() <tt>hashCode</tt>}
	 * method on a {@link List} containing a sequence of {@link Float}
	 * instances representing the elements of <tt>a</tt> in the same order.
	 * If <tt>a</tt> is <tt>null</tt>, this method returns 0.
	 *
	 * @param a the array whose hash value to compute
	 * @return a content-based hash code for <tt>a</tt>
	 * @since 1.5
	 */
	static int hashCode(EA<float>* a) {
		if (a == null)
			return 0;

		int result = 1;
		for (int i=0; i<a->length(); i++) {
			float element = (*a)[i];
			result = 31 * result + EFloat::floatToIntBits(element);
		}

		return result;
	}

	/**
	 * Returns a hash code based on the contents of the specified array.
	 * For any two <tt>double</tt> arrays <tt>a</tt> and <tt>b</tt>
	 * such that <tt>Arrays.equals(a, b)</tt>, it is also the case that
	 * <tt>Arrays.hashCode(a) == Arrays.hashCode(b)</tt>.
	 *
	 * <p>The value returned by this method is the same value that would be
	 * obtained by invoking the {@link List#hashCode() <tt>hashCode</tt>}
	 * method on a {@link List} containing a sequence of {@link Double}
	 * instances representing the elements of <tt>a</tt> in the same order.
	 * If <tt>a</tt> is <tt>null</tt>, this method returns 0.
	 *
	 * @param a the array whose hash value to compute
	 * @return a content-based hash code for <tt>a</tt>
	 * @since 1.5
	 */
	static int hashCode(EA<double>* a) {
		if (a == null)
			return 0;

		int result = 1;
		for (int i=0; i<a->length(); i++) {
			double element = (*a)[i];
			llong bits = EDouble::doubleToLLongBits(element);
			result = 31 * result + (int)(bits ^ (((ullong)bits) >> 32));
		}
		return result;
	}

	/**
	 * Returns a hash code based on the contents of the specified array.  If
	 * the array contains other arrays as elements, the hash code is based on
	 * their identities rather than their contents.  It is therefore
	 * acceptable to invoke this method on an array that contains itself as an
	 * element,  either directly or indirectly through one or more levels of
	 * arrays.
	 *
	 * <p>For any two arrays <tt>a</tt> and <tt>b</tt> such that
	 * <tt>Arrays.equals(a, b)</tt>, it is also the case that
	 * <tt>Arrays.hashCode(a) == Arrays.hashCode(b)</tt>.
	 *
	 * <p>The value returned by this method is equal to the value that would
	 * be returned by <tt>Arrays.asList(a).hashCode()</tt>, unless <tt>a</tt>
	 * is <tt>null</tt>, in which case <tt>0</tt> is returned.
	 *
	 * @param a the array whose content-based hash code to compute
	 * @return a content-based hash code for <tt>a</tt>
	 * @see #deepHashCode(Object[])
	 * @since 1.5
	 */
	static int hashCode(EA<EObject*>* a) {
		if (a == null)
			return 0;

		int result = 1;

		for (int i=0; i<a->length(); i++) {
			EObject* element = (*a)[i];
			result = 31 * result + (element == null ? 0 : element->hashCode());
		}

		return result;
	}

private:
	// Suppresses default constructor, ensuring non-instantiability.
	EArrays() {
	}

	/**
	 * Checks that {@code fromIndex} and {@code toIndex} are in
	 * the range and throws an exception if they aren't.
	 */
	static void rangeCheck(int arrayLength, int fromIndex, int toIndex) {
		if (fromIndex > toIndex) {
			throw EIllegalArgumentException(__FILE__, __LINE__,
					(EString("fromIndex(") + fromIndex + ") > toIndex(" + toIndex + ")").c_str());
		}
		if (fromIndex < 0) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, EString(fromIndex).c_str());
		}
		if (toIndex > arrayLength) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__, EString(toIndex).c_str());
		}
	}
};

} /* namespace efc */
#endif //!EARRAYS_HH_
