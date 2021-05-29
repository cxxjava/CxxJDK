/*
 * EIONetWrapper.hh
 *
 *  Created on: 2016-8-24
 *      Author: cxxjava@163.com
 */

#ifndef EIONETWRAPPER_HH_
#define EIONETWRAPPER_HH_

#include "../../inc/ENetWrapper.hh"

namespace efc {
namespace nio {

class EIONetWrapper: public ENetWrapper {
public:
	/**
	 * Convert Exception to SocketException
	 */
	static void translateToSocketException(EException& x) THROWS(ESocketException);
};

} /* namespace nio */
} /* namespace efc */
#endif /* EIONETWRAPPER_HH_ */
