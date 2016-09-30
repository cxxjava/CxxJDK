/*
 * ECopyOnWriteArrayList.hh
 *
 *  Created on: 2014-8-11
 *      Author: cxxjava@163.com
 */

#ifndef ECOPYONWRITEARRAYLIST_HH_
#define ECOPYONWRITEARRAYLIST_HH_

#include "EConcurrentList.hh"
#include "EConcurrentListIterator.hh"
#include "EReentrantLock.hh"
#include "ESharedArr.hh"
#include "ESharedArrLst.hh"
#include "ENoSuchElementException.hh"

namespace efc {

/**
 * A thread-safe variant of {@link java.util.ArrayList} in which all mutative
 * operations (<tt>add</tt>, <tt>set</tt>, and so on) are implemented by
 * making a fresh copy of the underlying array.
 *
 * <p> This is ordinarily too costly, but may be <em>more</em> efficient
 * than alternatives when traversal operations vastly outnumber
 * mutations, and is useful when you cannot or don't want to
 * synchronize traversals, yet need to preclude interference among
 * concurrent threads.  The "snapshot" style iterator method uses a
 * reference to the state of the array at the point that the iterator
 * was created. This array never changes during the lifetime of the
 * iterator, so interference is impossible and the iterator is
 * guaranteed not to throw <tt>ConcurrentModificationException</tt>.
 * The iterator will not reflect additions, removals, or changes to
 * the list since the iterator was created.  Element-changing
 * operations on iterators themselves (<tt>remove</tt>, <tt>set</tt>, and
 * <tt>add</tt>) are not supported. These methods throw
 * <tt>UnsupportedOperationException</tt>.
 *
 * <p>All elements are permitted, including <tt>null</tt>.
 *
 * <p>Memory consistency effects: As with other concurrent
 * collections, actions in a thread prior to placing an object into a
 * {@code CopyOnWriteArrayList}
 * <a href="package-summary.html#MemoryVisibility"><i>happen-before</i></a>
 * actions subsequent to the access or removal of that element from
 * the {@code CopyOnWriteArrayList} in another thread.
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since 1.5
 * @param <E> the type of elements held in this collection
 */

template<typename E>
class ECopyOnWriteArrayList: public EConcurrentList<E>
{
public:
	virtual ~ECopyOnWriteArrayList() {
		DELRC(array_);
	}

	/*
	 * Creates an empty list.
	 */
	ECopyOnWriteArrayList() {
		array_ = NEWRC(ea<E>)(0);
	}

	/**
	 * Creates a list containing the elements of the specified
	 * collection, in the order they are returned by the collection's
	 * iterator.
	 *
	 * @param c the collection of initially held elements
	 * @throws NullPointerException if the specified collection is null
	 */
	ECopyOnWriteArrayList(EConcurrentCollection<E>* c) {
		ea<E>* elements = NEWRC(ea<E>)(c->size());

		int i = 0;
		sp<EConcurrentIterator<E> > iter = c->iterator();
		while (iter->hasNext()) {
			(*elements)[i++] = iter->next();
		}

		array_ = elements;
	}

