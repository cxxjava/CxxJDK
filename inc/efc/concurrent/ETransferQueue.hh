/*
 * ETransferQueue.hh
 *
 *  Created on: 2015-12-1
 *      Author: cxxjava@163.com
 */

#ifndef ETRANSFERQUEUE_HH_
#define ETRANSFERQUEUE_HH_

#include "EBlockingQueue.hh"
#include "ETimeUnit.hh"

namespace efc {

/**
 * A {@link BlockingQueue} in which producers may wait for consumers
 * to receive elements.  A {@code TransferQueue} may be useful for
 * example in message passing applications in which producers
 * sometimes (using method {@link #transfer}) await receipt of
 * elements by consumers invoking {@code take} or {@code poll}, while
 * at other times enqueue elements (via method {@code put}) without
 * waiting for receipt.
 * {@linkplain #tryTransfer(Object) Non-blocking} and
 * {@linkplain #tryTransfer(Object,long,TimeUnit) time-out} versions of
 * {@code tryTransfer} are also available.
 * A {@code TransferQueue} may also be queried, via {@link
 * #hasWaitingConsumer}, whether there are any threads waiting for
 * items, which is a converse analogy to a {@code peek} operation.
 *
 * <p>Like other blocking queues, a {@code TransferQueue} may be
 * capacity bounded.  If so, an attempted transfer operation may
 * initially block waiting for available space, and/or subsequently
 * block waiting for reception by a consumer.  Note that in a queue
 * with zero capacity, such as {@link SynchronousQueue}, {@code put}
 * and {@code transfer} are effectively synonymous.
 *
 * <p>This interface is a member of the
 * <a href="{@docRoot}/../technotes/guides/collections/index.html">
 * Java Collections Framework</a>.
 *
 * @since 1.7
 * @param <E> the type of elements held in this collection
 */

template<typename E>
interface ETransferQueue : virtual public EBlockingQueue<E>
{
	virtual ~ETransferQueue(){}

	/**
	 * Transfers the element to a waiting consumer immediately, if possible.
	 *
	 * <p>More precisely, transfers the specified element immediately
	 * if there exists a consumer already waiting to receive it (in
	 * {@link #take} or timed {@link #poll(long,TimeUnit) poll}),
	 * otherwise returning {@code false} without enqueuing the element.
	 *
	 * @param e the element to transfer
	 * @return {@code true} if the element was transferred, else
	 *         {@code false}
	 * @throws ClassCastException if the class of the specified element
	 *         prevents it from being added to this queue
	 * @throws NullPointerException if the specified element is null
	 * @throws IllegalArgumentException if some property of the specified
	 *         element prevents it from being added to this queue
	 */
	virtual boolean tryTransfer(sp<E> e) = 0;

	/**
	 * Transfers the element to a consumer, waiting if necessary to do so.
	 *
	 * <p>More precisely, transfers the specified element immediately
	 * if there exists a consumer already waiting to receive it (in
	 * {@link #take} or timed {@link #poll(long,TimeUnit) poll}),
	 * else waits until the element is received by a consumer.
	 *
	 * @param e the element to transfer
	 * @throws InterruptedException if interrupted while waiting,
	 *         in which case the element is not left enqueued
	 * @throws ClassCastException if the class of the specified element
	 *         prevents it from being added to this queue
	 * @throws NullPointerException if the specified element is null
	 * @throws IllegalArgumentException if some property of the specified
	 *         element prevents it from being added to this queue
	 */
	virtual void transfer(sp<E> e) THROWS(EInterruptedException) = 0;

	/**
	 * Transfers the element to a consumer if it is possible to do so
	 * before the timeout elapses.
	 *
	 * <p>More precisely, transfers the specified element immediately
	 * if there exists a consumer already waiting to receive it (in
	 * {@link #take} or timed {@link #poll(long,TimeUnit) poll}),
	 * else waits until the element is received by a consumer,
	 * returning {@code false} if the specified wait time elapses
	 * before the element can be transferred.
	 *
	 * @param e the element to transfer
	 * @param timeout how long to wait before giving up, in units of
	 *        {@code unit}
	 * @param unit a {@code TimeUnit} determining how to interpret the
	 *        {@code timeout} parameter
	 * @return {@code true} if successful, or {@code false} if
	 *         the specified waiting time elapses before completion,
	 *         in which case the element is not left enqueued
	 * @throws InterruptedException if interrupted while waiting,
	 *         in which case the element is not left enqueued
	 * @throws ClassCastException if the class of the specified element
	 *         prevents it from being added to this queue
	 * @throws NullPointerException if the specified element is null
	 * @throws IllegalArgumentException if some property of the specified
	 *         element prevents it from being added to this queue
	 */
	virtual boolean tryTransfer(sp<E> e, llong timeout, ETimeUnit *unit)
			THROWS(EInterruptedException) = 0;

	/**
	 * Returns {@code true} if there is at least one consumer waiting
	 * to receive an element via {@link #take} or
	 * timed {@link #poll(long,TimeUnit) poll}.
	 * The return value represents a momentary state of affairs.
	 *
	 * @return {@code true} if there is at least one waiting consumer
	 */
	virtual boolean hasWaitingConsumer() = 0;

	/**
	 * Returns an estimate of the number of consumers waiting to
	 * receive elements via {@link #take} or timed
	 * {@link #poll(long,TimeUnit) poll}.  The return value is an
	 * approximation of a momentary state of affairs, that may be
	 * inaccurate if consumers have completed or given up waiting.
	 * The value may be useful for monitoring and heuristics, but
	 * not for synchronization control.  Implementations of this
	 * method are likely to be noticeably slower than those for
	 * {@link #hasWaitingConsumer}.
	 *
	 * @return the number of consumers waiting to receive elements
	 */
	virtual int getWaitingConsumerCount() = 0;
};

} /* namespace efc */
#endif /* ETRANSFERQUEUE_HH_ */
