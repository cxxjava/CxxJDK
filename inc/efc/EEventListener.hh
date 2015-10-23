/*
 * EEventListener.hh
 *
 *  Created on: 2015-2-6
 *      Author: cxxjava@163.com
 */

#ifndef EEVENTLISTENER_HH_
#define EEVENTLISTENER_HH_

#include "EBase.hh"

namespace efc {

/**
 * A tagging interface that all event listener interfaces must extend.
 * @since JDK1.1
 */
interface EEventListener : virtual public EObject {
	virtual ~EEventListener() {
	}
};

} /* namespace efc */
#endif /* EEVENTLISTENER_HH_ */
