/*
 * EAbstractExecutorService.hh
 *
 *  Created on: 2015-2-7
 *      Author: cxxjava@163.com
 */

#ifndef EABSTRACTEXECUTORSERVICE_HH_
#define EABSTRACTEXECUTORSERVICE_HH_

#include "./EExecutorService.hh"

namespace efc {

/**
 * Provides default implementations of {@link ExecutorService}
 * execution methods. This class implements the {@code submit},
 * {@code invokeAny} and {@code invokeAll} methods using a
 * {@link RunnableFuture} returned by {@code newTaskFor}, which defaults
 * to the {@link FutureTask} class provided in this package.  For example,
 * the implementation of {@code submit(Runnable)} creates an
 * associated {@code RunnableFuture} that is executed and
 * returned. Subclasses may override the {@code newTaskFor} methods
 * to return {@code RunnableFuture} implementations other than
 * {@code FutureTask}.
 *
 * <p><b>Extension example</b>. Here is a sketch of a class
 * that customizes {@link ThreadPoolExecutor} to use
 * a {@code CustomTask} class instead of the default {@code FutureTask}:
 *  <pre> {@code
 * public class CustomThreadPoolExecutor extends ThreadPoolExecutor {
 *
 *   static class CustomTask<V> implements RunnableFuture<V> {...}
 *
 *   protected <V> RunnableFuture<V> newTaskFor(Callable<V> c) {
 *       return new CustomTask<V>(c);
 *   }
 *   protected <V> RunnableFuture<V> newTaskFor(Runnable r, V v) {
 *       return new CustomTask<V>(r, v);
 *   }
 *   // ... add constructors, etc.
 * }}</pre>
 *
 * @since 1.5
 */

template<typename V>
class EExecutorCompletionService;

class EAbstractExecutorService: public EExecutorService {
public:
	virtual ~EAbstractExecutorService();

	// All others moved to EExecutorService.hh
};

inline efc::EAbstractExecutorService::~EAbstractExecutorService() {
}

} /* namespace efc */
#endif /* EABSTRACTEXECUTORSERVICE_HH_ */
