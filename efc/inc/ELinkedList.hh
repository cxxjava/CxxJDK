/*
 * ELinkedList.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#ifndef ELINKEDLIST_HH_
#define ELINKEDLIST_HH_

#include "EObject.hh"
#include "EList.hh"
#include "EDeque.hh"
#include "EAbstractList.hh"
#include "ENoSuchElementException.hh"
#include "EIndexOutOfBoundsException.hh"
#include "EIllegalStateException.hh"

namespace efc {

/**
 * Linked list implementation of the <tt>List</tt> interface.  Implements all
 * optional list operations, and permits all elements (including
 * <tt>null</tt>).  In addition to implementing the <tt>List</tt> interface,
 * the <tt>ELinkedList</tt> class provides uniformly named methods to
 * <tt>get</tt>, <tt>remove</tt> and <tt>insert</tt> an element at the
 * beginning and end of the list.  These operations allow linked lists to be
 * used as a stack, {@linkplain Queue queue}, or {@linkplain Deque
 * double-ended queue}. <p>
 *
 * The class implements the <tt>Deque</tt> interface, providing
 * first-in-first-out queue operations for <tt>add</tt>,
 * <tt>poll</tt>, along with other stack and deque operations.<p>
 *
 * All of the operations perform as could be expected for a doubly-linked
 * list.  Operations that index into the list will traverse the list from
 * the beginning or the end, whichever is closer to the specified index.<p>
 *
 * <p><strong>Note that this implementation is not synchronized.</strong>
 * If multiple threads access a linked list concurrently, and at least
 * one of the threads modifies the list structurally, it <i>must</i> be
 * synchronized externally.  (A structural modification is any operation
 * that adds or deletes one or more elements; merely setting the value of
 * an element is not a structural modification.)  This is typically
 * accomplished by synchronizing on some object that naturally
 * encapsulates the list.
 *
 * If no such object exists, the list should be "wrapped" using the
 * {@link Collections#synchronizedList Collections.synchronizedList}
 * method.  This is best done at creation time, to prevent accidental
 * unsynchronized access to the list:<pre>
 *   List list = Collections.synchronizedList(new ELinkedList(...));</pre>
 *
 * <p>The iterators returned by this class's <tt>iterator</tt> and
 * <tt>listIterator</tt> methods are <i>fail-fast</i>: if the list is
 * structurally modified at any time after the iterator is created, in
 * any way except through the Iterator's own <tt>remove</tt> or
 * <tt>add</tt> methods, the iterator will throw a {@link
 * ConcurrentModificationException}.  Thus, in the face of concurrent
 * modification, the iterator fails quickly and cleanly, rather than
 * risking arbitrary, non-deterministic behavior at an undetermined
 * time in the future.
 *
 * <p>Note that the fail-fast behavior of an iterator cannot be guaranteed
 * as it is, generally speaking, impossible to make any hard guarantees in the
 * presence of unsynchronized concurrent modification.  Fail-fast iterators
 * throw <tt>ConcurrentModificationException</tt> on a best-effort basis.
 * Therefore, it would be wrong to write a program that depended on this
 * exception for its correctness:   <i>the fail-fast behavior of iterators
 * should be used only to detect bugs.</i>
 *
 * <p>This class is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @version 1.67, 04/21/06
 * @see	    List
 * @see	    ArrayList
 * @see	    Vector
 * @since 1.2
 * @param <E> the type of elements held in this collection
 */

//=============================================================================
//Primitive Types.

