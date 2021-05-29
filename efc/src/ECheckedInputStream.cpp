/*
 * ECheckedInputStream.cpp
 *
 *  Created on: 2013-7-24
 *      Author: cxxjava@163.com
 */

#include "ECheckedInputStream.hh"

namespace efc {

#define TMP_BUFSIZE 512

ECheckedInputStream::~ECheckedInputStream() {
}

ECheckedInputStream::ECheckedInputStream(EInputStream* in, EChecksum* cksum, boolean owned) :
		EFilterInputStream(in, owned) {
	_cksum = cksum;
}

int ECheckedInputStream::read()
{
	int b = _in->read();
	if (b != -1) {
		_cksum->update(b);
	}
	return b;
}

int ECheckedInputStream::read(void* b, int len) {
	len = _in->read(b, len);
	if (len != -1) {
		_cksum->update((byte*)b, len);
	}
	return len;
}

long ECheckedInputStream::skip(long n) {
	byte* buf = (byte*)eso_malloc(TMP_BUFSIZE);
	long total = 0;
	while (total < n) {
		long len = n - total;
		len = read(buf, len < TMP_BUFSIZE ? (int) len : TMP_BUFSIZE);
		if (len == -1) {
			eso_free(buf);
			return total;
		}
		total += len;
	}
	eso_free(buf);
	return total;
}

EChecksum* ECheckedInputStream::getChecksum() {
	return _cksum;
}

} /* namespace efc */
