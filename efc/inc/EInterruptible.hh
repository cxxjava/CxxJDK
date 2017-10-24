/*
 * EInterruptible.hh
 *
 *  Created on: 2014-10-21
 *      Author: cxxjava@163.com
 */

#ifndef EINTERRUPTIBLE_HH_
#define EINTERRUPTIBLE_HH_

#include "EThread.hh"

namespace efc {

interface EInterruptible {

	virtual ~EInterruptible(){}

    virtual void interrupt(EThread* t) = 0;

};

} /* namespace efc */
#endif /* EINTERRUPTIBLE_HH_ */
