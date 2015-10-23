/*
 * ELinkedList.hh
 *
 *  Created on: 2013-3-25
 *      Author: Administrator
 */

#ifndef ELINKEDLIST_HH_
#define ELINKEDLIST_HH_

#include "EObject.hh"
#include "EList.hh"
#include "EDeque.hh"
#include "EAbstractList.hh"
#include "ENoSuchElementException.hh"
#include "EIndexOutOfBoundsException.hh"

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
 * @author  Josh Bloch
 * @version 1.67, 04/21/06
 * @see	    List
 * @see	    ArrayList
 * @see	    Vector
 * @since 1.2
 * @param <E> the type of elements held in this collection
 */

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
	ELinkedList(boolean autoFree = true) : _autoFree(autoFree), listSize(0) {
		header = new Entry<E>(0, null, null);
		header->next = header->prev = header;
	}

    ELinkedList(const ELinkedList<E>& that) : listSize(0) {
    	ELinkedList<E>* t = (ELinkedList<E>*)&that;

		header = new Entry<E>(0, null, null);
		header->next = header->prev = header;

		Entry<E> *e = t->header->next;
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
		header = new Entry<E>(0, null, null);
		header->next = header->prev = header;

		Entry<E> *e = t->header->next;
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

	virtual boolean removeAll(ECollection<E> *c) {
		//TODO...
		return false;
	}

	virtual boolean retainAll(ECollection<E> *c) {
		//TODO...
		return false;
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
		for (Entry<E> *e = header->next; e != header; e = e->next) {
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
		Entry<E> *e = header->next;
		while (e != header) {
			Entry<E> *next = e->next;
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
        Entry<E> *e = entry(index);
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
		for (Entry<E> *e = header->next; e != header; e = e->next) {
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
		for (Entry<E> *e = header->prev; e != header; e = e->prev) {
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
		for (Entry<E> *e = header->prev; e != header; e = e->prev) {
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
	virtual EIterator<E>* iterator(int index = 0) {
		return EAbstractList<E>::iterator(index);
	}

	virtual boolean isEmpty() {
		return listSize==0;
	}

	virtual EIterator<E>* descendingIterator() {
		return null;
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
		for (Entry<E>* x = header->next; x != header; x = x->next)
			ll->add(x->elem);

		return ll;
	}

private:
	template<typename _TELinkedListEntry>
	class Entry {
	public:
		E elem;
		Entry<_TELinkedListEntry> *next;
		Entry<_TELinkedListEntry> *prev;

		Entry(_TELinkedListEntry element, Entry<_TELinkedListEntry> *next,
				Entry<_TELinkedListEntry> *previous) {
			this->elem = element;
			this->next = next;
			this->prev = previous;
		}

		~Entry() {
		}
	};

	Entry<E> *header;
	boolean _autoFree;
	int listSize;

	Entry<E>* addBefore(E e,
			Entry<E> *entry) {
		Entry<E> *newEntry = new Entry<E>(e, entry,
				entry->prev);
		newEntry->prev->next = newEntry;
		newEntry->next->prev = newEntry;
		listSize++;
		return newEntry;
	}

	E remove(Entry<E> *e) {
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
	Entry<E>* entry(int index) {
		if (index < 0 || index >= listSize)
			throw EIndexOutOfBoundsException(
					EString::formatOf("Index: %d , Size: %d", index, listSize).c_str(),
					__FILE__, __LINE__);
		Entry<E> *e = header;
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

template<>
class ELinkedList<int> : public EAbstractList<int>,
		virtual public EList<int>,
		virtual public EDeque<int> {
public:
	virtual ~ELinkedList() {
		clear();
		delete header;
	}

	/**
	 * Constructs an empty list.
	 */
	ELinkedList() : listSize(0) {
		header = new Entry<int>(0, null, null);
		header->next = header->prev = header;
	}

    ELinkedList(const ELinkedList<int>& that) : listSize(0) {
    	ELinkedList<int>* t = (ELinkedList<int>*)&that;

		header = new Entry<int>(0, null, null);
		header->next = header->prev = header;

		Entry<int> *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}
	}

    ELinkedList<int>& operator= (const ELinkedList<int>& that) {
    	if (this == &that) return *this;

		ELinkedList<int>* t = (ELinkedList<int>*)&that;

		//1.
		clear();
		delete header;

		//2.
		header = new Entry<int>(0, null, null);
		header->next = header->prev = header;

		Entry<int> *e = t->header->next;
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
	virtual int getFirst() THROWS(ENoSuchElementException) {
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
	virtual int getLast() THROWS(ENoSuchElementException) {
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
	virtual int removeFirst() THROWS(ENoSuchElementException) {
		return remove(header->next);
	}

	/**
	 * Removes and returns the last element from this list.
	 *
	 * @return the last element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual int removeLast() THROWS(ENoSuchElementException) {
		return remove(header->prev);
	}

	/**
	 * Inserts the specified element at the beginning of this list.
	 *
	 * @param e the element to add
	 */
	virtual void addFirst(int e) {
		addBefore(e, header->next);
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 */
	virtual void addLast(int e) {
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
	virtual boolean contains(int o) {
		return indexOf(o) != -1;
	}

	virtual boolean removeAll(ECollection<int> *c) {
		//TODO...
		return false;
	}

	virtual boolean retainAll(ECollection<int> *c) {
		//TODO...
		return false;
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
	virtual boolean add(int e) {
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
	virtual boolean remove(int o) {
		for (Entry<int> *e = header->next; e != header; e = e->next) {
			if (e->elem == (o)) {
				remove(e);
				return true;
			}
		}
		return false;
	}

	/**
	 * Removes all of the elements from this list.
	 */
	virtual void clear() {
		Entry<int> *e = header->next;
		while (e != header) {
			Entry<int> *next = e->next;
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
	virtual int getAt(int index) THROWS(EIndexOutOfBoundsException) {
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
	virtual int setAt(int index, int element) THROWS(EIndexOutOfBoundsException) {
        Entry<int> *e = entry(index);
        int oldVal = e->elem;
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
	virtual void addAt(int index, int element) THROWS(EIndexOutOfBoundsException) {
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
	virtual int removeAt(int index) THROWS(EIndexOutOfBoundsException) {
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
	virtual int indexOf(int o) {
		int index = 0;
		for (Entry<int> *e = header->next; e != header; e = e->next) {
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
	virtual int lastIndexOf(int o) {
		int index = listSize;
		for (Entry<int> *e = header->prev; e != header; e = e->prev) {
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
	virtual int peek() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual int element() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual int poll() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list.
	 *
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual int remove() THROWS(ENoSuchElementException) {
		return removeFirst();
	}

	/**
	 * Adds the specified element as the tail (last element) of this list.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @since 1.5
	 */
	virtual boolean offer(int e) {
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
	virtual boolean offerFirst(int e) {
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
	virtual boolean offerLast(int e) {
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
	virtual int peekFirst() {
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
	virtual int peekLast() {
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
	virtual int pollFirst() {
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
	virtual int pollLast() {
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
	virtual void push(int e) {
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
	virtual int pop() {
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
	virtual boolean removeFirstOccurrence(int o) {
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
	virtual boolean removeLastOccurrence(int o) {
		for (Entry<int> *e = header->prev; e != header; e = e->prev) {
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
	virtual EIterator<int>* iterator(int index = 0) {
		return EAbstractList<int>::iterator(index);
	}

	virtual boolean isEmpty() {
		return listSize==0;
	}

	virtual EIterator<int>* descendingIterator() {
		return null;
	}

	/**
	 * Returns a shallow copy of this {@code LinkedList}. (The elements
	 * themselves are not cloned.)
	 *
	 * @return a shallow copy of this {@code LinkedList} instance
	 */
	virtual ELinkedList<int>* clone() {
		ELinkedList<int>* ll = new ELinkedList<int>();

		// Initialize clone with our elements
		for (Entry<int>* x = header->next; x != header; x = x->next)
			ll->add(x->elem);

		return ll;
	}

private:
	template<typename _TELinkedListEntry>
	class Entry {
	public:
		int elem;
		Entry<_TELinkedListEntry> *next;
		Entry<_TELinkedListEntry> *prev;

		Entry(_TELinkedListEntry element, Entry<_TELinkedListEntry> *next,
				Entry<_TELinkedListEntry> *previous) {
			this->elem = element;
			this->next = next;
			this->prev = previous;
		}

		~Entry() {
		}
	};

	Entry<int> *header;
	int listSize;

	Entry<int>* addBefore(int e,
			Entry<int> *entry) {
		Entry<int> *newEntry = new Entry<int>(e, entry,
				entry->prev);
		newEntry->prev->next = newEntry;
		newEntry->next->prev = newEntry;
		listSize++;
		return newEntry;
	}

	int remove(Entry<int> *e) {
		if (e == header)
			throw ENoSuchElementException(__FILE__, __LINE__);

		int result = e->elem;
		e->prev->next = e->next;
		e->next->prev = e->prev;
		e->elem = 0; //!
		delete e; //!
		listSize--;
		return result;
	}

	/**
	 * Returns the indexed entry.
	 */
	Entry<int>* entry(int index) {
		if (index < 0 || index >= listSize)
			throw EIndexOutOfBoundsException(
					EString::formatOf("Index: %d , Size: %d", index, listSize).c_str(),
					__FILE__, __LINE__);
		Entry<int> *e = header;
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

template<>
class ELinkedList<llong> : public EAbstractList<llong>,
		virtual public EList<llong>,
		virtual public EDeque<llong> {
public:
	virtual ~ELinkedList() {
		clear();
		delete header;
	}

	/**
	 * Constructs an empty list.
	 */
	ELinkedList() : listSize(0) {
		header = new Entry<llong>(0, null, null);
		header->next = header->prev = header;
	}

    ELinkedList(const ELinkedList<llong>& that) : listSize(0) {
		ELinkedList<llong>* t = (ELinkedList<llong>*)&that;

		header = new Entry<llong>(0, null, null);
		header->next = header->prev = header;

		Entry<llong> *e = t->header->next;
		while (e != t->header) {
			add(e->elem);
			e = e->next;
		}
	}

    ELinkedList<llong>& operator= (const ELinkedList<llong>& that) {
    	if (this == &that) return *this;

		ELinkedList<llong>* t = (ELinkedList<llong>*)&that;

		//1.
		clear();
		delete header;

		//2.
		header = new Entry<llong>(0, null, null);
		header->next = header->prev = header;

		Entry<llong> *e = t->header->next;
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
	virtual llong getFirst() THROWS(ENoSuchElementException) {
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
	virtual llong getLast() THROWS(ENoSuchElementException) {
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
	virtual llong removeFirst() THROWS(ENoSuchElementException) {
		return remove(header->next);
	}

	/**
	 * Removes and returns the last element from this list.
	 *
	 * @return the last element from this list
	 * @throws NoSuchElementException if this list is empty
	 */
	virtual llong removeLast() THROWS(ENoSuchElementException) {
		return remove(header->prev);
	}

	/**
	 * Inserts the specified element at the beginning of this list.
	 *
	 * @param e the element to add
	 */
	virtual void addFirst(llong e) {
		addBefore(e, header->next);
	}

	/**
	 * Appends the specified element to the end of this list.
	 *
	 * <p>This method is equivalent to {@link #add}.
	 *
	 * @param e the element to add
	 */
	virtual void addLast(llong e) {
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
	virtual boolean contains(llong o) {
		return indexOf(o) != -1;
	}

	virtual boolean removeAll(ECollection<llong> *c) {
		//TODO...
		return false;
	}

	virtual boolean retainAll(ECollection<llong> *c) {
		//TODO...
		return false;
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
	virtual boolean add(llong e) {
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
	virtual boolean remove(llong o) {
		for (Entry<llong> *e = header->next; e != header; e = e->next) {
			if (e->elem == (o)) {
				remove(e);
				return true;
			}
		}
		return false;
	}

	/**
	 * Removes all of the elements from this list.
	 */
	virtual void clear() {
		Entry<llong> *e = header->next;
		while (e != header) {
			Entry<llong> *next = e->next;
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
	virtual llong getAt(int index) THROWS(EIndexOutOfBoundsException) {
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
	virtual llong setAt(int index, llong element) THROWS(EIndexOutOfBoundsException) {
        Entry<llong> *e = entry(index);
        llong oldVal = e->elem;
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
	virtual void addAt(int index, llong element) THROWS(EIndexOutOfBoundsException) {
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
	virtual llong removeAt(int index) THROWS(EIndexOutOfBoundsException) {
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
	virtual int indexOf(llong o) {
		int index = 0;
		for (Entry<llong> *e = header->next; e != header; e = e->next) {
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
	virtual int lastIndexOf(llong o) {
		int index = listSize;
		for (Entry<llong> *e = header->prev; e != header; e = e->prev) {
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
	virtual llong peek() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves, but does not remove, the head (first element) of this list.
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual llong element() THROWS(ENoSuchElementException) {
		return getFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list
	 * @return the head of this list, or <tt>null</tt> if this list is empty
	 * @since 1.5
	 */
	virtual llong poll() THROWS(ENoSuchElementException) {
		if (listSize == 0)
			throw ENoSuchElementException(__FILE__, __LINE__);
		return removeFirst();
	}

	/**
	 * Retrieves and removes the head (first element) of this list.
	 *
	 * @return the head of this list
	 * @throws NoSuchElementException if this list is empty
	 * @since 1.5
	 */
	virtual llong remove() THROWS(ENoSuchElementException) {
		return removeFirst();
	}

	/**
	 * Adds the specified element as the tail (last element) of this list.
	 *
	 * @param e the element to add
	 * @return <tt>true</tt> (as specified by {@link Queue#offer})
	 * @since 1.5
	 */
	virtual boolean offer(llong e) {
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
	virtual boolean offerFirst(llong e) {
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
	virtual boolean offerLast(llong e) {
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
	virtual llong peekFirst() {
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
	virtual llong peekLast() {
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
	virtual llong pollFirst() {
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
	virtual llong pollLast() {
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
	virtual void push(llong e) {
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
	virtual llong pop() {
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
	virtual boolean removeFirstOccurrence(llong o) {
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
	virtual boolean removeLastOccurrence(llong o) {
		for (Entry<llong> *e = header->prev; e != header; e = e->prev) {
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
	virtual EIterator<llong>* iterator(int index = 0) {
		return EAbstractList<llong>::iterator(index);
	}

	virtual boolean isEmpty() {
		return listSize==0;
	}

	virtual EIterator<llong>* descendingIterator() {
		return null;
	}

	/**
	 * Returns a shallow copy of this {@code LinkedList}. (The elements
	 * themselves are not cloned.)
	 *
	 * @return a shallow copy of this {@code LinkedList} instance
	 */
	virtual ELinkedList<llong>* clone() {
		ELinkedList<llong>* ll = new ELinkedList<llong>();

		// Initialize clone with our elements
		for (Entry<llong>* x = header->next; x != header; x = x->next)
			ll->add(x->elem);

		return ll;
	}

private:
	template<typename _TELinkedListEntry>
	class Entry {
	public:
		llong elem;
		Entry<_TELinkedListEntry> *next;
		Entry<_TELinkedListEntry> *prev;

		Entry(_TELinkedListEntry element, Entry<_TELinkedListEntry> *next,
				Entry<_TELinkedListEntry> *previous) {
			this->elem = element;
			this->next = next;
			this->prev = previous;
		}

		~Entry() {
		}
	};

	Entry<llong> *header;
	int listSize;

	Entry<llong>* addBefore(llong e,
			Entry<llong> *entry) {
		Entry<llong> *newEntry = new Entry<llong>(e, entry,
				entry->prev);
		newEntry->prev->next = newEntry;
		newEntry->next->prev = newEntry;
		listSize++;
		return newEntry;
	}

	llong remove(Entry<llong> *e) {
		if (e == header)
			throw ENoSuchElementException(__FILE__, __LINE__);

		llong result = e->elem;
		e->prev->next = e->next;
		e->next->prev = e->prev;
		e->elem = 0; //!
		delete e; //!
		listSize--;
		return result;
	}

	/**
	 * Returns the indexed entry.
	 */
	Entry<llong>* entry(int index) {
		if (index < 0 || index >= listSize)
			throw EIndexOutOfBoundsException(
					EString::formatOf("Index: %d , Size: %d", index, listSize).c_str(),
					__FILE__, __LINE__);
		Entry<llong> *e = header;
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
