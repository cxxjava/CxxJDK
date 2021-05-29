/*
 * EMembershipRegistry.hh
 *
 *  Created on: 2016-8-23
 *      Author: cxxjava@163.com
 */

#ifndef EMEMBERSHIPREGISTRY_HH_
#define EMEMBERSHIPREGISTRY_HH_

#include "../../inc/EHashMap.hh"
#include "../../inc/ELinkedList.hh"
#include "../inc/EMembershipKey.hh"
#include "../../inc/EInetAddress.hh"
#include "../../inc/ENetworkInterface.hh"

namespace efc {
namespace nio {

/**
 * Simple registry of membership keys for a MulticastChannel.
 *
 * Instances of this object are not safe by multiple concurrent threads.
 */

class EMembershipRegistry: public EObject {
public:
	virtual ~EMembershipRegistry();

	/**
	 * Checks registry for membership of the group on the given
	 * network interface.
	 */
	EMembershipKey* checkMembership(EInetAddress* group, ENetworkInterface* interf,
								  EInetAddress* source);

	/**
	 * Add membership to the registry, returning a new membership key.
	 */
	void add(EMembershipKey* key);

	/**
	 * Remove a key from the registry
	 */
	void remove(EMembershipKey* key);

	/**
	 * Invalidate all keys in the registry
	 */
	void invalidateAll();

private:
	// map multicast group to keys
	EHashMap<EInetAddress*,EList<EMembershipKey*>* >* groups;// = null;
};

} /* namespace nio */
} /* namespace efc */
#endif /* EMEMBERSHIPREGISTRY_HH_ */
