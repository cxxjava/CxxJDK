/*
 * ECallable.hh
 *
 *  Created on: 2013-8-19
 *      Author: cxxjava@163.com
 */

#ifndef ECALLABLE_HH_
#define ECALLABLE_HH_

#include "ESharedPtr.hh"
#include "EException.hh"

namespace efc {

/**
 * A task that returns a result and may throw an exception.
 * Implementors define a single method with no arguments called
 * <tt>call</tt>.
 *
 * <p>The <tt>Callable</tt> interface is similar to {@link
 * java.lang.Runnable}, in that both are designed for classes whose
 * instances are potentially executed by another thread.  A
 * <tt>Runnable</tt>, however, does not return a result and cannot
 * throw a checked exception.
 *
 * <p> The {@link Executors} class contains utility methods to
 * convert from other common forms to <tt>Callable</tt> classes.
 *
 * @see Executor
 * @since 1.5
 * @author Doug Lea
 * @param <V> the result type of method <tt>call</tt>
 */

template<typename V>
interface ECallable : virtual public EObject {
	virtual ~ECallable(){}

	/**
	 * Computes a result, or throws an exception if unable to do so.
	 *
	 * @return computed result
	 * @throws Exception if unable to compute a result
	 */
	virtual sp<V> call() THROWS(EException) = 0;
};

} /* namespace efc */
#endif /* ECALLABLE_HH_ */
