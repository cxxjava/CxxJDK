/*
 * ESharedPtr.cpp
 *
 *  Created on: 2013-11-12
 *      Author: cxxjava@163.com
 */

#include "ESharedPtr.hh"
#include "ESpinLock.hh"

namespace efc {

ELock* gRCLock = new ESpinLock();

} /* namespace efc */
