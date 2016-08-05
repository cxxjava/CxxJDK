/*
 * ESelector.hh
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#ifndef ESELECTOR_HH_
#define ESELECTOR_HH_

#include "EHashSet.hh"
#include "EHashMap.hh"
#include "ESelectableChannel.hh"
#include "ESynchronizeable.hh"
#include "EAtomicBoolean.hh"
#include "EIOException.hh"
#include "EClosedSelectorException.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/src/share/classes/java/nio/channels/Selector.java

/**
 * A multiplexor of {@link SelectableChannel} objects.
 *
 * <p> A selector may be created by invoking the {@link #open open} method of
 * this class, which will use the system's default {@link
 * java.nio.channels.spi.SelectorProvider </code>selector provider<code>} to
 * create a new selector.  A selector may also be created by invoking the
 * {@link java.nio.channels.spi.SelectorProvider#openSelector openSelector}
 * method of a custom selector provider.  A selector remains open until it is
 * closed via its {@link #close close} method.
 *
 * <a name="ks">
 *
 * <p> A selectable channel's registration with a selector is represented by a
 * {@link SelectionKey} object.  A selector maintains three sets of selection
 * keys:
 *
 * <ul>
 *
 *   <li><p> The <i>key set</i> contains the keys representing the current
 *   channel registrations of this selector.  This set is returned by the
 *   {@link #keys() keys} method. </p></li>
 *
 *   <li><p> The <i>selected-key set</i> is the set of keys such that each
 *   key's channel was detected to be ready for at least one of the operations
 *   identified in the key's interest set during a prior selection operation.
 *   This set is returned by the {@link #selectedKeys() selectedKeys} method.
 *   The selected-key set is always a subset of the key set. </p></li>
 *
 *   <li><p> The <i>cancelled-key</i> set is the set of keys that have been
 *   cancelled but whose channels have not yet been deregistered.  This set is
 *   not directly accessible.  The cancelled-key set is always a subset of the
 *   key set. </p></li>
 *
 * </ul>
 *
 * <p> All three sets are empty in a newly-created selector.
 *
 * <p> A key is added to a selector's key set as a side effect of registering a
 * channel via the channel's {@link SelectableChannel#register(Selector,int)
 * register} method.  Cancelled keys are removed from the key set during
 * selection operations.  The key set itself is not directly modifiable.
 *
 * <p> A key is added to its selector's cancelled-key set when it is cancelled,
 * whether by closing its channel or by invoking its {@link SelectionKey#cancel
 * cancel} method.  Cancelling a key will cause its channel to be deregistered
 * during the next selection operation, at which time the key will removed from
 * all of the selector's key sets.
 *
 * <a name="sks"><p> Keys are added to the selected-key set by selection
 * operations.  A key may be removed directly from the selected-key set by
 * invoking the set's {@link java.util.Set#remove(java.lang.Object) remove}
 * method or by invoking the {@link java.util.Iterator#remove() remove} method
 * of an {@link java.util.Iterator </code>iterator<code>} obtained from the
 * set.  Keys are never removed from the selected-key set in any other way;
 * they are not, in particular, removed as a side effect of selection
 * operations.  Keys may not be added directly to the selected-key set. </p>
 *
 *
 * <a name="selop">
 * <h4>Selection</h4>
 *
 * <p> During each selection operation, keys may be added to and removed from a
 * selector's selected-key set and may be removed from its key and
 * cancelled-key sets.  Selection is performed by the {@link #select()}, {@link
 * #select(long)}, and {@link #selectNow()} methods, and involves three steps:
 * </p>
 *
 * <ol>
 *
 *   <li><p> Each key in the cancelled-key set is removed from each key set of
 *   which it is a member, and its channel is deregistered.  This step leaves
 *   the cancelled-key set empty. </p></li>
 *
 *   <li><p> The underlying operating system is queried for an update as to the
 *   readiness of each remaining channel to perform any of the operations
 *   identified by its key's interest set as of the moment that the selection
 *   operation began.  For a channel that is ready for at least one such
 *   operation, one of the following two actions is performed: </p>
 *
 *   <ol type=a>
 *
 *     <li><p> If the channel's key is not already in the selected-key set then
 *     it is added to that set and its ready-operation set is modified to
 *     identify exactly those operations for which the channel is now reported
 *     to be ready.  Any readiness information previously recorded in the ready
 *     set is discarded.  </p></li>
 *
 *     <li><p> Otherwise the channel's key is already in the selected-key set,
 *     so its ready-operation set is modified to identify any new operations
 *     for which the channel is reported to be ready.  Any readiness
 *     information previously recorded in the ready set is preserved; in other
 *     words, the ready set returned by the underlying system is
 *     bitwise-disjoined into the key's current ready set. </p></li>
 *
 *   </ol></li>
 *
 *   If all of the keys in the key set at the start of this step have empty
 *   interest sets then neither the selected-key set nor any of the keys'
 *   ready-operation sets will be updated.
 *
 *   <li><p> If any keys were added to the cancelled-key set while step (2) was
 *   in progress then they are processed as in step (1). </p></li>
 *
 * </ol>
 *
 * <p> Whether or not a selection operation blocks to wait for one or more
 * channels to become ready, and if so for how long, is the only essential
 * difference between the three selection methods. </p>
 *
 *
 * <h4>Concurrency</h4>
 *
 * <p> Selectors are themselves safe for use by multiple concurrent threads;
 * their key sets, however, are not.
 *
 * <p> The selection operations synchronize on the selector itself, on the key
 * set, and on the selected-key set, in that order.  They also synchronize on
 * the cancelled-key set during steps (1) and (3) above.
 *
 * <p> Changes made to the interest sets of a selector's keys while a
 * selection operation is in progress have no effect upon that operation; they
 * will be seen by the next selection operation.
 *
 * <p> Keys may be cancelled and channels may be closed at any time.  Hence the
 * presence of a key in one or more of a selector's key sets does not imply
 * that the key is valid or that its channel is open.  Application code should
 * be careful to synchronize and check these conditions as necessary if there
 * is any possibility that another thread will cancel a key or close a channel.
 *
 * <p> A thread blocked in one of the {@link #select()} or {@link
 * #select(long)} methods may be interrupted by some other thread in one of
 * three ways:
 *
 * <ul>
 *
 *   <li><p> By invoking the selector's {@link #wakeup wakeup} method,
 *   </p></li>
 *
 *   <li><p> By invoking the selector's {@link #close close} method, or
 *   </p></li>
 *
 *   <li><p> By invoking the blocked thread's {@link
 *   java.lang.Thread#interrupt() interrupt} method, in which case its
 *   interrupt status will be set and the selector's {@link #wakeup wakeup}
 *   method will be invoked. </p></li>
 *
 * </ul>
 *
 * <p> The {@link #close close} method synchronizes on the selector and all
 * three key sets in the same order as in a selection operation.
 *
 * <a name="ksc">
 *
 * <p> A selector's key and selected-key sets are not, in general, safe for use
 * by multiple concurrent threads.  If such a thread might modify one of these
 * sets directly then access should be controlled by synchronizing on the set
 * itself.  The iterators returned by these sets' {@link
 * java.util.Set#iterator() iterator} methods are <i>fail-fast:</i> If the set
 * is modified after the iterator is created, in any way except by invoking the
 * iterator's own {@link java.util.Iterator#remove() remove} method, then a
 * {@link java.util.ConcurrentModificationException} will be thrown. </p>
 *
 *
 * @since 1.4
 *
 * @see SelectableChannel
 * @see SelectionKey
 */


