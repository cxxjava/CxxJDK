/*
 * EURLString.hh
 *
 *  Created on: 2013-3-16
 *      Author: cxxjava@163.com
 */

#ifndef __EURLString_H__
#define __EURLString_H__

#include "EString.hh"

namespace efc {

class EURLString : public EString
{
public:
	virtual ~EURLString(){}

	EURLString(const char* s);

	/**
	 * Concat of a string.
	 */
	EURLString& concat(const char* s, int len=-1);

	/**
	 * Concat of a format string.
	 */
	EURLString& fmtcat(const char* fmt, ...);

	/**
	 *Concat of a encoded URL string.
	 */
	EURLString& enccat(const char* s, int len=-1);

	/**
	 * Concat of a decoded URL string.
	 */
	EURLString& deccat(const char* s);
};

} /* namespace efc */
#endif //!__EURLString_H__
