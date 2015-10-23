/*
 * EAbstractOwnableSynchronizer.hh
 *
 *  Created on: 2014-9-23
 *      Author: cxxjava@163.com
 */

#ifndef EABSTRACTOWNABLESYNCHRONIZER_HH_
#define EABSTRACTOWNABLESYNCHRONIZER_HH_

#include "EThread.hh"

namespace efc {

/**
 * A synchronizer that may be exclusively owned by a thread.  This
 * class provides a basis for creating locks and related synchronizers
 * that may entail a notion of ownership.  The
 * <tt>AbstractOwnableSynchronizer</tt> class itself does not manage or
 * use this information. However, subclasses and tools may use
 * appropriately maintained values to help control and monitor access
 * and provide diagnostics.
 *
 * @since 1.6
 * @author Doug Lea
 */

abstract class EAbstractOwnableSynchronizer : virtual public EObject {
protected:
    /**
     * Empty constructor for use by subclasses.
     */
    EAbstractOwnableSynchronizer() : exclusiveOwnerThread(null) { }

    /**
     * Sets the thread that currently owns exclusive access. A
     * <tt>null</tt> argument indicates that no thread owns access.
     * This method does not otherwise impose any synchronization or
     * <tt>volatile</tt> field accesses.
     */
    void setExclusiveOwnerThread(EThread* t) {
        exclusiveOwnerThread = t;
    }

    /**
     * Returns the thread last set by
     * <tt>setExclusiveOwnerThread</tt>, or <tt>null</tt> if never
     * set.  This method does not otherwise impose any synchronization
     * or <tt>volatile</tt> field accesses.
     * @return the owner thread
     */
    EThread* getExclusiveOwnerThread() {
        return exclusiveOwnerThread;
    }

private:
    /**
     * The current owner of exclusive mode synchronization.
     */
    EThread* exclusiveOwnerThread;
};

} /* namespace efc */
#endif /* EABSTRACTOWNABLESYNCHRONIZER_HH_ */
