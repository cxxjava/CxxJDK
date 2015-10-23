/*
 * EEventListenerProxy.hh
 *
 *  Created on: 2015-2-6
 *      Author: cxxjava@163.com
 */

#ifndef EEVENTLISTENERPROXY_HH_
#define EEVENTLISTENERPROXY_HH_

#include "EEventListener.hh"

namespace efc {

/**
 * An abstract wrapper class for an {@code EventListener} class
 * which associates a set of additional parameters with the listener.
 * Subclasses must provide the storage and accessor methods
 * for the additional arguments or parameters.
 * <p>
 * For example, a bean which supports named properties
 * would have a two argument method signature for adding
 * a {@code PropertyChangeListener} for a property:
 * <pre>
 * public void addPropertyChangeListener(String propertyName,
 *                                       PropertyChangeListener listener)
 * </pre>
 * If the bean also implemented the zero argument get listener method:
 * <pre>
 * public PropertyChangeListener[] getPropertyChangeListeners()
 * </pre>
 * then the array may contain inner {@code PropertyChangeListeners}
 * which are also {@code PropertyChangeListenerProxy} objects.
 * <p>
 * If the calling method is interested in retrieving the named property
 * then it would have to test the element to see if it is a proxy class.
 *
 * @since 1.4
 */

template<typename T>
abstract class EEventListenerProxy: virtual public EEventListener {
private:
	T listener;

public:
	/**
	 * Creates a proxy for the specified listener.
	 *
	 * @param listener  the listener object
	 */
	EEventListenerProxy(T listener) {
		this->listener = listener;
	}

	/**
	 * Returns the listener associated with the proxy.
	 *
	 * @return  the listener associated with the proxy
	 */
	T getListener() {
		return this->listener;
	}
};

} /* namespace efc */
#endif /* EEVENTLISTENERPROXY_HH_ */
