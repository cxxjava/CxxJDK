/*
 * EInterruptible.hh
 *
 *  Created on: 2014-10-21
 *      Author: cxxjava@163.com
 */

#ifndef EINTERRUPTIBLE_HH_
#define EINTERRUPTIBLE_HH_

#include "EBase.hh"

namespace efc {

interface EInterruptible {

	virtual ~EInterruptible(){}

    virtual void interrupt() = 0;

};

} /* namespace efc */
#endif /* EINTERRUPTIBLE_HH_ */
