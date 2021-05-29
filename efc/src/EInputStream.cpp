/*
 * EInputStream.hh
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EInputStream.hh"

namespace efc {

// SKIP_BUFFER_SIZE is used to determine the size of skipBuffer
#define SKIP_BUFFER_SIZE 2048

int EInputStream::read()
{
	unsigned char c;
	int r = read((byte*)&c, 1);
	return (r == -1) ? -1 : (int)c;
}

long EInputStream::skip(long n)
{
	long remaining = n;
	int nr;
	byte localSkipBuffer[SKIP_BUFFER_SIZE];;
		
	if (n <= 0) {
	    return 0;
	}

	while (remaining > 0) {
	    nr = read(localSkipBuffer, 
		      (int) ES_MIN(SKIP_BUFFER_SIZE, remaining));
	    if (nr < 0) {
			break;
	    }
	    remaining -= nr;
	}
	
	return n - remaining;
}

long EInputStream::available()
{
	return 0;
}

void EInputStream::close()
{
}

} /* namespace efc */
