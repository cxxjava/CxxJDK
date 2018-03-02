/*
 * ECharacter.hh
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#ifndef ECHARACTER_HH_
#define ECHARACTER_HH_

#include "EObject.hh"
#include "EString.hh"

namespace efc {

/**
 * The {@code Character} class wraps a value of the primitive
 * type {@code char} in an object. An object of type
 * {@code Character} contains a single field whose type is
 * {@code char}.
 * <p>
 * In addition, this class provides several methods for determining
 * a character's category (lowercase letter, digit, etc.) and for converting
 * characters from uppercase to lowercase and vice versa.
 * <p>
 * Character information is based on the Unicode Standard, version 6.2.0.
 * <p>
 * The methods and data of class {@code Character} are defined by
 * the information in the <i>UnicodeData</i> file that is part of the
 * Unicode Character Database maintained by the Unicode
 * Consortium. This file specifies various properties including name
 * and general category for every defined Unicode code point or
 * character range.
 * <p>
 * The file and its description are available from the Unicode Consortium at:
 * <ul>
 * <li><a href="http://www.unicode.org">http://www.unicode.org</a>
 * </ul>
 *
 * <h3><a name="unicode">Unicode Character Representations</a></h3>
 *
 * <p>The {@code char} data type (and therefore the value that a
 * {@code Character} object encapsulates) are based on the
 * original Unicode specification, which defined characters as
 * fixed-width 16-bit entities. The Unicode Standard has since been
 * changed to allow for characters whose representation requires more
 * than 16 bits.  The range of legal <em>code point</em>s is now
 * U+0000 to U+10FFFF, known as <em>Unicode scalar value</em>.
 * (Refer to the <a
 * href="http://www.unicode.org/reports/tr27/#notation"><i>
 * definition</i></a> of the U+<i>n</i> notation in the Unicode
 * Standard.)
 *
 * <p><a name="BMP">The set of characters from U+0000 to U+FFFF</a> is
 * sometimes referred to as the <em>Basic Multilingual Plane (BMP)</em>.
 * <a name="supplementary">Characters</a> whose code points are greater
 * than U+FFFF are called <em>supplementary character</em>s.  The Java
 * platform uses the UTF-16 representation in {@code char} arrays and
 * in the {@code String} and {@code StringBuffer} classes. In
 * this representation, supplementary characters are represented as a pair
 * of {@code char} values, the first from the <em>high-surrogates</em>
 * range, (&#92;uD800-&#92;uDBFF), the second from the
 * <em>low-surrogates</em> range (&#92;uDC00-&#92;uDFFF).
 *
 * <p>A {@code char} value, therefore, represents Basic
 * Multilingual Plane (BMP) code points, including the surrogate
 * code points, or code units of the UTF-16 encoding. An
 * {@code int} value represents all Unicode code points,
 * including supplementary code points. The lower (least significant)
 * 21 bits of {@code int} are used to represent Unicode code
 * points and the upper (most significant) 11 bits must be zero.
 * Unless otherwise specified, the behavior with respect to
 * supplementary characters and surrogate {@code char} values is
 * as follows:
 *
 * <ul>
 * <li>The methods that only accept a {@code char} value cannot support
 * supplementary characters. They treat {@code char} values from the
 * surrogate ranges as undefined characters. For example,
 * {@code Character.isLetter('\u005CuD840')} returns {@code false}, even though
 * this specific value if followed by any low-surrogate value in a string
 * would represent a letter.
 *
 * <li>The methods that accept an {@code int} value support all
 * Unicode characters, including supplementary characters. For
 * example, {@code Character.isLetter(0x2F81A)} returns
 * {@code true} because the code point value represents a letter
 * (a CJK ideograph).
 * </ul>
 *
 * <p>In the Java SE API documentation, <em>Unicode code point</em> is
 * used for character values in the range between U+0000 and U+10FFFF,
 * and <em>Unicode code unit</em> is used for 16-bit
 * {@code char} values that are code units of the <em>UTF-16</em>
 * encoding. For more information on Unicode terminology, refer to the
 * <a href="http://www.unicode.org/glossary/">Unicode Glossary</a>.
 *
 * @since   1.0
 */

class ECharacter : public EObject {
public:
	/**
	 * The minimum radix available for conversion to and from strings.
	 * The constant value of this field is the smallest value permitted
	 * for the radix argument in radix-conversion methods such as the
	 * {@code digit} method, the {@code forDigit} method, and the
	 * {@code toString} method of class {@code Integer}.
	 *
	 * @see     Character#digit(char, int)
	 * @see     Character#forDigit(int, int)
	 * @see     Integer#toString(int, int)
	 * @see     Integer#valueOf(String)
	 */
	static const int MIN_RADIX = 2;

	/**
	 * The maximum radix available for conversion to and from strings.
	 * The constant value of this field is the largest value permitted
	 * for the radix argument in radix-conversion methods such as the
	 * {@code digit} method, the {@code forDigit} method, and the
	 * {@code toString} method of class {@code Integer}.
	 *
	 * @see     Character#digit(char, int)
	 * @see     Character#forDigit(int, int)
	 * @see     Integer#toString(int, int)
	 * @see     Integer#valueOf(String)
	 */
	static const int MAX_RADIX = 36;

	/**
	 * The constant value of this field is the smallest value of type
	 * <code>char</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const char MIN_VALUE = -128;

	/**
	 * The constant value of this field is the largest value of type
	 * <code>char</code>.
	 *
	 * @since   JDK1.0.2
	 */
	static const char MAX_VALUE = 127;

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
	virtual boolean equals(EObject* obj);

	/**
	 * Returns a String object representing this character's value.
	 * Converts this <code>Character</code> object to a string. The
	 * result is a string whose length is <code>1</code>. The string's
	 * sole component is the primitive <code>char</code> value represented
	 * by this object.
	 *
	 * @return  a string representation of this object.
	 */
	virtual EString toString();

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
