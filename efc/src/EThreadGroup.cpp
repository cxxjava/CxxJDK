/*
 * EThreadGroup.cpp
 *
 *  Created on: 2014-9-29
 *      Author: cxxjava@163.com
 */

#include "EThreadGroup.hh"
#include "EMath.hh"
#include "ESystem.hh"
#include "EArrays.hh"
#include "EIllegalThreadStateException.hh"

namespace efc {

//@see: openjdk-8/src/share/classes/java/lang/ThreadGroup.java

EThreadGroup::~EThreadGroup() {
	delete threads;
	delete groups;
	delete cond;
}

EThreadGroup::EThreadGroup() :
		destroyed(false ), nUnstartedThreads(0), nthreads(0), threads(null), ngroups(
				0), groups(null), lock(ES_THREAD_MUTEX_NESTED), cond(lock.newCondition()) {
	this->name = "system";
	this->maxPriority = EThread::MAX_PRIORITY;
	this->parent = null;
}

EThreadGroup::EThreadGroup(const char* name) :
		destroyed(false ), nUnstartedThreads(0), nthreads(0), threads(null), ngroups(
				0), groups(null), lock(ES_THREAD_MUTEX_NESTED), cond(lock.newCondition()) {
	sp<EThreadGroup> parent = EThread::currentThread()->getThreadGroup();

	this->name = name;
	this->maxPriority = parent->maxPriority;
	this->parent = parent;
	//parent->add(this); //FIXME: bug!!!
}

EThreadGroup::EThreadGroup(sp<EThreadGroup> parent, const char* name) :
		destroyed(false ), nUnstartedThreads(0), nthreads(0), threads(null), ngroups(
				0), groups(null), lock(ES_THREAD_MUTEX_NESTED), cond(lock.newCondition()) {
	this->name = name;
	this->maxPriority = parent->maxPriority;
	this->parent = parent;
	//parent->add(this); //FIXME: bug!!!
}

const char* EThreadGroup::getName() {
	return name.c_str();
}

sp<EThreadGroup> EThreadGroup::getParent() {
	if (parent != null)
		parent->checkAccess();
	return parent;
}

int EThreadGroup::getMaxPriority() {
	return maxPriority;
}

boolean EThreadGroup::isDestroyed() {
	SYNCBLOCK(&lock) {
		return destroyed;
    }}
}

void EThreadGroup::setMaxPriority(int pri) {
	int ngroupsSnapshot = 0;
	EA<sp<EThreadGroup> >* groupsSnapshot = null;
	SYNCBLOCK(&lock) {
		checkAccess();
		if (pri < EThread::MIN_PRIORITY || pri > EThread::MAX_PRIORITY) {
			return;
		}
		maxPriority = (parent != null) ? EMath::min(pri, parent->maxPriority) : pri;
		ngroupsSnapshot = ngroups;
		if (groups != null) {
			groupsSnapshot = groups->clone(0, ngroupsSnapshot);
		}
    }}
	for (int i = 0; i < ngroupsSnapshot; i++) {
		(*groupsSnapshot)[i]->setMaxPriority(pri);
	}
	delete groupsSnapshot;
}

boolean EThreadGroup::parentOf(sp<EThreadGroup> g) {
	for (; g != null; g = g->parent) {
		if (g == this) {
			return true ;
		}
	}
	return false ;
}

void EThreadGroup::checkAccess() {
	//SecurityManager security = System.getSecurityManager();
	//if (security != null) {
	//	security.checkAccess(this);
	//}
}

int EThreadGroup::activeCount() {
	int result;
	// Snapshot sub-group data so we don't hold this lock
	// while our children are computing.
	int ngroupsSnapshot = 0;
	EA<sp<EThreadGroup> >* groupsSnapshot = null;
	SYNCBLOCK(&lock) {
		if (destroyed) {
			return 0;
		}
		result = nthreads;
		ngroupsSnapshot = ngroups;
		if (groups != null) {
			groupsSnapshot = groups->clone(0, ngroupsSnapshot);
		}
    }}
	for (int i = 0; i < ngroupsSnapshot; i++) {
		result += (*groupsSnapshot)[i]->activeCount();
	}
	delete groupsSnapshot;
	return result;
}

int EThreadGroup::enumerate(EA<EThread*>* list) {
	checkAccess();
	return enumerate(list, 0, true);
}

