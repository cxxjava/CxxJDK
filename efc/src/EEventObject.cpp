/*
 * EEventObject.cpp
 *
 *  Created on: 2015-2-6
 *      Author: cxxjava@163.com
 */

#include "EEventObject.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

EEventObject::EEventObject(EObject* source) {
	if (source == null)
		throw EIllegalArgumentException(__FILE__, __LINE__, "null source");

	this->source = source;
}

/**
 * The object on which the Event initially occurred.
 *
 * @return   The object on which the Event initially occurred.
 */
EObject* EEventObject::getSource() {
	return source;
}

/**
 * Returns a String representation of this EventObject.
 *
 * @return  A a String representation of this EventObject.
 */
EString EEventObject::toString() {
	return EString::formatOf("EEventObject[%p]", source);
}

} /* namespace efc */
