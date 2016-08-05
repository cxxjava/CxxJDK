/*
 * EObserver.hh
 *
 *  Created on: 2013-3-19
 *      Author: Administrator
 */

#ifndef EOBSERVER_HH_
#define EOBSERVER_HH_

#include "EObject.hh"

namespace efc {

class EObservable;

/**
 * A class can implement the <code>Observer</code> interface when it
 * wants to be informed of changes in observable objects.
 *
 * @see     java.util.Observable
 * @since   JDK1.0
 */

interface EObserver : virtual public EObject
{
	virtual ~EObserver(){}

	/**
	 * This method is called whenever the observed object is changed. An
	 * application calls an <tt>Observable</tt> object's
	 * <code>notifyObservers</code> method to have all the object's
	 * observers notified of the change.
	 *
	 * @param   o     the observable object.
	 * @param   arg   an argument passed to the <code>notifyObservers</code>
	 *                 method.
	 */
	virtual void update(EObservable* o, void* arg) = 0;
};

} /* namespace efc */
#endif /* EOBSERVER_HH_ */