int EThreadGroup::enumerate(EA<EThread*>* list, boolean recurse) {
	checkAccess();
	return enumerate(list, 0, recurse);
}

int EThreadGroup::activeGroupCount() {
	int ngroupsSnapshot = 0;
	EA<sp<EThreadGroup> >* groupsSnapshot = null;
	SYNCBLOCK(&lock) {
		if (destroyed) {
			return 0;
		}
		ngroupsSnapshot = ngroups;
		if (groups != null) {
			groupsSnapshot = groups->clone(0, ngroupsSnapshot);
		}
    }}
	int n = ngroupsSnapshot;
	for (int i = 0; i < ngroupsSnapshot; i++) {
		n += (*groupsSnapshot)[i]->activeGroupCount();
	}
	delete groupsSnapshot;
	return n;
}

int EThreadGroup::enumerate(EA<sp<EThreadGroup> >* list) {
	checkAccess();
	return enumerate(list, 0, true);
}

int EThreadGroup::enumerate(EA<sp<EThreadGroup> >* list, boolean recurse) {
	checkAccess();
	return enumerate(list, 0, recurse);
}

void EThreadGroup::interrupt() {
	int ngroupsSnapshot = 0;
	EA<sp<EThreadGroup> >* groupsSnapshot = null;
	SYNCBLOCK(&lock) {
		checkAccess();
		for (int i = 0; i < nthreads; i++) {
			(*threads)[i]->interrupt();
		}
		ngroupsSnapshot = ngroups;
		if (groups != null) {
			groupsSnapshot = groups->clone(0, ngroupsSnapshot);
		}
    }}
	for (int i = 0; i < ngroupsSnapshot; i++) {
		(*groupsSnapshot)[i]->interrupt();
	}
	delete groupsSnapshot;
}

void EThreadGroup::destroy() {
	int ngroupsSnapshot = 0;
	EA<sp<EThreadGroup> >* groupsSnapshot = null;
	SYNCBLOCK(&lock) {
		checkAccess();
		if (destroyed || (nthreads > 0)) {
			throw EIllegalThreadStateException(__FILE__, __LINE__);
		}
		ngroupsSnapshot = ngroups;
		if (groups != null) {
			groupsSnapshot = groups->clone(0, ngroupsSnapshot);
		}
		if (parent != null) {
			destroyed = true;
			ngroups = 0;
			groups = null;
			nthreads = 0;
			delete threads; //!
			threads = null;
		}
    }}
	for (int i = 0; i < ngroupsSnapshot; i += 1) {
		(*groupsSnapshot)[i]->destroy();
	}
	delete groupsSnapshot;
	if (parent != null) {
		parent->remove(shared_from_this());
	}
}

void EThreadGroup::uncaughtException(EThread* t, EThrowable* e) {
	if (parent != null) {
		parent->uncaughtException(t, e);
	} else {
		EThread::UncaughtExceptionHandler* ueh =
			EThread::getDefaultUncaughtExceptionHandler();
		if (ueh != null) {
			ueh->uncaughtException(t, e);
		}
		//else if (!(e instanceof ThreadDeath)) {
		else {
			ESystem::err->printf("Exception in thread \"%s\" ",
							   t->getName());
			e->printStackTrace();
		}
	}
}

EString EThreadGroup::toString() {
	return EString::formatOf("EThreadGroup[name=%s,maxpri=%d]", getName(), maxPriority);
}

void EThreadGroup::addUnstarted() {
	SYNCBLOCK(&lock) {
		if (destroyed) {
			throw EIllegalThreadStateException(__FILE__, __LINE__);
		}
		nUnstartedThreads++;
    }}
}

void EThreadGroup::add(EThread* t) {
	SYNCBLOCK(&lock) {
		if (destroyed) {
			throw EIllegalThreadStateException(__FILE__, __LINE__);
		}

		//cxxjava: when less one thread start then add sub group!
		if (parent != null && nthreads == 0) {
			sp<EThreadGroup> g = shared_from_this();
			ES_ASSERT(t->getThreadGroup() == g);
			parent->add(g);
		}

		if (threads == null) {
			threads = new EA<EThread*>(4, false);
		} else if (nthreads == threads->length()) {
			EA<EThread*>* old = threads;
			threads = EArrays::copyOf(old, nthreads * 2);
			delete old;
		}
		(*threads)[nthreads] = t;

		// This is done last so it doesn't matter in case the
		// thread is killed
		nthreads++;

		// The thread is now a fully fledged member of the group, even
		// though it may, or may not, have been started yet. It will prevent
		// the group from being destroyed so the unstarted Threads count is
		// decremented.
		nUnstartedThreads--;
    }}
}