//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
//@see: openjdk-8/src/share/classes/sun/nio/ch/SelectorImpl.java

/**
 * Base implementation class for selectors.
 *
 * <p> This class encapsulates the low-level machinery required to implement
 * the interruption of selection operations.  A concrete selector class must
 * invoke the {@link #begin begin} and {@link #end end} methods before and
 * after, respectively, invoking an I/O operation that might block
 * indefinitely.  In order to ensure that the {@link #end end} method is always
 * invoked, these methods should be used within a
 * <tt>try</tt>&nbsp;...&nbsp;<tt>finally</tt> block:
 *
 * <blockquote><pre>
 * try {
 *     begin();
 *     // Perform blocking I/O operation here
 *     ...
 * } finally {
 *     end();
 * }</pre></blockquote>
 *
 * <p> This class also defines methods for maintaining a selector's
 * cancelled-key set and for removing a key from its channel's key set, and
 * declares the abstract {@link #register register} method that is invoked by a
 * selectable channel's {@link AbstractSelectableChannel#register register}
 * method in order to perform the actual work of registering a channel.  </p>
 *
 *
 * @since 1.4
 */

class ESelector : public ESynchronizeable {
public:
	virtual ~ESelector();

	/**
	 * Opens a selector.
	 *
	 * <p> The new selector is created by invoking the {@link
	 * java.nio.channels.spi.SelectorProvider#openSelector openSelector} method
	 * of the system-wide default {@link
	 * java.nio.channels.spi.SelectorProvider} object.  </p>
	 *
	 * @return  A new selector
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 */
	static ESelector* open() THROWS(EIOException);

