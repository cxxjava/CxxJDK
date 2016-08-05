/*
 * EChannel.hh
 *
 *  Created on: 2014-2-8
 *      Author: cxxjava@163.com
 */

#ifndef ECHANNEL_HH_
#define ECHANNEL_HH_

#include "EObject.hh"
#include "EIOException.hh"

namespace efc {
namespace nio {

/**
 * A nexus for I/O operations.
 *
 * <p> A channel represents an open connection to an entity such as a hardware
 * device, a file, a network socket, or a program component that is capable of
 * performing one or more distinct I/O operations, for example reading or
 * writing.
 *
 * <p> A channel is either open or closed.  A channel is open upon creation,
 * and once closed it remains closed.  Once a channel is closed, any attempt to
 * invoke an I/O operation upon it will cause a {@link ClosedChannelException}
 * to be thrown.  Whether or not a channel is open may be tested by invoking
 * its {@link #isOpen isOpen} method.
 *
 * <p> Channels are, in general, intended to be safe for multithreaded access
 * as described in the specifications of the interfaces and classes that extend
 * and implement this interface.
 *
 *
 * @since 1.4
 */

interface EChannel : virtual public EObject {
	virtual ~EChannel(){}

	/**
	 * Tells whether or not this channel is open.  </p>
	 *
	 * @return <tt>true</tt> if, and only if, this channel is open
	 */
	virtual boolean isOpen() = 0;

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
	virtual void close() THROWS(EIOException) = 0;
};

} /* namespace nio */
} /* namespace efc */
#endif /* ECHANNEL_HH_ */