void EThreadGroup::remove(EThread* t) {
	SYNCBLOCK(&lock) {
		if (destroyed) {
			return;
		}
		for (int i = 0 ; i < nthreads ; i++) {
			if ((*threads)[i] == t) {
				ESystem::arraycopy(*threads, i + 1, *threads, i, --nthreads - i);
				// Zap dangling reference to the dead thread so that
				// the garbage collector will collect it.
				(*threads)[nthreads] = null;
				break;
			}
		}

		//@see: threadTerminated(t)
		if (nthreads == 0) {
			cond->signalAll();
		}
		/* @see:
		if (daemon && (nthreads == 0) &&
			(nUnstartedThreads == 0) && (ngroups == 0))
		*/
		if ((nthreads == 0) &&
			(nUnstartedThreads == 0) && (ngroups == 0))
		{
			destroy();
		}
    }}
}

int EThreadGroup::enumerate(EA<EThread*>* list, int n, boolean recurse) {
	int ngroupsSnapshot = 0;
	EA<sp<EThreadGroup> >* groupsSnapshot = null;
	SYNCBLOCK(&lock) {
		if (destroyed) {
			return 0;
		}
		int nt = nthreads;
		if (nt > list->length() - n) {
			nt = list->length() - n;
		}
		for (int i = 0; i < nt; i++) {
			if ((*threads)[i]->isAlive()) {
				(*list)[n++] = (*threads)[i];
			}
		}
		if (recurse) {
			ngroupsSnapshot = ngroups;
			if (groups != null) {
				groupsSnapshot = groups->clone(0, ngroupsSnapshot);
			}
		}
    }}
	if (recurse) {
		for (int i = 0 ; i < ngroupsSnapshot ; i++) {
			n = (*groupsSnapshot)[i]->enumerate(list, n, true);
		}
	}
	delete groupsSnapshot;
	list->setAutoFree(false);
	return n;
}

int EThreadGroup::enumerate(EA<sp<EThreadGroup> >* list, int n, boolean recurse) {
	int ngroupsSnapshot = 0;
	EA<sp<EThreadGroup> >* groupsSnapshot = null;
	SYNCBLOCK(&lock) {
		if (destroyed) {
			return 0;
		}
		int ng = ngroups;
		if (ng > list->length() - n) {
			ng = list->length() - n;
		}
		if (ng > 0) {
			ESystem::arraycopy(*groups, 0, *list, n, ng);
			n += ng;
		}
		if (recurse) {
			ngroupsSnapshot = ngroups;
			if (groups != null) {
				groupsSnapshot = groups->clone(0, ngroupsSnapshot);
			}
		}
    }}
	if (recurse) {
		for (int i = 0 ; i < ngroupsSnapshot ; i++) {
			n = (*groupsSnapshot)[i]->enumerate(list, n, true);
		}
	}
	delete groupsSnapshot;
	return n;
}

void EThreadGroup::add(sp<EThreadGroup> g) {
	SYNCBLOCK(&lock) {
		if (destroyed) {
			throw EIllegalThreadStateException(__FILE__, __LINE__);
		}
		if (groups == null) {
			groups = new EA<sp<EThreadGroup> >(4);
		} else if (ngroups == groups->length()) {
			EA<sp<EThreadGroup> >* old = groups;
			groups = old->clone(0, ngroups * 2);
			delete old;
		}
		(*groups)[ngroups] = g;

		// This is done last so it doesn't matter in case the
		// thread is killed
		ngroups++;
    }}
}

void EThreadGroup::remove(sp<EThreadGroup> g) {
	SYNCBLOCK(&lock) {
		if (destroyed) {
			return;
		}
		for (int i = 0 ; i < ngroups ; i++) {
			if ((*groups)[i] == g) {
				ngroups -= 1;
				ESystem::arraycopy(*groups, i + 1, *groups, i, ngroups - i);
				// Zap dangling reference to the dead group so that
				// the garbage collector will collect it.
				(*groups)[ngroups] = null;
				break;
			}
		}
		if (nthreads == 0) {
			cond->signalAll();
		}
    }}
}

} /* namespace efc */
