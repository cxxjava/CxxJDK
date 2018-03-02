/*
 * ESaslException.hh
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef ESASLEXCEPTION_HH_
#define ESASLEXCEPTION_HH_

#include "EIOException.hh"

namespace efc {

#define ESASLEXCEPTION       ESaslException(__FILE__, __LINE__, errno)
#define ESASLEXCEPTIONS(msg) ESaslException(__FILE__, __LINE__, msg)

/**
 * This class represents an error that has occurred when using SASL.
 *
 * @since 1.5
 *
 */

class ESaslException: public EIOException {
public:
	virtual ~ESaslException() {
		delete _exception;
	}

	/**
	 * Constructs an <code>ESaslException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	ESaslException(const char *_file_, int _line_, int errn = 0) :
			EIOException(_file_, _line_, errn), _exception(null) {
	}

	/**
	 * Constructs an <code>ESaslException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	ESaslException(const char *_file_, int _line_, const char *s, int errn = 0) :
		EIOException(_file_, _line_, s, errn), _exception(null) {
	}
    
    /**
     * Constructs an <code>ESaslException</code> with the specified detail message.
     *
     * @param   _file_   __FILE__
     * @param   _line_   __LINE__
     * @param   cause    the cause (which is saved for later retrieval by the
     *         {@link #getCause()} method).  (A {@code null} value is
     *         permitted, and indicates that the cause is nonexistent or
     *         unknown.)
     */
    ESaslException(const char *_file_, int _line_, EThrowable* cause) :
        EIOException(_file_, _line_, cause) {
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
	ESaslException(const char *_file_, int _line_, const char *s, EThrowable* cause) :
		EIOException(_file_, _line_, s, cause), _exception(null) {
	}

	/*
	 * Override Throwable.getCause() to ensure deserialized object from
	 * JSR 28 would return same value for getCause() (i.e., _exception).
	 */
	virtual EThrowable* getCause() {
		return _exception;
	}

	/*
	 * Override Throwable.initCause() to match getCause() by updating
	 * _exception as well.
	 */
	virtual EThrowable* initCause(EThrowable* cause) {
		EIOException::initCause(cause);
		if (cause) _exception = new EThrowable(*cause);
		return this;
	}

	/**
	 * Returns the string representation of this exception.
	 * The string representation contains
	 * this exception's class name, its detailed message, and if
	 * it has a root exception, the string representation of the root
	 * exception. This string representation
	 * is meant for debugging and not meant to be interpreted
	 * programmatically.
	 * @return The non-null string representation of this exception.
	 * @see java.lang.Throwable#getMessage
	 */
	// Override Throwable.toString() to conform to JSR 28
	virtual EString toString() {
		EString answer = EIOException::toString();
		if (_exception != null && _exception != this) {
			answer << " [Caused by " << _exception->toString() << "]";
		}
		return answer;
	}

private:
	/**
	 * The possibly null root cause exception.
	 * @serial
	 */
	// Required for serialization interoperability with JSR 28
	EThrowable* _exception;
};

} /* namespace efc */
#endif /* ESASLEXCEPTION_HH_ */
