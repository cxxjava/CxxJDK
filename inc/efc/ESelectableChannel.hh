/*
 * ESelectableChannel.hh
 *
 *  Created on: 2013-12-16
 *      Author: cxxjava@163.com
 */

#ifndef ESELECTABLECHANNEL_HH_
#define ESELECTABLECHANNEL_HH_

#include "ELock.hh"
#include "EArray.hh"
#include "EChannel.hh"
#include "EReentrantLock.hh"
#include "EIOException.hh"
#include "EClosedChannelException.hh"
#include "EIllegalArgumentException.hh"
#include "EIllegalBlockingModeException.hh"

namespace efc {
namespace nio {

class ESelector;
class ESelectionKey;

/**
 * A channel that can be multiplexed via a {@link Selector}.
 *
 * <p> In order to be used with a selector, an instance of this class must
 * first be <i>registered</i> via the {@link #register(Selector,int,Object)
 * register} method.  This method returns a new {@link SelectionKey} object
 * that represents the channel's registration with the selector.
 *
 * <p> Once registered with a selector, a channel remains registered until it
 * is <i>deregistered</i>.  This involves deallocating whatever resources were
 * allocated to the channel by the selector.
 *
 * <p> A channel cannot be deregistered directly; instead, the key representing
 * its registration must be <i>cancelled</i>.  Cancelling a key requests that
 * the channel be deregistered during the selector's next selection operation.
 * A key may be cancelled explicitly by invoking its {@link
 * SelectionKey#cancel() cancel} method.  All of a channel's keys are cancelled
 * implicitly when the channel is closed, whether by invoking its {@link
 * Channel#close close} method or by interrupting a thread blocked in an I/O
 * operation upon the channel.
 *
 * <p> If the selector itself is closed then the channel will be deregistered,
 * and the key representing its registration will be invalidated, without
 * further delay.
 *
 * <p> A channel may be registered at most once with any particular selector.
 *
 * <p> Whether or not a channel is registered with one or more selectors may be
 * determined by invoking the {@link #isRegistered isRegistered} method.
 *
 * <p> Selectable channels are safe for use by multiple concurrent
 * threads. </p>
 *
 *
 * <a name="bm">
 * <h4>Blocking mode</h4>
 *
 * A selectable channel is either in <i>blocking</i> mode or in
 * <i>non-blocking</i> mode.  In blocking mode, every I/O operation invoked
 * upon the channel will block until it completes.  In non-blocking mode an I/O
 * operation will never block and may transfer fewer bytes than were requested
 * or possibly no bytes at all.  The blocking mode of a selectable channel may
 * be determined by invoking its {@link #isBlocking isBlocking} method.
 *
 * <p> Newly-created selectable channels are always in blocking mode.
 * Non-blocking mode is most useful in conjunction with selector-based
 * multiplexing.  A channel must be placed into non-blocking mode before being
 * registered with a selector, and may not be returned to blocking mode until
 * it has been deregistered.
 *
 *
 * @author Mark Reinhold
 * @author JSR-51 Expert Group
 * @version 1.35, 05/11/17
 * @since 1.4
 *
 * @see SelectionKey
 * @see Selector
 */

abstract class ESelectableChannel : virtual public EChannel {
public:
	virtual ~ESelectableChannel();

	/**
	 * Tells whether or not this channel is open.  </p>
	 *
	 * @return <tt>true</tt> if, and only if, this channel is open
	 */
	virtual boolean isOpen();

	/**
	 * Closes this channel.
	 *
	 * <p> After a channel is closed, any further attempt to invoke I/O
	 * operations upon it will cause a {@link ClosedChannelException} to be
	 * thrown.
	 *
	 * <p> If this channel is already closed then invoking this method has no
	 * effect.
	 *
	 * <p> This method may be invoked at any time.  If some other thread has
	 * already invoked it, however, then another invocation will block until
	 * the first invocation is complete, after which it will return without
	 * effect. </p>
	 *
	 * @throws  IOException  If an I/O error occurs
	 */
	virtual void close() THROWS(EIOException);

	/**
	 * Tells whether or not this channel is currently registered with any
	 * selectors.  A newly-created channel is not registered.
	 *
	 * <p> Due to the inherent delay between key cancellation and channel
	 * deregistration, a channel may remain registered for some time after all
	 * of its keys have been cancelled.  A channel may also remain registered
	 * for some time after it is closed.  </p>
	 *
	 * @return <tt>true</tt> if, and only if, this channel is registered
	 */
	boolean isRegistered();
	//
	// sync(keySet) { return isRegistered; }

	/**
	 * Retrieves the key representing the channel's registration with the given
	 * selector.  </p>
	 *
	 * @return  The key returned when this channel was last registered with the
	 *          given selector, or <tt>null</tt> if this channel is not
	 *          currently registered with that selector
	 */
	ESelectionKey* keyFor(ESelector* sel);
	//
	// sync(keySet) { return findKey(sel); }

