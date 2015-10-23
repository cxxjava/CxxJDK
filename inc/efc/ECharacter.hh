/*
 * ECharacter.hh
 *
 *  Created on: 2013-3-21
 *      Author: Administrator
 */

#ifndef ECHARACTER_HH_
#define ECHARACTER_HH_

#include "EObject.hh"
#include "EString.hh"

namespace efc {

/**
 * The minimum radix available for conversion to and from Strings.
 */
#define CHARACTER_MIN_RADIX   2

/**
 * The maximum radix available for conversion to and from Strings.
 */
#define CHARACTER_MAX_RADIX   36

class ECharacter : public EObject {
public:
	/**
	 * The constant value of this field is the smallest value of type
	 * <code>char</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const char MIN_VALUE;// = -128;

	/**
	 * The constant value of this field is the largest value of type
	 * <code>char</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const char MAX_VALUE;// = 127;

public:
	virtual ~ECharacter() {
	}

	ECharacter(char value);

	/**
	 * Returns the value of this Character object.
	 * @return  the primitive <code>char</code> value represented by
	 *          this object.
	 */
	char charValue();

	/**
	 * Compares this object against the specified object.
	 * The result is <code>true</code> if and only if the argument is not
	 * <code>null</code> and is a <code>Character</code> object that
	 * represents the same <code>char</code> value as this object.
	 *
	 * @param   obj   the object to compare with.
	 * @return  <code>true</code> if the objects are the same;
	 *          <code>false</code> otherwise.
	 */
	boolean equals(char that);

	/**
	 * Returns a String object representing this character's value.
	 * Converts this <code>Character</code> object to a string. The
	 * result is a string whose length is <code>1</code>. The string's
	 * sole component is the primitive <code>char</code> value represented
	 * by this object.
	 *
	 * @return  a string representation of this object.
	 */
	EString toString();

	/**
	 * Determines if the specified character is a lowercase character.
	 *
	 * @param   ch   the character to be tested.
	 * @return  <code>true</code> if the character is lowercase;
	 *          <code>false</code> otherwise.
	 * @since   JDK1.0
	 */
	static boolean isLowerCase(char ch);

	/**
	 * Determines if the specified character is an uppercase character.
	 * @param   ch   the character to be tested.
	 * @return  <code>true</code> if the character is uppercase;
	 *          <code>false</code> otherwise.
	 */
	static boolean isUpperCase(char ch);

	/**
	 * Determines if the specified character is a digit.
	 *
	 * @param   ch   the character to be tested.
	 * @return  <code>true</code> if the character is a digit;
	 *          <code>false</code> otherwise.
	 * @since   JDK1.0
	 */
	static boolean isDigit(char ch);

	/**
	 * The given character is mapped to its lowercase equivalent; if the
	 * character has no lowercase equivalent, the character itself is
	 * returned.
	 * <p>
	 * Note that by default CLDC only supports
	 * the ISO Latin-1 range of characters.
	 *
	 * @param   ch   the character to be converted.
	 * @return  the lowercase equivalent of the character, if any;
	 *          otherwise the character itself.
	 * @see     java.lang.Character#isLowerCase(char)
	 * @see     java.lang.Character#isUpperCase(char)
	 * @see     java.lang.Character#toUpperCase(char)
	 * @since   JDK1.0
	 */
	static char toLowerCase(char ch);

	/**
	 * Converts the character argument to uppercase; if the
	 * character has no uppercase equivalent, the character itself is
	 * returned.
	 * <p>
	 * Note that by default CLDC only supports
	 * the ISO Latin-1 range of characters.
	 *
	 * @param   ch   the character to be converted.
	 * @return  the uppercase equivalent of the character, if any;
	 *          otherwise the character itself.
	 * @see     java.lang.Character#isLowerCase(char)
	 * @see     java.lang.Character#isUpperCase(char)
	 * @see     java.lang.Character#toLowerCase(char)
	 * @since   JDK1.0
	 */
	static char toUpperCase(char ch);

	/**
	 * Returns the numeric value of the character <code>ch</code> in the
	 * specified radix.
	 *
	 * @param   ch      the character to be converted.
	 * @param   radix   the radix.
	 * @return  the numeric value represented by the character in the
	 *          specified radix.
	 * @see     java.lang.Character#isDigit(char)
	 * @since   JDK1.0
	 */
	static int digit(char ch, int radix);

private:
	/**
	 * The value of the Character.
	 */
	char m_value;
};

} /* namespace efc */
#endif /* ECHARACTER_HH_ */
