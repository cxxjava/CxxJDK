/*
 * EObservable.cpp
 *
 *  Created on: 2015-2-4
 *      Author: cxxjava@163.com
 */

#include "EObservable.hh"
#include "ENullPointerException.hh"

namespace efc {

EObservable::EObservable() : changed(false) {
	obs = new EVector<sp<EObserver>*>();
	obs->setThreadSafe(false);
}

EObservable::~EObservable() {
	delete obs;
}

void EObservable::addObserver(sp<EObserver>& o) {
	if (o == null)
		throw ENullPointerException(__FILE__, __LINE__);

	SYNCHRONIZED(this) {
		if (!obs->contains(&o)) {
			obs->addElement(new sp<EObserver>(o));
		}
    }}
}

void EObservable::deleteObserver(sp<EObserver>& o) {
	SYNCHRONIZED(this) {
		obs->removeElement(&o);
    }}
}

void EObservable::notifyObservers() {
	notifyObservers(null);
}

static EArrayList<sp<EObserver>*>* observers_clone(EArrayList<sp<EObserver>*>* other)
{
	EArrayList<sp<EObserver>*>* dest = new EArrayList<sp<EObserver>*>(true, other->size());

	sp<EIterator<sp<EObserver>*> > iter = other->iterator();
	while (iter->hasNext()) {
		dest->add(new sp<EObserver>(*(iter->next())));
	}

	return dest;
}

void EObservable::notifyObservers(void* arg) {
	/*
	 * a temporary array buffer, used as a snapshot of the state of
	 * current Observers.
	 */
	EArrayList<sp<EObserver>*>* arrLocal;

	SYNCHRONIZED (this) {
		/* We don't want the Observer doing callbacks into
		 * arbitrary code while holding its own Monitor.
		 * The code where we extract each Observable from
		 * the Vector and store the state of the Observer
		 * needs synchronization, but notifying observers
		 * does not (should not).  The worst result of any
		 * potential race-condition here is that:
		 * 1) a newly-added Observer will miss a
		 *   notification in progress
		 * 2) a recently unregistered Observer will be
		 *   wrongly notified when it doesn't care
		 */
		if (!changed)
			return;
		arrLocal = observers_clone(obs);
		clearChanged();
    }}

	for (int i = arrLocal->size()-1; i>=0; i--) {
		sp<EObserver>* o = arrLocal->getAt(i);
		(*o)->update(this, arg);
	}

	delete arrLocal;
}

void EObservable::deleteObservers() {
	SYNCHRONIZED(this) {
		obs->removeAllElements();
    }}
}

void EObservable::setChanged() {
	SYNCHRONIZED(this) {
		changed = true;
    }}
}

void EObservable::clearChanged() {
	SYNCHRONIZED(this) {
		changed = false;
    }}
}

boolean EObservable::hasChanged() {
	SYNCHRONIZED(this) {
		return changed;
    }}
}

int EObservable::countObservers() {
	SYNCHRONIZED(this) {
		return obs->size();
    }}
}

} /* namespace efc */
