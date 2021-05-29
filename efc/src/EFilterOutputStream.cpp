/*
 * EFilterOutputStream.cpp
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EFilterOutputStream.hh"

namespace efc {

EFilterOutputStream::~EFilterOutputStream() {
	if (_owned) {
		delete _out;
	}
}

EFilterOutputStream::EFilterOutputStream(EOutputStream *out, boolean owned) :
		_out(out), _owned(owned), closed(false) {
}

void EFilterOutputStream::write(const void *b, int len)
{
	_out->write(b, len);
}

void EFilterOutputStream::write(const char *s)
{
	EOutputStream::write(s);
}

void EFilterOutputStream::write(int b)
{
	EOutputStream::write(b);
}

void EFilterOutputStream::flush()
{
	_out->flush();
}

void EFilterOutputStream::close()
{
	if (closed) {
		return;
	}
	closed = true;

	try {
	  flush();
	} catch (...) {
	}
	_out->close();
}

} /* namespace efc */
