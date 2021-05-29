/*
 * ESocketOptions.cpp
 *
 *  Created on: 2013-3-14
 *      Author: cxxjava@163.com
 */

#include "ESocketOptions.hh"

namespace efc {

const int ESocketOptions::_SO_REUSEADDR = 0x0004;
const int ESocketOptions::_SO_KEEPALIVE = 0x0008;
const int ESocketOptions::_SO_BROADCAST = 0x0020;
const int ESocketOptions::_SO_LINGER = 0x0080;
const int ESocketOptions::_SO_TIMEOUT = 0x1006;
const int ESocketOptions::_SO_SNDBUF = 0x1001;
const int ESocketOptions::_SO_RCVBUF = 0x1002;
const int ESocketOptions::_SO_OOBINLINE = 0x1003;

const int ESocketOptions::_TCP_NODELAY = 0x0001;

const int ESocketOptions::_IP_MULTICAST_IF = 0x10;
const int ESocketOptions::_IP_MULTICAST_IF2 = 0x1f;
const int ESocketOptions::_IP_MULTICAST_LOOP = 0x12;
const int ESocketOptions::_IP_TOS = 0x3;

const int ESocketOptions::_IP_ADD_MEMBERSHIP = 0x0c;
const int ESocketOptions::_IP_DROP_MEMBERSHIP = 0x0d;
const int ESocketOptions::_IP_MULTICAST_TTL = 0x0a;

} /* namespace efc */