	/**
	 * Creates a list holding a copy of the given array.
	 *
	 * @param toCopyIn the array (a copy of this array is used as the
	 *        internal array)
	 * @throws NullPointerException if the specified array is null
	 */
	ECopyOnWriteArrayList(ea<E>* toCopyIn) {
		array_ = arrayClone(toCopyIn);
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	int size() {
		int n;
		ea<E>* elements = GETRC(array_);
		n = elements->length();
		DELRC(elements);
		return n;
	}

	/**
	 * Returns <tt>true</tt> if this list contains no elements.
	 *
	 * @return <tt>true</tt> if this list contains no elements
	 */
	boolean isEmpty() {
		return size() == 0;
	}

	/**
	 * Returns <tt>true</tt> if this list contains the specified element.
	 * More formally, returns <tt>true</tt> if and only if this list contains
	 * at least one element <tt>e</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;e==null&nbsp;:&nbsp;o.equals(e))</tt>.
	 *
	 * @param o element whose presence in this list is to be tested
	 * @return <tt>true</tt> if this list contains the specified element
	 */
	boolean contains(E* o) {
		ea<E>* elements = GETRC(array_);
		boolean b = (indexOf(o, elements, 0, elements->length()) >= 0);
		DELRC(elements);
		return b;
	}

	/**
	 * {@inheritDoc}
	 */
	int indexOf(E* o) {
		ea<E>* elements = GETRC(array_);
		int i = indexOf(o, elements, 0, elements->length());
		DELRC(elements);
		return i;
	}

	/**
	 * Returns the index of the first occurrence of the specified element in
	 * this list, searching forwards from <tt>index</tt>, or returns -1 if
	 * the element is not found.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(i&nbsp;&gt;=&nbsp;index&nbsp;&amp;&amp;&nbsp;(e==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;e.equals(get(i))))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param e element to search for
	 * @param index index to start searching from
	 * @return the index of the first occurrence of the element in
	 *         this list at position <tt>index</tt> or later in the list;
	 *         <tt>-1</tt> if the element is not found.
	 * @throws IndexOutOfBoundsException if the specified index is negative
	 */
	int indexOf(E* e, int index) {
		ea<E>* elements = GETRC(array_);
		int i = indexOf(e, elements, index, elements->length());
		DELRC(elements);
		return i;
	}

	/**
	 * {@inheritDoc}
	 */
	int lastIndexOf(E* o) {
		ea<E>* elements = GETRC(array_);
		int i = lastIndexOf(o, elements, elements->length() - 1);
		DELRC(elements);
		return i;
	}

	/**
	 * Returns the index of the last occurrence of the specified element in
	 * this list, searching backwards from <tt>index</tt>, or returns -1 if
	 * the element is not found.
	 * More formally, returns the highest index <tt>i</tt> such that
	 * <tt>(i&nbsp;&lt;=&nbsp;index&nbsp;&amp;&amp;&nbsp;(e==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;e.equals(get(i))))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param e element to search for
	 * @param index index to start searching backwards from
	 * @return the index of the last occurrence of the element at position
	 *         less than or equal to <tt>index</tt> in this list;
	 *         -1 if the element is not found.
	 * @throws IndexOutOfBoundsException if the specified index is greater
	 *         than or equal to the current size of this list
	 */
	int lastIndexOf(E* e, int index) {
		ea<E>* elements = GETRC(array_);
		int i = lastIndexOf(e, elements, index);
		DELRC(elements);
		return i;
	}

	/**
	 * Returns a shallow copy of this list.  (The elements themselves
	 * are not copied.)
	 *
	 * @return a clone of this list
	 */
	ECopyOnWriteArrayList<E>* clone() {
		return new ECopyOnWriteArrayList<E>((EConcurrentCollection<E>*)(this));
	}

	/**
	 * Returns an array containing all of the elements in this list
	 * in proper sequence (from first to last element).
	 *
	 * <p>The returned array will be "safe" in that no references to it are
	 * maintained by this list.  (In other words, this method must allocate
	 * a new array).  The caller is thus free to modify the returned array.
	 *
	 * <p>This method acts as bridge between array-based and collection-based
	 * APIs.
	 *
	 * @return an array containing all the elements in this list
	 */
	ea<E> toArray() {
		ea<E>* elements = GETRC(array_);
		eal<E> r(elements->length());
		for (int i=0; i<elements->length(); i++) {
			r.add((*elements)[i]);
		}
		DELRC(elements);
		return r.toArray();
	}

	/**
	 * {@inheritDoc}
	 *
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	sp<E> getAt(int index) {
		sp<E> e;
		ea<E>* elements = GETRC(array_);
		e = getAt(elements, index);
		DELRC(elements);
		return e;
	}

	/**
	     * Replaces the element at the specified position in this list with the
	     * specified element.
	     *
	     * @throws IndexOutOfBoundsException {@inheritDoc}
	     */
	sp<E> setAt(int index, E* element) {
		sp<E> e(element);
		return setAt(index, e);
	}
	sp<E> setAt(int index, sp<E> element) {
		SYNCBLOCK(&lock_) {
			ea<E>* elements = GETRC(array_);

			sp<E> oldValue = getAt(elements, index);

			if (oldValue != element) {
				ea<E>* newElements = arrayClone(elements);
				(*newElements)[index] = element;
				setArray(newElements);
			} else {
				// Not quite a no-op; ensures volatile write semantics
				setArray(elements);
			}

			DELRC(elements);

			return oldValue;
        }}
	}