	/**
	 * Registers this channel with the given selector, returning a selection
	 * key.
	 *
	 * <p> If this channel is currently registered with the given selector then
	 * the selection key representing that registration is returned.  The key's
	 * interest set will have been changed to <tt>ops</tt>, as if by invoking
	 * the {@link SelectionKey#interestOps(int) interestOps(int)} method.  If
	 * the <tt>att</tt> argument is not <tt>null</tt> then the key's attachment
	 * will have been set to that value.  A {@link CancelledKeyException} will
	 * be thrown if the key has already been cancelled.
	 *
	 * <p> Otherwise this channel has not yet been registered with the given
	 * selector, so it is registered and the resulting new key is returned.
	 * The key's initial interest set will be <tt>ops</tt> and its attachment
	 * will be <tt>att</tt>.
	 *
	 * <p> This method may be invoked at any time.  If this method is invoked
	 * while another invocation of this method or of the {@link
	 * #configureBlocking(boolean) configureBlocking} method is in progress
	 * then it will first block until the other operation is complete.  This
	 * method will then synchronize on the selector's key set and therefore may
	 * block if invoked concurrently with another registration or selection
	 * operation involving the same selector. </p>
	 *
	 * <p> If this channel is closed while this operation is in progress then
	 * the key returned by this method will have been cancelled and will
	 * therefore be invalid. </p>
	 *
	 * @param  sel
	 *         The selector with which this channel is to be registered
	 *
	 * @param  ops
	 *         The interest set for the resulting key
	 *
	 * @param  att
	 *         The attachment for the resulting key; may be <tt>null</tt>
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  IllegalBlockingModeException
	 *          If this channel is in blocking mode
	 *
	 * @throws  IllegalSelectorException
	 *          If this channel was not created by the same provider
	 *          as the given selector
	 *
	 * @throws  CancelledKeyException
	 *          If this channel is currently registered with the given selector
	 *          but the corresponding key has already been cancelled
	 *
	 * @throws  IllegalArgumentException
	 *          If a bit in the <tt>ops</tt> set does not correspond to an
	 *          operation that is supported by this channel, that is, if
	 *          <tt>set & ~validOps() != 0</tt>
	 *
	 * @return  A key representing the registration of this channel with
	 *          the given selector
	 */
	ESelectionKey* register_(ESelector* sel, int ops,
			void* att = null)
					THROWS3(EClosedChannelException, EIllegalArgumentException, EIllegalBlockingModeException);
	//
	// sync(regLock) {
	//   sync(keySet) { look for selector }
	//   if (channel found) { set interest ops -- may block in selector;
	//                        return key; }
	//   create new key -- may block somewhere in selector;
	//   sync(keySet) { add key; }
	//   attach(attachment);
	//   return key;
	// }

	/**
	 * Adjusts this channel's blocking mode.
	 *
	 * <p> If this channel is registered with one or more selectors then an
	 * attempt to place it into blocking mode will cause an {@link
	 * IllegalBlockingModeException} to be thrown.
	 *
	 * <p> This method may be invoked at any time.  The new blocking mode will
	 * only affect I/O operations that are initiated after this method returns.
	 * For some implementations this may require blocking until all pending I/O
	 * operations are complete.
	 *
	 * <p> If this method is invoked while another invocation of this method or
	 * of the {@link #register(Selector, int) register} method is in progress
	 * then it will first block until the other operation is complete. </p>
	 *
	 * @param  block  If <tt>true</tt> then this channel will be placed in
	 *                blocking mode; if <tt>false</tt> then it will be placed
	 *                non-blocking mode
	 *
	 * @return  This selectable channel
	 *
	 * @throws  ClosedChannelException
	 *          If this channel is closed
	 *
	 * @throws  IllegalBlockingModeException
	 *          If <tt>block</tt> is <tt>true</tt> and this channel is
	 *          registered with one or more selectors
	 *
	 * @throws IOException
	 *         If an I/O error occurs
	 */
	ESelectableChannel* configureBlocking(boolean block) THROWS(EIOException);
	//
	// sync(regLock) {
	//   sync(keySet) { throw IBME if block && isRegistered; }
	//   change mode;
	// }

	/**
	 * Tells whether or not every I/O operation on this channel will block
	 * until it completes.  A newly-created channel is always in blocking mode.
	 *
	 * <p> If this channel is closed then the value returned by this method is
	 * not specified. </p>
	 *
	 * @return <tt>true</tt> if, and only if, this channel is in blocking mode
	 */
	boolean isBlocking();

	/**
	 * Retrieves the object upon which the {@link #configureBlocking
	 * configureBlocking} and {@link #register register} methods synchronize.
	 * This is often useful in the implementation of adaptors that require a
	 * specific blocking mode to be maintained for a short period of time.
	 * </p>
	 *
	 * @return  The blocking-mode lock object
	 */
	ELock* blockingLock();

	//@see AbstractSelectableChannel.java

	void removeKey(ESelectionKey* k);


	//@see SelChImpl.java

	/**
	 * Returns an <a href="SelectionKey.html#opsets">operation set</a>
	 * identifying this channel's supported operations.  The bits that are set
	 * in this integer value denote exactly the operations that are valid for
	 * this channel.  This method always returns the same value for a given
	 * concrete channel class. </p>
	 *
	 * @return  The valid-operation set
	 */
	virtual int validOps() = 0;

