/*
 * EInetSocketAddress.cpp
 *
 *  Created on: 2013-3-25
 *      Author: cxxjava@163.com
 */

#include "EInetSocketAddress.hh"

namespace efc {

EInetSocketAddress::~EInetSocketAddress()
{
	//
}

EInetSocketAddress::EInetSocketAddress() : hostname(null), addr(null), port(0) {
}

EInetSocketAddress::EInetSocketAddress(int port) : hostname(null)
{
	if (port < 0 || port > 0xFFFF) {
	    throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("port out of range:%d", port).c_str());
	}
	this->port = port;
	this->addr = new EInetAddress(EInetAddress::anyLocalAddress());
}

EInetSocketAddress::EInetSocketAddress(EInetAddress *addr, int port) : hostname(null)
{
	if (port < 0 || port > 0xFFFF) {
	    throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("port out of range:%d", port).c_str());
	}
	this->port = port;
	if (addr == null) {
		this->addr = new EInetAddress(EInetAddress::anyLocalAddress());
	}
	else {
		this->addr = new EInetAddress(*addr);
	}
}

EInetSocketAddress::EInetSocketAddress(int address, int port) : hostname(null)
{
	if (port < 0 || port > 0xFFFF) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("port out of range:%d", port).c_str());
	}
	this->port = port;
	this->addr = new EInetAddress(null, address);
}

EInetSocketAddress::EInetSocketAddress(const char* host, int port) : hostname(null)
{
	if (port < 0 || port > 0xFFFF) {
	    throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("port out of range:%d", port).c_str());
	}
	if (host == null) {
	    throw EIllegalArgumentException(__FILE__, __LINE__, "hostname can't be null");
	}

	try {
		this->addr = new EInetAddress(EInetAddress::getByName(host));
	} catch(EUnknownHostException& e) {
		this->hostname = new EString(host);
		this->addr = null;
	}
	this->port = port;
}

EInetSocketAddress EInetSocketAddress::createUnresolved(const char* host, int port)
{
	if (port < 0 || port > 0xFFFF) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("port out of range:%d", port).c_str());
	}
	EInetSocketAddress s;
	s.port = port;
	s.hostname = new EString(host);
	s.addr = null;
	return s;
}

int EInetSocketAddress::getPort()
{
	return port;
}

void EInetSocketAddress::setPort(int port) {
	this->port = port;
}

EInetAddress* EInetSocketAddress::getAddress()
{
	return addr.get();
}

const char* EInetSocketAddress::getHostName()
{
	if (hostname != null)
		return hostname->c_str();
	if (addr != null)
		return addr->getHostName();
	return null;
}

EString EInetSocketAddress::getHostString() THROWS(ENullPointerException)
{
	if (hostname != null)
		return hostname->c_str();
	if (addr != null) {
		const char* host = addr->getHostName();
		if (host) {
			return EString(host);
		} else {
			return addr->getHostAddress();
		}
	}
	return null;
}

boolean EInetSocketAddress::isUnresolved()
{
	return addr == null;
}

EString EInetSocketAddress::toString()
{
	if (isUnresolved()) {
		return EString::formatOf("%s:%d", (hostname != null) ? hostname->c_str() : "null", port);
	} else {
		return addr->toString() + ":" + port;
	}
}

boolean EInetSocketAddress::equals(EInetSocketAddress* obj)
{
	if (!obj) return false;

	boolean sameIP = false;
	if (this->addr != null)
		sameIP = this->addr->equals(obj->addr.get());
	else if (this->hostname != null)
        sameIP = (obj->addr == null) &&
            this->hostname->equals(obj->hostname.get());
    else
        sameIP = (obj->addr == null) && (obj->hostname == null);
	return sameIP && (this->port == obj->port);
}

boolean EInetSocketAddress::equals(EObject* obj) {
	EInetSocketAddress* that = dynamic_cast<EInetSocketAddress*>(obj);
	return equals(that);
}

int EInetSocketAddress::hashCode()
{
	if (addr != null)
		return addr->hashCode() + port;
	if (hostname != null)
		return hostname->hashCode() + port;
	return port;
}

} /* namespace efc */
