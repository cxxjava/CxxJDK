/*
 * EMath.cpp
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#include "EMath.hh"
#include "ERandom.hh"
#include "EThread.hh"
#include "ESynchronizeable.hh"

namespace efc {

const double EMath::E = 2.7182818284590452354;
const double EMath::PI = 3.14159265358979323846;
ERandom* EMath::randomNumberGenerator;

DEFINE_STATIC_INITZZ_BEGIN(EMath)
EThread::_initzz_(); // for ERandom::lock
randomNumberGenerator =  EMath::getRandom();
DEFINE_STATIC_INITZZ_END

ERandom* EMath::getRandom() {
	static ERandom random;
	return &random;
}

double EMath::toDegrees(double rads)
{
  return rads / 0.017453292519943295;
}

double EMath::toRadians(double degrees)
{
  return degrees * 0.017453292519943295;
}

double EMath::random() {
	randomNumberGenerator = getRandom();
	return randomNumberGenerator->nextDouble();
}

} /* namespace efc */