	// Internal state:
	//   keySet, may be empty but is never null, typ. a tiny array
	//   boolean isRegistered, protected by key set
	//   regLock, lock object to prevent duplicate registrations
	//   boolean isBlocking, protected by regLock

	/**
	 * Adds the specified ops if present in interestOps. The specified
	 * ops are turned on without affecting the other ops.
	 *
	 * @return  true iff the new value of sk.readyOps() set by this method
	 *          contains at least one bit that the previous value did not
	 *          contain
	 */
	virtual boolean translateAndUpdateReadyOps(int ops, ESelectionKey* sk) = 0;

	/**
	 * Sets the specified ops if present in interestOps. The specified
	 * ops are turned on, and all other ops are turned off.
	 *
	 * @return  true iff the new value of sk.readyOps() set by this method
	 *          contains at least one bit that the previous value did not
	 *          contain
	 */
	virtual boolean translateAndSetReadyOps(int ops, ESelectionKey* sk) = 0;

	virtual void translateAndSetInterestOps(int ops, ESelectionKey* sk) = 0;

	virtual void kill() THROWS(EIOException) = 0;
	
	virtual int getFDVal() = 0;


protected:
	ESelectableChannel();

	/**
	 * Marks the beginning of an I/O operation that might block indefinitely.
	 *
	 * <p> This method should be invoked in tandem with the {@link #end end}
	 * method, using a <tt>try</tt>&nbsp;...&nbsp;<tt>finally</tt> block as
	 * shown <a href="#be">above</a>, in order to implement asynchronous
	 * closing and interruption for this channel.  </p>
	 */
	void begin();

	/**
	 * Marks the end of an I/O operation that might block indefinitely.
	 *
	 * <p> This method should be invoked in tandem with the {@link #begin
	 * begin} method, using a <tt>try</tt>&nbsp;...&nbsp;<tt>finally</tt> block
	 * as shown <a href="#be">above</a>, in order to implement asynchronous
	 * closing and interruption for this channel.  </p>
	 *
	 * @param  completed
	 *         <tt>true</tt> if, and only if, the I/O operation completed
	 *         successfully, that is, had some effect that would be visible to
	 *         the operation's invoker
	 *
	 * @throws  AsynchronousCloseException
	 *          If the channel was asynchronously closed
	 *
	 * @throws  ClosedByInterruptException
	 *          If the thread blocked in the I/O operation was interrupted
	 */
	void end(boolean completed) THROWS(EAsynchronousCloseException);

	/**
	 * Closes this channel.
	 *
	 * <p> This method is invoked by the {@link #close close} method in order
	 * to perform the actual work of closing the channel.  This method is only
	 * invoked if the channel has not yet been closed, and it is never invoked
	 * more than once.
	 *
	 * <p> An implementation of this method must arrange for any other thread
	 * that is blocked in an I/O operation upon this channel to return
	 * immediately, either by throwing an exception or by returning normally.
	 * </p>
	 *
	 * @throws  IOException
	 *          If an I/O error occurs while closing the channel
	 */
	void implCloseChannel() THROWS(EIOException);

	/**
	 * Adjusts this channel's blocking mode.
	 *
	 * <p> This method is invoked by the {@link #configureBlocking
	 * configureBlocking} method in order to perform the actual work of
	 * changing the blocking mode.  This method is only invoked if the new mode
	 * is different from the current mode.  </p>
	 *
	 * @throws IOException
	 *         If an I/O error occurs
	 */
	virtual void implConfigureBlocking(boolean block) THROWS(EIOException) = 0;

	/**
	 * Closes this selectable channel.
	 *
	 * <p> This method is invoked by the {@link java.nio.channels.Channel#close
	 * close} method in order to perform the actual work of closing the
	 * channel.  This method is only invoked if the channel has not yet been
	 * closed, and it is never invoked more than once.
	 *
	 * <p> An implementation of this method must arrange for any other thread
	 * that is blocked in an I/O operation upon this channel to return
	 * immediately, either by throwing an exception or by returning normally.
	 * </p>
	 */
	virtual void implCloseSelectableChannel() THROWS(EIOException) = 0;

private:
	// Keys that have been created by registering this channel with selectors.
	// They are saved because if this channel is closed the keys must be
	// deregistered.  Protected by keyLock.
	//
	EArray<ESelectionKey*> *keys_;// = null;
	int keyCount_;// = 0;

	// Lock for key set and count
	ELock* keyLock_;

	// Lock for registration and configureBlocking operations
	ELock* regLock_;

	// Blocking mode, protected by regLock
	boolean blocking_;// = true;

	// Lock for interruptible channels.
	EReentrantLock closeLock_;
	volatile boolean open_;	// = true;

	ESelectionKey* findKey(ESelector* sel);
	void addKey(ESelectionKey* k);
	boolean haveValidKeys();
};

} /* namespace nio */
} /* namespace efc */
#endif /* ESELECTABLECHANNEL_HH_ */
