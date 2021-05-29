/*
 * EOutputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EOutputStream.hh"

namespace efc {

void EOutputStream::write(int b)
{
	char c = (char)b;
	write((byte*)&c, 1);
}

void EOutputStream::write(const char* s)
{
	write(s, eso_strlen(s));
}

void EOutputStream::flush()
{
}

void EOutputStream::close()
{
}

} /* namespace efc */
