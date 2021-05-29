/*
 * EFilterInputStream.cpp
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EFilterInputStream.hh"

namespace efc {

EFilterInputStream::~EFilterInputStream() {
	if (_owned) {
		delete _in;
	}
}

EFilterInputStream::EFilterInputStream(EInputStream *in, boolean owned) :
		_in(in), _owned(owned) {
}

int EFilterInputStream::read(void *b, int len)
{
	return _in->read(b, len);
}

long EFilterInputStream::skip(long n)
{
	return _in->skip(n);
}

long EFilterInputStream::available()
{
	return _in->available();
}

void EFilterInputStream::close()
{
	_in->close();
}

} /* namespace efc */
