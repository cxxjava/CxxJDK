/*
 * EChannelInputStream.cpp
 *
 *  Created on: 2014-2-8
 *      Author: cxxjava@163.com
 */

#include "../inc/EChannelInputStream.hh"
#include "../../inc/EUnsupportedOperationException.hh"

namespace efc {
namespace nio {

EChannelInputStream::EChannelInputStream(EByteChannel* ch) :
		bb(null), bs(null) {
	this->ch = ch;
}

EChannelInputStream::~EChannelInputStream() {
	delete this->bb;
}

int EChannelInputStream::read() {
	SYNCHRONIZED(this) {
		return EInputStream::read();
    }}
}

int EChannelInputStream::read(void* b, int len) {
	SYNCHRONIZED(this) {
		if (len < 0) {
			throw EINDEXOUTOFBOUNDSEXCEPTION;
		} else if (len == 0)
			return 0;

		ES_ASSERT(b);

		if (this->bs != b) {
			delete bb;
			bb = EIOByteBuffer::wrap(b, len);
		}
		bb->limit(ES_MIN(len, bb->capacity()));
		bb->position(0);
		this->bs = b;
		return read(bb);
    }}
}

void EChannelInputStream::close() {
	ch->close();
}

int EChannelInputStream::read(EIOByteBuffer* bb) {
	throw EUNSUPPORTEDOPERATIONEXCEPTION;
}

} /* namespace nio */
} /* namespace efc */
