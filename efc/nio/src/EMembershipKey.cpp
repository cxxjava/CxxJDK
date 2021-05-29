/*
 * EMembershipKey.cpp
 *
 *  Created on: 2016-8-23
 *      Author: cxxjava@163.com
 */

#include "../inc/EMembershipKey.hh"
#include "../inc/EMulticastChannel.hh"
#include "../inc/EDatagramChannel.hh"
#include "../../inc/EIllegalStateException.hh"

namespace efc {
namespace nio {

//@see: openjdk-8/src/share/classes/java/nio/channels/MembershipKey.java
//@see: openjdk-8/src/share/classes/sun/nio/ch/MembershipKeyImpl.java

EMembershipKey::~EMembershipKey() {
	delete blockedSet;
}

EMembershipKey::EMembershipKey(sp<EMulticastChannel> ch, EInetAddress* group,
		ENetworkInterface* interf, EInetAddress* source, int groupAddress,
		int interfAddress, int sourceAddress) :
		ch(ch),
		group_(group),
		interf(interf),
		source_(source),
		valid(true),
		blockedSet(null),
		groupAddress_(groupAddress),
		interfAddress_(interfAddress),
		sourceAddress_(sourceAddress) {
	//
}

int EMembershipKey::groupAddress() {
	return groupAddress_;
}

int EMembershipKey::interfaceAddress() {
	return interfAddress_;
}

int EMembershipKey::source() {
	return sourceAddress_;
}

boolean EMembershipKey::isValid() {
	return valid;
}

void EMembershipKey::drop() {
	// delegate to channel
	dynamic_cast<EDatagramChannel*>(ch.get())->drop(this);
}

void EMembershipKey::invalidate() {
	valid = false;
}

EMembershipKey* EMembershipKey::block(EInetAddress* toBlock) {
	if (source_ != null)
		throw EIllegalStateException(__FILE__, __LINE__, "key is source-specific");

	SYNCBLOCK(&stateLock) {
		if ((blockedSet != null) && blockedSet->contains(toBlock)) {
			// already blocked, nothing to do
			return this;
		}

		dynamic_cast<EDatagramChannel*>(ch.get())->block(this, toBlock);

		// created blocked set if required and add source address
		if (blockedSet == null)
			blockedSet = new EHashSet<EInetAddress*>();
		blockedSet->add(toBlock);
    }}
	return this;
}

EMembershipKey* EMembershipKey::unblock(EInetAddress* toUnblock) {
	SYNCBLOCK(&stateLock) {
		if ((blockedSet == null) || !blockedSet->contains(toUnblock))
			throw EIllegalStateException(__FILE__, __LINE__, "not blocked");

		dynamic_cast<EDatagramChannel*>(ch.get())->unblock(this, toUnblock);

		blockedSet->remove(toUnblock);
    }}
	return this;
}

sp<EMulticastChannel> EMembershipKey::channel() {
	return ch;
}

EInetAddress* EMembershipKey::group() {
	return group_;
}

ENetworkInterface* EMembershipKey::networkInterface() {
	return interf;
}

EInetAddress* EMembershipKey::sourceAddress() {
	return source_;
}

EString EMembershipKey::toString() {
	EString sb('<');
	sb.append(group_->getHostAddress());
	sb.append(',');
	sb.append(interf->getName());
	if (source_ != null) {
		sb.append(',');
		sb.append(source_->getHostAddress());
	}
	sb.append('>');
	return sb;
}

} /* namespace nio */
} /* namespace efc */
