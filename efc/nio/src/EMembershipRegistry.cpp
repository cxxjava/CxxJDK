/*
 * EMembershipRegistry.cpp
 *
 *  Created on: 2016-8-23
 *      Author: cxxjava@163.com
 */

#include "./EMembershipRegistry.hh"
#include "../../inc/EIllegalStateException.hh"

namespace efc {
namespace nio {

EMembershipRegistry::~EMembershipRegistry() {
	delete groups;
}

EMembershipKey* EMembershipRegistry::checkMembership(EInetAddress* group, ENetworkInterface* interf,
								  EInetAddress* source) {
	if (groups != null) {
		EList<EMembershipKey*>* keys = groups->get(group);
		if (keys != null) {
			sp<EIterator<EMembershipKey*> > iter = keys->iterator();
			while (iter->hasNext()) {
				EMembershipKey* key = iter->next();
				if (key->networkInterface()->equals(interf)) {
					// already a member to receive all packets so return
					// existing key or detect conflict
					if (source == null) {
						if (key->sourceAddress() == null)
							return key;
						throw EIllegalStateException(__FILE__, __LINE__, "Already a member to receive all packets");
					}

					// already have source-specific membership so return key
					// or detect conflict
					if (key->sourceAddress() == null)
						throw EIllegalStateException(__FILE__, __LINE__, "Already have source-specific membership");
					if (source->equals(key->sourceAddress()))
						return key;
				}
			}
		}
	}
	return null;
}

void EMembershipRegistry::add(EMembershipKey* key) {
	EInetAddress* group = key->group();
	EList<EMembershipKey*>* keys;
	if (groups == null) {
		groups = new EHashMap<EInetAddress*,EList<EMembershipKey*>*>();
		keys = null;
	} else {
		keys = groups->get(group);
	}
	if (keys == null) {
		keys = new ELinkedList<EMembershipKey*>();
		groups->put(new EInetAddress(*group), keys);
	}
	keys->add(key);
}

void EMembershipRegistry::remove(EMembershipKey* key) {
	EInetAddress* group = key->group();
	EList<EMembershipKey*>* keys = groups->get(group);
	if (keys != null) {
		sp<EIterator<EMembershipKey*> > i = keys->iterator();
		while (i->hasNext()) {
			if (i->next() == key) {
				i->remove();
				break;
			}
		}
		if (keys->isEmpty()) {
			delete groups->remove(group);
		}
	}
}

void EMembershipRegistry::invalidateAll() {
	if (groups != null) {
		sp<EIterator<EInetAddress*> > i1 = groups->keySet()->iterator();
		while (i1->hasNext()) {
			EInetAddress* group = i1->next();
			EList<EMembershipKey*>* list = groups->get(group);
			sp<EIterator<EMembershipKey*> >i2 = list->iterator();
			while (i2->hasNext()) {
				EMembershipKey* key = i2->next();
				key->invalidate();
			}
		}
	}
}

} /* namespace nio */
} /* namespace efc */