    /**
     * Tells whether or not this selector is open.  </p>
     *
     * @return <tt>true</tt> if, and only if, this selector is open
     */
    boolean isOpen();

    /**
	 * Closes this selector.
	 *
	 * <p> If a thread is currently blocked in one of this selector's selection
	 * methods then it is interrupted as if by invoking the selector's {@link
	 * #wakeup wakeup} method.
	 *
	 * <p> Any uncancelled keys still associated with this selector are
	 * invalidated, their channels are deregistered, and any other resources
	 * associated with this selector are released.
	 *
	 * <p> If this selector is already closed then invoking this method has no
	 * effect.
	 *
	 * <p> After a selector is closed, any further attempt to use it, except by
	 * invoking this method or the {@link #wakeup wakeup} method, will cause a
	 * {@link ClosedSelectorException} to be thrown. </p>
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 */
	void close() THROWS(EIOException);

	/**
	 * Returns this selector's key set.
	 *
	 * <p> The key set is not directly modifiable.  A key is removed only after
	 * it has been cancelled and its channel has been deregistered.  Any
	 * attempt to modify the key set will cause an {@link
	 * UnsupportedOperationException} to be thrown.
	 *
	 * <p> The key set is <a href="#ksc">not thread-safe</a>. </p>
	 *
	 * @return  This selector's key set
	 *
	 * @throws  ClosedSelectorException
	 *          If this selector is closed
	 */
	ESet<ESelectionKey*>* keys();

	/**
	 * Returns this selector's selected-key set.
	 *
	 * <p> Keys may be removed from, but not directly added to, the
	 * selected-key set.  Any attempt to add an object to the key set will
	 * cause an {@link UnsupportedOperationException} to be thrown.
	 *
	 * <p> The selected-key set is <a href="#ksc">not thread-safe</a>. </p>
	 *
	 * @return  This selector's selected-key set
	 *
	 * @throws  ClosedSelectorException
	 *          If this selector is closed
	 */
	ESet<ESelectionKey*>* selectedKeys();

	/**
	 * Selects a set of keys whose corresponding channels are ready for I/O
	 * operations.
	 *
	 * <p> This method performs a blocking <a href="#selop">selection
	 * operation</a>.  It returns only after at least one channel is selected,
	 * this selector's {@link #wakeup wakeup} method is invoked, the current
	 * thread is interrupted, or the given timeout period expires, whichever
	 * comes first.
	 *
	 * <p> This method does not offer real-time guarantees: It schedules the
	 * timeout as if by invoking the {@link Object#wait(long)} method. </p>
	 *
	 * @param  timeout  If positive, block for up to <tt>timeout</tt>
	 *                  milliseconds, more or less, while waiting for a
	 *                  channel to become ready; if zero, block indefinitely;
	 *                  must not be negative
	 *
	 * @return  The number of keys, possibly zero,
	 *          whose ready-operation sets were updated
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 *
	 * @throws  ClosedSelectorException
	 *          If this selector is closed
	 *
	 * @throws  IllegalArgumentException
	 *          If the value of the timeout argument is negative
	 */
	int select(llong timeout=0) THROWS(EIOException);

	/**
	 * Selects a set of keys whose corresponding channels are ready for I/O
	 * operations.
	 *
	 * <p> This method performs a non-blocking <a href="#selop">selection
	 * operation</a>.  If no channels have become selectable since the previous
	 * selection operation then this method immediately returns zero.
	 *
	 * <p> Invoking this method clears the effect of any previous invocations
	 * of the {@link #wakeup wakeup} method.  </p>
	 *
	 * @return  The number of keys, possibly zero, whose ready-operation sets
	 *          were updated by the selection operation
	 *
	 * @throws  IOException
	 *          If an I/O error occurs
	 *
	 * @throws  ClosedSelectorException
	 *          If this selector is closed
	 */
	int selectNow() THROWS(EIOException);

