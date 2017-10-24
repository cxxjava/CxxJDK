/*
 * EReference.hh
 *
 *  Created on: 2014-11-26
 *      Author: cxxjava@163.com
 */

#ifndef EREFERENCE_HH_
#define EREFERENCE_HH_

#include "EObject.hh"
#include "ESharedPtr.hh"

namespace efc {

/**
 * This class is for reference objects.  This class defines the
 * operations common to all reference objects.  Because reference objects are
 * implemented in close cooperation with the garbage collector, this class may
 * not be subclassed directly.
 *
 * @since    1.2
 */
template<typename T>
class EReference : public EObject {
public:
	EReference(sp<T>& o) : referent(o) {
	}

	/* -- Referent accessor and setters -- */

	/**
	 * Returns this reference object's referent.  If this reference object has
	 * been cleared, either by the program or by the garbage collector, then
	 * this method returns <code>null</code>.
	 *
	 * @return   The object to which this reference refers, or
	 *           <code>null</code> if this reference object has been cleared
	 */
	sp<T> get() {
		return referent.lock();
	}

	/**
	 * Clears this reference object.  Invoking this method will not cause this
	 * object to be enqueued.
	 *
	 * <p> This method is invoked only by Java code; when the garbage collector
	 * clears references it does so directly, without invoking this method.
	 */
	void clear() {
		referent.reset();
	}

private:
	sp<T> referent;
};

} /* namespace efc */
#endif /* EREFERENCE_HH_ */
