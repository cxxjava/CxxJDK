#ifndef __ESentry_H__
#define __ESentry_H__

#include "ELock.hh"

namespace efc {

/**
 * @brief A simple wrapper for \c EReentrantLock objects.
 * You should not use this class directly but use the
 * \c SYNCHRONIZED macro.
 *
 * To achieve a more Java-like behaviour this
 * macro can be used as a substitution for the \c synchronized keyword in java
 * to synchronize a whole block of c++ code on
 * a certain \c EReentrantLock:
 *
 * \code
 * EReentrantLock* lock = new EReentrantLock();
 *
 * {
 *   ESentry sentry(lock);
 *   ...
 * }
 * \endcode
 *
 * @deprecated Use \c Synchronizeable instead
 * @class ESentry
 */
class ESentry {
public:
	~ESentry();
	ESentry(ELock* lock);
	boolean begin();
	void end();
private:
	ELock* _lock;
	boolean _only;
};

} /* namespace efc */
#endif //!__ESentry_H__
