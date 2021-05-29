/*
 * EFileLock.cpp
 *
 *  Created on: 2014-2-15
 *      Author: cxxjava@163.com
 */

#include "../inc/EFileLock.hh"
#include "../inc/EFileChannel.hh"
#include "../../inc/EIllegalArgumentException.hh"
#include "../inc/EClosedChannelException.hh"

namespace efc {
namespace nio {

EFileLock::~EFileLock() {
	//
}

EFileLock::EFileLock(EFileChannel* channel, long position, long size,
		boolean shared) {
	if (position < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative position");
	if (size < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative size");
	if (position + size < 0)
		throw EIllegalArgumentException(__FILE__, __LINE__, "Negative position + size");
	this->channel_ = channel;
	this->position_ = position;
	this->size_ = size;
	this->shared_ = shared;
	this->valid_ = true;
}

boolean EFileLock::isValid() {
	SYNCHRONIZED(this) {
		return valid_;
    }}
}

void EFileLock::invalidate() {
	SYNCHRONIZED(this) {
		valid_ = false;
    }}
}

void EFileLock::release() {
	SYNCHRONIZED(this) {
		if (!channel_->isOpen())
			throw EClosedChannelException(__FILE__, __LINE__);
		if (valid_) {
			channel_->release(this);
			valid_ = false;
		}
    }}
}

EFileChannel* EFileLock::channel() {
	return channel_;
}

long EFileLock::position() {
	return position_;
}

long EFileLock::size() {
	return size_;
}

boolean EFileLock::isShared() {
	return shared_;
}

boolean EFileLock::overlaps(long position, long size) {
	if (position + size <= this->position_)
		return false;               // That is below this
	if (this->position_ + this->size_ <= position)
		return false;               // This is below that
	return true;
}

EString EFileLock::toString() {
	return EString::formatOf("[%lld:%lld %s %s]", position_, size_,
			(shared_ ? "shared" : "exclusive"),
			(valid_ ? "valid" : "invalid"));
}

} /* namespace nio */
} /* namespace efc */