	/**
     * Appends the specified element to the end of this list.
     *
     * @param e element to be appended to this list
     * @return <tt>true</tt> (as specified by {@link Collection#add})
     */
	boolean add(E* e) {
		sp<E> x(e);
		return add(x);
	}
    boolean add(sp<E> e) {
    	SYNCBLOCK(&lock_) {
    		ea<E>* elements = GETRC(array_);

			int len = elements->length();
			ea<E>* newElements = arrayClone(elements, 0, len + 1);
			(*newElements)[len] = e;
			setArray(newElements);

			DELRC(elements);

			return true;
        }}
    }

	/**
     * Inserts the specified element at the specified position in this
     * list. Shifts the element currently at that position (if any) and
     * any subsequent elements to the right (adds one to their indices).
     *
     * @throws IndexOutOfBoundsException {@inheritDoc}
     */
	void addAt(int index, E* element) {
		sp<E> e(element);
		addAt(index, e);
	}

	void addAt(int index, sp<E> element) {
		SYNCBLOCK(&lock_) {
			ea<E>* elements = GETRC(array_);

			int len = elements->length();
			if (index > len || index < 0)
				throw EIndexOutOfBoundsException(__FILE__, __LINE__);
			ea<E>* newElements;
			int numMoved = len - index;
			if (numMoved == 0)
				newElements = arrayClone(elements, 0, len + 1);
			else {
				newElements = NEWRC(ea<E>)(len + 1);
				/* @see:
				System.arraycopy(elements, 0, newElements, 0, index);
				System.arraycopy(elements, index, newElements, index + 1, numMoved);
				*/
				for (int i=0; i<index; i++) {
					(*newElements)[i] = (*elements)[i];
				}
				for (int j=0; j<numMoved; j++) {
					(*newElements)[j+index+1] = (*elements)[j+index];
				}
			}
			(*newElements)[index] = element;
			setArray(newElements);

			DELRC(elements);
        }}
    }

    /**
     * Removes the element at the specified position in this list.
     * Shifts any subsequent elements to the left (subtracts one from their
     * indices).  Returns the element that was removed from the list.
     *
     * @throws IndexOutOfBoundsException {@inheritDoc}
     */
    sp<E> removeAt(int index) {
    	SYNCBLOCK(&lock_) {
    		sp<E> oldValue;
    		ea<E>* elements = GETRC(array_);
    		try {
				oldValue = getAt(elements, index);
				int len = elements->length();
				int numMoved = len - index - 1;
				if (numMoved == 0)
					setArray(arrayClone(elements, 0, len - 1));
				else {
					ea<E>* newElements = NEWRC(ea<E>)(len - 1);
					/* @see:
	                System.arraycopy(elements, 0, newElements, 0, index);
	                System.arraycopy(elements, index + 1, newElements, index, numMoved);
	                */
					for (int i=0; i<index; i++) {
						(*newElements)[i] = (*elements)[i];
					}
					for (int j=0; j<numMoved; j++) {
						(*newElements)[j+index] = (*elements)[j+index+1];
					}
					setArray(newElements);
				}
    		} catch (...) {
				DELRC(elements);
				throw;
			} finally {
				DELRC(elements);
			}

            return oldValue;
        }}
    }

    /**
     * Removes the first occurrence of the specified element from this list,
     * if it is present.  If this list does not contain the element, it is
     * unchanged.  More formally, removes the element with the lowest index
     * <tt>i</tt> such that
     * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>
     * (if such an element exists).  Returns <tt>true</tt> if this list
     * contained the specified element (or equivalently, if this list
     * changed as a result of the call).
     *
     * @param o element to be removed from this list, if present
     * @return <tt>true</tt> if this list contained the specified element
     */
    boolean remove(E* o) {
    	SYNCBLOCK(&lock_) {
    		boolean result = false;

    	    ea<E>* elements = GETRC(array_);

            int len = elements->length();
            if (len != 0) {
                // Copy while searching for element to remove
                // This wins in the normal case of element being present
                int newlen = len - 1;
                ea<E>* newElements = NEWRC(ea<E>)(newlen);

                for (int i = 0; i < newlen; ++i) {
                    if (eq(o, (*elements)[i].get())) {
                        // found one;  copy remaining and exit
                        for (int k = i + 1; k < len; ++k)
                            (*newElements)[k-1] = (*elements)[k];
                        setArray(newElements);
                        result = true;
                    } else
                    	(*newElements)[i] = (*elements)[i];
                }

                // special handling for last cell
                if (eq(o, (*elements)[newlen].get())) {
                    setArray(newElements);
                    result = true;
                }
                else {
                	DELRC(newElements);
                }
            }

            DELRC(elements);

            return result;
        }}
    }

    /**
     * Append the element if not present.
     *
     * @param e element to be added to this list, if absent
     * @return <tt>true</tt> if the element was added
     */
    boolean addIfAbsent(E* e) {
    	sp<E> x(e);
    	return addIfAbsent(x);
    }
    boolean addIfAbsent(sp<E> e) {
    	SYNCBLOCK(&lock_) {
            // Copy while checking if already present.
            // This wins in the most common case where it is not present

    		ea<E>* elements = GETRC(array_);

            int len = elements->length();
            ea<E>* newElements = NEWRC(ea<E>)(len + 1);
            for (int i = 0; i < len; ++i) {
                if (eq(e.get(), (*elements)[i].get())) {
                	DELRC(elements);
                	delete newElements;
                    return false; // exit, throwing away copy
                }
                else
                    (*newElements)[i] = (*elements)[i];
            }
            (*newElements)[len] = e;
            setArray(newElements);

            DELRC(elements);

            return true;
        }}
    }

    /**
     * Removes all of the elements from this list.
     * The list will be empty after this call returns.
     */
    void clear() {
    	SYNCBLOCK(&lock_) {
    		ea<E>* elements = NEWRC(ea<E>)(0);
    		setArray(elements);
        }}
    }

    /**
	 * Returns an iterator over the elements in this list in proper sequence.
	 *
	 * <p>The returned iterator provides a snapshot of the state of the list
	 * when the iterator was constructed. No synchronization is needed while
	 * traversing the iterator. The iterator does <em>NOT</em> support the
	 * <tt>remove</tt> method.
	 *
	 * @return an iterator over the elements in this list in proper sequence
	 */
    sp<EConcurrentIterator<E> > iterator() {
    	ea<E>* elements = GETRC(array_);
    	EConcurrentIterator<E>* iter = new COWIterator(elements, 0);
    	DELRC(elements);
    	return iter;
    }

    /**
     * {@inheritDoc}
     *
     * <p>The returned iterator provides a snapshot of the state of the list
     * when the iterator was constructed. No synchronization is needed while
     * traversing the iterator. The iterator does <em>NOT</em> support the
     * {@code remove}, {@code set} or {@code add} methods.
     *
     * @throws IndexOutOfBoundsException {@inheritDoc}
     */
    sp<EConcurrentListIterator<E> > listIterator(int index = 0) {
    	ea<E>* elements = GETRC(array_);
    	EConcurrentListIterator<E>* iter = new COWIterator(elements, index);
		DELRC(elements);
		return iter;
    }

