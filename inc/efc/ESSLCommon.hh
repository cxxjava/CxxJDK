/*
 * ESSLCommon.hh
 *
 *  Created on: 2017-1-14
 *      Author: cxxjava@163.com
 */

#ifndef ESSLCOMMON_HH_
#define ESSLCOMMON_HH_

#include "es_config.h"

#ifdef HAVE_OPENSSL

#include "EString.hh"

namespace efc {

class ESSLCommon {
public:
	DECLARE_STATIC_INITZZ;

public:
	/**
	 * Convenience function to call getErrors() with the current errno value.
	 *
	 * Make sure that you only call this when there was no intervening operation
	 * since the last OpenSSL error that may have changed the current errno value.
	 */
	static EString getErrors();

	/**
	 * Examine OpenSSL's error stack, and return a string description of the
	 * errors.
	 *
	 * This operation removes the errors from OpenSSL's error stack.
	 */
	static EString getErrors(unsigned long errnoCopy);
};

} /* namespace efc */
#endif //!HAVE_OPENSSL
#endif /* ESSLCOMMON_HH_ */
