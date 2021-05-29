/*
 * EInterfaceAddress.cpp
 *
 *  Created on: 2016-8-12
 *      Author: cxxjava@163.com
 */

#include "EInterfaceAddress.hh"

namespace efc {

EInterfaceAddress::~EInterfaceAddress() {
	//
}

EInterfaceAddress::EInterfaceAddress(): maskLength(0) {
}

EInterfaceAddress::EInterfaceAddress(sp<EInetAddress> address,
		sp<EInetAddress> broadcast, short mask) : maskLength(mask) {
	this->address = address;
	this->broadcast = broadcast;
}

EInetAddress* EInterfaceAddress::getAddress() {
	return address.get();
}

EInetAddress* EInterfaceAddress::getBroadcast() {
	return broadcast.get();
}

short EInterfaceAddress::getNetworkPrefixLength() {
	return maskLength;
}

bool EInterfaceAddress::equals(EObject* obj) {
	EInterfaceAddress* cmp = dynamic_cast<EInterfaceAddress*>(obj);
	if (!cmp)
		return false;
	if ( !(address == null ? cmp->address == null : address->equals(cmp->address.get())) )
		return false;
	if ( !(broadcast  == null ? cmp->broadcast == null : broadcast->equals(cmp->broadcast.get())) )
		return false;
	if (maskLength != cmp->maskLength)
		return false;
	return true;
}

int EInterfaceAddress::hashCode() {
	return address->hashCode() + ((broadcast != null) ? broadcast->hashCode() : 0) + maskLength;
}

EString EInterfaceAddress::toString() {
	return address->toString() + "/" + maskLength + " [" + (broadcast != null ? broadcast->toString() : "") + "]";
}

} /* namespace efc */