protected:
	/**
	 * Sets the array.
	 */
	void setArray(ea<E>* a) {
		ea<E>* oa = GETRC(array_);
		array_ = a;
		DELRC(oa); //!
		DELRC(oa); //!
	}

private:
	/** The lock protecting all mutators */
	EReentrantLock lock_;

	/** The array, accessed only via getArray/setArray. */
	ea<E>* volatile array_;

	/**
	 * Test for equality, coping with nulls.
	 */
	static boolean eq(EObject* o1, EObject* o2) {
		return (o1 == null ? o2 == null : o1->equals(o2));
	}

	/**
	 * static version of indexOf, to allow repeated calls without
	 * needing to re-acquire array each time.
	 * @param o element to search for
	 * @param elements the array
	 * @param index first index to search
	 * @param fence one past last index to search
	 * @return index of element, or -1 if absent
	 */
	static int indexOf(E* o, ea<E>* elements,
							   int index, int fence) {
		if (o == null) {
			for (int i = index; i < fence; i++)
				if ((*elements)[i] == null)
					return i;
		} else {
			for (int i = index; i < fence; i++)
				if (o->equals((*elements)[i].get()))
					return i;
		}
		return -1;
	}

	/**
	 * static version of lastIndexOf.
	 * @param o element to search for
	 * @param elements the array
	 * @param index first index to search
	 * @return index of element, or -1 if absent
	 */
	static int lastIndexOf(E* o, ea<E>* elements, int index) {
		if (o == null) {
			for (int i = index; i >= 0; i--)
				if ((*elements)[i] == null)
					return i;
		} else {
			for (int i = index; i >= 0; i--)
				if (o->equals((*elements)[i].get()))
					return i;
		}
		return -1;
	}

	sp<E> getAt(ea<E>* a, int index) {
		return (*a)[index];
	}

	ea<E>* arrayClone(ea<E>* array, int offset=0, int newLength=-1) {
		int oldLength = array->length();
		if (offset < 0 || offset > oldLength) {
			throw EIndexOutOfBoundsException(__FILE__, __LINE__);
		}

		if (newLength < 0) {
			newLength = oldLength - offset;
		}

		ea<E>* newEA = NEWRC(ea<E>)(newLength);
		int n = ES_MIN(oldLength - offset, newLength);
		for (int i=0; i<n; i++) {
			(*newEA)[i] = (*array)[i+offset];
		}

		return newEA;
	}

	class COWIterator: public EConcurrentListIterator<E> {
	public:
		virtual ~COWIterator() {
			DELRC(snapshot); //!
		}

		COWIterator(ea<E>* elements, int initialCursor) {
			cursor = initialCursor;
			snapshot = GETRC(elements);
		}

		boolean hasNext() {
			return cursor < snapshot->length();
		}

		boolean hasPrevious() {
			return cursor > 0;
		}

		sp<E> next() {
			if (! hasNext())
				throw ENoSuchElementException(__FILE__, __LINE__);
			return (*snapshot)[cursor++];
		}

		sp<E> previous() {
			if (! hasPrevious())
				throw ENoSuchElementException(__FILE__, __LINE__);
			return (*snapshot)[--cursor];
		}

		int nextIndex() {
			return cursor;
		}

		int previousIndex() {
			return cursor-1;
		}

		/**
		 * Not supported. Always throws UnsupportedOperationException.
		 * @throws UnsupportedOperationException always; <tt>remove</tt>
		 *         is not supported by this iterator.
		 */
		void remove() {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

		/**
		 * Not supported. Always throws UnsupportedOperationException.
		 * @throws UnsupportedOperationException always; <tt>set</tt>
		 *         is not supported by this iterator.
		 */
		void set(sp<E> e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

		/**
		 * Not supported. Always throws UnsupportedOperationException.
		 * @throws UnsupportedOperationException always; <tt>add</tt>
		 *         is not supported by this iterator.
		 */
		void add(sp<E> e) {
			throw EUnsupportedOperationException(__FILE__, __LINE__);
		}

	private:
		/** Snapshot of the array **/
		ea<E>* snapshot;
		/** Index of element to be returned by subsequent call to next.  */
		int cursor;
	};
};

} /* namespace efc */
#endif /* ECOPYONWRITEARRAYLIST_HH_ */
