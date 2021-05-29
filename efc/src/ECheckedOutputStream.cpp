/*
 * ECheckedOutputStream.cpp
 *
 *  Created on: 2017-12-8
 *      Author: cxxjava@163.com
 */

#include "ECheckedOutputStream.hh"

namespace efc {

ECheckedOutputStream::~ECheckedOutputStream() {
	//
}

ECheckedOutputStream::ECheckedOutputStream(EOutputStream* out, EChecksum* cksum, boolean owned) :
		EFilterOutputStream(out, owned) {
	this->cksum = cksum;
}

void ECheckedOutputStream::write(int b) {
	_out->write(b);
	cksum->update(b);
}

void ECheckedOutputStream::write(const void *b, int len) {
	_out->write(b, len);
	cksum->update((byte*)b, len);
}

EChecksum* ECheckedOutputStream::getChecksum() {
	return cksum;
}

} /* namespace efc */