template<typename E>
class ELinkedList : public EAbstractList<E>,
		virtual public EList<E>,
		virtual public EDeque<E> {
public:
	virtual ~ELinkedList() {
		clear();
		delete header;
	}

	/**
	 * Constructs an empty list.
	 */
	ELinkedList() : listSize(0) {
		header = new Entry(0, null, null);
		header->next = header->prev = header;
	}

    ELinkedList(const ELinkedList<E>& that) : listSize(0) {
    	ELinkedList<E>* t = (ELinkedList<E>*)&that;

		header = new Entry(0, null, null);
		header->next = header->prev = header;

		Entry *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}
	}

    ELinkedList<E>& operator= (const ELinkedList<E>& that) {
    	if (this == &that) return *this;

		ELinkedList<E>* t = (ELinkedList<E>*)&that;

		//1.
		clear();
		delete header;

		//2.
		header = new Entry(0, null, null);
		header->next = header->prev = header;

		Entry *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}

		return *this;
	}

	/**
	 * Returns the first element in this list.
	 *
	 * @return the first element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E getFirst() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return header->next->elem;
	}

	/**
	 * Returns the last element in this list.
	 *
	 * @return the last element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E getLast() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return header->prev->elem;
	}

	/**
	 * Removes and returns the first element from this list.
	 *
	 * @return the first element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E removeFirst() THROWS(ENoSuchElementException) {
		return remove(header->next);
	}

	/**
	 * Removes and returns the last element from this list.
	 *
	 * @return the last element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E removeLast() THROWS(ENoSuchElementException) {
		return remove(header->prev);
	}

	/**
	 * Inserts the specified element at the beginning of this list.
	 *
	 * @param e the element to add
	 */
	virtual void addFirst(E e) {
		addBefore(e, header->next);
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 */
	virtual void addLast(E e) {
		addBefore(e, header);
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
	virtual boolean contains(E o) {
		return indexOf(o) != -1;
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	virtual int size() {
		return listSize;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #addLast}.
	 *
	 * @param e element to be appended to this list
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 */
	virtual boolean add(E e) {
		addBefore(e, header);
		return true;
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
	virtual boolean remove(E o) {
		for (Entry *e = header->next; e != header; e = e->next) {
			if (e->elem == (o)) {
				E v = remove(e);
				return true;
			}
		}
		return false;
	}

	/**
	 * Removes all of the elements from this list.
	 */
	virtual void clear() {
		Entry *e = header->next;
		while (e != header) {
			Entry *next = e->next;
			delete e;
			e = next;
		}
        header->next = header->prev = header;
        listSize = 0;
	}

	// Positional Access Operations

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E getAt(int index) THROWS(EIndexOutOfBoundsException) {
        return entry(index)->elem;
	}

	/**
	 * Replaces the element at the specified position in this list with the
	 * specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		Entry *e = entry(index);
        E oldVal = e->elem;
        e->elem = element;
        return oldVal;
	}

	/**
	 * Inserts the specified element at the specified position in this list.
	 * Shifts the element currently at that position (if any) and any
	 * subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual void addAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
        addBefore(element, (index==listSize ? header : entry(index)));
	}

	/**
	 * Removes the element at the specified position in this list.  Shifts any
	 * subsequent elements to the left (subtracts one from their indices).
	 * Returns the element that was removed from the list.
	 *
	 * @param index the index of the element to be removed
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E removeAt(int index) THROWS(EIndexOutOfBoundsException) {
        return remove(entry(index));
	}

	// Search Operations

	/**
	 * Returns the index of the first occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @return the index of the first occurrence of the specified element in
	 *         this list, or -1 if this list does not contain the element
	 */
	virtual int indexOf(E o) {
		int index = 0;
		for (Entry *e = header->next; e != header; e = e->next) {
			if (o == (e->elem))
				return index;
			index++;
		}
		return -1;
	}

	/**
	 * Returns the index of the last occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the highest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @return the index of the last occurrence of the specified element in
	 *         this list, or -1 if this list does not contain the element
	 */
	virtual int lastIndexOf(E o) {
		int index = listSize;
		for (Entry *e = header->prev; e != header; e = e->prev) {
			index--;
			if (o == (e->elem))
				return index;
		}
		return -1;
	}

	// Queue operations.

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual E peek() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual E element() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual E poll() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENOSUCHELEMENTEXCEPTION;
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list.
	 *
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual E remove() THROWS(ENoSuchElementException) {
		return removeFirst();
	}

	/**
	 * Adds the specified element as the tail (last element) of this list.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @since 1.5
	 */
	virtual boolean offer(E e) {
		return add(e);
	}

	// Deque operations
	/**
	 * Inserts the specified element at the front of this list.
	 *
	 * @param e the element to insert
	 * @return <tt>true</tt> (as specified by {@link Deque#offerFirst})
	 * @since 1.6
	 */
	virtual boolean offerFirst(E e) {
		addFirst(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this list.
	 *
	 * @param e the element to insert
	 * @return <tt>true</tt> (as specified by {@link Deque#offerLast})
	 * @since 1.6
	 */
	virtual boolean offerLast(E e) {
		addLast(e);
		return true;
	}

	/**
	 * Retrieves, but does not remove, the first element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the first element of this list, or <tt>null</tt>
	 *         if this list is empty
	 * @since 1.6
	 */
	virtual E peekFirst() {
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the last element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the last element of this list, or <tt>null</tt>
	 *         if this list is empty
	 * @since 1.6
	 */
	virtual E peekLast() {
		return getLast();
	}

	/**
	 * Retrieves and removes the first element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the first element of this list, or <tt>null</tt> if
	 *     this list is empty
	 * @since 1.6
	 */
	virtual E pollFirst() {
		return removeFirst();
	}

	/**
	 * Retrieves and removes the last element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the last element of this list, or <tt>null</tt> if
	 *     this list is empty
	 * @since 1.6
	 */
	virtual E pollLast() {
		return removeLast();
	}

	/**
	 * Pushes an element onto the stack represented by this list.  In other
	 * words, inserts the element at the front of this list.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 * @since 1.6
	 */
	virtual void push(E e) {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this list.  In other
	 * words, removes and returns the first element of this list.
	 *
	 * <p>This method is equivalent to {@link #removeFirst()}.
	 *
	 * @return the element at the front of this list (which is the top
	 *         of the stack represented by this list)
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.6
	 */
	virtual E pop() {
		return removeFirst();
	}

	/**
	 * Removes the first occurrence of the specified element in this
	 * list (when traversing the list from head to tail).  If the list
	 * does not contain the element, it is unchanged.
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if the list contained the specified element
	 * @since 1.6
	 */
	virtual boolean removeFirstOccurrence(E o) {
		return remove(o);
	}

	/**
	 * Removes the last occurrence of the specified element in this
	 * list (when traversing the list from head to tail).  If the list
	 * does not contain the element, it is unchanged.
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if the list contained the specified element
	 * @since 1.6
	 */
	virtual boolean removeLastOccurrence(E o) {
		for (Entry *e = header->prev; e != header; e = e->prev) {
			if (o == (e->elem)) {
				remove(e);
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns a list-iterator of the elements in this list (in proper
	 * sequence), starting at the specified position in the list.
	 * Obeys the general contract of <tt>List.listIterator(int)</tt>.<p>
	 *
	 * The list-iterator is <i>fail-fast</i>: if the list is structurally
	 * modified at any time after the Iterator is created, in any way except
	 * through the list-iterator's own <tt>remove</tt> or <tt>add</tt>
	 * methods, the list-iterator will throw a
	 * <tt>ConcurrentModificationException</tt>.  Thus, in the face of
	 * concurrent modification, the iterator fails quickly and cleanly, rather
	 * than risking arbitrary, non-deterministic behavior at an undetermined
	 * time in the future.
	 *
	 * @param index index of the first element to be returned from the
	 *              list-iterator (by a call to <tt>next</tt>)
	 * @return a ListIterator of the elements in this list (in proper
	 *         sequence), starting at the specified position in the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 * @see List#listIterator(int)
	 */
	virtual sp<EIterator<E> > iterator(int index = 0) {
		return new ListItr(this, index);
	}

	/**
	 * Returns a list-iterator of the elements in this list (in proper
	 * sequence), starting at the specified position in the list.
	 * Obeys the general contract of {@code List.listIterator(int)}.<p>
	 *
	 * The list-iterator is <i>fail-fast</i>: if the list is structurally
	 * modified at any time after the Iterator is created, in any way except
	 * through the list-iterator's own {@code remove} or {@code add}
	 * methods, the list-iterator will throw a
	 * {@code ConcurrentModificationException}.  Thus, in the face of
	 * concurrent modification, the iterator fails quickly and cleanly, rather
	 * than risking arbitrary, non-deterministic behavior at an undetermined
	 * time in the future.
	 *
	 * @param index index of the first element to be returned from the
	 *              list-iterator (by a call to {@code next})
	 * @return a ListIterator of the elements in this list (in proper
	 *         sequence), starting at the specified position in the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 * @see List#listIterator(int)
	 */
	virtual sp<EListIterator<E> > listIterator(int index = 0) {
		return new ListItr(this, index);
	}

	virtual sp<EIterator<E> > descendingIterator() {
		return null;
	}

	virtual boolean isEmpty() {
		return listSize==0;
	}

	/**
	 * Returns a shallow copy of this {@code LinkedList}. (The elements
	 * themselves are not cloned.)
	 *
	 * @return a shallow copy of this {@code LinkedList} instance
	 */
	virtual ELinkedList<E>* clone() {
		ELinkedList<E>* ll = new ELinkedList<E>();

		// Initialize clone with our elements
		for (Entry* x = header->next; x != header; x = x->next)
			ll->add(x->elem);

		return ll;
	}

private:
	class Entry {
	public:
		E elem;
		Entry *next;
		Entry *prev;

		Entry(E element, Entry *next,
				Entry *previous) {
			this->elem = element;
			this->next = next;
			this->prev = previous;
		}

		~Entry() {
		}
	};

	class ListItr : public EListIterator<E> {
	private:
		ELinkedList<E>* self;

		Entry* lastReturned;
		Entry* next_;
        int nextIndex_;
	public:
        ListItr(ELinkedList<E>* list, int index) : self(list), lastReturned(null), next_(null) {
            // assert isPositionIndex(index);
        	if (index < 0 || index > self->listSize)
				throw EIndexOutOfBoundsException(__FILE__, __LINE__,
						EString::formatOf("Index: %d , Size: %d", index, self->listSize).c_str());

        	next_ = (index == self->listSize) ? null : self->entry(index);
        	nextIndex_ = index;
        }

        boolean hasNext() {
            return nextIndex_ < self->listSize;
        }

        E next() {
            if (!hasNext())
                throw ENoSuchElementException(__FILE__, __LINE__);

            lastReturned = next_;
            next_ = next_->next;
            nextIndex_++;
            return lastReturned->elem;
        }

        boolean hasPrevious() {
            return nextIndex_ > 0;
        }

        E previous() {
            if (!hasPrevious())
                throw ENoSuchElementException(__FILE__, __LINE__);

            lastReturned = next_ = (next_ == null) ? self->header->prev : next_->prev;
            nextIndex_--;
            return lastReturned->elem;
        }

        int nextIndex() {
            return nextIndex_;
        }

        int previousIndex() {
            return nextIndex_ - 1;
        }

        void remove() {
            if (lastReturned == null)
                throw ENoSuchElementException(__FILE__, __LINE__);

            Entry* lastNext = lastReturned->next;
            E v = self->remove(lastReturned);
            if (next_ == lastReturned)
            	next_ = lastNext;
            else
            	nextIndex_--;
            lastReturned = null;
        }

        E moveOut() {
        	if (lastReturned == null)
				throw ENoSuchElementException(__FILE__, __LINE__);

			Entry* lastNext = lastReturned->next;
			E v = self->remove(lastReturned);
			if (next_ == lastReturned)
				next_ = lastNext;
			else
				nextIndex_--;
			lastReturned = null;
			return v;
		}

        void set(E e) {
            if (lastReturned == null)
                throw EIllegalStateException(__FILE__, __LINE__);
            E v = lastReturned->elem;
            lastReturned->elem = e;
        }

        void add(E e) {
            lastReturned = null;
            if (next_ == null)
            	self->addLast(e);
            else
            	self->addBefore(e, next_);
            nextIndex_++;
        }
    };

	Entry *header;
	int listSize;

	Entry* addBefore(E e,
			Entry *entry) {
		Entry *newEntry = new Entry(e, entry,
				entry->prev);
		newEntry->prev->next = newEntry;
		newEntry->next->prev = newEntry;
		listSize++;
		return newEntry;
	}

	E remove(Entry *e) {
		if (e == header)
			throw ENoSuchElementException(__FILE__, __LINE__);

		E result = e->elem;
		e->prev->next = e->next;
		e->next->prev = e->prev;
		delete e; //!
		listSize--;
		return result;
	}

	/**
	 * Returns the indexed entry.
	 */
	Entry* entry(int index) {
		if (index < 0 || index >= listSize)
			throw EIndexOutOfBoundsException(__FILE__, __LINE__,
					EString::formatOf("Index: %d , Size: %d", index, listSize).c_str());
		Entry *e = header;
		if (index < (listSize >> 1)) {
			for (int i = 0; i <= index; i++)
				e = e->next;
		} else {
			for (int i = listSize; i > index; i--)
				e = e->prev;
		}
		return e;
	}
};

//=============================================================================
//Native pointer Types.

template<typename T>
class ELinkedList<T*> : public EAbstractList<T*>,
		virtual public EList<T*>,
		virtual public EDeque<T*> {
public:
	typedef T* E;

	virtual ~ELinkedList() {
		clear();
		delete header;
	}

	/**
	 * Constructs an empty list.
	 */
	ELinkedList(boolean autoFree = true) : _autoFree(autoFree), listSize(0) {
		header = new Entry(null, null, null);
		header->next = header->prev = header;
	}

    ELinkedList(const ELinkedList<E>& that) : listSize(0) {
    	ELinkedList<E>* t = (ELinkedList<E>*)&that;

		header = new Entry(null, null, null);
		header->next = header->prev = header;

		Entry *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}

		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);
	}

    ELinkedList<E>& operator= (const ELinkedList<E>& that) {
    	if (this == &that) return *this;

		ELinkedList<E>* t = (ELinkedList<E>*)&that;

		//1.
		clear();
		delete header;

		//2.
		header = new Entry(null, null, null);
		header->next = header->prev = header;

		Entry *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}

		this->setAutoFree(t->getAutoFree());
		t->setAutoFree(false);

		return *this;
	}

	void setAutoFree(boolean autoFree = true) {
		_autoFree = autoFree;
	}

	boolean getAutoFree() {
		return _autoFree;
	}

	/**
	 * Returns the first element in this list.
	 *
	 * @return the first element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E getFirst() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return header->next->elem;
	}

	/**
	 * Returns the last element in this list.
	 *
	 * @return the last element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E getLast() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return header->prev->elem;
	}

	/**
	 * Removes and returns the first element from this list.
	 *
	 * @return the first element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E removeFirst() THROWS(ENoSuchElementException) {
		return remove(header->next);
	}

	/**
	 * Removes and returns the last element from this list.
	 *
	 * @return the last element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E removeLast() THROWS(ENoSuchElementException) {
		return remove(header->prev);
	}

	/**
	 * Inserts the specified element at the beginning of this list.
	 *
	 * @param e the element to add
	 */
	virtual void addFirst(E e) {
		addBefore(e, header->next);
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 */
	virtual void addLast(E e) {
		addBefore(e, header);
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
	virtual boolean contains(E o) {
		return indexOf(o) != -1;
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	virtual int size() {
		return listSize;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #addLast}.
	 *
	 * @param e element to be appended to this list
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 */
	virtual boolean add(E e) {
		addBefore(e, header);
		return true;
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
	virtual boolean remove(E o) {
		for (Entry *e = header->next; e != header; e = e->next) {
			if (e->elem->equals(o)) {
				E v = remove(e);
				if (_autoFree && v) {
					delete v;
				}
				return true;
			}
		}
		return false;
	}

	/**
	 * Removes all of the elements from this list.
	 */
	virtual void clear() {
		Entry *e = header->next;
		while (e != header) {
			Entry *next = e->next;
			if (_autoFree && e) {
				delete e->elem;
			}
			delete e;
			e = next;
		}
        header->next = header->prev = header;
        listSize = 0;
	}

	// Positional Access Operations

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E getAt(int index) THROWS(EIndexOutOfBoundsException) {
        return entry(index)->elem;
	}

	/**
	 * Replaces the element at the specified position in this list with the
	 * specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		Entry *e = entry(index);
        E oldVal = e->elem;
        e->elem = element;
        return oldVal;
	}

	/**
	 * Inserts the specified element at the specified position in this list.
	 * Shifts the element currently at that position (if any) and any
	 * subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual void addAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
        addBefore(element, (index==listSize ? header : entry(index)));
	}

	/**
	 * Removes the element at the specified position in this list.  Shifts any
	 * subsequent elements to the left (subtracts one from their indices).
	 * Returns the element that was removed from the list.
	 *
	 * @param index the index of the element to be removed
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E removeAt(int index) THROWS(EIndexOutOfBoundsException) {
        return remove(entry(index));
	}

	// Search Operations

	/**
	 * Returns the index of the first occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @return the index of the first occurrence of the specified element in
	 *         this list, or -1 if this list does not contain the element
	 */
	virtual int indexOf(E o) {
		int index = 0;
		for (Entry *e = header->next; e != header; e = e->next) {
			if (o->equals(e->elem))
				return index;
			index++;
		}
		return -1;
	}

	/**
	 * Returns the index of the last occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the highest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @return the index of the last occurrence of the specified element in
	 *         this list, or -1 if this list does not contain the element
	 */
	virtual int lastIndexOf(E o) {
		int index = listSize;
		for (Entry *e = header->prev; e != header; e = e->prev) {
			index--;
			if (o->equals(e->elem))
				return index;
		}
		return -1;
	}

	// Queue operations.

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual E peek() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			return null;
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual E element() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual E poll() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			return null;
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list.
	 *
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual E remove() THROWS(ENoSuchElementException) {
		return removeFirst();
	}

	/**
	 * Adds the specified element as the tail (last element) of this list.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @since 1.5
	 */
	virtual boolean offer(E e) {
		return add(e);
	}

	// Deque operations
	/**
	 * Inserts the specified element at the front of this list.
	 *
	 * @param e the element to insert
	 * @return <tt>true</tt> (as specified by {@link Deque#offerFirst})
	 * @since 1.6
	 */
	virtual boolean offerFirst(E e) {
		addFirst(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this list.
	 *
	 * @param e the element to insert
	 * @return <tt>true</tt> (as specified by {@link Deque#offerLast})
	 * @since 1.6
	 */
	virtual boolean offerLast(E e) {
		addLast(e);
		return true;
	}

	/**
	 * Retrieves, but does not remove, the first element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the first element of this list, or <tt>null</tt>
	 *         if this list is empty
	 * @since 1.6
	 */
	virtual E peekFirst() {
		if (listSize == 0)
			return null;
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the last element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the last element of this list, or <tt>null</tt>
	 *         if this list is empty
	 * @since 1.6
	 */
	virtual E peekLast() {
		if (listSize == 0)
			return null;
		return getLast();
	}

	/**
	 * Retrieves and removes the first element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the first element of this list, or <tt>null</tt> if
	 *     this list is empty
	 * @since 1.6
	 */
	virtual E pollFirst() {
		if (listSize == 0)
			return null;
		return removeFirst();
	}

	/**
	 * Retrieves and removes the last element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the last element of this list, or <tt>null</tt> if
	 *     this list is empty
	 * @since 1.6
	 */
	virtual E pollLast() {
		if (listSize == 0)
			return null;
		return removeLast();
	}

	/**
	 * Pushes an element onto the stack represented by this list.  In other
	 * words, inserts the element at the front of this list.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 * @since 1.6
	 */
	virtual void push(E e) {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this list.  In other
	 * words, removes and returns the first element of this list.
	 *
	 * <p>This method is equivalent to {@link #removeFirst()}.
	 *
	 * @return the element at the front of this list (which is the top
	 *         of the stack represented by this list)
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.6
	 */
	virtual E pop() {
		return removeFirst();
	}

	/**
	 * Removes the first occurrence of the specified element in this
	 * list (when traversing the list from head to tail).  If the list
	 * does not contain the element, it is unchanged.
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if the list contained the specified element
	 * @since 1.6
	 */
	virtual boolean removeFirstOccurrence(E o) {
		return remove(o);
	}

	/**
	 * Removes the last occurrence of the specified element in this
	 * list (when traversing the list from head to tail).  If the list
	 * does not contain the element, it is unchanged.
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if the list contained the specified element
	 * @since 1.6
	 */
	virtual boolean removeLastOccurrence(E o) {
		for (Entry *e = header->prev; e != header; e = e->prev) {
			if (o->equals(e->elem)) {
				remove(e);
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns a list-iterator of the elements in this list (in proper
	 * sequence), starting at the specified position in the list.
	 * Obeys the general contract of <tt>List.listIterator(int)</tt>.<p>
	 *
	 * The list-iterator is <i>fail-fast</i>: if the list is structurally
	 * modified at any time after the Iterator is created, in any way except
	 * through the list-iterator's own <tt>remove</tt> or <tt>add</tt>
	 * methods, the list-iterator will throw a
	 * <tt>ConcurrentModificationException</tt>.  Thus, in the face of
	 * concurrent modification, the iterator fails quickly and cleanly, rather
	 * than risking arbitrary, non-deterministic behavior at an undetermined
	 * time in the future.
	 *
	 * @param index index of the first element to be returned from the
	 *              list-iterator (by a call to <tt>next</tt>)
	 * @return a ListIterator of the elements in this list (in proper
	 *         sequence), starting at the specified position in the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 * @see List#listIterator(int)
	 */
	virtual sp<EIterator<E> > iterator(int index = 0) {
		return new ListItr(this, index);
	}

	/**
	 * Returns a list-iterator of the elements in this list (in proper
	 * sequence), starting at the specified position in the list.
	 * Obeys the general contract of {@code List.listIterator(int)}.<p>
	 *
	 * The list-iterator is <i>fail-fast</i>: if the list is structurally
	 * modified at any time after the Iterator is created, in any way except
	 * through the list-iterator's own {@code remove} or {@code add}
	 * methods, the list-iterator will throw a
	 * {@code ConcurrentModificationException}.  Thus, in the face of
	 * concurrent modification, the iterator fails quickly and cleanly, rather
	 * than risking arbitrary, non-deterministic behavior at an undetermined
	 * time in the future.
	 *
	 * @param index index of the first element to be returned from the
	 *              list-iterator (by a call to {@code next})
	 * @return a ListIterator of the elements in this list (in proper
	 *         sequence), starting at the specified position in the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 * @see List#listIterator(int)
	 */
	virtual sp<EListIterator<E> > listIterator(int index = 0) {
		return new ListItr(this, index);
	}

	virtual sp<EIterator<E> > descendingIterator() {
		return null;
	}

	virtual boolean isEmpty() {
		return listSize==0;
	}

	/**
	 * Returns a shallow copy of this {@code LinkedList}. (The elements
	 * themselves are not cloned.)
	 *
	 * @return a shallow copy of this {@code LinkedList} instance
	 */
	virtual ELinkedList<E>* clone() {
		ELinkedList<E>* ll = new ELinkedList<E>(false);

		// Initialize clone with our elements
		for (Entry* x = header->next; x != header; x = x->next)
			ll->add(x->elem);

		return ll;
	}

private:
	class Entry {
	public:
		E elem;
		Entry *next;
		Entry *prev;

		Entry(E element, Entry *next,
				Entry *previous) {
			this->elem = element;
			this->next = next;
			this->prev = previous;
		}

		~Entry() {
		}
	};

	class ListItr : public EListIterator<E> {
	private:
		ELinkedList<E>* self;

		Entry* lastReturned;
		Entry* next_;
        int nextIndex_;
	public:
        ListItr(ELinkedList<E>* list, int index) : self(list), lastReturned(null), next_(null) {
            // assert isPositionIndex(index);
        	if (index < 0 || index > self->listSize)
				throw EIndexOutOfBoundsException(__FILE__, __LINE__,
						EString::formatOf("Index: %d , Size: %d", index, self->listSize).c_str());

        	next_ = (index == self->listSize) ? null : self->entry(index);
        	nextIndex_ = index;
        }

        boolean hasNext() {
            return nextIndex_ < self->listSize;
        }

        E next() {
            if (!hasNext())
                throw ENoSuchElementException(__FILE__, __LINE__);

            lastReturned = next_;
            next_ = next_->next;
            nextIndex_++;
            return lastReturned->elem;
        }

        boolean hasPrevious() {
            return nextIndex_ > 0;
        }

        E previous() {
            if (!hasPrevious())
                throw ENoSuchElementException(__FILE__, __LINE__);

            lastReturned = next_ = (next_ == null) ? self->header->prev : next_->prev;
            nextIndex_--;
            return lastReturned->elem;
        }

        int nextIndex() {
            return nextIndex_;
        }

        int previousIndex() {
            return nextIndex_ - 1;
        }

        void remove() {
            if (lastReturned == null)
                throw ENoSuchElementException(__FILE__, __LINE__);

            Entry* lastNext = lastReturned->next;
            E v = self->remove(lastReturned);
            if (self->getAutoFree() && v) {
				delete v;
			}
            if (next_ == lastReturned)
            	next_ = lastNext;
            else
            	nextIndex_--;
            lastReturned = null;
        }

        E moveOut() {
        	if (lastReturned == null)
				throw ENoSuchElementException(__FILE__, __LINE__);

			Entry* lastNext = lastReturned->next;
			E v = self->remove(lastReturned);
			if (next_ == lastReturned)
				next_ = lastNext;
			else
				nextIndex_--;
			lastReturned = null;
			return v;
		}

        void set(E e) {
            if (lastReturned == null)
                throw EIllegalStateException(__FILE__, __LINE__);
            E v = lastReturned->elem;
            if (self->getAutoFree() && v) {
				delete v;
			}
            lastReturned->elem = e;
        }

        void add(E e) {
            lastReturned = null;
            if (next_ == null)
            	self->addLast(e);
            else
            	self->addBefore(e, next_);
            nextIndex_++;
        }
    };

	Entry *header;
	boolean _autoFree;
	int listSize;

	Entry* addBefore(E e,
			Entry *entry) {
		Entry *newEntry = new Entry(e, entry,
				entry->prev);
		newEntry->prev->next = newEntry;
		newEntry->next->prev = newEntry;
		listSize++;
		return newEntry;
	}

	E remove(Entry *e) {
		if (e == header)
			throw ENoSuchElementException(__FILE__, __LINE__);

		E result = e->elem;
		e->prev->next = e->next;
		e->next->prev = e->prev;
		e->elem = null; //!
		delete e; //!
		listSize--;
		return result;
	}

	/**
	 * Returns the indexed entry.
	 */
	Entry* entry(int index) {
		if (index < 0 || index >= listSize)
			throw EIndexOutOfBoundsException(__FILE__, __LINE__,
					EString::formatOf("Index: %d , Size: %d", index, listSize).c_str());
		Entry *e = header;
		if (index < (listSize >> 1)) {
			for (int i = 0; i <= index; i++)
				e = e->next;
		} else {
			for (int i = listSize; i > index; i--)
				e = e->prev;
		}
		return e;
	}
};

//=============================================================================
//Shared pointer Types.

template<typename T>
class ELinkedList<sp<T> > : public EAbstractList<sp<T> >,
		virtual public EList<sp<T> >,
		virtual public EDeque<sp<T> > {
public:
	typedef sp<T> E;

	virtual ~ELinkedList() {
		clear();
		delete header;
	}

	/**
	 * Constructs an empty list.
	 */
	ELinkedList() : listSize(0) {
		header = new Entry(null, null, null);
		header->next = header->prev = header;
	}

    ELinkedList(const ELinkedList<E>& that) : listSize(0) {
    	ELinkedList<E>* t = (ELinkedList<E>*)&that;

		header = new Entry(null, null, null);
		header->next = header->prev = header;

		Entry *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}
	}

    ELinkedList<E>& operator= (const ELinkedList<E>& that) {
    	if (this == &that) return *this;

		ELinkedList<E>* t = (ELinkedList<E>*)&that;

		//1.
		clear();
		delete header;

		//2.
		header = new Entry(null, null, null);
		header->next = header->prev = header;

		Entry *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}

		return *this;
	}

	/**
	 * Returns the first element in this list.
	 *
	 * @return the first element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E getFirst() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return header->next->elem;
	}

	/**
	 * Returns the last element in this list.
	 *
	 * @return the last element in this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E getLast() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return header->prev->elem;
	}

	/**
	 * Removes and returns the first element from this list.
	 *
	 * @return the first element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E removeFirst() THROWS(ENoSuchElementException) {
		return remove(header->next);
	}

	/**
	 * Removes and returns the last element from this list.
	 *
	 * @return the last element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual E removeLast() THROWS(ENoSuchElementException) {
		return remove(header->prev);
	}

	/**
	 * Inserts the specified element at the beginning of this list.
	 *
	 * @param e the element to add
	 */
	virtual void addFirst(E e) {
		addBefore(e, header->next);
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 */
	virtual void addLast(E e) {
		addBefore(e, header);
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
	virtual boolean contains(T* o) {
		return indexOf(o) != -1;
	}

	/**
	 * Returns the number of elements in this list.
	 *
	 * @return the number of elements in this list
	 */
	virtual int size() {
		return listSize;
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #addLast}.
	 *
	 * @param e element to be appended to this list
	 * @return <tt>true</tt> (as specified by {@link Collection#add})
	 */
	virtual boolean add(E e) {
		addBefore(e, header);
		return true;
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
	virtual boolean remove(T* o) {
		for (Entry *e = header->next; e != header; e = e->next) {
			if (e->elem->equals(o)) {
				E v = remove(e);
				return true;
			}
		}
		return false;
	}

	/**
	 * Removes all of the elements from this list.
	 */
	virtual void clear() {
		Entry *e = header->next;
		while (e != header) {
			Entry *next = e->next;
			delete e;
			e = next;
		}
        header->next = header->prev = header;
        listSize = 0;
	}

	// Positional Access Operations

	/**
	 * Returns the element at the specified position in this list.
	 *
	 * @param index index of the element to return
	 * @return the element at the specified position in this list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E getAt(int index) THROWS(EIndexOutOfBoundsException) {
        return entry(index)->elem;
	}

	/**
	 * Replaces the element at the specified position in this list with the
	 * specified element.
	 *
	 * @param index index of the element to replace
	 * @param element element to be stored at the specified position
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E setAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
		Entry *e = entry(index);
        E oldVal = e->elem;
        e->elem = element;
        return oldVal;
	}

	/**
	 * Inserts the specified element at the specified position in this list.
	 * Shifts the element currently at that position (if any) and any
	 * subsequent elements to the right (adds one to their indices).
	 *
	 * @param index index at which the specified element is to be inserted
	 * @param element element to be inserted
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual void addAt(int index, E element) THROWS(EIndexOutOfBoundsException) {
        addBefore(element, (index==listSize ? header : entry(index)));
	}

	/**
	 * Removes the element at the specified position in this list.  Shifts any
	 * subsequent elements to the left (subtracts one from their indices).
	 * Returns the element that was removed from the list.
	 *
	 * @param index the index of the element to be removed
	 * @return the element previously at the specified position
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 */
	virtual E removeAt(int index) THROWS(EIndexOutOfBoundsException) {
        return remove(entry(index));
	}

	// Search Operations

	/**
	 * Returns the index of the first occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the lowest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @return the index of the first occurrence of the specified element in
	 *         this list, or -1 if this list does not contain the element
	 */
	virtual int indexOf(T* o) {
		int index = 0;
		for (Entry *e = header->next; e != header; e = e->next) {
			if (e->elem->equals(o))
				return index;
			index++;
		}
		return -1;
	}

	/**
	 * Returns the index of the last occurrence of the specified element
	 * in this list, or -1 if this list does not contain the element.
	 * More formally, returns the highest index <tt>i</tt> such that
	 * <tt>(o==null&nbsp;?&nbsp;get(i)==null&nbsp;:&nbsp;o.equals(get(i)))</tt>,
	 * or -1 if there is no such index.
	 *
	 * @param o element to search for
	 * @return the index of the last occurrence of the specified element in
	 *         this list, or -1 if this list does not contain the element
	 */
	virtual int lastIndexOf(T* o) {
		int index = listSize;
		for (Entry *e = header->prev; e != header; e = e->prev) {
			index--;
			if (e->elem->equals(o))
				return index;
		}
		return -1;
	}

	// Queue operations.

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual E peek() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			return null;
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual E element() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual E poll() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			return null;
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list.
	 *
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual E remove() THROWS(ENoSuchElementException) {
		return removeFirst();
	}

	/**
	 * Adds the specified element as the tail (last element) of this list.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @since 1.5
	 */
	virtual boolean offer(E e) {
		return add(e);
	}

	// Deque operations
	/**
	 * Inserts the specified element at the front of this list.
	 *
	 * @param e the element to insert
	 * @return <tt>true</tt> (as specified by {@link Deque#offerFirst})
	 * @since 1.6
	 */
	virtual boolean offerFirst(E e) {
		addFirst(e);
		return true;
	}

	/**
	 * Inserts the specified element at the end of this list.
	 *
	 * @param e the element to insert
	 * @return <tt>true</tt> (as specified by {@link Deque#offerLast})
	 * @since 1.6
	 */
	virtual boolean offerLast(E e) {
		addLast(e);
		return true;
	}

	/**
	 * Retrieves, but does not remove, the first element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the first element of this list, or <tt>null</tt>
	 *         if this list is empty
	 * @since 1.6
	 */
	virtual E peekFirst() {
		if (listSize == 0)
			return null;
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the last element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the last element of this list, or <tt>null</tt>
	 *         if this list is empty
	 * @since 1.6
	 */
	virtual E peekLast() {
		if (listSize == 0)
			return null;
		return getLast();
	}

	/**
	 * Retrieves and removes the first element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the first element of this list, or <tt>null</tt> if
	 *     this list is empty
	 * @since 1.6
	 */
	virtual E pollFirst() {
		if (listSize == 0)
			return null;
		return removeFirst();
	}

	/**
	 * Retrieves and removes the last element of this list,
	 * or returns <tt>null</tt> if this list is empty.
	 *
	 * @return the last element of this list, or <tt>null</tt> if
	 *     this list is empty
	 * @since 1.6
	 */
	virtual E pollLast() {
		if (listSize == 0)
			return null;
		return removeLast();
	}

	/**
	 * Pushes an element onto the stack represented by this list.  In other
	 * words, inserts the element at the front of this list.
	 *
	 * <p>This method is equivalent to {@link #addFirst}.
	 *
	 * @param e the element to push
	 * @since 1.6
	 */
	virtual void push(E e) {
		addFirst(e);
	}

	/**
	 * Pops an element from the stack represented by this list.  In other
	 * words, removes and returns the first element of this list.
	 *
	 * <p>This method is equivalent to {@link #removeFirst()}.
	 *
	 * @return the element at the front of this list (which is the top
	 *         of the stack represented by this list)
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.6
	 */
	virtual E pop() {
		return removeFirst();
	}

	/**
	 * Removes the first occurrence of the specified element in this
	 * list (when traversing the list from head to tail).  If the list
	 * does not contain the element, it is unchanged.
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if the list contained the specified element
	 * @since 1.6
	 */
	virtual boolean removeFirstOccurrence(T* o) {
		return remove(o);
	}

	/**
	 * Removes the last occurrence of the specified element in this
	 * list (when traversing the list from head to tail).  If the list
	 * does not contain the element, it is unchanged.
	 *
	 * @param o element to be removed from this list, if present
	 * @return <tt>true</tt> if the list contained the specified element
	 * @since 1.6
	 */
	virtual boolean removeLastOccurrence(T* o) {
		for (Entry *e = header->prev; e != header; e = e->prev) {
			if (e->elem->equals(o)) {
				remove(e);
				return true;
			}
		}
		return false;
	}

	/**
	 * Returns a list-iterator of the elements in this list (in proper
	 * sequence), starting at the specified position in the list.
	 * Obeys the general contract of <tt>List.listIterator(int)</tt>.<p>
	 *
	 * The list-iterator is <i>fail-fast</i>: if the list is structurally
	 * modified at any time after the Iterator is created, in any way except
	 * through the list-iterator's own <tt>remove</tt> or <tt>add</tt>
	 * methods, the list-iterator will throw a
	 * <tt>ConcurrentModificationException</tt>.  Thus, in the face of
	 * concurrent modification, the iterator fails quickly and cleanly, rather
	 * than risking arbitrary, non-deterministic behavior at an undetermined
	 * time in the future.
	 *
	 * @param index index of the first element to be returned from the
	 *              list-iterator (by a call to <tt>next</tt>)
	 * @return a ListIterator of the elements in this list (in proper
	 *         sequence), starting at the specified position in the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 * @see List#listIterator(int)
	 */
	virtual sp<EIterator<E> > iterator(int index = 0) {
		return new ListItr(this, index);
	}

	/**
	 * Returns a list-iterator of the elements in this list (in proper
	 * sequence), starting at the specified position in the list.
	 * Obeys the general contract of {@code List.listIterator(int)}.<p>
	 *
	 * The list-iterator is <i>fail-fast</i>: if the list is structurally
	 * modified at any time after the Iterator is created, in any way except
	 * through the list-iterator's own {@code remove} or {@code add}
	 * methods, the list-iterator will throw a
	 * {@code ConcurrentModificationException}.  Thus, in the face of
	 * concurrent modification, the iterator fails quickly and cleanly, rather
	 * than risking arbitrary, non-deterministic behavior at an undetermined
	 * time in the future.
	 *
	 * @param index index of the first element to be returned from the
	 *              list-iterator (by a call to {@code next})
	 * @return a ListIterator of the elements in this list (in proper
	 *         sequence), starting at the specified position in the list
	 * @throws IndexOutOfBoundsException {@inheritDoc}
	 * @see List#listIterator(int)
	 */
	virtual sp<EListIterator<E> > listIterator(int index = 0) {
		return new ListItr(this, index);
	}

	virtual sp<EIterator<E> > descendingIterator() {
		return null;
	}

	virtual boolean isEmpty() {
		return listSize==0;
	}

	/**
	 * Returns a shallow copy of this {@code LinkedList}. (The elements
	 * themselves are not cloned.)
	 *
	 * @return a shallow copy of this {@code LinkedList} instance
	 */
	virtual ELinkedList<E>* clone() {
		ELinkedList<E>* ll = new ELinkedList<E>();

		// Initialize clone with our elements
		for (Entry* x = header->next; x != header; x = x->next)
			ll->add(x->elem);

		return ll;
	}

private:
	class Entry {
	public:
		E elem;
		Entry *next;
		Entry *prev;

		Entry(E element, Entry *next,
				Entry *previous) {
			this->elem = element;
			this->next = next;
			this->prev = previous;
		}

		~Entry() {
		}
	};

	class ListItr : public EListIterator<E> {
	private:
		ELinkedList<E>* self;

		Entry* lastReturned;
		Entry* next_;
        int nextIndex_;
	public:
        ListItr(ELinkedList<E>* list, int index) : self(list), lastReturned(null), next_(null) {
            // assert isPositionIndex(index);
        	if (index < 0 || index > self->listSize)
				throw EIndexOutOfBoundsException(__FILE__, __LINE__,
						EString::formatOf("Index: %d , Size: %d", index, self->listSize).c_str());

        	next_ = (index == self->listSize) ? null : self->entry(index);
        	nextIndex_ = index;
        }

        boolean hasNext() {
            return nextIndex_ < self->listSize;
        }

        E next() {
            if (!hasNext())
                throw ENoSuchElementException(__FILE__, __LINE__);

            lastReturned = next_;
            next_ = next_->next;
            nextIndex_++;
            return lastReturned->elem;
        }

        boolean hasPrevious() {
            return nextIndex_ > 0;
        }

        E previous() {
            if (!hasPrevious())
                throw ENoSuchElementException(__FILE__, __LINE__);

            lastReturned = next_ = (next_ == null) ? self->header->prev : next_->prev;
            nextIndex_--;
            return lastReturned->elem;
        }

        int nextIndex() {
            return nextIndex_;
        }

        int previousIndex() {
            return nextIndex_ - 1;
        }

        void remove() {
            if (lastReturned == null)
                throw ENoSuchElementException(__FILE__, __LINE__);

            Entry* lastNext = lastReturned->next;
            E v = self->remove(lastReturned);
            if (next_ == lastReturned)
            	next_ = lastNext;
            else
            	nextIndex_--;
            lastReturned = null;
        }

        E moveOut() {
        	if (lastReturned == null)
				throw ENoSuchElementException(__FILE__, __LINE__);

			Entry* lastNext = lastReturned->next;
			E v = self->remove(lastReturned);
			if (next_ == lastReturned)
				next_ = lastNext;
			else
				nextIndex_--;
			lastReturned = null;
			return v;
		}

        void set(E e) {
            if (lastReturned == null)
                throw EIllegalStateException(__FILE__, __LINE__);
            E v = lastReturned->elem;
            lastReturned->elem = e;
        }

        void add(E e) {
            lastReturned = null;
            if (next_ == null)
            	self->addLast(e);
            else
            	self->addBefore(e, next_);
            nextIndex_++;
        }
    };

	Entry *header;
	int listSize;

	Entry* addBefore(E e,
			Entry *entry) {
		Entry *newEntry = new Entry(e, entry,
				entry->prev);
		newEntry->prev->next = newEntry;
		newEntry->next->prev = newEntry;
		listSize++;
		return newEntry;
	}

	E remove(Entry *e) {
		if (e == header)
			throw ENoSuchElementException(__FILE__, __LINE__);

		E result = e->elem;
		e->prev->next = e->next;
		e->next->prev = e->prev;
		e->elem = null; //!
		delete e; //!
		listSize--;
		return result;
	}

	/**
	 * Returns the indexed entry.
	 */
	Entry* entry(int index) {
		if (index < 0 || index >= listSize)
			throw EIndexOutOfBoundsException(__FILE__, __LINE__,
					EString::formatOf("Index: %d , Size: %d", index, listSize).c_str());
		Entry *e = header;
		if (index < (listSize >> 1)) {
			for (int i = 0; i <= index; i++)
				e = e->next;
		} else {
			for (int i = listSize; i > index; i--)
				e = e->prev;
		}
		return e;
	}
};

} /* namespace efc */
#endif //!ELINKEDLIST_HH_
