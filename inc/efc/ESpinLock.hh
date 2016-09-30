/*
 * ESpinLock.hh
 *
 *  Created on: 2013-3-18
 *      Author: cxxjava@163.com
 */

#ifndef ESpinLock_HH_
#define ESpinLock_HH_

#include "ELock.hh"
#include "ETimeUnit.hh"
#include "EUnsupportedOperationException.hh"

namespace efc {

/**
 * Spin Lock.
 *
 * <p>It is recommended practice to <em>always</em> immediately
 * follow a call to {@code lock} with a {@code try} block, most
 * typically in a before/after construction such as:
 *
 * <pre>
 * class X {
 *   private final ReentrantLock lock = new ReentrantLock();
 *   // ...
 *
 *   public void m() {
 *     lock.lock();  // block until condition holds
 *     try {
 *       // ... method body
 *     } finally {
 *       lock.unlock()
 *     }
 *   }
 * }
 * </pre>
 */

class ESpinLock : virtual public ELock{
public:
	ESpinLock();
	~ESpinLock();

	void lock();
	void lockInterruptibly() THROWS(EInterruptedException);
	boolean tryLock();
	boolean tryLock(llong time, ETimeUnit* unit) THROWS(EInterruptedException);
	void unlock();

	ECondition* newCondition() {throw EUNSUPPORTEDOPERATIONEXCEPTION;}

private:
	es_thread_spin_t *m_Spin;
};

} /* namespace efc */
#endif /* ESpinLock_HH_ */
