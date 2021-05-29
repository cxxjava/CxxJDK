/*
 * ECharacter.cpp
 *
 *  Created on: 2013-3-21
 *      Author: cxxjava@163.com
 */

#include "ECharacter.hh"
#include "EInteger.hh"

namespace efc {

ECharacter::ECharacter(char value) {
	m_value = value;
}

char ECharacter::charValue()
{
	return m_value;
}

boolean ECharacter::equals(char that)
{
	return (m_value == that);
}

boolean ECharacter::equals(EObject* obj) {
	if (!obj) return false;
	ECharacter* that = dynamic_cast<ECharacter*>(obj);
	if (!that) return false;
	return (m_value == that->charValue());
}

EString ECharacter::toString()
{
	return EString::valueOf(m_value);
}

boolean ECharacter::isLowerCase(char ch)
{
	return eso_islower(ch);
}

boolean ECharacter::isUpperCase(char ch)
{
	return eso_isupper(ch);
}

boolean ECharacter::isDigit(char ch)
{
	return eso_isdigit(ch);
}

char ECharacter::toLowerCase(char ch)
{
	return eso_tolower(ch);
}

char ECharacter::toUpperCase(char ch)
{
	return eso_toupper(ch);
}

int ECharacter::digit(char ch, int radix)
{
	return EInteger::toDigit(ch, radix);
}

} /* namespace efc */
