/*
 * EURISyntaxException.h
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#ifndef EURISYNTAXEXCEPTION_H_
#define EURISYNTAXEXCEPTION_H_

#include "EException.hh"
#include "ENullPointerException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

#define EURISYNTAXEXCEPTION       EURISyntaxException(__FILE__, __LINE__, errno)
#define EURISYNTAXEXCEPTIONS(msg) EURISyntaxException(__FILE__, __LINE__, msg)

/**
 * Checked exception thrown to indicate that a string could not be parsed as a
 * URI reference.
 *
 * @see URI
 * @since 1.4
 */

class EURISyntaxException: public EException {
public:
	/**
	 * Constructs an <code>EURISyntaxException</code> with no
	 * detail message.
	 *
	 * @param   _file_   __FILE__
	 * @param   _line_   __LINE__
	 * @param   errn     errno
	 */
	EURISyntaxException(const char *_file_, int _line_, int errn = 0) :
			EException(_file_, _line_, errn) {
	}

	/**
	 * Constructs an <code>EURISyntaxException</code> with the
	 * specified detail message.
	 *
	 * @param   _file_   __FILE__.
	 * @param   _line_   __LINE__.
	 * @param   s   the detail message.
	 */
	EURISyntaxException(const char *_file_, int _line_, const char *s, int errn = 0) :
			EException(_file_, _line_, s, errn) {
	}

	/**
     * Constructs an instance from the given input string, reason, and error
     * index.
     *
     * @param  input   The input string
     * @param  reason  A string explaining why the input could not be parsed
     * @param  index   The index at which the parse error occurred,
     *                 or {@code -1} if the index is not known
     *
     * @throws  NullPointerException
     *          If either the input or reason strings are {@code null}
     *
     * @throws  IllegalArgumentException
     *          If the error index is less than {@code -1}
     */
	EURISyntaxException(const char *_file_, int _line_, const char* input, const char* reason, int index) :
			EException(_file_, _line_, reason) {
		if ((input == null) || (reason == null))
			throw ENullPointerException(__FILE__, __LINE__);
		if (index < -1)
			throw EIllegalArgumentException(__FILE__, __LINE__);
		this->input = input;
		this->index = index;
	}

	/**
	 * Constructs an instance from the given input string and reason.  The
	 * resulting object will have an error index of {@code -1}.
	 *
	 * @param  input   The input string
	 * @param  reason  A string explaining why the input could not be parsed
	 *
	 * @throws  NullPointerException
	 *          If either the input or reason strings are {@code null}
	 */
	EURISyntaxException(const char *_file_, int _line_, const char* input, const char* reason) :
			EException(_file_, _line_, reason) {
		if ((input == null) || (reason == null))
			throw ENullPointerException(__FILE__, __LINE__);
		this->input = input;
		this->index = -1;
	}

    /**
     * Returns the input string.
     *
     * @return  The input string
     */
    const char* getInput() {
    	return input.c_str();
    }

    /**
	 * Returns a string explaining why the input string could not be parsed.
	 *
	 * @return  The reason string
	 */
    const char* getReason() {
		return EException::getMessage();
	}

    /**
	 * Returns an index into the input string of the position at which the
	 * parse error occurred, or {@code -1} if this position is not known.
	 *
	 * @return  The error index
	 */
	int getIndex() {
		return index;
	}

	/**
	 * Returns a string describing the parse error.  The resulting string
	 * consists of the reason string followed by a colon character
	 * ({@code ':'}), a space, and the input string.  If the error index is
	 * defined then the string {@code " at index "} followed by the index, in
	 * decimal, is inserted after the reason string and before the colon
	 * character.
	 *
	 * @return  A string describing the parse error
	 */
	virtual const char* getMessage() {
		if (message.isEmpty()) {
			message.append(getReason());
			if (index > -1) {
				message.append(" at index ");
				message.append(index);
			}
			message.append(": ");
			message.append(input);
		}
		return message.c_str();
	}

private:
    EString input;
    int index;
    EString message;
};

} /* namespace efc */
#endif /* EURISYNTAXEXCEPTION_H_ */
