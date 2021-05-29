/*
 * EDatagramPacket.cpp
 *
 *  Created on: 2016-8-8
 *      Author: cxxjava@163.com
 */

#include "EDatagramPacket.hh"
#include "ENullPointerException.hh"
#include "EIllegalArgumentException.hh"

namespace efc {

EDatagramPacket::~EDatagramPacket() {
	//
}

EDatagramPacket::EDatagramPacket(EA<byte>& buf, int offset, int length) {
	setData(buf, offset, length);
	//this->address = null;
	this->port = -1;
}

EDatagramPacket::EDatagramPacket(sp<EA<byte> > buf, int offset, int length) {
	setData(buf, offset, length);
	//this->address = null;
	this->port = -1;
}

EDatagramPacket::EDatagramPacket(EA<byte>& buf, int length) {
	setData(buf, 0, length);
	//this->address = null;
	this->port = -1;
}

EDatagramPacket::EDatagramPacket(sp<EA<byte> > buf, int length) {
	setData(buf, 0, length);
	//this->address = null;
	this->port = -1;
}

EDatagramPacket::EDatagramPacket(EA<byte>& buf) {
	setData(buf, 0, buf.length());
	this->port = -1;
}

EDatagramPacket::EDatagramPacket(sp<EA<byte> > buf) {
	setData(buf, 0, buf->length());
	this->port = -1;
}

EDatagramPacket::EDatagramPacket(EA<byte>& buf, int offset, int length,
		EInetAddress* address, int port) {
	setData(buf, offset, length);
	setAddress(address);
	setPort(port);
}

EDatagramPacket::EDatagramPacket(sp<EA<byte> > buf, int offset, int length,
		EInetAddress* address, int port) {
	setData(buf, offset, length);
	setAddress(address);
	setPort(port);
}

EDatagramPacket::EDatagramPacket(EA<byte>& buf, int offset, int length,
		EInetSocketAddress* address) {
	setData(buf, offset, length);
	setSocketAddress(address);
}

EDatagramPacket::EDatagramPacket(sp<EA<byte> > buf, int offset, int length,
		EInetSocketAddress* address) {
	setData(buf, offset, length);
	setSocketAddress(address);
}

EDatagramPacket::EDatagramPacket(EA<byte>& buf, int length,
		EInetAddress* address, int port) {
	setData(buf, 0, length);
	setAddress(address);
	setPort(port);
}

EDatagramPacket::EDatagramPacket(sp<EA<byte> > buf, int length,
		EInetAddress* address, int port) {
	setData(buf, 0, length);
	setAddress(address);
	setPort(port);
}

EDatagramPacket::EDatagramPacket(EA<byte>& buf, int length,
		EInetSocketAddress* address) {
	setData(buf, 0, length);
	setSocketAddress(address);
}

EDatagramPacket::EDatagramPacket(sp<EA<byte> > buf, int length,
		EInetSocketAddress* address) {
	setData(buf, 0, length);
	setSocketAddress(address);
}

EInetAddress* EDatagramPacket::getAddress() {
	SYNCHRONIZED(this) {
		return address.get();
    }}
}

int EDatagramPacket::getPort() {
	SYNCHRONIZED(this) {
		return port;
    }}
}

EA<byte>* EDatagramPacket::getData() {
	SYNCHRONIZED(this) {
		return buf.get();
    }}
}

int EDatagramPacket::getOffset() {
	SYNCHRONIZED(this) {
		return offset;
    }}
}

int EDatagramPacket::getLength() {
	SYNCHRONIZED(this) {
		return length;
    }}
}

void EDatagramPacket::setData(EA<byte>& buf, int offset, int length) {
	SYNCHRONIZED(this) {
		/* this will check to see if buf is null */
		if (length < 0 || offset < 0 ||
			(length + offset) < 0 ||
			((length + offset) > buf.length())) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "illegal length or offset");
		}
		this->buf = new EA<byte>(buf);
		this->length = length;
		this->bufLength = length;
		this->offset = offset;
    }}
}

void EDatagramPacket::setData(sp<EA<byte> > buf, int offset, int length) {
	SYNCHRONIZED(this) {
		/* this will check to see if buf is null */
		if (buf == null || length < 0 || offset < 0 ||
			(length + offset) < 0 ||
			((length + offset) > buf->length())) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "illegal length or offset");
		}
		this->buf = buf;
		this->length = length;
		this->bufLength = length;
		this->offset = offset;
    }}
}

void EDatagramPacket::setAddress(EInetAddress* iaddr) {
	ES_ASSERT(iaddr);
	SYNCHRONIZED(this) {
		address = new EInetAddress(*iaddr);
    }}
}

void EDatagramPacket::setPort(int iport) {
	if (iport < 0 || iport > 0xFFFF) {
		EString msg("Port out of range:"); msg << iport;
		throw EIllegalArgumentException(__FILE__, __LINE__, msg.c_str());
	}
	SYNCHRONIZED(this) {
		port = iport;
    }}
}

void EDatagramPacket::setSocketAddress(EInetSocketAddress* address) {
	SYNCHRONIZED(this) {
		if (address == null) // || !(address instanceof InetSocketAddress))
			throw EIllegalArgumentException(__FILE__, __LINE__, "unsupported address type");
		//@see: InetSocketAddress addr = (InetSocketAddress) address;
		if (address->isUnresolved())
			throw EIllegalArgumentException(__FILE__, __LINE__, "unresolved address");
		setAddress(address->getAddress());
		setPort(address->getPort());
    }}
}

sp<EInetSocketAddress> EDatagramPacket::getSocketAddress() {
	SYNCHRONIZED(this) {
		return new EInetSocketAddress(getAddress(), getPort());
    }}
}

void EDatagramPacket::setData(EA<byte>& buf) {
	SYNCHRONIZED(this) {
		this->buf = new EA<byte>(buf);
		this->offset = 0;
		this->length = buf.length();
		this->bufLength = buf.length();
    }}
}

void EDatagramPacket::setData(sp<EA<byte> > buf) {
	SYNCHRONIZED(this) {
		if (buf == null) {
			throw ENullPointerException(__FILE__, __LINE__, "null packet buffer");
		}
		this->buf = buf;
		this->offset = 0;
		this->length = buf->length();
		this->bufLength = buf->length();
    }}
}

void EDatagramPacket::setLength(int length) {
	SYNCHRONIZED(this) {
		if ((length + offset) > buf->length() || length < 0 ||
			(length + offset) < 0) {
			throw EIllegalArgumentException(__FILE__, __LINE__, "illegal length");
		}
		this->length = length;
		this->bufLength = this->length;
    }}
}

} /* namespace efc */
