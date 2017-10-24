/*
 * ERejectedExecutionHandler.hh
 *
 *  Created on: 2015-3-5
 *      Author: cxxjava@163.com
 */

#ifndef EREJECTEDEXECUTIONHANDLER_HH_
#define EREJECTEDEXECUTIONHANDLER_HH_

#include "../EObject.hh"

namespace efc {

interface ERunnable;
class EThreadPoolExecutor;

/**
 * A handler for tasks that cannot be executed by a {@link ThreadPoolExecutor}.
 *
 * @since 1.5
 */

interface ERejectedExecutionHandler : virtual public EObject {
	virtual ~ERejectedExecutionHandler(){}

	/**
	 * Method that may be invoked by a {@link ThreadPoolExecutor} when
	 * {@link ThreadPoolExecutor#execute execute} cannot accept a
	 * task.  This may occur when no more threads or queue slots are
	 * available because their bounds would be exceeded, or upon
	 * shutdown of the Executor.
	 *
	 * <p>In the absence of other alternatives, the method may throw
	 * an unchecked {@link RejectedExecutionException}, which will be
	 * propagated to the caller of {@code execute}.
	 *
	 * @param r the runnable task requested to be executed
	 * @param executor the executor attempting to execute this task
	 * @throws RejectedExecutionException if there is no remedy
	 */
	virtual void rejectedExecution(sp<ERunnable> r, EThreadPoolExecutor* executor) = 0;
};

} /* namespace efc */
#endif /* EREJECTEDEXECUTIONHANDLER_HH_ */
