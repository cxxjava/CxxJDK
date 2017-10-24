/*
 * ESelectionKey.hh
 *
 *  Created on: 2013-9-4
 *      Author: cxxjava@163.com
 */

#ifndef ESELECTIONKEY_HH_
#define ESELECTIONKEY_HH_

#include "./ESelector.hh"
#include "./ESelectableChannel.hh"
#include "./ECancelledKeyException.hh"
#include "../../inc/ESynchronizeable.hh"
#include "../../inc/concurrent/EAtomicReference.hh"

namespace efc {
namespace nio {

/**
 * A token representing the registration of a {@link SelectableChannel} with a
 * {@link Selector}.
 *
 * <p> A selection key is created each time a channel is registered with a
 * selector.  A key remains valid until it is <i>cancelled</i> by invoking its
 * {@link #cancel cancel} method, by closing its channel, or by closing its
 * selector.  Cancelling a key does not immediately remove it from its
 * selector; it is instead added to the selector's <a
 * href="Selector.html#ks"><i>cancelled-key set</i></a> for removal during the
 * next selection operation.  The validity of a key may be tested by invoking
 * its {@link #isValid isValid} method.
 *
 * <a name="opsets">
 *
 * <p> A selection key contains two <i>operation sets</i> represented as
 * integer values.  Each bit of an operation set denotes a category of
 * selectable operations that are supported by the key's channel.
 *
 * <ul>
 *
 *   <li><p> The <i>interest set</i> determines which operation categories will
 *   be tested for readiness the next time one of the selector's selection
 *   methods is invoked.  The interest set is initialized with the value given
 *   when the key is created; it may later be changed via the {@link
 *   #interestOps(int)} method. </p></li>
 *
 *   <li><p> The <i>ready set</i> identifies the operation categories for which
 *   the key's channel has been detected to be ready by the key's selector.
 *   The ready set is initialized to zero when the key is created; it may later
 *   be updated by the selector during a selection operation, but it cannot be
 *   updated directly. </p></li>
 *
 * </ul>
 *
 * <p> That a selection key's ready set indicates that its channel is ready for
 * some operation category is a hint, but not a guarantee, that an operation in
 * such a category may be performed by a thread without causing the thread to
 * block.  A ready set is most likely to be accurate immediately after the
 * completion of a selection operation.  It is likely to be made inaccurate by
 * external events and by I/O operations that are invoked upon the
 * corresponding channel.
 *
 * <p> This class defines all known operation-set bits, but precisely which
 * bits are supported by a given channel depends upon the type of the channel.
 * Each subclass of {@link SelectableChannel} defines an {@link
 * SelectableChannel#validOps() validOps()} method which returns a set
 * identifying just those operations that are supported by the channel.  An
 * attempt to set or test an operation-set bit that is not supported by a key's
 * channel will result in an appropriate run-time exception.
 *
 * <p> It is often necessary to associate some application-specific data with a
 * selection key, for example an object that represents the state of a
 * higher-level protocol and handles readiness notifications in order to
 * implement that protocol.  Selection keys therefore support the
 * <i>attachment</i> of a single arbitrary object to a key.  An object can be
 * attached via the {@link #attach attach} method and then later retrieved via
 * the {@link #attachment() attachment} method.
 *
 * <p> Selection keys are safe for use by multiple concurrent threads.  The
 * operations of reading and writing the interest set will, in general, be
 * synchronized with certain operations of the selector.  Exactly how this
 * synchronization is performed is implementation-dependent: In a naive
 * implementation, reading or writing the interest set may block indefinitely
 * if a selection operation is already in progress; in a high-performance
 * implementation, reading or writing the interest set may block briefly, if at
 * all.  In any case, a selection operation will always use the interest-set
 * value that was current at the moment that the operation began.  </p>
 *
 *
 * @version 1.26, 06/06/28
 * @since 1.4
 *
 * @see SelectableChannel
 * @see Selector
 */

class ESelectionKey : public ESynchronizeable {
public:
	virtual ~ESelectionKey();

	// -- Channel and selector operations --

	/**
	 * Returns the channel for which this key was created.  This method will
	 * continue to return the channel even after the key is cancelled.  </p>
	 *
	 * @return  This key's channel
	 */
	ESelectableChannel* channel();

	/**
	 * Returns the selector for which this key was created.  This method will
	 * continue to return the selector even after the key is cancelled.  </p>
	 *
	 * @return  This key's selector
	 */
	ESelector* selector();

	/**
	 * Tells whether or not this key is valid.
	 *
	 * <p> A key is valid upon creation and remains so until it is cancelled,
	 * its channel is closed, or its selector is closed.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if, this key is valid
	 */
	boolean isValid();

