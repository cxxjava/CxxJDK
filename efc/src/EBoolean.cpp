/*
 * EBoolean.cpp
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#include "EBoolean.hh"

namespace efc {

EBoolean::EBoolean(boolean value) {
	_value = value;
}

EBoolean::EBoolean(const char* s) {
	_value = toBoolean(s);
}

boolean EBoolean::parseBoolean(const char* s) {
	return toBoolean(s);
}

boolean EBoolean::booleanValue() {
	return _value;
}

EBoolean EBoolean::valueOf(boolean b) {
	return EBoolean(b);
}

EBoolean EBoolean::valueOf(const char* s) {
	return EBoolean(toBoolean(s));
}

EString EBoolean::toString(boolean b) {
	return EString(b ? "true" : "false");
}

EString EBoolean::toString() {
	return EString(_value ? "true" : "false");
}

int EBoolean::hashCode() {
	return _value ? 1231 : 1237;
}

boolean EBoolean::equals(EBoolean* obj) {
	if (!obj) return false;
	return _value == obj->booleanValue();
}

boolean EBoolean::equals(EObject* obj) {
	if (this == obj) {
		return true;
	}
	EBoolean* that = dynamic_cast<EBoolean*>(obj);
	if (!that) {
		return false;
	}
	return _value == that->booleanValue();
}

int EBoolean::compareTo(EBoolean* b) {
	return (b->_value == _value ? 0 : (_value ? 1 : -1));
}

boolean EBoolean::toBoolean(const char* name) {
	return ((name != null) && (!eso_strcasecmp(name, "true") || eso_atol(name)));
}

} /* namespace efc */
