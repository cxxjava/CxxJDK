/*
 * EURLString.hhcpp
 *
 *  Created on: 2013-3-16
 *      Author: cxxjava@163.com
 */

#include "EURLString.hh"

namespace efc {

EURLString::EURLString(const char* s) : EString(s)
{
}

EURLString& EURLString::enccat(const char* s, int len)
{
	es_string_t *to = NULL;
	to = eso_murlencode(&to, s, len);
	concat(to);
	eso_mfree(to);
	return (*this);
}

EURLString& EURLString::deccat(const char* s)
{
	es_string_t *to = NULL;
	to = eso_murldecode(&to, s);
	concat(to);
	eso_mfree(to);
	return (*this);
}

EURLString& EURLString::concat(const char* s, int len)
{
	EString::concat(s, len);
	return (*this);
}

EURLString& EURLString::fmtcat(const char* fmt, ...)
{
	va_list args;
	EString estr;

	va_start(args, fmt);
	estr.vformat(fmt, args);
	va_end(args);

	return concat(estr.c_str());
}

} /* namespace efc */