	/**
	 * Causes the first selection operation that has not yet returned to return
	 * immediately.
	 *
	 * <p> If another thread is currently blocked in an invocation of the
	 * {@link #select()} or {@link #select(long)} methods then that invocation
	 * will return immediately.  If no selection operation is currently in
	 * progress then the next invocation of one of these methods will return
	 * immediately unless the {@link #selectNow()} method is invoked in the
	 * meantime.  In any case the value returned by that invocation may be
	 * non-zero.  Subsequent invocations of the {@link #select()} or {@link
	 * #select(long)} methods will block as usual unless this method is invoked
	 * again in the meantime.
	 *
	 * <p> Invoking this method more than once between two successive selection
	 * operations has the same effect as invoking it just once.  </p>
	 *
	 * @return  This selector
	 */
	virtual ESelector* wakeup() = 0;

	/**
	 *
	 */
	virtual void putEventOps(ESelectionKey* sk, int ops);

protected:
	friend class ESelectableChannel;
	friend class ESocketChannel;
	friend class EServerSocketChannel;
	friend class ESelectionKey;

	/**
	 * Retrieves this selector's cancelled-key set.
	 *
	 * <p> This set should only be used while synchronized upon it.  </p>
	 *
	 * @return  The cancelled-key set
	 */
	ESet<ESelectionKey*>* cancelledKeys();

	/**
	 * Registers the given channel with this selector.
	 *
	 * <p> This method is invoked by a channel's {@link
	 * AbstractSelectableChannel#register register} method in order to perform
	 * the actual work of registering the channel with this selector.  </p>
	 *
	 * @param  ch
	 *         The channel to be registered
	 *
	 * @param  ops
	 *         The initial interest set, which must be valid
	 *
	 * @param  att
	 *         The initial attachment for the resulting key
	 *
	 * @return  A new key representing the registration of the given channel
	 *          with this selector
	 */
	ESelectionKey* register_(ESelectableChannel* ch, int ops, void* att);

	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
	void cancel(ESelectionKey* k);

	//@see: openjdk-8/jdk/src/share/classes/sun/nio/ch/SelectorImpl.java
    void processDeregisterQueue() THROWS(EIOException);

	/**
	 * Remove a channel's file descriptor from epoll
	 */
	void release(ESelectableChannel* channel);

	/**
	 * Removes the given key from its channel's key set.
	 *
	 * <p> This method must be invoked by the selector for each channel that it
	 * deregisters.  </p>
	 *
	 * @param  key
	 *         The selection key to be removed
	 */
	void deregister(ESelectionKey* key);

	/**
	 * Marks the beginning of an I/O operation that might block indefinitely.
	 *
	 * <p> This method should be invoked in tandem with the {@link #end end}
	 * method, using a <tt>try</tt>&nbsp;...&nbsp;<tt>finally</tt> block as
	 * shown <a href="#be">above</a>, in order to implement interruption for
	 * this selector.
	 *
	 * <p> Invoking this method arranges for the selector's {@link
	 * Selector#wakeup wakeup} method to be invoked if a thread's {@link
	 * Thread#interrupt interrupt} method is invoked while the thread is
	 * blocked in an I/O operation upon the selector.  </p>
	 */
	void begin();

	/**
	 * Marks the end of an I/O operation that might block indefinitely.
	 *
	 * <p> This method should be invoked in tandem with the {@link #begin begin}
	 * method, using a <tt>try</tt>&nbsp;...&nbsp;<tt>finally</tt> block as
	 * shown <a href="#be">above</a>, in order to implement interruption for
	 * this selector.  </p>
	 */
	void end();

protected:
	// -- Interruption machinery --
	EInterruptible* interruptor_;// = null;

	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
    EAtomicBoolean selectorOpen_;

    // Public views of the key sets
    EHashSet<ESelectionKey*>* keys_; // = new HashSet();  // Immutable
	ELock* keysLock_;

	// Reference views of the key sets
	EHashSet<ESelectionKey*>* selectedKeys_; // Removal allowed, but not addition
	ELock* selectedKeysLock_;

	//@see: openjdk-8/src/share/classes/java/nio/channels/spi/AbstractSelector.java
    EHashSet<ESelectionKey*>* cancelledKeys_;// = new HashSet();
    ELock* cancelledKeysLock_;

 	/**
	 * Initializes a new instance of this class.
	 */
    ESelector();

	//@see; openjdk-8/src/share/classes/sun/nio/ch/SelectorImpl.java
	virtual void implRegister(ESelectionKey* ski) = 0;
	virtual void implDereg(ESelectionKey* ski) THROWS(EIOException) = 0;
	virtual void implClose() THROWS(EIOException) = 0;
	virtual int doSelect(llong timeout) THROWS(EIOException) = 0;

private:
	int lockAndDoSelect(llong timeout) THROWS(EIOException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* ESELECTOR_HH_ */
