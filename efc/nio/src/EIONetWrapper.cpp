/*
 * EIONetWrapper.cpp
 *
 *  Created on: 2016-8-24
 *      Author: cxxjava@163.com
 */

#include "./EIONetWrapper.hh"
#include "../inc/EClosedChannelException.hh"
#include "../../inc/ESocketException.hh"
#include "../../inc/ENotYetConnectedException.hh"
#include "../../inc/EAlreadyBoundException.hh"
#include "../../inc/ENotYetBoundException.hh"
#include "../../inc/EUnsupportedAddressTypeException.hh"
#include "../../inc/EUnresolvedAddressException.hh"
#include "../../inc/ERuntimeException.hh"

namespace efc {
namespace nio {

void EIONetWrapper::translateToSocketException(EException& x) {
	if (instanceof<ESocketException>(x))
		throw dynamic_cast<ESocketException&>(x);

	if (instanceof<EClosedChannelException>(x)) {
		throw ESocketException(__FILE__, __LINE__, "Socket is closed", &x);
	}
	else if (instanceof<ENotYetConnectedException>(x)) {
		throw ESocketException(__FILE__, __LINE__, "Socket is not connected", &x);
	}
	else if (instanceof<EAlreadyBoundException>(x)) {
		throw ESocketException(__FILE__, __LINE__, "Already bound", &x);
	}
	else if (instanceof<ENotYetBoundException>(x)) {
		throw ESocketException(__FILE__, __LINE__, "Socket is not bound yet", &x);
	}
	else if (instanceof<EUnsupportedAddressTypeException>(x)) {
		throw ESocketException(__FILE__, __LINE__, "Unsupported address type", &x);
	}
	else if (instanceof<EUnresolvedAddressException>(x)) {
		throw ESocketException(__FILE__, __LINE__, "Unresolved address", &x);
	}
	else if (instanceof<ERuntimeException>(x))
		throw dynamic_cast<ERuntimeException&>(x);
	else
		throw ERuntimeException(__FILE__, __LINE__, "Untranslated exception", &x);
}

} /* namespace nio */
} /* namespace efc */