	/**
	 * Requests that the registration of this key's channel with its selector
	 * be cancelled.  Upon return the key will be invalid and will have been
	 * added to its selector's cancelled-key set.  The key will be removed from
	 * all of the selector's key sets during the next selection operation.
	 *
	 * <p> If this key has already been cancelled then invoking this method has
	 * no effect.  Once cancelled, a key remains forever invalid. </p>
	 *
	 * <p> This method may be invoked at any time.  It synchronizes on the
	 * selector's cancelled-key set, and therefore may block briefly if invoked
	 * concurrently with a cancellation or selection operation involving the
	 * same selector.  </p>
	 */
	void cancel();

	// -- Operation-set accessors --

	/**
	 * Retrieves this key's interest set.
	 *
	 * <p> It is guaranteed that the returned set will only contain operation
	 * bits that are valid for this key's channel.
	 *
	 * <p> This method may be invoked at any time.  Whether or not it blocks,
	 * and for how long, is implementation-dependent.  </p>
	 *
	 * @return  This key's interest set
	 *
	 * @throws  CancelledKeyException
	 *          If this key has been cancelled
	 */
	int interestOps();

	/**
	 * Sets this key's interest set to the given value.
	 *
	 * <p> This method may be invoked at any time.  Whether or not it blocks,
	 * and for how long, is implementation-dependent.  </p>
	 *
	 * @param  ops  The new interest set
	 *
	 * @return  This selection key
	 *
	 * @throws  IllegalArgumentException
	 *          If a bit in the set does not correspond to an operation that
	 *          is supported by this key's channel, that is, if
	 *          <tt>set & ~(channel().validOps()) != 0</tt>
	 *
	 * @throws  CancelledKeyException
	 *          If this key has been cancelled
	 */
	ESelectionKey* interestOps(int ops);

	/**
	 * Retrieves this key's ready-operation set.
	 *
	 * <p> It is guaranteed that the returned set will only contain operation
	 * bits that are valid for this key's channel.  </p>
	 *
	 * @return  This key's ready-operation set
	 *
	 * @throws  CancelledKeyException
	 *          If this key has been cancelled
	 */
	int readyOps();

	// -- Operation bits and bit-testing convenience methods --

	/**
	 * Operation-set bit for read operations.
	 *
	 * <p> Suppose that a selection key's interest set contains
	 * <tt>OP_READ</tt> at the start of a <a
	 * href="Selector.html#selop">selection operation</a>.  If the selector
	 * detects that the corresponding channel is ready for reading, has reached
	 * end-of-stream, has been remotely shut down for further reading, or has
	 * an error pending, then it will add <tt>OP_READ</tt> to the key's
	 * ready-operation set and add the key to its selected-key&nbsp;set.  </p>
	 */
	static const int OP_READ = 1 << 0;

	/**
	 * Operation-set bit for write operations.  </p>
	 *
	 * <p> Suppose that a selection key's interest set contains
	 * <tt>OP_WRITE</tt> at the start of a <a
	 * href="Selector.html#selop">selection operation</a>.  If the selector
	 * detects that the corresponding channel is ready for writing, has been
	 * remotely shut down for further writing, or has an error pending, then it
	 * will add <tt>OP_WRITE</tt> to the key's ready set and add the key to its
	 * selected-key&nbsp;set.  </p>
	 */
	static const int OP_WRITE = 1 << 2;

	/**
	 * Operation-set bit for socket-connect operations.  </p>
	 *
	 * <p> Suppose that a selection key's interest set contains
	 * <tt>OP_CONNECT</tt> at the start of a <a
	 * href="Selector.html#selop">selection operation</a>.  If the selector
	 * detects that the corresponding socket channel is ready to complete its
	 * connection sequence, or has an error pending, then it will add
	 * <tt>OP_CONNECT</tt> to the key's ready set and add the key to its
	 * selected-key&nbsp;set.  </p>
	 */
	static const int OP_CONNECT = 1 << 3;

	/**
	 * Operation-set bit for socket-accept operations.  </p>
	 *
	 * <p> Suppose that a selection key's interest set contains
	 * <tt>OP_ACCEPT</tt> at the start of a <a
	 * href="Selector.html#selop">selection operation</a>.  If the selector
	 * detects that the corresponding server-socket channel is ready to accept
	 * another connection, or has an error pending, then it will add
	 * <tt>OP_ACCEPT</tt> to the key's ready set and add the key to its
	 * selected-key&nbsp;set.  </p>
	 */
	static const int OP_ACCEPT = 1 << 4;

