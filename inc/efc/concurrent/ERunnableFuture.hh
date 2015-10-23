/*
 * ERunnableFuture.hh
 *
 *  Created on: 2013-8-19
 *      Author: cxxjava@163.com
 */

#ifndef ERUNNABLEFUTURE_HH_
#define ERUNNABLEFUTURE_HH_

#include "ERunnable.hh"
#include "EFuture.hh"

namespace efc {

/**
 * A {@link Future} that is {@link Runnable}. Successful execution of
 * the <tt>run</tt> method causes completion of the <tt>Future</tt>
 * and allows access to its results.
 * @see FutureTask
 * @see Executor
 * @since 1.6
 * @author Doug Lea
 * @param <V> The result type returned by this Future's <tt>get</tt> method
 */

template<typename V>
interface ERunnableFuture: virtual public ERunnable, virtual public EFuture<V> {
	virtual ~ERunnableFuture(){}

	/**
	 * Sets this Future to the result of its computation
	 * unless it has been cancelled.
	 */
	virtual void run() = 0;
};

} /* namespace efc */
#endif /* ERUNNABLEFUTURE_HH_ */
