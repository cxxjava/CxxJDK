/*
 * EAuthenticationException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EAUTHENTICATIONEXCEPTION_HH_
#define EAUTHENTICATIONEXCEPTION_HH_

#include "ESaslException.hh"

namespace efc {

#define EAUTHENTICATIONEXCEPTION       EAuthenticationException(__FILE__, __LINE__, errno)
#define EAUTHENTICATIONEXCEPTIONS(msg) EAuthenticationException(__FILE__, __LINE__, msg)

/**
 * This exception is thrown by a SASL mechanism implementation
 * to indicate that the SASL
 * exchange has failed due to reasons related to authentication, such as
 * an invalid identity, passphrase, or key.
 * <p>
 * Note that the lack of an AuthenticationException does not mean that
 * the failure was not due to an authentication error.  A SASL mechanism
 * implementation might throw the more general SaslException instead of
 * AuthenticationException if it is unable to determine the nature
 * of the failure, or if does not want to disclose the nature of
 * the failure, for example, due to security reasons.
 *
 * @since 1.5
 *
 */

class EAuthenticationException: public ESaslException {
public:
	/**
	 * Constructs an <code>EAuthenticationException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EAuthenticationException(const char *_file_, int _line_, int errn = 0) :
		ESaslException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EAuthenticationException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EAuthenticationException(const char *_file_, int _line_, const char *s, int errn = 0) :
		ESaslException(_file_, _line_, s, errn) {
	}

	/**
	 * Constructs an <code>EAuthenticationException</code> with the specified detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	EAuthenticationException(const char *_file_, int _line_, EThrowable* cause) :
		ESaslException(_file_, _line_, cause) {
	}

	/**
	 * Constructs a new exception with the specified detail message and
	 * cause.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   s   the detail message.
	 * @param   cause    the cause (which is saved for later retrieval by the
	 *         {@link #getCause()} method).  (A {@code null} value is
	 *         permitted, and indicates that the cause is nonexistent or
	 *         unknown.)
	 */
	EAuthenticationException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
		ESaslException(_file_, _line_, s, cause) {
	}
};

} /* namespace efc */
#endif /* EAUTHENTICATIONEXCEPTION_HH_ */
