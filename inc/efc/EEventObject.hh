/*
 * EEventObject.hh
 *
 *  Created on: 2015-2-6
 *      Author: cxxjava@163.com
 */

#ifndef EEVENTOBJECT_HH_
#define EEVENTOBJECT_HH_

#include "EString.hh"

namespace efc {

/**
 * <p>
 * The root class from which all event state objects shall be derived.
 * <p>
 * All Events are constructed with a reference to the object, the "source",
 * that is logically deemed to be the object upon which the Event in question
 * initially occurred upon.
 *
 * @since JDK1.1
 */

class EEventObject: public EObject {
public:
	/**
	 * Constructs a prototypical Event.
	 *
	 * @param    source    The object on which the Event initially occurred.
	 * @exception  IllegalArgumentException  if source is null.
	 */
	EEventObject(EObject* source);

	/**
	 * The object on which the Event initially occurred.
	 *
	 * @return   The object on which the Event initially occurred.
	 */
	EObject* getSource();

	/**
	 * Returns a String representation of this EventObject.
	 *
	 * @return  A a String representation of this EventObject.
	 */
	EString toString();

protected:
	/**
	 * The object on which the Event initially occurred.
	 */
	EObject* source;
};

} /* namespace efc */
#endif /* EEVENTOBJECT_HH_ */