	/**
	 * Tests whether this key's channel is ready for reading.
	 *
	 * <p> An invocation of this method of the form <tt>k.isReadable()</tt>
	 * behaves in exactly the same way as the expression
	 *
	 * <blockquote><pre>
	 * k.readyOps()&nbsp;&amp;&nbsp;OP_READ&nbsp;!=&nbsp;0</pre></blockquote>
	 *
	 * <p> If this key's channel does not support read operations then this
	 * method always returns <tt>false</tt>.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if,
	 *          <tt>readyOps()</tt>&nbsp;<tt>&</tt>&nbsp;<tt>OP_READ</tt> is
	 *          nonzero
	 *
	 * @throws  CancelledKeyException
	 *          If this key has been cancelled
	 */
	boolean isReadable();

	/**
	 * Tests whether this key's channel is ready for writing.
	 *
	 * <p> An invocation of this method of the form <tt>k.isWritable()</tt>
	 * behaves in exactly the same way as the expression
	 *
	 * <blockquote><pre>
	 * k.readyOps()&nbsp;&amp;&nbsp;OP_WRITE&nbsp;!=&nbsp;0</pre></blockquote>
	 *
	 * <p> If this key's channel does not support write operations then this
	 * method always returns <tt>false</tt>.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if,
	 *          <tt>readyOps()</tt>&nbsp;<tt>&</tt>&nbsp;<tt>OP_WRITE</tt>
	 *          is nonzero
	 *
	 * @throws  CancelledKeyException
	 *          If this key has been cancelled
	 */
	boolean isWritable();

	/**
	 * Tests whether this key's channel has either finished, or failed to
	 * finish, its socket-connection operation.
	 *
	 * <p> An invocation of this method of the form <tt>k.isConnectable()</tt>
	 * behaves in exactly the same way as the expression
	 *
	 * <blockquote><pre>
	 * k.readyOps()&nbsp;&amp;&nbsp;OP_CONNECT&nbsp;!=&nbsp;0</pre></blockquote>
	 *
	 * <p> If this key's channel does not support socket-connect operations
	 * then this method always returns <tt>false</tt>.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if,
	 *          <tt>readyOps()</tt>&nbsp;<tt>&</tt>&nbsp;<tt>OP_CONNECT</tt>
	 *          is nonzero
	 *
	 * @throws  CancelledKeyException
	 *          If this key has been cancelled
	 */
	boolean isConnectable();

	/**
	 * Tests whether this key's channel is ready to accept a new socket
	 * connection.
	 *
	 * <p> An invocation of this method of the form <tt>k.isAcceptable()</tt>
	 * behaves in exactly the same way as the expression
	 *
	 * <blockquote><pre>
	 * k.readyOps()&nbsp;&amp;&nbsp;OP_ACCEPT&nbsp;!=&nbsp;0</pre></blockquote>
	 *
	 * <p> If this key's channel does not support socket-accept operations then
	 * this method always returns <tt>false</tt>.  </p>
	 *
	 * @return  <tt>true</tt> if, and only if,
	 *          <tt>readyOps()</tt>&nbsp;<tt>&</tt>&nbsp;<tt>OP_ACCEPT</tt>
	 *          is nonzero
	 *
	 * @throws  CancelledKeyException
	 *          If this key has been cancelled
	 */
	boolean isAcceptable();

	// -- Attachments --

	/**
	 * Attaches the given object to this key.
	 *
	 * <p> An attached object may later be retrieved via the {@link #attachment()
	 * attachment} method.  Only one object may be attached at a time; invoking
	 * this method causes any previous attachment to be discarded.  The current
	 * attachment may be discarded by attaching <tt>null</tt>.  </p>
	 *
	 * @param  ob
	 *         The object to be attached; may be <tt>null</tt>
	 *
	 * @return  The previously-attached object, if any,
	 *          otherwise <tt>null</tt>
	 */
	void* attach(void* ob);

	/**
	 * Retrieves the current attachment.  </p>
	 *
	 * @return  The object currently attached to this key,
	 *          or <tt>null</tt> if there is no attachment
	 */
	void* attachment();

public:
	/**
	 * package-private
	 */
	int getIndex();
	void setIndex(int i);
	void nioReadyOps(int ops);
	int nioReadyOps();
	ESelectionKey* nioInterestOps(int ops);
	int nioInterestOps();
	void invalidate();

protected:
	friend class ESelector;

	ESelectionKey(ESelectableChannel* ch, ESelector* sel);

private:
	ESelectableChannel* channel_;
	ESelector* selector_;
	EAtomicReference<void> attachment_;
	volatile boolean valid_;// = true;
	volatile int interestOps_;
	int readyOps_;

	// Index for a pollfd array in Selector that this key is registered with
	int index_;

	boolean binded_;

	void ensureValid();
};

} /* namespace nio */
} /* namespace efc */
#endif /* ESELECTIONKEY_HH_ */
