/*
 * ENativeThreadSet.hh
 *
 *  Created on: 2014-2-15
 *      Author: cxxjava@163.com
 */

#ifndef ENATIVETHREADSET_HH_
#define ENATIVETHREADSET_HH_

#include "EArrayList.hh"
#include "ESynchronizeable.hh"

namespace efc {
namespace nio {

// Special-purpose data structure for sets of native threads

class ENativeThreadSet: public ESynchronizeable {
public:
	~ENativeThreadSet();
	ENativeThreadSet(int n);

	// Adds the current native thread to this set, returning its index so that
	// it can efficiently be removed later.
	//
	int add();

	// Removes the thread at the given index.
	//
	void remove(int i);

	// Signals all threads in this set.
	//
	void signal();

private:
	EArrayList<llong>* elts;
};

} /* namespace nio */
} /* namespace efc */
#endif /* ENATIVETHREADSET_HH_ */
